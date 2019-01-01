/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef NETWORK_INTERFACE_HPP
#define NETWORK_INTERFACE_HPP

#include "basictypes.hpp"
#include "channel.hpp"
#include "endpoint.hpp"
#include "interface_table.hpp"
#include "misc.hpp"
#include "sending_stats.hpp"

#include "cstdmf/timer_handler.hpp"



namespace Mercury
{

class Channel;
class ChannelFinder;
class ChannelTimeOutHandler;
class CondemnedChannels;
class DelayedChannels;
class DispatcherCoupling;
class IrregularChannels;
class KeepAliveChannels;
class OnceOffSender;
class PacketMonitor;
class PacketReceiverStats;
class RequestManager;

enum NetworkInterfaceType
{
	NETWORK_INTERFACE_INTERNAL,
	NETWORK_INTERFACE_EXTERNAL
};

/**
 *	This class manages sets of channels.
 */
class NetworkInterface : public TimerHandler
{
public:
	/**
	 *  The desired receive buffer size on a socket
	 */
	static const int RECV_BUFFER_SIZE;
	static const char * USE_BWMACHINED;

	NetworkInterface( EventDispatcher * pMainDispatcher,
		NetworkInterfaceType interfaceType,
		uint16 listeningPort = 0, const char * listeningInterface = NULL );
	~NetworkInterface();

	void attach( EventDispatcher & mainDispatcher );
	void detach();

	void prepareForShutdown();
	void stopPingingAnonymous();

	void processUntilChannelsEmpty( float timeout = 10.f );

	bool recreateListeningSocket( uint16 listeningPort,
							const char * listeningInterface );

	Reason registerWithMachined( const std::string & name, int id /*,
		bool isRegister*/ );

	bool registerChannel( Channel & channel );
	bool deregisterChannel( Channel & channel );

	void onAddressDead( const Address & addr );

	bool isDead( const Address & addr ) const;

	enum IndexedChannelFinderResult
	{
		INDEXED_CHANNEL_HANDLED,
		INDEXED_CHANNEL_NOT_HANDLED,
		INDEXED_CHANNEL_CORRUPTED
	};

	Channel * findChannel( const Address & addr,
							bool createAnonymous  = false );

	INLINE Channel & findOrCreateChannel( const Address & addr );

	IndexedChannelFinderResult findIndexedChannel( ChannelID channelID,
			const Mercury::Address & srcAddr,
			Packet * pPacket, ChannelPtr & pChannel ) const;

	ChannelPtr findCondemnedChannel( ChannelID channelID ) const;

	void registerChannelFinder( ChannelFinder * pFinder );

	void delAnonymousChannel( const Address & addr );

	void setIrregularChannelsResendPeriod( float seconds );

	bool hasUnackedPackets() const;
	bool deleteFinishedChannels();

	void onChannelGone( Channel * pChannel );
	void onChannelTimeOut( Channel * pChannel );

	void cancelRequestsFor( Channel * pChannel );
	void cancelRequestsFor( ReplyMessageHandler * pHandler, Reason reason );

	void delayedSend( Channel & channel );
	void sendIfDelayed( Channel & channel );

	// -------------------------------------------------------------------------
	// Section: Processing
	// -------------------------------------------------------------------------

	Reason processPacketFromStream( const Address & addr,
		BinaryIStream & data );

	// -------------------------------------------------------------------------
	// Section: Accessors
	// -------------------------------------------------------------------------

	CondemnedChannels & condemnedChannels() { return *pCondemnedChannels_; }
	IrregularChannels & irregularChannels()	{ return *pIrregularChannels_; }
	KeepAliveChannels & keepAliveChannels()	{ return *pKeepAliveChannels_; }

	EventDispatcher & dispatcher()			{ return *pDispatcher_; }
	EventDispatcher & mainDispatcher()		{ return *pMainDispatcher_; }

	InterfaceTable & interfaceTable();
	const InterfaceTable & interfaceTable() const;

	const PacketReceiverStats & receivingStats() const;
	const SendingStats & sendingStats() const	{ return sendingStats_; }

	ChannelTimeOutHandler * pChannelTimeOutHandler() const
		{ return pChannelTimeOutHandler_; }

	void pChannelTimeOutHandler( ChannelTimeOutHandler * pHandler )
		{ pChannelTimeOutHandler_ = pHandler; }

	INLINE const Address & address() const;

	void setPacketMonitor( PacketMonitor * pPacketMonitor );

	bool isExternal() const				{ return isExternal_; }

	bool isVerbose() const				{ return isVerbose_; }
	void isVerbose( bool value )		{ isVerbose_ = value; }

	Endpoint & socket()					{ return socket_; }

	// Sending related
	OnceOffSender & onceOffSender()		{ return *pOnceOffSender_; }

	void dropNextSend( bool v = true ) { dropNextSend_ = v; }
	bool isDroppingNextSend() const		{ return dropNextSend_; }

	void shouldUseChecksums( bool b )	{ shouldUseChecksums_ = b; } 
	bool shouldUseChecksums() const		{ return shouldUseChecksums_; }

	const char * c_str() const { return socket_.c_str(); }

	void setLatency( float latencyMin, float latencyMax );
	void setLossRatio( float lossRatio );

	// This property is so that data can be associated with a interface. Message
	// handlers get access to the interface that received the message and can 
	// get access to this data. Bots use this so that they know which
	// ServerConnection should handle the message.
	void * pExtensionData() const			{ return pExtensionData_; }
	void pExtensionData( void * pData )		{ pExtensionData_ = pData; }

	unsigned int numBytesReceivedForMessage( uint8 msgID ) const;

	// -------------------------------------------------------------------------
	// Section: Sending
	// -------------------------------------------------------------------------
	void send( const Address & address, Bundle & bundle,
		Channel * pChannel = NULL );
	void sendPacket( const Address & addr, Packet * pPacket,
			Channel * pChannel, bool isResend );
	void sendRescheduledPacket( const Address & address, Packet * pPacket,
						Channel * pChannel );

	Reason basicSendWithRetries( const Address & addr, Packet * pPacket );
	Reason basicSendSingleTry( const Address & addr, Packet * pPacket );

	bool rescheduleSend( const Address & addr, Packet * pPacket );

	bool hasArtificialLossOrLatency() const
	{
		return (artificialLatencyMin_ != 0) || (artificialLatencyMax_ != 0) ||
			(artificialDropPerMillion_ != 0);
	}

	bool isGood() const
	{
		return (socket_ != -1) && !address_.isNone();
	}

	void onPacketIn( const Address & addr, const Packet & packet );
	void onPacketOut( const Address & addr, const Packet & packet );

#if ENABLE_WATCHERS
	static WatcherPtr pWatcher();
#endif

private:
	void finaliseRequestManager();

	enum TimeoutType
	{
		TIMEOUT_DEFAULT = 0,
		TIMEOUT_RECENTLY_DEAD_CHANNEL
	};

	virtual void handleTimeout( TimerHandle handle, void * arg );

	SeqNum getNextSequenceID();	// defined in the .cpp

	void closeSocket();

	// -------------------------------------------------------------------------
	// Section: Private data
	// -------------------------------------------------------------------------

	Endpoint		socket_;

	// The address of this socket.
	Address	address_;

	PacketReceiver * pPacketReceiver_;

	/// External interfaces remember recently deregistered channels for a little
	// while and drop incoming packets from those addresses.  This is to avoid
	/// processing packets from recently disconnected clients, especially ones
	/// using channel filters.  Attempting to process these packets generates
	/// spurious corruption warnings because they are processed raw once the
	/// channel is gone.
	///
	/// The mapping is addresses of recently deregistered channels, mapped to
	/// the timer ID for when they will time out.
	typedef std::map< Address, TimerHandle > RecentlyDeadChannels;
	RecentlyDeadChannels recentlyDeadChannels_;

	typedef std::map< Address, Channel * >	ChannelMap;
	ChannelMap					channelMap_;

	IrregularChannels * pIrregularChannels_;
	CondemnedChannels * pCondemnedChannels_;
	KeepAliveChannels * pKeepAliveChannels_;

	DelayedChannels * 			pDelayedChannels_;

	ChannelFinder *				pChannelFinder_;

	ChannelTimeOutHandler *		pChannelTimeOutHandler_;

	/// Indicates whether this is listening on an external interface.
	const bool isExternal_;

	bool isVerbose_;

	EventDispatcher * pDispatcher_;
	EventDispatcher * pMainDispatcher_;

	SeqNum nextSequenceID_;

	InterfaceTable * pInterfaceTable_;

	// Must be below dispatcher_ and pInterfaceTable_
	RequestManager * pRequestManager_;

	void * pExtensionData_;

	// -------------------------------------------------------------------------
	// Section: Sending
	// -------------------------------------------------------------------------
	OnceOffSender *				pOnceOffSender_;

	PacketMonitor *				pPacketMonitor_;

	/// State flag used in debugging to indicate that the next outgoing packet
	/// should be dropped
	bool	dropNextSend_;

	// Of every million packets sent, this many packets will be dropped for
	// debugging.
	int		artificialDropPerMillion_;

	// In milliseconds
	int		artificialLatencyMin_;
	int		artificialLatencyMax_;

	bool shouldUseChecksums_;

	SendingStats	sendingStats_;
};

} // namespace Mercury

#ifdef CODE_INLINE
#include "network_interface.ipp"
#endif

#endif // NETWORK_INTERFACE_HPP
