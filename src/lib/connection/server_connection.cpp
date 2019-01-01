/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "server_connection.hpp"

#include "baseapp_ext_interface.hpp"
#include "client_interface.hpp"
#include "data_download.hpp"
#include "download_segment.hpp"
#include "login_interface.hpp"
#include "message_handlers.hpp"
#include "server_message_handler.hpp"


#include "cstdmf/concurrency.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/memory_stream.hpp"

#include "math/vector3.hpp"

#include "network/portmap.hpp"
#include "network/encryption_filter.hpp"
#include "network/compression_stream.hpp"
#include "network/interface_table.hpp"
#include "network/network_interface.hpp"
#include "network/nub_exception.hpp"
#include "network/once_off_packet.hpp"
#include "network/packet_receiver_stats.hpp"
#include "network/public_key_cipher.hpp"

DECLARE_DEBUG_COMPONENT2( "Connect", 0 )

#ifndef CODE_INLINE
#include "server_connection.ipp"
#endif

namespace // anonymous
{
/// The number of seconds of inactivity before a connection is closed.
const float DEFAULT_INACTIVITY_TIMEOUT = 60.f;			// 60 seconds

// How often the network statistics should be updated.
const float UPDATE_STATS_PERIOD = 1.f;	// 1 second

// Output a warning message if we receive packets further apart than this (in milliseconds)
const int PACKET_DELTA_WARNING_THRESHOLD = 3000;

EntityMessageHandler g_entityMessageHandler;

} // namespace (anonymous)

#ifdef WIN32
#pragma warning (disable:4355)	// this used in base member initialiser list
#endif


/**
 *	This static members stores the number of updates per second to expect from
 *	the server.
 */
float ServerConnection::s_updateFrequency_ = 10.f;


/**
 *	Constructor.
 */
ServerConnection::ServerConnection() :
	sessionKey_( 0 ),
	username_(),
	numMovementBytes_(),
	numNonMovementBytes_(),
	numOverheadBytes_(),
	pHandler_( NULL ),
	id_( EntityID( -1 ) ),
	spaceID_( 0 ),
	bandwidthFromServer_( 0 ),
	pTime_( NULL ),
	lastReceiveTime_( 0 ),
	lastSendTime_( 0.0 ),
	minSendInterval_( 1.01/20.0 ),
	sendTimeReportThreshold_( 10.0 ),
	dispatcher_(),
	pInterface_( NULL ),
	pChannel_( NULL ),
	tryToReconfigurePorts_( false ),
	entitiesEnabled_( false ),
	inactivityTimeout_( DEFAULT_INACTIVITY_TIMEOUT ),
	digest_(),
	serverTimeHandler_(),
	errorMsg_(),
	sendingSequenceNumber_( 0 ),
	// (EntityID idAlias_[256]),
	passengerToVehicle_(),
	// (Vector3 sentPositions_[256]),
	referencePosition_(),
	controlledEntities_(),
	dataDownloads_(),
#ifdef USE_OPENSSL
	pFilter_( new Mercury::EncryptionFilter() ),
#else
	pFilter_( NULL ),
#endif
	pLogOnParamsEncoder_( NULL ),
	timerHandle_(),
	condemnedInterfaces_(),
	// see also initialiseConnectionState
	FIRST_AVATAR_UPDATE_MESSAGE(
		ClientInterface::avatarUpdateNoAliasFullPosYawPitchRoll.id() ),
	LAST_AVATAR_UPDATE_MESSAGE(
		ClientInterface::avatarUpdateAliasNoPosNoDir.id() )
{
	this->pInterface( new Mercury::NetworkInterface( &dispatcher_,
					  Mercury::NETWORK_INTERFACE_EXTERNAL ) ),
	// Timer for updating statistics
	timerHandle_ = this->dispatcher().addTimer(
					static_cast< int >( UPDATE_STATS_PERIOD * 1000000 ), this );

	// Ten samples at one second each
	numMovementBytes_.monitorRateOfChange( 10 );
	numNonMovementBytes_.monitorRateOfChange( 10 );
	numOverheadBytes_.monitorRateOfChange( 10 );

	tryToReconfigurePorts_ = false;
	this->initialiseConnectionState();

	memset( &digest_, 0, sizeof( digest_ ) );
}


/**
 *	Destructor
 */
ServerConnection::~ServerConnection()
{
	timerHandle_.cancel();

	// disconnect if we didn't already do so
	this->disconnect();

	// Destroy our channel.  This must be done immediately (i.e. we can't
	// just condemn it) because the ~Channel() must execute before
	// ~NetworkInterface().
	if (pChannel_)
	{
		// delete pChannel_;
		pChannel_->destroy();
		pChannel_ = NULL;
	}

	delete pInterface_;
}


/**
 *	This private method initialises or reinitialises our state related to a
 *	connection. It should be called before a new connection is made.
 */
void ServerConnection::initialiseConnectionState()
{
	id_ = EntityID( -1 );
	spaceID_ = SpaceID( -1 );
	bandwidthFromServer_ = 0;

	lastReceiveTime_ = 0;
	lastSendTime_ = 0.0;

	entitiesEnabled_ = false;

	serverTimeHandler_ = ServerTimeHandler();

	sendingSequenceNumber_ = 0;

	memset( idAlias_, 0, sizeof( idAlias_ ) );


	controlledEntities_.clear();
}

/**
 *	This private helper method registers the mercury interfaces with the
 *	provided network interface, so that it will serve them.  This should be
 *	done for every network interface that might receive messages from the
 *	server, which includes the temporary network interfaces used in the BaseApp
 *	login process.
 */
void ServerConnection::registerInterfaces(
		Mercury::NetworkInterface & networkInterface )
{
	ClientInterface::registerWithInterface( networkInterface );

	for (Mercury::MessageID id = 128; id != 255; id++)
	{
		networkInterface.interfaceTable().serve( ClientInterface::entityMessage,
			id, &g_entityMessageHandler );
	}

	// Message handlers have access to the network interface that the message
	// arrived on. Set the ServerConnection here so that the message handler
	// knows which one to deliver the message to. (This is used by bots).
	networkInterface.pExtensionData( this );
}


THREADLOCAL( bool ) g_isMainThread( false );


/**
 *	This method logs in to the named server with the input username and
 *	password.
 *
 *	@param pHandler		The handler to receive messages from this connection.
 *	@param serverName	The name of the server to connect to.
 *	@param username		The username to log on with.
 *	@param password		The password to log on with.
 *	@param publicKeyPath	The path to locate the public key to use for
 *						encrypting login communication.
 *	@param port			The port that the server should talk to us on.
 */
LogOnStatus ServerConnection::logOn( ServerMessageHandler * pHandler,
	const char * serverName,
	const char * username,
	const char * password,
	uint16 port )
{
	LoginHandlerPtr pLoginHandler = this->logOnBegin(
		serverName, username, password, port );

	// Note: we could well get other messages here and not just the
	// reply (if the proxy sends before we get the reply), but we
	// don't add ourselves to the master/slave system before then
	// so we won't be found ... and in the single servconn case the
	// channel (data) won't be created until then so the the intermediate
	// messages will be resent.

	while (!pLoginHandler->done())
	{
		dispatcher_.processUntilBreak();
	}

	LogOnStatus status = this->logOnComplete( pLoginHandler, pHandler );

	if (status == LogOnStatus::LOGGED_ON)
	{
		this->enableEntities();
	}

	return status;
}


/**
 *	This method begins an asynchronous login
 */
LoginHandlerPtr ServerConnection::logOnBegin(
	const char * serverName,
	const char * username,
	const char * password,
	uint16 port )
{
	std::string key = pFilter_ ? pFilter_->key() : "";
	LogOnParamsPtr pParams = new LogOnParams( username, password, key );

	pParams->digest( this->digest() );

	g_isMainThread = true;

	// make sure we are not already logged on
	if (this->online())
	{
		return new LoginHandler( this, LogOnStatus::ALREADY_ONLINE_LOCALLY );
	}

	this->initialiseConnectionState();

	TRACE_MSG( "ServerConnection::logOnBegin: "
		"server:%s username:%s\n", serverName, pParams->username().c_str() );

	username_ = pParams->username();

	// Register the interfaces if they have not already been registered.
	this->registerInterfaces( this->networkInterface() );

	// Find out where we want to log in to
	uint16 loginPort = port ? port : PORT_LOGIN;

	const char * serverPort = strchr( serverName, ':' );
	std::string serverNameStr;
	if (serverPort != NULL)
	{
		loginPort = atoi( serverPort+1 );
		serverNameStr.assign( serverName, serverPort - serverName );
		serverName = serverNameStr.c_str();
	}

	Mercury::Address loginAddr( 0, htons( loginPort ) );
	if (Endpoint::convertAddress( serverName, (u_int32_t&)loginAddr.ip ) != 0 ||
		loginAddr.ip == 0)
	{
		ERROR_MSG( "ServerConnection::logOnBegin: "
				"Could not find server '%s'\n", serverName );
		return new LoginHandler( this, LogOnStatus::DNS_LOOKUP_FAILED );
	}

	// Create a LoginHandler and start the handshake
	LoginHandlerPtr pLoginHandler = new LoginHandler( this );
	pLoginHandler->start( loginAddr, pParams );
	return pLoginHandler;
}

/**
 *	This method completes an asynchronous login.
 *
 *	Note: Don't call this from within processing the bundle that contained
 *	the reply if you have multiple ServConns, as it could stuff up processing
 *	for another of the ServConns.
 */
LogOnStatus ServerConnection::logOnComplete(
	LoginHandlerPtr pLoginHandler,
	ServerMessageHandler * pHandler )
{
	LogOnStatus status = LogOnStatus::UNKNOWN_ERROR;

	MF_ASSERT_DEV( pLoginHandler != NULL );

	if (this->online())
	{
		status = LogOnStatus::ALREADY_ONLINE_LOCALLY;
	}

	status = pLoginHandler->status();

	if ((status == LogOnStatus::LOGGED_ON) &&
			!this->online())
	{
		WARNING_MSG( "ServerConnection::logOnComplete: "
				"Already logged off\n" );

		status = LogOnStatus::CANCELLED;
		errorMsg_ = "Already logged off";
	}

	if (status == LogOnStatus::LOGGED_ON)
	{
		DEBUG_MSG( "ServerConnection::logOn: status==LOGGED_ON\n" );

		const LoginReplyRecord & result = pLoginHandler->replyRecord();

		DEBUG_MSG( "ServerConnection::logOn: from: %s\n",
				   this->networkInterface().address().c_str() );
		DEBUG_MSG( "ServerConnection::logOn: to:   %s\n",
				result.serverAddr.c_str() );

		// We establish our channel to the BaseApp in
		// BaseAppLoginHandler::handleMessage - this is just a sanity check.
		if (result.serverAddr != this->addr())
		{
			char winningAddr[ 256 ];
			strncpy( winningAddr, this->addr().c_str(), sizeof( winningAddr ) );

			WARNING_MSG( "ServerConnection::logOnComplete: "
				"BaseApp address on login reply (%s) differs from winning "
				"BaseApp reply (%s)\n",
				result.serverAddr.c_str(), winningAddr );
		}
	}
	else if (status == LogOnStatus::CONNECTION_FAILED)
	{
		ERROR_MSG( "ServerConnection::logOnComplete: Logon failed (%s)\n",
				pLoginHandler->errorMsg().c_str() );
		status = LogOnStatus::CONNECTION_FAILED;
		errorMsg_ = pLoginHandler->errorMsg();
	}
	else if (status == LogOnStatus::DNS_LOOKUP_FAILED)
	{
 		errorMsg_ = "DNS lookup failed";
		ERROR_MSG( "ServerConnection::logOnComplete: Logon failed: %s\n",
				errorMsg_.c_str() );
	}
	else
	{
		errorMsg_ = pLoginHandler->errorMsg();
		INFO_MSG( "ServerConnection::logOnComplete: Logon failed: %s\n",
				errorMsg_.c_str() );
	}

	// Release the reply handler
	pLoginHandler = NULL;

	// Get out if we didn't log on
	if (status != LogOnStatus::LOGGED_ON)
	{
		return status;
	}

	// Yay we logged on!

	id_ = 0;

	// Send an initial packet to the proxy to open up a hole in any
	// firewalls on our side of the connection.
	this->send();

	// DEBUG_MSG( "ServerConnection::logOn: sent initial message to server\n" );

	// Install the user's server message handler until we disconnect
	// (serendipitous leftover argument from when it used to be called
	// here to create the player entity - glad I didn't clean that up :)
	pHandler_ = pHandler;

	// Disconnect if we do not receive anything for this number of seconds.
	this->channel().startInactivityDetection( inactivityTimeout_ );

	return status;
}


/**
 *	This method enables the receipt of entity and related messages from the
 *	server, i.e. the bulk of game traffic. The server does not start sending
 *	to us until we are ready for it. This should be called shortly after login.
 */
void ServerConnection::enableEntities()
{
	// Ok cell, we are ready for entity updates now.
	BaseAppExtInterface::enableEntitiesArgs & args =
		BaseAppExtInterface::enableEntitiesArgs::start(
			this->bundle(), Mercury::RELIABLE_DRIVER );

	args.dummy = 0;

	DEBUG_MSG( "ServerConnection::enableEntities: Enabling entities\n" );

	this->send();

	entitiesEnabled_ = true;
}


/**
 * 	This method returns whether or not we are online with the server.
 *	If a login is in progress, it will still return false.
 */
bool ServerConnection::online() const
{
	return pChannel_ != NULL;
}


/**
 *	This method removes the channel. This should be called when an exception
 *	occurs during processing input, or when sending data fails. The layer above
 *	can detect this by checking the online method once per frame. This is safer
 *	than using a callback, since a disconnect may happen at inconvenient times,
 *	e.g. when sending.
 */
void ServerConnection::disconnect( bool informServer )
{
	if (!this->online())
	{
		return;
	}

	if (informServer)
	{
		BaseAppExtInterface::disconnectClientArgs::start( this->bundle(),
			Mercury::RELIABLE_NO ).reason = 0; // reason not yet used.

		this->channel().send();
	}

	// Destroy our channel
	// delete pChannel_;
	if (pChannel_ != NULL)
	{
		pChannel_->destroy();
		pChannel_ = NULL;
	}

	// clear in-progress proxy data downloads
	for (uint i = 0; i < dataDownloads_.size(); ++i)
		delete dataDownloads_[i];
	dataDownloads_.clear();

	// forget the handler and the session key
	pHandler_ = NULL;
	sessionKey_ = 0;

	// Rebind the network interface so that we have a new port.
	this->networkInterface().recreateListeningSocket( 0, NULL );
}


/**
 *	This method sets the NetworkInterface associated with this ServerConnection.
 */
void ServerConnection::pInterface( Mercury::NetworkInterface * pInterface )
{
	if (pInterface_ != NULL)
	{
		pInterface_->pChannelTimeOutHandler( NULL );
	}

	pInterface_ = pInterface;

	if (pInterface_ != NULL)
	{
		pInterface_->pChannelTimeOutHandler( this );
	}
}


/**
 *  This method returns the ServerConnection's channel.
 */
Mercury::Channel & ServerConnection::channel()
{
	// Don't call this before this->online() is true.
	MF_ASSERT_DEV( pChannel_ );
	return *pChannel_;
}


const Mercury::Address & ServerConnection::addr() const
{
	MF_ASSERT_DEV( pChannel_ );
	return pChannel_->addr();
}


/**
 * 	This method adds a message to the server to inform it of the
 * 	new position and direction (and the rest) of an entity under our control.
 *	The server must have explicitly given us control of that entity first.
 *
 *	@param id			ID of entity.
 *	@param spaceID		ID of the space the entity is in.
 *	@param vehicleID	ID of the innermost vehicle the entity is on.
 * 	@param pos			Local position of the entity.
 * 	@param yaw			Local direction of the entity.
 * 	@param pitch		Local direction of the entity.
 * 	@param roll			Local direction of the entity.
 * 	@param onGround		Whether or not the entity is on terrain (if present).
 * 	@param globalPos	Approximate global position of the entity
 */
void ServerConnection::addMove( EntityID id, SpaceID spaceID,
	EntityID vehicleID, const Vector3 & pos, float yaw, float pitch, float roll,
	bool onGround, const Vector3 & globalPos )
{
	if (this->offline())
		return;

	if (spaceID != spaceID_)
	{
		ERROR_MSG( "ServerConnection::addMove: "
					"Attempted to move %d from space %u to space %u\n",
				id, spaceID_, spaceID );
		return;
	}

	if (!this->isControlledLocally( id ))
	{
		ERROR_MSG( "ServerConnection::addMove: "
				"Tried to add a move for entity id %d that we do not control\n",
			id );
		// be assured that even if we did not return here the server
		// would not accept the position update regardless!
		return;
	}

	bool changedVehicle = false;
	EntityID currVehicleID = this->getVehicleID( id );

	if (vehicleID != currVehicleID)
	{
		this->setVehicle( id, vehicleID );
		changedVehicle = true;
	}

	Coord coordPos( BW_HTONF( pos.x ), BW_HTONF( pos.y ), BW_HTONF( pos.z ) );
	YawPitchRoll dir( yaw , pitch, roll );

	Mercury::Bundle & bundle = this->bundle();

	if (id == id_)
	{
		// TODO: When on a vehicle, the reference number is not used and so does not
		// need to be sent (and remembered).

		uint8 refNum = sendingSequenceNumber_;
		sentPositions_[ sendingSequenceNumber_ ] = globalPos;
		++sendingSequenceNumber_;

		if (!changedVehicle)
		{
			BaseAppExtInterface::avatarUpdateImplicitArgs & upArgs =
				BaseAppExtInterface::avatarUpdateImplicitArgs::start(
						bundle, Mercury::RELIABLE_NO );

			upArgs.pos = coordPos;
			upArgs.dir = dir;
			upArgs.refNum = refNum;
		}
		else
		{
			BaseAppExtInterface::avatarUpdateExplicitArgs & upArgs =
				BaseAppExtInterface::avatarUpdateExplicitArgs::start( bundle,
						Mercury::RELIABLE_NO );

			upArgs.spaceID = BW_HTONL( spaceID );
			upArgs.vehicleID = BW_HTONL( vehicleID );
			upArgs.onGround = onGround;

			upArgs.pos = coordPos;
			upArgs.dir = dir;
			upArgs.refNum = refNum;
		}
	}
	else
	{
		if (!changedVehicle)
		{
			BaseAppExtInterface::avatarUpdateWardImplicitArgs & upArgs =
				BaseAppExtInterface::avatarUpdateWardImplicitArgs::start(
						bundle, Mercury::RELIABLE_NO );

			upArgs.ward = BW_HTONL( id );

			upArgs.pos = coordPos;
			upArgs.dir = dir;
		}
		else
		{
			BaseAppExtInterface::avatarUpdateWardExplicitArgs & upArgs =
				BaseAppExtInterface::avatarUpdateWardExplicitArgs::start(
						bundle, Mercury::RELIABLE_NO );

			upArgs.ward = BW_HTONL( id );
			upArgs.spaceID = BW_HTONL( spaceID );
			upArgs.vehicleID = BW_HTONL( vehicleID );
			upArgs.onGround = onGround;

			upArgs.pos = coordPos;
			upArgs.dir = dir;
		}
	}

	// Currently even when we control an entity we keep getting updates
	// for it but just ignore them. This is so we can get the various
	// prefixes. We could set the vehicle info ourself but changing
	// the space ID is not so straightforward. However the main
	// advantage of this approach is not having to change the server to
	// know about the entities that we control. Unfortunately it is
	// quite inefficient - both for sending unnecessary explicit
	// updates (sends for about twice as long) and for getting tons
	// of unwanted position updates, mad worse by the high likelihood
	// of controlled entities being near to the client. Oh well,
	// it'll do for now.
}


/**
 * 	This method is called to start a new message to the proxy.
 * 	Note that proxy messages cannot be sent on a bundle after
 * 	entity messages.
 *
 * 	@param messageId	The message to send.
 *
 * 	@return	A stream to write the message on.
 */
BinaryOStream & ServerConnection::startProxyMessage( int messageId )
{
	if (this->offline())
	{
		CRITICAL_MSG( "ServerConnection::startProxyMessage: "
				"Called when not connected to server!\n" );
	}

	static Mercury::InterfaceElement anie = BaseAppExtInterface::entityMessage;
	// 0x80 to indicate it is an entity message, 0x40 to indicate that it is for
	// the base.
	anie.id( ((uchar)messageId) | 0xc0 );
	this->bundle().startMessage( anie );

	return this->bundle();
}


/**
 * 	This message sends an entity message for the player's avatar.
 *
 * 	@param messageId	The message to send.
 *
 * 	@return A stream to write the message on.
 */
BinaryOStream & ServerConnection::startAvatarMessage( int messageId )
{
	return this->startEntityMessage( messageId, 0 );
}

/**
 * 	This message sends an entity message to a given entity.
 *
 * 	@param messageId	The message to send.
 * 	@param entityId		The id of the entity to receive the message.
 *
 * 	@return A stream to write the message on.
 */
BinaryOStream & ServerConnection::startEntityMessage( int messageId,
		EntityID entityId )
{
	if (this->offline())
	{
		CRITICAL_MSG( "ServerConnection::startEntityMessage: "
				"Called when not connected to server!\n" );
	}

	static Mercury::InterfaceElement anie = BaseAppExtInterface::entityMessage;
	anie.id( ((uchar)messageId) | 0x80 );
	this->bundle().startMessage( anie );
	this->bundle() << entityId;

	return this->bundle();
}


/**
 *	This method implements the TimeOutHandler method. It is caused when the
 *	channel to the server times out.
 */
void ServerConnection::onTimeOut( Mercury::Channel * pChannel )
{
	if (this->online())
	{
		ERROR_MSG( "ServerConnection::onTimeOut(%d): "
				"Disconnecting due to channel timing out.\n", 
			id_ );

		this->disconnect();
	}
}


/**
 *	This method processes all pending network messages. They are passed to the
 *	input handler that was specified in logOnComplete.
 *
 *	@return	Returns true if any packets were processed.
 */
bool ServerConnection::processInput()
{
	// process any pending packets
	// (they may not be for us in a multi servconn environment)
	bool gotAnyPackets = false;
	bool gotAPacket = false;

	do
	{
		// Check packetsIn separately as it changes in the network dispatcher.
		uint numPackets = this->packetsIn();

		gotAPacket = (dispatcher_.processOnce( false ) != 0);
		gotAPacket |= (this->packetsIn() != numPackets);

		gotAnyPackets |= gotAPacket;
	}
	while (gotAPacket);

	// Don't bother collecting statistics if we're not online.
	if (!this->online())
	{
		return gotAnyPackets;
	}

	// see how long that processing took
	if (gotAnyPackets)
	{
		uint64 currTimeStamp = timestamp();
		
		if (lastReceiveTime_ != 0)
		{
			uint64 delta = (currTimeStamp - lastReceiveTime_)
							* uint64( 1000 ) / stampsPerSecond();
			int deltaInMS = int( delta );

			if (deltaInMS > PACKET_DELTA_WARNING_THRESHOLD)
			{
				WARNING_MSG( "ServerConnection::processInput(%d): "
						"There were %d ms between packets\n", 
					id_, deltaInMS );
			}
		}

		lastReceiveTime_ = currTimeStamp;
	}

	// Delete any condemned network interfaces
	CondemnedInterfaces::iterator iInterface = condemnedInterfaces_.begin();
	while (iInterface != condemnedInterfaces_.end())
	{
		delete (*iInterface);
		++iInterface;
	}
	condemnedInterfaces_.clear();

	return gotAnyPackets;
}


/**
 *	This method handles an entity script message from the server.
 *
 *	@param messageID		Message Id.
 *	@param data		Stream containing message data.
 *	@param length	Number of bytes in the message.
 */
void ServerConnection::handleEntityMessage( int messageID, BinaryIStream & data )
{
	// Get the entity id off the stream
	EntityID objectID;
	data >> objectID;

//	DEBUG_MSG( "ServerConnection::handleMessage: %d\n", messageID );
	if (pHandler_)
	{
		const int PROPERTY_FLAG = 0x40;

		if (messageID & PROPERTY_FLAG)
		{
			pHandler_->onEntityProperty( objectID,
				messageID & ~PROPERTY_FLAG, data );
		}
		else
		{
			pHandler_->onEntityMethod( objectID,
				messageID, data );
		}
	}
}


// -----------------------------------------------------------------------------
// Section: avatarUpdate and related message handlers
// -----------------------------------------------------------------------------

/**
 *	This method handles the relativePositionReference message. It is used to
 *	indicate the position that should be used as the base for future relative
 *	positions.
 */
void ServerConnection::relativePositionReference(
	const ClientInterface::relativePositionReferenceArgs & args )
{
	referencePosition_ =
		::calculateReferencePosition( sentPositions_[ args.sequenceNumber ] );
}


/**
 *	This method handles the relativePosition message. It is used to indicate the
 *	position that should be used as the base for future relative positions.
 */
void ServerConnection::relativePosition(
		const ClientInterface::relativePositionArgs & args )
{
	referencePosition_ = args.position;
}


/**
 *	This method indicates that the vehicle an entity is on has changed.
 */
void ServerConnection::setVehicle(
	const ClientInterface::setVehicleArgs & args )
{
	this->setVehicle( args.passengerID, args.vehicleID );
}


/**
 *	This method changes the vehicle an entity is on has changed.
 */
void ServerConnection::setVehicle( EntityID passengerID, EntityID vehicleID )
{
	if (vehicleID)
	{
		passengerToVehicle_[ passengerID ] = vehicleID;
	}
	else
	{
		passengerToVehicle_.erase( passengerID );
	}
}


#define AVATAR_UPDATE_GET_POS_ORIGIN										\
		const Vector3 & originPos =											\
			(vehicleID == 0) ? referencePosition_ : Vector3::zero();		\

#define AVATAR_UPDATE_GET_POS_FullPos										\
		AVATAR_UPDATE_GET_POS_ORIGIN										\
		args.position.unpackXYZ( pos.x, pos.y, pos.z );						\
		args.position.getXYZError( posError.x, posError.y, posError.z );	\
		pos += originPos;													\

#define AVATAR_UPDATE_GET_POS_OnChunk										\
		AVATAR_UPDATE_GET_POS_ORIGIN										\
		pos.y = -13000.f;													\
		args.position.unpackXZ( pos.x, pos.z );								\
		args.position.getXZError( posError.x, posError.z );					\
		/* TODO: This is not correct. Need to implement this later. */		\
		pos.x += originPos.x;												\
		pos.z += originPos.z;												\

#define AVATAR_UPDATE_GET_POS_OnGround										\
		AVATAR_UPDATE_GET_POS_ORIGIN										\
		pos.y = -13000.f;													\
		args.position.unpackXZ( pos.x, pos.z );								\
		args.position.getXZError( posError.x, posError.z );					\
		pos.x += originPos.x;												\
		pos.z += originPos.z;												\

#define AVATAR_UPDATE_GET_POS_NoPos											\
		pos.set( -13000.f, -13000.f, -13000.f );							\

#define AVATAR_UPDATE_GET_DIR_YawPitchRoll									\
		float yaw = 0.f, pitch = 0.f, roll = 0.f;							\
		args.dir.get( yaw, pitch, roll );									\

#define AVATAR_UPDATE_GET_DIR_YawPitch										\
		float yaw = 0.f, pitch = 0.f, roll = 0.f;							\
		args.dir.get( yaw, pitch );											\

#define AVATAR_UPDATE_GET_DIR_Yaw											\
		float yaw = int8ToAngle( args.dir );								\
		float pitch = 0.f;													\
		float roll = 0.f;													\

#define AVATAR_UPDATE_GET_DIR_NoDir											\
		float yaw = 0.f, pitch = 0.f, roll = 0.f;							\

#define AVATAR_UPDATE_GET_ID_NoAlias	args.id;
#define AVATAR_UPDATE_GET_ID_Alias		idAlias_[ args.idAlias ];

#define IMPLEMENT_AVUPMSG( ID, POS, DIR )									\
void ServerConnection::avatarUpdate##ID##POS##DIR(							\
		const ClientInterface::avatarUpdate##ID##POS##DIR##Args & args )	\
{																			\
	if (pHandler_ != NULL)													\
	{																		\
		Vector3 pos;														\
		Vector3 posError( 0.f, 0.f, 0.f );									\
																			\
		EntityID id = AVATAR_UPDATE_GET_ID_##ID								\
		EntityID vehicleID = this->getVehicleID( id );						\
																			\
		AVATAR_UPDATE_GET_POS_##POS											\
																			\
		AVATAR_UPDATE_GET_DIR_##DIR											\
																			\
		/* Ignore updates from controlled entities */						\
		if (this->isControlledLocally( id ))								\
			return;															\
																			\
		pHandler_->onEntityMoveWithError( id, spaceID_, vehicleID,			\
			pos, posError, yaw, pitch, roll, true );						\
	}																		\
}


IMPLEMENT_AVUPMSG( NoAlias, FullPos, YawPitchRoll )
IMPLEMENT_AVUPMSG( NoAlias, FullPos, YawPitch )
IMPLEMENT_AVUPMSG( NoAlias, FullPos, Yaw )
IMPLEMENT_AVUPMSG( NoAlias, FullPos, NoDir )
IMPLEMENT_AVUPMSG( NoAlias, OnChunk, YawPitchRoll )
IMPLEMENT_AVUPMSG( NoAlias, OnChunk, YawPitch )
IMPLEMENT_AVUPMSG( NoAlias, OnChunk, Yaw )
IMPLEMENT_AVUPMSG( NoAlias, OnChunk, NoDir )
IMPLEMENT_AVUPMSG( NoAlias, OnGround, YawPitchRoll )
IMPLEMENT_AVUPMSG( NoAlias, OnGround, YawPitch )
IMPLEMENT_AVUPMSG( NoAlias, OnGround, Yaw )
IMPLEMENT_AVUPMSG( NoAlias, OnGround, NoDir )
IMPLEMENT_AVUPMSG( NoAlias, NoPos, YawPitchRoll )
IMPLEMENT_AVUPMSG( NoAlias, NoPos, YawPitch )
IMPLEMENT_AVUPMSG( NoAlias, NoPos, Yaw )
IMPLEMENT_AVUPMSG( NoAlias, NoPos, NoDir )
IMPLEMENT_AVUPMSG( Alias, FullPos, YawPitchRoll )
IMPLEMENT_AVUPMSG( Alias, FullPos, YawPitch )
IMPLEMENT_AVUPMSG( Alias, FullPos, Yaw )
IMPLEMENT_AVUPMSG( Alias, FullPos, NoDir )
IMPLEMENT_AVUPMSG( Alias, OnChunk, YawPitchRoll )
IMPLEMENT_AVUPMSG( Alias, OnChunk, YawPitch )
IMPLEMENT_AVUPMSG( Alias, OnChunk, Yaw )
IMPLEMENT_AVUPMSG( Alias, OnChunk, NoDir )
IMPLEMENT_AVUPMSG( Alias, OnGround, YawPitchRoll )
IMPLEMENT_AVUPMSG( Alias, OnGround, YawPitch )
IMPLEMENT_AVUPMSG( Alias, OnGround, Yaw )
IMPLEMENT_AVUPMSG( Alias, OnGround, NoDir )
IMPLEMENT_AVUPMSG( Alias, NoPos, YawPitchRoll )
IMPLEMENT_AVUPMSG( Alias, NoPos, YawPitch )
IMPLEMENT_AVUPMSG( Alias, NoPos, Yaw )
IMPLEMENT_AVUPMSG( Alias, NoPos, NoDir )


/**
 *	This method handles a detailed position and direction update.
 */
void ServerConnection::detailedPosition(
	const ClientInterface::detailedPositionArgs & args )
{
	EntityID entityID = args.id;
	EntityID vehicleID = this->getVehicleID( entityID );

	this->detailedPositionReceived(
		entityID, spaceID_, 0, args.position );

	if ((pHandler_ != NULL) &&
			!this->isControlledLocally( entityID ))
	{
		pHandler_->onEntityMoveWithError(
			entityID,
			spaceID_,
			vehicleID,
			args.position,
			Vector3::zero(),
			args.direction.yaw,
			args.direction.pitch,
			args.direction.roll,
			false );
	}
}

/**
 *	This method handles a forced position and direction update.
 *	This is when an update is being forced back for an (ordinarily)
 *	client controlled entity, including for the player. Usually this is
 *	due to a physics correction from the server, but it could be for any
 *	reason decided by the server (e.g. server-initiated teleport).
 */
void ServerConnection::forcedPosition(
	const ClientInterface::forcedPositionArgs & args )
{
	if (args.id == id_)
	{
		if ((spaceID_ != 0) &&
				(spaceID_ != args.spaceID) &&
				(pHandler_ != NULL))
		{
			pHandler_->spaceGone( spaceID_ );
		}

		spaceID_ = args.spaceID;

		BaseAppExtInterface::ackPhysicsCorrectionArgs & ackArgs =
					BaseAppExtInterface::ackPhysicsCorrectionArgs::start(
							this->bundle() );

		ackArgs.dummy = 0;
	}
	else
	{
		BaseAppExtInterface::ackWardPhysicsCorrectionArgs & ackArgs =
					BaseAppExtInterface::ackWardPhysicsCorrectionArgs::start(
							this->bundle() );

		ackArgs.ward = BW_HTONL( args.id );
		ackArgs.dummy = 0;
	}


	// finally tell the handler about it
	if (pHandler_ != NULL)
	{
		pHandler_->onEntityMoveWithError(
			args.id,
			args.spaceID,
			args.vehicleID,
			args.position,
			Vector3::zero(),
			args.direction.yaw,
			args.direction.pitch,
			args.direction.roll,
			false );
	}
}


/**
 *	The server is telling us whether or not we are controlling this entity
 */
void ServerConnection::controlEntity(
	const ClientInterface::controlEntityArgs & args )
{
	if (args.on)
	{
		controlledEntities_.insert( args.id );
	}
	else
	{
		controlledEntities_.erase( args.id );
	}

	// tell the message handler about it
	if (pHandler_ != NULL)
	{
		pHandler_->onEntityControl( args.id, args.on );
	}
}


/**
 *	This method is called when a detailed position for an entity has been
 *	received.
 */
void ServerConnection::detailedPositionReceived( EntityID id,
	SpaceID spaceID, EntityID vehicleID, const Vector3 & position )
{
	if ((id == id_) && (vehicleID == 0))
	{
		referencePosition_ = ::calculateReferencePosition( position );
	}
}



// -----------------------------------------------------------------------------
// Section: Statistics methods
// -----------------------------------------------------------------------------

static void (*s_bandwidthFromServerMutator)( int bandwidth ) = NULL;
void setBandwidthFromServerMutator( void (*mutatorFn)( int bandwidth ) )
{
	s_bandwidthFromServerMutator = mutatorFn;
}

/**
 *	This method gets the bandwidth that this server connection should receive
 *	from the server.
 *
 *	@return		The current downstream bandwidth in bits per second.
 */
int ServerConnection::bandwidthFromServer() const
{
	return bandwidthFromServer_;
}


/**
 *	This method sets the bandwidth that this server connection should receive
 *	from the server.
 *
 *	@param bandwidth	The bandwidth in bits per second.
 */
void ServerConnection::bandwidthFromServer( int bandwidth )
{
	if (s_bandwidthFromServerMutator == NULL)
	{
		ERROR_MSG( "ServerConnection::bandwidthFromServer: Cannot comply "
			"since no mutator set with 'setBandwidthFromServerMutator'\n" );
		return;
	}

	const int MIN_BANDWIDTH = 0;
	const int MAX_BANDWIDTH = PACKET_MAX_SIZE * NETWORK_BITS_PER_BYTE * 10 / 2;

	bandwidth = Math::clamp( MIN_BANDWIDTH, bandwidth, MAX_BANDWIDTH );

	(*s_bandwidthFromServerMutator)( bandwidth );

	// don't set it now - wait to hear back from the server
	//bandwidthFromServer_ = bandwidth;
}



/**
 *	This method returns the number of bits received per second.
 */
double ServerConnection::bpsIn() const
{
	return this->networkInterface().receivingStats().bitsPerSecond();
}


/**
 *	This method returns the number of bits sent per second.
 */
double ServerConnection::bpsOut() const
{
	return this->networkInterface().sendingStats().bitsPerSecond();
}


/**
 *	This method returns the number of packets received per second.
 */
double ServerConnection::packetsPerSecondIn() const
{
	return this->networkInterface().receivingStats().packetsPerSecond();
}


/**
 *	This method returns the number of packets sent per second.
 */
double ServerConnection::packetsPerSecondOut() const
{
	return this->networkInterface().sendingStats().packetsPerSecond();
}


/**
 *	This method returns the number of messages received per second.
 */
double ServerConnection::messagesPerSecondIn() const
{
	return this->networkInterface().receivingStats().messagesPerSecond();
}


/**
 *	This method returns the number of messages sent per second.
 */
double ServerConnection::messagesPerSecondOut() const
{
	return this->networkInterface().sendingStats().messagesPerSecond();
}


/**
 *	This method returns the percentage of movement bytes received.
 */
double ServerConnection::movementBytesPercent() const
{
	return this->getRatePercent( numMovementBytes_ );
}

/**
 *	This method returns the percentage of non-movement bytes received
 */
double ServerConnection::nonMovementBytesPercent() const
{
	return this->getRatePercent( numNonMovementBytes_ );
}


/**
 *	This method returns the percentage of overhead bytes received
 */
double ServerConnection::overheadBytesPercent() const
{
	return this->getRatePercent( numOverheadBytes_ );
}


/**
 *	This method returns the percent that the input stat makes up of
 *	total bandwidth.
 */
double ServerConnection::getRatePercent( const Stat & stat ) const
{
	double total =
		numMovementBytes_.getRateOfChange() +
		numNonMovementBytes_.getRateOfChange() +
		numOverheadBytes_.getRateOfChange();

	if (total > 0.0)
	{
		return stat.getRateOfChange()/total * 100.0;
	}
	else
	{
		return 0.0;
	}
}


/**
 *	This method returns the total number of bytes received that are associated
 *	with movement messages.
 */
int ServerConnection::movementBytesTotal() const
{
	return numMovementBytes_.total();
}


/**
 *	This method returns the total number of bytes received that are associated
 *	with non-movement messages.
 */
int ServerConnection::nonMovementBytesTotal() const
{
	return numNonMovementBytes_.total();
}


/**
 *	This method returns the total number of bytes received that are associated
 *	with packet overhead.
 */
int ServerConnection::overheadBytesTotal() const
{
	return numOverheadBytes_.total();
}


uint32 ServerConnection::packetsIn() const
{
	return pInterface_ ? pInterface_->receivingStats().numPacketsReceived() : 0;
}

void ServerConnection::handleTimeout( TimerHandle handle, void * arg )
{
	this->updateStats();
}


/**
 *	This method updates the timing statistics of the server connection.
 */
void ServerConnection::updateStats()
{
	if (pInterface_ == NULL)
	{
		return;
	}

	const Mercury::NetworkInterface & iface = this->networkInterface();
	const Mercury::PacketReceiverStats & stats = iface.receivingStats();
	const Mercury::InterfaceTable & ifTable = iface.interfaceTable();

	uint32 newMovementBytes =
		ifTable[ ClientInterface::relativePositionReference.id() ].numBytesReceived();

	for (int i = FIRST_AVATAR_UPDATE_MESSAGE;
			i <= LAST_AVATAR_UPDATE_MESSAGE; i++)
	{
		newMovementBytes += ifTable[ i ].numBytesReceived();
	}

	numMovementBytes_.setTotal( newMovementBytes );
	numOverheadBytes_.setTotal( stats.numOverheadBytesReceived() );
	numNonMovementBytes_.setTotal( stats.numBytesReceived() -
		numOverheadBytes_.total() - newMovementBytes );

	numMovementBytes_.tick( UPDATE_STATS_PERIOD );
	numNonMovementBytes_.tick( UPDATE_STATS_PERIOD );
	numOverheadBytes_.tick( UPDATE_STATS_PERIOD );
}


/**
 *	This method sends the current bundle to the server.
 */
void ServerConnection::send()
{
	// get out now if we are not connected
	if (this->offline())
		return;

	// record the time we last did a send
	if (pTime_)
	{
		double sinceLastSendTime = *pTime_ - lastSendTime_;
		if (lastSendTime_ != 0.0 && 
				sinceLastSendTime > sendTimeReportThreshold_)
		{
			WARNING_MSG( "ServerConnection::send(%d): "
					"Time since last send to server: %.0fms\n",
				id_,
				sinceLastSendTime * 1e3 );
		}
		lastSendTime_ = *pTime_;
	}

	// get the channel to send the bundle
	this->channel().send();

	const int OVERFLOW_LIMIT = 1024;

	// TODO: #### Make a better check that is dependent on both the window
	// size and time since last heard from the server.
	if (this->channel().sendWindowUsage() > OVERFLOW_LIMIT)
	{
		WARNING_MSG( "ServerConnection::send(%d): "
				"Disconnecting since channel %s has overflowed.\n",
			id_, this->channel().c_str() );

		this->disconnect();
	}
}



/**
 * 	This method primes outgoing bundles with the authenticate message once it
 * 	has been received.
 */
void ServerConnection::primeBundle( Mercury::Bundle & bundle )
{
	if (sessionKey_)
	{
		bundle.startMessage( BaseAppExtInterface::authenticate,
				Mercury::RELIABLE_PASSENGER );

		bundle << sessionKey_;
	}
}


/**
 * 	This method returns the number of unreliable messages that are streamed on
 *  by primeBundle().
 */
int ServerConnection::numUnreliableMessages() const
{
	return sessionKey_ ? 1 : 0;
}


/**
 *	This method requests the server to send update information for the entity
 *	with the input id. This must be called after receiving an onEntityEnter
 *	message to allow message and incremental property updates to flow.
 *
 *  @param id		ID of the entity whose update is requested.
 *	@param stamps	A vector containing the known cache event stamps. If none
 *					are known, stamps is empty.
 */
void ServerConnection::requestEntityUpdate( EntityID id,
	const CacheStamps & stamps )
{
	if (this->offline())
		return;

	this->bundle().startMessage( BaseAppExtInterface::requestEntityUpdate );
	this->bundle() << id;

	CacheStamps::const_iterator iter = stamps.begin();

	while (iter != stamps.end())
	{
		this->bundle() << (*iter);

		iter++;
	}
}


/**
 *	This method returns the approximate round-trip time to the server.
 */
float ServerConnection::latency() const
{
	return pChannel_ ? float( pChannel_->roundTripTimeInSeconds() ) : 0.f;
}


// -----------------------------------------------------------------------------
// Section: Server Timing code
// -----------------------------------------------------------------------------
// TODO:PM This section is really just here to give a better time to the
// filtering. This should be reviewed.

/**
 *	This method returns the server time estimate based on the input client time.
 */
double ServerConnection::serverTime( double clientTime ) const
{
	return serverTimeHandler_.serverTime( clientTime );
}


/**
 *	This method returns the server time associated with the last packet that was
 *	received from the server.
 */
double ServerConnection::lastMessageTime() const
{
	return serverTimeHandler_.lastMessageTime();
}


/**
 * 	This method returns the game time associated with the last packet that was
 * 	received from the server.
 */
GameTime ServerConnection::lastGameTime() const
{
	return serverTimeHandler_.lastGameTime();
}


const double ServerConnection::ServerTimeHandler::UNINITIALISED_TIME = -1000.0;

/**
 *	The constructor for ServerTimeHandler.
 */
ServerConnection::ServerTimeHandler::ServerTimeHandler() :
	tickByte_( 0 ),
	timeAtSequenceStart_( UNINITIALISED_TIME ),
	gameTimeAtSequenceStart_( 0 )
{
}


/**
 * 	This method is called when the server sends a new gametime.
 * 	This should be after the sequence number for the current packet
 * 	has been set.
 *
 *	@param newGameTime	The current game time in ticks.
 */
void ServerConnection::ServerTimeHandler::gameTime( GameTime newGameTime,
		double currentTime )
{
	tickByte_ = uint8( newGameTime );
	gameTimeAtSequenceStart_ = newGameTime - tickByte_;
	timeAtSequenceStart_ = currentTime -
		double(tickByte_) / ServerConnection::updateFrequency();
}


/**
 *	This method is called when a new tick sync message is received from the
 *	server. It is used to synchronise between client and server time.
 *
 *	@param newSeqNum	The sequence number just received. This increases by one
 *						for each packets and packets should be received at 10Hz.
 *
 *	@param currentTime	This is the time that the client currently thinks it is.
 *						We need to pass it in since this file does not have
 *						access to the app file.
 */
void ServerConnection::ServerTimeHandler::tickSync( uint8 newSeqNum,
		double currentTime )
{
	const float updateFrequency = ServerConnection::updateFrequency();
	const double SEQUENCE_PERIOD = 256.0/updateFrequency;
	const int SEQUENCE_PERIOD_INT = 256;

	// Have we started yet?

	if (timeAtSequenceStart_ == UNINITIALISED_TIME)
	{
		// The first one is always like this.
		// INFO_MSG( "ServerTimeHandler::sequenceNumber: "
		//	"Have not received gameTime message yet.\n" );
		return;
	}

	tickByte_ = newSeqNum;

	// Want to adjust the time so that the client does not get too out of sync.
	double timeError = currentTime - this->lastMessageTime();
	int numSeqsOut = BW_ROUND_TO_INT( timeError/SEQUENCE_PERIOD );

	if (numSeqsOut != 0)
	{
		timeAtSequenceStart_ += numSeqsOut * SEQUENCE_PERIOD;
		gameTimeAtSequenceStart_ += numSeqsOut * SEQUENCE_PERIOD_INT;
		timeError -= numSeqsOut * SEQUENCE_PERIOD;
	}

	const double MAX_TIME_ERROR = 0.05;
	const double MAX_TIME_ADJUST = 0.005;

	if (timeError > MAX_TIME_ERROR)
	{
		timeAtSequenceStart_ += MF_MIN( timeError, MAX_TIME_ADJUST );
	}
	else if (-timeError > MAX_TIME_ERROR)
	{
		timeAtSequenceStart_ += MF_MAX( timeError, -MAX_TIME_ADJUST );
	}
}


/**
 *	This method returns the time that this client thinks the server is at.
 */
double
ServerConnection::ServerTimeHandler::serverTime( double clientTime ) const
{
	return (gameTimeAtSequenceStart_ / ServerConnection::updateFrequency()) +
		(clientTime - timeAtSequenceStart_);
}


/**
 *	This method returns the server time associated with the last packet that was
 *	received from the server.
 */
double ServerConnection::ServerTimeHandler::lastMessageTime() const
{
	return timeAtSequenceStart_ +
		double(tickByte_) / ServerConnection::updateFrequency();
}


/**
 * 	This method returns the game time of the current message.
 */
GameTime ServerConnection::ServerTimeHandler::lastGameTime() const
{
	return gameTimeAtSequenceStart_ + tickByte_;
}


/**
 *	This method initialises the watcher information for this object.
 */
void ServerConnection::initDebugInfo()
{
#if ENABLE_WATCHERS
	MF_WATCH(  "Comms/Desired bps in",
		*this,
		MF_ACCESSORS( int, ServerConnection, bandwidthFromServer ) );

	MF_WATCH( "Comms/bps in",	*this, &ServerConnection::bpsIn );
	MF_WATCH( "Comms/bps out",	*this, &ServerConnection::bpsOut );

	MF_WATCH( "Comms/PacketsSec in ", *this,
		&ServerConnection::packetsPerSecondIn );
	MF_WATCH( "Comms/PacketsSec out", *this,
		&ServerConnection::packetsPerSecondOut );

	MF_WATCH( "Comms/Messages in",	*this,
		&ServerConnection::messagesPerSecondIn );
	MF_WATCH( "Comms/Messages out",	*this,
		&ServerConnection::messagesPerSecondOut );

	MF_WATCH( "Comms/Expected Freq", ServerConnection::s_updateFrequency_,
		Watcher::WT_READ_ONLY );

	MF_WATCH( "Comms/Game Time", *this, &ServerConnection::lastGameTime );

	MF_WATCH( "Comms/Movement pct", *this, &ServerConnection::movementBytesPercent);
	MF_WATCH( "Comms/Non-move pct", *this, &ServerConnection::nonMovementBytesPercent);
	MF_WATCH( "Comms/Overhead pct", *this, &ServerConnection::overheadBytesPercent);

	MF_WATCH( "Comms/Movement total", *this, &ServerConnection::movementBytesTotal);
	MF_WATCH( "Comms/Non-move total", *this, &ServerConnection::nonMovementBytesTotal);
	MF_WATCH( "Comms/Overhead total", *this, &ServerConnection::overheadBytesTotal);

	MF_WATCH( "Comms/Latency", *this, &ServerConnection::latency );

	WatcherPtr interfaceWatcher = 
		new BaseDereferenceWatcher( Mercury::NetworkInterface::pWatcher() );

	Watcher::rootWatcher().addChild( "Comms/Interface", interfaceWatcher,
									 &pInterface_ );
	Watcher::rootWatcher().addChild( "NetworkInterface", interfaceWatcher,
									 &pInterface_ );
#endif // ENABLE_WATCHERS
}


// -----------------------------------------------------------------------------
// Section: Mercury message handlers
// -----------------------------------------------------------------------------

/**
 *	This method authenticates the server to the client. Its use is optional,
 *	and determined by the server that we are connected to upon login.
 */
void ServerConnection::authenticate(
	const ClientInterface::authenticateArgs & args )
{
	if (args.key != sessionKey_)
	{
		ERROR_MSG( "ServerConnection::authenticate: "
				   "Unexpected key! (%x, wanted %x)\n",
				   args.key, sessionKey_ );
		return;
	}
}



/**
 * 	This message handles a bandwidthNotification message from the server.
 */
void ServerConnection::bandwidthNotification(
	const ClientInterface::bandwidthNotificationArgs & args )
{
	// TRACE_MSG( "ServerConnection::bandwidthNotification: %d\n", args.bps);
	bandwidthFromServer_ = args.bps;
}


/**
 *	This method handles the message from the server that informs us how
 *	frequently it is going to send to us.
 */
void ServerConnection::updateFrequencyNotification(
		const ClientInterface::updateFrequencyNotificationArgs & args )
{
	s_updateFrequency_ = (float)args.hertz;
}



/**
 *	This method handles a tick sync message from the server. It is used as
 *	a timestamp for the messages in the packet.
 */
void ServerConnection::tickSync(
	const ClientInterface::tickSyncArgs & args )
{
	serverTimeHandler_.tickSync( args.tickByte, this->appTime() );
}


/**
 *	This method handles a setGameTime message from the server.
 *	It is used to adjust the current (server) game time.
 */
void ServerConnection::setGameTime(
	const ClientInterface::setGameTimeArgs & args )
{
	serverTimeHandler_.gameTime( args.gameTime, this->appTime() );
}


/**
 *	This method handles a resetEntities call from the server.
 */
void ServerConnection::resetEntities(
	const ClientInterface::resetEntitiesArgs & args )
{
	// proxy must have received our enableEntities if it is telling
	// us to reset entities (even if we haven't yet received any player
	// creation msgs due to reordering)
	MF_ASSERT_DEV( entitiesEnabled_ );

	// clear existing stale packet
	this->send();

	controlledEntities_.clear();
	passengerToVehicle_.clear();

	// forget about the base player entity too if so indicated
	if (!args.keepPlayerOnBase)
	{
		id_ = 0;

		// delete proxy data downloads in progress
		for (uint i = 0; i < dataDownloads_.size(); ++i)
			delete dataDownloads_[i];
		dataDownloads_.clear();
		// but not resource downloads as they're non-entity and should continue
	}

	// refresh bundle prefix
	this->send();

	// re-enable entities, which serves to ack the resetEntities
	// and flush/sync the incoming channel
	entitiesEnabled_ = false;
	this->enableEntities();

	// and finally tell the client about it so it can clear out
	// all (or nigh all) its entities
	if (pHandler_)
	{
		pHandler_->onEntitiesReset( args.keepPlayerOnBase );
	}
}


/**
 *	This method handles a createPlayer call from the base.
 */
void ServerConnection::createBasePlayer( BinaryIStream & stream )
{
	// we have new player id
	EntityID playerID = 0;
	stream >> playerID;

	INFO_MSG( "ServerConnection::createBasePlayer: id %u\n", playerID );

	// this is now our player id
	id_ = playerID;

	EntityTypeID playerType = EntityTypeID(-1);
	stream >> playerType;

	if (pHandler_)
	{	// just get base data here
		pHandler_->onBasePlayerCreate( id_, playerType,
			stream );
	}
}


/**
 *	This method handles a createCellPlayer call from the cell.
 */
void ServerConnection::createCellPlayer( BinaryIStream & stream )
{
	MF_ASSERT( id_ != 0 );
	INFO_MSG( "ServerConnection::createCellPlayer: id %u\n", id_ );

	EntityID vehicleID;
	Position3D pos;
	Direction3D	dir;
	stream >> spaceID_ >> vehicleID >> pos >> dir;

	// assume that we control this entity too
	controlledEntities_.insert( id_ );

	this->setVehicle( id_, vehicleID );

	if (pHandler_)
	{	// just get cell data here
		pHandler_->onCellPlayerCreate( id_,
			spaceID_, vehicleID, pos, dir.yaw, dir.pitch, dir.roll,
			stream );
		// pHandler_->onEntityEnter( id_, spaceID, vehicleID );
	}

	this->detailedPositionReceived( id_, spaceID_, vehicleID, pos );

	// The channel to the server is now regular
	this->channel().isLocalRegular( true );
	this->channel().isRemoteRegular( true );
}



/**
 *	This method handles keyed data about a particular space from the server.
 */
void ServerConnection::spaceData( BinaryIStream & stream )
{
	SpaceID spaceID;
	SpaceEntryID spaceEntryID;
	uint16 key;
	std::string data;

	stream >> spaceID >> spaceEntryID >> key;
	int length = stream.remainingLength();
	data.assign( (char*)stream.retrieve( length ), length );

	TRACE_MSG( "ServerConnection::spaceData(%d): space %u key %hu\n",
		id_, spaceID, key );

	if (pHandler_)
	{
		pHandler_->spaceData( spaceID, spaceEntryID, key, data );
	}
}


/**
 *	This method handles the message from the server that an entity has entered
 *	our Area of Interest (AoI).
 */
void ServerConnection::enterAoI( const ClientInterface::enterAoIArgs & args )
{
	// Set this even if args.idAlias is NO_ID_ALIAS.
	idAlias_[ args.idAlias ] = args.id;

	if (pHandler_)
	{
		//TRACE_MSG( "ServerConnection::enterAoI: Entity = %d\n", args.id );
		pHandler_->onEntityEnter( args.id, spaceID_, 0 );
	}
}


/**
 *	This method handles the message from the server that an entity has entered
 *	our Area of Interest (AoI).
 */
void ServerConnection::enterAoIOnVehicle(
	const ClientInterface::enterAoIOnVehicleArgs & args )
{
	// Set this even if args.idAlias is NO_ID_ALIAS.
	idAlias_[ args.idAlias ] = args.id;
	this->setVehicle( args.id, args.vehicleID );

	if (pHandler_)
	{
		//TRACE_MSG( "ServerConnection::enterAoI: Entity = %d\n", args.id );
		pHandler_->onEntityEnter( args.id, spaceID_, args.vehicleID );
	}
}



/**
 *	This method handles the message from the server that an entity has left our
 *	Area of Interest (AoI).
 */
void ServerConnection::leaveAoI( BinaryIStream & stream )
{
	EntityID id;
	stream >> id;

	// TODO: What if the entity just leaves the AoI and then returns?
	if (controlledEntities_.erase( id ))
	{
		if (pHandler_)
		{
			pHandler_->onEntityControl( id, false );
		}
	}

	if (pHandler_)
	{
		CacheStamps stamps( stream.remainingLength() / sizeof(EventNumber) );

		CacheStamps::iterator iter = stamps.begin();

		while (iter != stamps.end())
		{
			stream >> (*iter);

			iter++;
		}

		pHandler_->onEntityLeave( id, stamps );
	}

	passengerToVehicle_.erase( id );
}


/**
 *	This method handles a createEntity call from the server.
 */
void ServerConnection::createEntity( BinaryIStream & rawStream )
{
	CompressionIStream stream( rawStream );

	EntityID id;
	stream >> id;

	MF_ASSERT_DEV( id != EntityID( -1 ) )	// old-style deprecated hack

	EntityTypeID type;
	stream >> type;

	Vector3 pos( 0.f, 0.f, 0.f );
	int8 compressedYaw = 0;
	int8 compressedPitch = 0;
	int8 compressedRoll = 0;

	stream >> pos >> compressedYaw >> compressedPitch >> compressedRoll;

	float yaw = int8ToAngle( compressedYaw );
	float pitch = int8ToAngle( compressedPitch );
	float roll = int8ToAngle( compressedRoll );

	EntityID vehicleID = this->getVehicleID( id );

	if (pHandler_)
	{
		pHandler_->onEntityCreate( id, type,
			spaceID_, vehicleID, pos, yaw, pitch, roll,
			stream );
	}

	this->detailedPositionReceived( id, spaceID_, vehicleID, pos );
}


/**
 *	This method handles an updateEntity call from the server.
 */
void ServerConnection::updateEntity( BinaryIStream & stream )
{
	if (pHandler_)
	{
		EntityID id;
		stream >> id;
		pHandler_->onEntityProperties( id, stream );
	}
}


/**
 *	This method handles voice data that comes from another client.
 */
void ServerConnection::voiceData( const Mercury::Address & srcAddr,
	BinaryIStream & stream )
{
	if (pHandler_)
	{
		pHandler_->onVoiceData( srcAddr, stream );
	}
	else
	{
		ERROR_MSG( "ServerConnection::voiceData: "
			"Got voice data before a handler has been set.\n" );
	}
}


/**
 *	This method handles a message from the server telling us that we need to
 *	restore a state back to a previous point.
 */
void ServerConnection::restoreClient( BinaryIStream & stream )
{
	EntityID	id;
	SpaceID		spaceID;
	EntityID	vehicleID;
	Position3D	pos;
	Direction3D	dir;

	stream >> id >> spaceID >> vehicleID >> pos >> dir;

	if (pHandler_)
	{
		this->setVehicle( id, vehicleID );
		pHandler_->onRestoreClient( id, spaceID, vehicleID, pos, dir, stream );
	}
	else
	{
		ERROR_MSG( "ServerConnection::restoreClient(%d): "
				"No handler. Maybe already logged off.\n",
			id_ );
	}


	if (this->offline()) return;

	BaseAppExtInterface::restoreClientAckArgs args;
	// TODO: Put on a proper ack id.
	args.id = 0;
	this->bundle() << args;
	this->send();
}


/**
 *	This method is called when something goes wrong with the BaseApp and we need
 *	to recover.
 */
void ServerConnection::switchBaseApp( BinaryIStream & stream )
{
	// Save away the message handler as it is cleared when disconnecting.
	ServerMessageHandler * pHandler = pHandler_;
	Mercury::Address baseAddr;
	bool shouldReset;
	stream >> baseAddr;
	stream >> shouldReset;

	INFO_MSG( "ServerConnection::switchBaseApp(%d): "
			"Switching from %s to %s\n", 
		id_, pChannel_->addr().c_str(), baseAddr.c_str() );

	SessionKey sessionKey = sessionKey_;
	this->disconnect( /* informServer: */ false );

	LoginHandlerPtr pLoginHandler = new LoginHandler( this );
	pLoginHandler->startWithBaseAddr( baseAddr, sessionKey );

	while (!pLoginHandler->done())
	{
		dispatcher_.processUntilBreak();
	}

	pHandler_ = pHandler;

	if (shouldReset)
	{
		ClientInterface::resetEntitiesArgs args;
		args.keepPlayerOnBase = false;
		this->resetEntities( args );
	}
}


/**
 *  The header for a resource download from the server.
 */
void ServerConnection::resourceHeader( BinaryIStream & stream )
{
	// First destream the ID and make sure it isn't already in use
	uint16 id;
	stream >> id;

	DataDownload *pDD;
	DataDownloadMap::iterator it = dataDownloads_.find( id );

	// Usually there shouldn't be an existing download for this id, so make a
	// new one and map it in
	if (it == dataDownloads_.end())
	{
		pDD = new DataDownload( id );
		dataDownloads_[ id ] = pDD;
	}
	else
	{
		// If a download with this ID already exists and has a description,
		// we've got a problem
		if (it->second->pDesc() != NULL)
		{
			ERROR_MSG( "ServerConnection::resourceHeader: "
				"Collision between new and existing download IDs (%hu), "
				"download is likely to be corrupted\n", id );
			return;
		}

		// Otherwise, it just means data for this download arrived before the
		// header, which is weird, but not necessarily a problem
		else
		{
			WARNING_MSG( "ServerConnection::resourceHeader: "
				"Data for download #%hu arrived before the header\n",
				id );

			pDD = it->second;
		}
	}

	// Destream the description
	pDD->setDesc( stream );
}


/**
 *	The server is giving us a fragment of a resource that we have requested.
 */
void ServerConnection::resourceFragment( BinaryIStream & stream )
{
	uint32 argsLength = sizeof( ClientInterface::ResourceFragmentArgs );

	ClientInterface::ResourceFragmentArgs & args =
		*(ClientInterface::ResourceFragmentArgs*)stream.retrieve( argsLength );

	// Get existing DataDownload record if there is one
	DataDownload *pData;
	DataDownloadMap::iterator it = dataDownloads_.find( args.rid );

	if (it != dataDownloads_.end())
	{
		pData = it->second;
	}
	else
	{
		// Getting to here means header hasn't arrived by the time data arrives,
		// but there's a warning in resourceHeader() about this so we'll just
		// handle it silently here to avoid duplicate warnings.
		pData = new DataDownload( args.rid );
		dataDownloads_[ args.rid ] = pData;
	}

	int length = stream.remainingLength();

	DownloadSegment *pSegment = new DownloadSegment(
		(char*)stream.retrieve( length ), length, args.seq );

	pData->insert( pSegment, args.flags == 1 );

	// If this DataDownload is now complete, invoke script callback and destroy
	if (pData->complete())
	{
		MemoryOStream stream;
		pData->write( stream );

		if (pHandler_ != NULL)
			pHandler_->onStreamComplete( pData->id(), *pData->pDesc(), stream );

		dataDownloads_.erase( pData->id() );
		delete pData;
	}
}


/**
 *	This method handles a message from the server telling us that we have been
 *	disconnected.
 */
void ServerConnection::loggedOff( const ClientInterface::loggedOffArgs & args )
{
	INFO_MSG( "ServerConnection::loggedOff(%d): "
			"The server has disconnected us. reason = %d\n", 
		id_, args.reason );
	this->disconnect( /*informServer:*/ false );
}


/**
 *	Adds the given network interface to the condemned list. This list is
 *	cleared at the end of each call to processInput() when the network
 *	processing machinery is not actually in use.
 *
 *	@param pNetworkInterface		The network interface to be destroyed.
 */
void ServerConnection::addCondemnedInterface( 
		Mercury::NetworkInterface * pNetworkInterface )
{
	condemnedInterfaces_.push_back( pNetworkInterface );
}

// -----------------------------------------------------------------------------
// Section: Mercury
// -----------------------------------------------------------------------------

#define DEFINE_INTERFACE_HERE
#include "login_interface.hpp"

#define DEFINE_INTERFACE_HERE
#include "baseapp_ext_interface.hpp"

#define DEFINE_SERVER_HERE
#include "client_interface.hpp"

// server_connection.cpp
