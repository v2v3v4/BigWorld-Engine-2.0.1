/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PROXY_HPP
#define PROXY_HPP

#include "base.hpp"
#include "baseapp_int_interface.hpp"
#include "data_downloads.hpp"
#include "proxy_rate_limit_callback.hpp"
#include "rate_limit_message_filter.hpp"

#include "connection/baseapp_ext_interface.hpp"
#include "connection/client_interface.hpp"

#include "cstdmf/memory_stream.hpp"
#include "cstdmf/concurrency.hpp"

#include "network/channel.hpp"

#include "pyscript/stl_to_py.hpp"

#include "mailbox.hpp"

#ifdef INSTRUMENTATION
	//#define LOG_INSTRUMENTATION
		// don't write out stats to "proxy.log" from destructor
#endif

class BaseEntityMailBox;
class ClientEntityMailBox;
class DownloadStreamer;
class EventHistoryStats;
class MemoryOStream;
class ProxyPusher;


/*~ class BigWorld.Proxy
 *	@components{ base }
 *
 *	The Proxy is a special type of Base that has an associated Client. As such,
 *	it handles all the server updates for that Client. There is no direct script
 *	call to create a Proxy specifically.
 *
 */

/**
 *	This class is used to represent a proxy. A proxy is a special type of base.
 *	It has an associated client.
 */
class Proxy: public Base
{
	// Py_InstanceHeader( Proxy )
	Py_Header( Proxy, Base )

public:
	static const int MAX_INCOMING_PACKET_SIZE = 1024;
	static const int MAX_OUTGOING_PACKET_SIZE = 1024;

	Proxy( EntityID id, DatabaseID dbID, EntityType * pType );
	~Proxy();

 	void onClientDeath( bool shouldExpectClient = true );

	void onDestroy();

	void restoreClient();

	void writeBackupData( BinaryOStream & stream );
	void offload( const Mercury::Address & dstAddr );
	void transferClient( const Mercury::Address & dstAddr, bool shouldReset );

	Mercury::Address readBackupData( BinaryIStream & stream, bool hasChannel );
	void onRestored( bool hasChannel, const Mercury::Address & clientAddr );

	void proxyRestoreTo();
	void onEnableWitnessAck();

	/// @name Accessors
	//@{
	bool hasClient() const			
	{ 
		return (pClientChannel_ != NULL) || 
			(pBufferedClientBundle_.get() != NULL); 
	}
	bool isClientChannel( Mercury::Channel * pClientChannel ) const	
	{ return (pClientChannel_ != NULL) && (pClientChannel == pClientChannel_); }

	bool entitiesEnabled() const	{ return entitiesEnabled_; }

	const Mercury::Address & clientAddr() const;
	SessionKey sessionKey() const				{ return sessionKey_; }

	ClientEntityMailBox * pClientEntityMailBox() const
									{ return pClientEntityMailBox_; }
	//@}

	// Script related methods
	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PY_AUTO_METHOD_DECLARE( RETOK, giveClientTo, ARG( PyObjectPtr, END ) )
	bool giveClientTo( PyObjectPtr pDestProxy );
	bool giveClientLocally( Proxy * pLocalDest );

	bool attachToClient( const Mercury::Address & clientAddr,
		Mercury::ReplyID loginReplyID = Mercury::REPLY_ID_NONE );

	void detachFromClient();
	void logOffClient();

	PyObject * pyGet_client();
	PY_RO_ATTRIBUTE_SET( client )
	PY_RO_ATTRIBUTE_DECLARE( hasClient(), hasClient )
	PY_RO_ATTRIBUTE_DECLARE( clientAddr(), clientAddr )

	PY_RO_ATTRIBUTE_DECLARE( entitiesEnabled_, entitiesEnabled )

	PY_RO_ATTRIBUTE_DECLARE( wardsHolder_, wards )

	PY_WO_ATTRIBUTE_GET( bandwidthPerSecond ) // read-only for now
	int pySet_bandwidthPerSecond( PyObject * value );

	PY_RO_ATTRIBUTE_DECLARE( roundTripTime(), roundTripTime )
	PY_RO_ATTRIBUTE_DECLARE( timeSinceHeardFromClient(),
			timeSinceHeardFromClient )
	PY_RW_ATTRIBUTE_DECLARE( latencyTriggersHolder_, latencyTriggers )
	PY_RO_ATTRIBUTE_DECLARE( latencyAtLastCheck_, latencyLast )

	PY_AUTO_METHOD_DECLARE( RETOWN, streamStringToClient,
		ARG( PyObjectPtr, OPTARG( PyObjectPtr, NULL, OPTARG( int, -1, END ) ) ) )
	PyObject * streamStringToClient(
		PyObjectPtr pData, PyObjectPtr pDesc, int id );

	PY_AUTO_METHOD_DECLARE( RETOWN, streamFileToClient,
		ARG( std::string, OPTARG( PyObjectPtr, NULL, OPTARG( int, -1, END ) ) ) )
	PyObject * streamFileToClient( const std::string & path,
			PyObjectPtr pDesc, int id );

	/* Internal Interface */

	void cellEntityCreated();
	void cellEntityDestroyed( const Mercury::Address * pSrcAddr );

	// Start of messages forwarded from cell ...

	void sendToClient( const BaseAppIntInterface::sendToClientArgs & args );
	void createCellPlayer( const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header,BinaryIStream & data );
	void spaceData( const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header,BinaryIStream & data );
	void enterAoI( const BaseAppIntInterface::enterAoIArgs & args );
	void enterAoIOnVehicle( const BaseAppIntInterface::enterAoIOnVehicleArgs & args );
	void leaveAoI( const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header, BinaryIStream & data );
	void createEntity( const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header, BinaryIStream & data );
	void updateEntity( const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header, BinaryIStream & data );
	void acceptClient( const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header, BinaryIStream & data );

	// The following bit of unattractive macros is used to declare many of
	// the message handlers for messages sent from the cell that are to be
	// forwarded to the client. This includes the handlers for all of the
	// avatarUpdate messages.
#define MF_BEGIN_COMMON_RELIABLE_MSG( MESSAGE )								\
	void MESSAGE( const BaseAppIntInterface::MESSAGE##Args & args );

#define MF_BEGIN_COMMON_PASSENGER_MSG MF_BEGIN_COMMON_RELIABLE_MSG
#define MF_BEGIN_COMMON_UNRELIABLE_MSG MF_BEGIN_COMMON_RELIABLE_MSG

#define MF_COMMON_ARGS( ARGS )
#define MF_END_COMMON_MSG()
#define MF_COMMON_ISTREAM( NAME, XSTREAM )
#define MF_COMMON_OSTREAM( NAME, XSTREAM )

#include "connection/common_client_interface.hpp"

#undef MF_BEGIN_COMMON_RELIABLE_MSG
#undef MF_BEGIN_COMMON_UNRELIABLE_MSG
#undef MF_COMMON_ARGS
#undef MF_END_COMMON_MSG
#undef MF_COMMON_ISTREAM
#undef MF_COMMON_OSTREAM

	void detailedPosition(
		const BaseAppIntInterface::detailedPositionArgs & args );

	void forcedPosition( const BaseAppIntInterface::forcedPositionArgs & args );

	void modWard( const BaseAppIntInterface::modWardArgs & args );

	// ... end of messages forwarded from cell.

	void pipeIntMessage( int type, BinaryIStream & data, uint length );

	bool sendToClient( bool expectData = false );

	/* External Interface */

	// receive an update from the client for our position
	void avatarUpdateImplicit(
		const BaseAppExtInterface::avatarUpdateImplicitArgs & args );
	void avatarUpdateExplicit(
		const BaseAppExtInterface::avatarUpdateExplicitArgs & args );
	void avatarUpdateWardImplicit(
		const BaseAppExtInterface::avatarUpdateWardImplicitArgs & args );
	void avatarUpdateWardExplicit(
		const BaseAppExtInterface::avatarUpdateWardExplicitArgs & args );

	void ackPhysicsCorrection(
		const BaseAppExtInterface::ackPhysicsCorrectionArgs & args );
	void ackWardPhysicsCorrection(
		const BaseAppExtInterface::ackWardPhysicsCorrectionArgs & args );

	void requestEntityUpdate( const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header, BinaryIStream & data );

	void enableEntities( const BaseAppExtInterface::enableEntitiesArgs & args );

	void restoreClientAck(
		const BaseAppExtInterface::restoreClientAckArgs & args );

	void disconnectClient(
			const BaseAppExtInterface::disconnectClientArgs & args );

	void pipeExtMessage( int type, BinaryIStream & data, uint length );

	void handleExtMessage( int type, BinaryIStream & data, uint length );

	// selectors for which direction delay and loss calls affect
	enum
	{
		whichUDPIncoming = 1,
		whichUDPOutgoing = 2,
		whichUDPBoth = 3
	};

	// set a delay for this client
	void delay(uint msecMin, uint msecMax = 0, int whichUDP = whichUDPBoth);
	// set a loss for this client
	void loss(float percantageLoss, int whichUDP = whichUDPBoth);

	static WatcherPtr pWatcher();

	double roundTripTime() const;
	double timeSinceHeardFromClient() const;

	bool isRestoringClient() const	{ return isRestoringClient_; }

	typedef std::vector< EntityID > Wards;
	typedef std::vector< float > LatencyTriggers;

	void callClientMethod( const Mercury::Address & srcAddr,
						   Mercury::UnpackedMessageHeader & header, 
						   BinaryIStream &data );

	const std::string & encryptionKey() const { return encryptionKey_; }
	void encryptionKey( const std::string & data ) { encryptionKey_ = data; }

	RateLimitMessageFilterPtr pRateLimiter() 		{ return pRateLimiter_; }

	static EventHistoryStats & privateClientStats();

	void cellBackupHasWitness( bool v ) 	{ cellBackupHasWitness_ = v; }

	void regenerateSessionKey();

	bool basePlayerCreatedOnClient() const
		{ return basePlayerCreatedOnClient_; }

private:
	static DownloadStreamer & downloadStreamer();

	// -------------------------------------------------------------------------
	// Section: Client channel stuff
	// -------------------------------------------------------------------------

	class ClientBundlePrimer : public Mercury::BundlePrimer
	{
	public:
		ClientBundlePrimer( Proxy & proxy ) : proxy_( proxy ) {}
		void primeBundle( Mercury::Bundle & bundle );
		int numUnreliableMessages() const;

	private:
		Proxy & proxy_;
	};

	friend class ClientBundlePrimer;

	bool hasOutstandingEnableWitness() const
		{ return numOutstandingEnableWitness_ != 0; }

	Mercury::Channel & clientChannel() { return *pClientChannel_; }
	Mercury::Bundle & clientBundle() 
	{ 
		return pClientChannel_ ? pClientChannel_->bundle() : 
			*pBufferedClientBundle_;
	}

	void setClientChannel( Mercury::Channel * pChannel );
	int addOpportunisticData( Mercury::Bundle * b );
	void sendEnableDisableWitness( bool enable = true,
			bool isRestore = false );
	void logBadWardWarning( EntityID ward );

	Mercury::Channel *	pClientChannel_;
	ClientBundlePrimer	clientBundlePrimer_;

	std::string			encryptionKey_;
	SessionKey			sessionKey_;

	ClientEntityMailBox * pClientEntityMailBox_;
	friend class ClientEntityMailBox;

	bool		entitiesEnabled_;
	bool		basePlayerCreatedOnClient_;

	Wards			wards_;
	PySTLSequenceHolder< Wards >			wardsHolder_;
	GameTime		lastModWardTime_;

	LatencyTriggers	latencyTriggers_;
	PySTLSequenceHolder< LatencyTriggers >	latencyTriggersHolder_;

	float	latencyAtLastCheck_;

	bool	isRestoringClient_;

	DataDownloads dataDownloads_;

	// The amount of bandwidth that this proxy would like have to stream
	// downloads to the attached client, in bytes/tick.
	int 				downloadRate_;

	// The actual streaming rate that this proxy was at the last time it had to
	// throttle back due to packet loss.
	int					apparentStreamingLimit_;

	// The average earliest unacked packet age for this client, used as a
	// baseline for calculating when packet loss has occurred.
	float				avgUnackedPacketAge_;

	// The number of packets sent to this client in the previous tick
	int 				prevPacketsSent_;

	// The total number of bytes downloaded to this client
	uint64				totalBytesDownloaded_;

	void modifyDownloadRate( int delta );
	int scaledDownloadRate() const;

	ProxyPusher *	pProxyPusher_;

	GameTime			lastLatencyCheckTime_;

	std::auto_ptr< Mercury::Bundle > pBufferedClientBundle_;

	/**
	 *	Pass ownership of the rate limiter and its associated callback object
	 *	to this proxy.
	 *
	 *	@param pRateLimiter 		the RateLimitMessageFilter object
	 */
	void pRateLimiter( RateLimitMessageFilterPtr pRateLimiter )
	{
		pRateLimiter_ = pRateLimiter;
		if (pRateLimiter_)
		{
			pRateLimiter_->setCallback( &rateLimitCallback_ );
		}
	}


	RateLimitMessageFilterPtr		pRateLimiter_;
	ProxyRateLimitCallback			rateLimitCallback_;

	bool			cellHasWitness_;
	bool			cellBackupHasWitness_;
	int				numOutstandingEnableWitness_;
};

#endif // PROXY_HPP
