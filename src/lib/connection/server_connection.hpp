/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SERVER_CONNECTION_HPP
#define SERVER_CONNECTION_HPP

#include "client_interface.hpp"
#include "login_handler.hpp"
#include "login_interface.hpp"

#include "cstdmf/md5.hpp"
#include "cstdmf/stdmf.hpp"

#include "math/stat_with_rates_of_change.hpp"
#include "math/vector3.hpp"

#include "network/channel.hpp"
#include "network/encryption_filter.hpp"
#include "network/event_dispatcher.hpp"
#include "network/public_key_cipher.hpp"

#include <map>
#include <set>
#include <string>

class DataDownload;
class ServerMessageHandler;
class StreamEncoder;

namespace Mercury
{
	class NetworkInterface;
}

/**
 *	This class is used to represent a connection to the server.
 *
 *	@ingroup network
 */
class ServerConnection :
		public Mercury::BundlePrimer,
		public Mercury::ChannelTimeOutHandler,
		public TimerHandler
{
public:
	ServerConnection();
	~ServerConnection();

	bool processInput();
	void registerInterfaces( Mercury::NetworkInterface & networkInterface );

	void setInactivityTimeout( float seconds );

	LogOnStatus logOn( ServerMessageHandler * pHandler,
		const char * serverName,
		const char * username,
		const char * password,
		uint16 port = 0 );

	LoginHandlerPtr logOnBegin(
		const char * serverName,
		const char * username,
		const char * password,
		uint16 port = 0 );

	LogOnStatus logOnComplete(
		LoginHandlerPtr pLRH,
		ServerMessageHandler * pHandler );

	void enableReconfigurePorts() { tryToReconfigurePorts_ = true; }
	void enableEntities();

	bool online() const;
	bool offline() const				{ return !this->online(); }
	void disconnect( bool informServer = true );

	void channel( Mercury::Channel & channel ) { pChannel_ = &channel; }
	void pInterface( Mercury::NetworkInterface * pInterface );
	bool hasInterface() const 
		{ return pInterface_ != NULL; }


	// Stuff that would normally be provided by ChannelOwner, however we can't
	// derive from it because of destruction order.
	Mercury::Channel & channel();
	Mercury::Bundle & bundle() { return this->channel().bundle(); }
	const Mercury::Address & addr() const;

	Mercury::EncryptionFilterPtr pFilter() { return pFilter_; }

	void addMove( EntityID id, SpaceID spaceID, EntityID vehicleID,
		const Vector3 & pos, float yaw, float pitch, float roll,
		bool onGround, const Vector3 & globalPos );
	//void rcvMoves( EntityID id );

	BinaryOStream & startProxyMessage( int messageId );
	BinaryOStream & startAvatarMessage( int messageId );
	BinaryOStream & startEntityMessage( int messageId, EntityID entityId );

	void requestEntityUpdate( EntityID id,
		const CacheStamps & stamps = CacheStamps() );

	const std::string & errorMsg() const	{ return errorMsg_; }

	EntityID connectedID() const			{ return id_; }
	void sessionKey( SessionKey key ) { sessionKey_ = key; }


	/**
	 *	Return the logon params encoding method.
	 *
	 *	@return The method used to encode the LogOnParams. NULL indicates that
	 *	no encoding be used.
	 */
	StreamEncoder * pLogOnParamsEncoder() const 
		{ return pLogOnParamsEncoder_; }


	/**
	 *	Set the logon params encoding method.
	 *
	 *	@param pEncoding 	The new encoding. This class takes no
	 *						responsibility for deleting pEncoding.
	 */
	void pLogOnParamsEncoder( StreamEncoder * pEncoder )
		{ pLogOnParamsEncoder_ = pEncoder; }

	// ---- Statistics ----

	float latency() const;

	double bpsIn() const;
	double bpsOut() const;

	double packetsPerSecondIn() const;
	double packetsPerSecondOut() const;

	double messagesPerSecondIn() const;
	double messagesPerSecondOut() const;

	int		bandwidthFromServer() const;
	void	bandwidthFromServer( int bandwidth );

	double movementBytesPercent() const;
	double nonMovementBytesPercent() const;
	double overheadBytesPercent() const;

	int movementBytesTotal() const;
	int nonMovementBytesTotal() const;
	int overheadBytesTotal() const;

	uint32 packetsIn() const;

	void pTime( const double * pTime );

	/**
	 *	This method is used to return the pointer to current time.
	 *	It is used for server statistics and for syncronising between
	 *	client and server time.
	 */
	const double * pTime()					{ return pTime_; }

	double		serverTime( double gameTime ) const;
	double 		lastMessageTime() const;
	GameTime	lastGameTime() const;

	double		lastSendTime() const	{ return lastSendTime_; }
	double		minSendInterval() const	{ return minSendInterval_; }

	// ---- InterfaceMinder handlers ----
	void authenticate(
		const ClientInterface::authenticateArgs & args );
	void bandwidthNotification(
		const ClientInterface::bandwidthNotificationArgs & args );
	void updateFrequencyNotification(
		const ClientInterface::updateFrequencyNotificationArgs & args );

	void setGameTime( const ClientInterface::setGameTimeArgs & args );

	void resetEntities( const ClientInterface::resetEntitiesArgs & args );
	void createBasePlayer( BinaryIStream & stream );
	void createCellPlayer( BinaryIStream & stream );

	void spaceData( BinaryIStream & stream );

	void enterAoI( const ClientInterface::enterAoIArgs & args );
	void enterAoIOnVehicle(
		const ClientInterface::enterAoIOnVehicleArgs & args );
	void leaveAoI( BinaryIStream & stream );

	void createEntity( BinaryIStream & stream );
	void updateEntity( BinaryIStream & stream );

	// This unattractive bit of macros is used to declare all of the handlers
	// for (fixed length) messages sent from the cell to the client. It includes
	// methods such as all of the avatarUpdate handlers.
#define MF_BEGIN_COMMON_RELIABLE_MSG( MESSAGE )	\
	void MESSAGE( const ClientInterface::MESSAGE##Args & args );

#define MF_BEGIN_COMMON_PASSENGER_MSG MF_BEGIN_COMMON_RELIABLE_MSG
#define MF_BEGIN_COMMON_UNRELIABLE_MSG MF_BEGIN_COMMON_RELIABLE_MSG

#define MF_COMMON_ARGS( ARGS )
#define MF_END_COMMON_MSG()
#define MF_COMMON_ISTREAM( NAME, XSTREAM )
#define MF_COMMON_OSTREAM( NAME, XSTREAM )
#include "common_client_interface.hpp"
#undef MF_BEGIN_COMMON_RELIABLE_MSG
#undef MF_BEGIN_COMMON_PASSENGER_MSG
#undef MF_BEGIN_COMMON_UNRELIABLE_MSG
#undef MF_COMMON_ARGS
#undef MF_END_COMMON_MSG
#undef MF_COMMON_ISTREAM
#undef MF_COMMON_OSTREAM

	void detailedPosition( const ClientInterface::detailedPositionArgs & args );
	void forcedPosition( const ClientInterface::forcedPositionArgs & args );
	void controlEntity( const ClientInterface::controlEntityArgs & args );

	void voiceData( const Mercury::Address & srcAddr,
					BinaryIStream & stream );

	void restoreClient( BinaryIStream & stream );
	void switchBaseApp( BinaryIStream & stream );

	void resourceHeader( BinaryIStream & stream );
	void resourceFragment( BinaryIStream & stream );

	void loggedOff( const ClientInterface::loggedOffArgs & args );

	void handleEntityMessage( int messageID, BinaryIStream & data );

	void setMessageHandler( ServerMessageHandler * pHandler )
								{ if (pHandler_ != NULL) pHandler_ = pHandler; }

	/// This method returns the network interface for this connection.
	Mercury::NetworkInterface & networkInterface() { return *pInterface_; }
	const Mercury::NetworkInterface & networkInterface() const
													{ return *pInterface_; }

	Mercury::EventDispatcher & dispatcher() { return dispatcher_; };

	void initDebugInfo();

	void digest( const MD5::Digest & digest )	{ digest_ = digest; }
	const MD5::Digest digest() const			{ return digest_; }

	double sendTimeReportThreshold() const 
	{ return sendTimeReportThreshold_; } 

	void sendTimeReportThreshold( double threshold ) 
	{ sendTimeReportThreshold_ = threshold; } 

	void send();

	void addCondemnedInterface( Mercury::NetworkInterface * pInterface );

	/**
	 *	The frequency of updates from the server.
	 */
	static const float & updateFrequency()		{ return s_updateFrequency_; }

private:
	typedef StatWithRatesOfChange< uint32 > Stat;

	// Not defined.
	ServerConnection( const ServerConnection & );
	ServerConnection& operator=( const ServerConnection & );

	double appTime() const;
	void updateStats();

	void onTimeOut( Mercury::Channel * pChannel );

	void setVehicle( EntityID passengerID, EntityID vehicleID );
	EntityID getVehicleID( EntityID passengerID ) const
	{
		PassengerToVehicleMap::const_iterator iter =
			passengerToVehicle_.find( passengerID );

		return iter == passengerToVehicle_.end() ? 0 : iter->second;
	}

	void initialiseConnectionState();

	virtual void primeBundle( Mercury::Bundle & bundle );
	virtual int numUnreliableMessages() const;

	virtual void handleTimeout( TimerHandle handle, void * arg );

	double getRatePercent( const Stat & stat ) const;

	/**
	 *	This method returns whether the entity with the input id is controlled
	 *	locally by this client as opposed to controlled by the server.
	 */
	bool isControlledLocally( EntityID id ) const
	{
		return controlledEntities_.find( id ) != controlledEntities_.end();
	}

	void detailedPositionReceived( EntityID id, SpaceID spaceID,
		EntityID vehicleID, const Vector3 & position );


	// ---- Data members ----
	SessionKey		sessionKey_;
	std::string	username_;

	// ---- Statistics ----

	Stat numMovementBytes_;
	Stat numNonMovementBytes_;
	Stat numOverheadBytes_;

	ServerMessageHandler * pHandler_;

	EntityID	id_;
	SpaceID		spaceID_;
	int			bandwidthFromServer_;

	const double * pTime_;
	uint64 	lastReceiveTime_;
	double	lastSendTime_;
	double	minSendInterval_;
	double	sendTimeReportThreshold_;


	Mercury::EventDispatcher dispatcher_;
	Mercury::NetworkInterface*	pInterface_;
	Mercury::Channel*			pChannel_;

	bool		tryToReconfigurePorts_;
	bool		entitiesEnabled_;

	float				inactivityTimeout_;
	MD5::Digest			digest_;

	/// This is a simple class to handle what time the client thinks is on the
	/// server.
	class ServerTimeHandler
	{
	public:
		ServerTimeHandler();

		void tickSync( uint8 newSeqNum, double currentTime );
		void gameTime( GameTime gameTime, double currentTime );

		double		serverTime( double gameTime ) const;
		double 		lastMessageTime() const;
		GameTime 	lastGameTime() const;


	private:
		static const double UNINITIALISED_TIME;

		uint8 tickByte_;
		double timeAtSequenceStart_;
		GameTime gameTimeAtSequenceStart_;
	} serverTimeHandler_;

	std::string errorMsg_;

	uint8	sendingSequenceNumber_;

	EntityID	idAlias_[ 256 ];

	typedef std::map< EntityID, EntityID > PassengerToVehicleMap;
	PassengerToVehicleMap passengerToVehicle_;

	Vector3		sentPositions_[ 256 ];
	Vector3		referencePosition_;

	typedef std::set< EntityID >		ControlledEntities;
	ControlledEntities controlledEntities_;

	typedef std::map< uint16, DataDownload* > DataDownloadMap;
	DataDownloadMap dataDownloads_;

	Mercury::EncryptionFilterPtr pFilter_;

	StreamEncoder * pLogOnParamsEncoder_;

	TimerHandle timerHandle_;

	typedef std::vector< Mercury::NetworkInterface * > CondemnedInterfaces;
	CondemnedInterfaces condemnedInterfaces_;

	const int FIRST_AVATAR_UPDATE_MESSAGE;
	const int LAST_AVATAR_UPDATE_MESSAGE;

	static float s_updateFrequency_;	// frequency of updates from the server.
};

#ifdef CODE_INLINE
#include "server_connection.ipp"
#endif



#endif // SERVER_CONNECTION_HPP
