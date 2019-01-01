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

#include "network_interface.hpp"

#ifndef CODE_INLINE
#include "network_interface.ipp"
#endif

#include "bundle.hpp"
#include "channel_finder.hpp"
#include "condemned_channels.hpp"
#include "delayed_channels.hpp"
#include "event_dispatcher.hpp"
#include "error_reporter.hpp"
#include "interface_table.hpp"
#include "irregular_channels.hpp"
#include "keepalive_channels.hpp"
#include "machined_utils.hpp"
#include "once_off_packet.hpp"
#include "packet_monitor.hpp"
#include "packet_receiver.hpp"
#include "request_manager.hpp"
#include "rescheduled_sender.hpp"


#ifdef PLAYSTATION3
#include "ps3_compatibility.hpp"
#endif

namespace Mercury
{



// -----------------------------------------------------------------------------
// Section: Constants
// -----------------------------------------------------------------------------

/**
 *  How much receive buffer we want for sockets.
 */
const int NetworkInterface::RECV_BUFFER_SIZE = 16 * 1024 * 1024; // 16MB
const char * NetworkInterface::USE_BWMACHINED = "bwmachined";


// -----------------------------------------------------------------------------
// Section: Construction
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
NetworkInterface::NetworkInterface( Mercury::EventDispatcher * pMainDispatcher,
		NetworkInterfaceType networkInterfaceType,
		uint16 listeningPort, const char * listeningInterface ) :
	socket_(),
	address_( Address::NONE ),
	pPacketReceiver_( NULL ),
	recentlyDeadChannels_(),
	channelMap_(),
	pIrregularChannels_( new IrregularChannels() ),
	pCondemnedChannels_( new CondemnedChannels() ),
	pKeepAliveChannels_( new KeepAliveChannels() ),
	pDelayedChannels_( new DelayedChannels() ),
	pChannelFinder_( NULL ),
	pChannelTimeOutHandler_( NULL ),
	isExternal_( networkInterfaceType == NETWORK_INTERFACE_EXTERNAL ),
	isVerbose_( true ),
	pDispatcher_( new EventDispatcher ),
	pMainDispatcher_( NULL ),
	nextSequenceID_( 1 ),
	pInterfaceTable_( new InterfaceTable( *pDispatcher_ ) ),
	pRequestManager_( new RequestManager( isExternal_, 
		*pDispatcher_ ) ),
	pExtensionData_( NULL ),
	pOnceOffSender_( new OnceOffSender() ),
	pPacketMonitor_( NULL ),
	dropNextSend_( false ),
	artificialDropPerMillion_( 0 ),
	artificialLatencyMin_( 0 ),
	artificialLatencyMax_( 0 ),
	shouldUseChecksums_( false ),
	sendingStats_()
{
	pPacketReceiver_ = new PacketReceiver( socket_, *this );

	// This registers the file descriptor and so needs to be done after
	// initialising fdReadSet_ etc.
	this->recreateListeningSocket( listeningPort, listeningInterface );

	if (pMainDispatcher != NULL)
	{
		this->attach( *pMainDispatcher );
	}

	// and put our request manager in as the reply handler
	this->interfaceTable().serve( InterfaceElement::REPLY,
		InterfaceElement::REPLY.id(), pRequestManager_ );
}


/**
 *	This method initialises this object.
 */
void NetworkInterface::attach( EventDispatcher & mainDispatcher )
{
	MF_ASSERT( pMainDispatcher_ == NULL );

	pMainDispatcher_ = &mainDispatcher;

	mainDispatcher.attach( this->dispatcher() );

	pDelayedChannels_->init( this->mainDispatcher() );
	sendingStats_.init( this->mainDispatcher() );
}


/**
 *	This method prepares this NetworkInterface for being shut down.
 */
void NetworkInterface::prepareForShutdown()
{
	this->finaliseRequestManager();
}


/**
 *	This method destroys the RequestManager. It should be called when shutting
 *	down to ensure that no ReplyMessageHandler instances are cancelled when the
 *	server is in a bad state (e.g. after the App has been destroyed.
 */
void NetworkInterface::finaliseRequestManager()
{
	this->interfaceTable().serve( InterfaceElement::REPLY,
		InterfaceElement::REPLY.id(), NULL );

	RequestManager * pRequestManager = pRequestManager_;
	pRequestManager_ = NULL;
	delete pRequestManager;
}


/**
 *	Destructor.
 */
NetworkInterface::~NetworkInterface()
{
	this->interfaceTable().deregisterWithMachined( this->address() );

	// This cancels outstanding requests. Need to make sure no more are added.
	this->finaliseRequestManager();

	// Delete any channels this owns.
	ChannelMap::iterator iter = channelMap_.begin();
	while (iter != channelMap_.end())
	{
		ChannelMap::iterator oldIter = iter++;
		Channel * pChannel = oldIter->second;

		if (pChannel->isOwnedByInterface())
		{
			pChannel->destroy();
		}
		else
		{
			WARNING_MSG( "NetworkInterface::~NetworkInterface: "
					"Channel to %s is still registered\n",
				pChannel->c_str() );
		}
	}

	this->detach();

	this->closeSocket();

	delete pDelayedChannels_;
	pDelayedChannels_ = NULL;

	delete pIrregularChannels_;
	pIrregularChannels_ = NULL;

	delete pKeepAliveChannels_;
	pKeepAliveChannels_ = NULL;

	delete pCondemnedChannels_;
	pCondemnedChannels_ = NULL;

	delete pOnceOffSender_;
	pOnceOffSender_ = NULL;

	delete pInterfaceTable_;
	pInterfaceTable_ = NULL;

	delete pPacketReceiver_;
	pPacketReceiver_ = NULL;

	delete pDispatcher_;
	pDispatcher_ = NULL;
}


/**
 * Disassociates this interface from a dispatcher.
 */
void NetworkInterface::detach()
{
	if (pMainDispatcher_ != NULL )
	{
		sendingStats_.fini();
		pDelayedChannels_->fini( this->mainDispatcher() );

		pMainDispatcher_->detach( this->dispatcher() );

		RecentlyDeadChannels::iterator iter = recentlyDeadChannels_.begin();

		while (iter != recentlyDeadChannels_.end())
		{
			iter->second.cancel();

			++iter;
		}

		pMainDispatcher_ = NULL;
	}
}

/**
 *	This method prepares this network interface for shutting down. It will stop
 *	pinging anonymous channels.
 */
void NetworkInterface::stopPingingAnonymous()
{
	this->keepAliveChannels().stopMonitoring(
			this->dispatcher() );
}


/**
 *  This method processes events until all registered channels have no unacked
 *  packets, and there are no more unacked once-off packets.
 */
void NetworkInterface::processUntilChannelsEmpty( float timeout )
{
	bool done = false;
	uint64 startTime = timestamp();
	uint64 endTime = startTime + uint64( timeout * stampsPerSecondD() );

	while (!done && (timestamp() < endTime))
	{
		this->mainDispatcher().processOnce();
		this->dispatcher().processOnce();

		done = !this->hasUnackedPackets();

		if (!this->deleteFinishedChannels())
		{
			done = false;
		}

		// Wait 100ms
#if defined( PLAYSTATION3 )
		sys_timer_usleep( 100000 );
#elif !defined( _WIN32 )
		usleep( 100000 );
#else
		Sleep( 100 );
#endif
	}

	this->mainDispatcher().errorReporter().reportPendingExceptions(
			true /*reportBelowThreshold*/ );

	if (!done)
	{
		WARNING_MSG( "NetworkInterface::processUntilChannelsEmpty: "
			"Timed out after %.1fs, unacked packets may have been lost\n",
			timeout );
	}
}


/**
 *	This method closes the socket associated with this interface. It will also
 *	deregister the interface if necessary.
 */
void NetworkInterface::closeSocket()
{
	// first unregister any existing interfaces.
	if (socket_.good())
	{
		this->interfaceTable().deregisterWithMachined( this->address() );

		this->dispatcher().deregisterFileDescriptor( socket_ );
		socket_.close();
		socket_.detach();	// in case close failed
	}
}


/**
 *	This method discards the existing socket and attempts to create a new one
 *	with the given parameters. The interfaces served by a nub are re-registered
 *	on the new socket if successful.
 *
 *	@param listeningPort		The port to listen on.
 *	@param listeningInterface	The network interface to listen on.
 *	@return	true if successful, otherwise false.
 */
bool NetworkInterface::recreateListeningSocket( uint16 listeningPort,
	const char * listeningInterface )
{
	this->closeSocket();


// 	TRACE_MSG( "Mercury::NetworkInterface:recreateListeningSocket: %s\n",
// 		listeningInterface ? listeningInterface : "NULL" );

	// clear this unless it gets set otherwise
	address_.ip = 0;
	address_.port = 0;
	address_.salt = 0;

	// make the socket
	socket_.socket( SOCK_DGRAM );

	if (!socket_.good())
	{
		ERROR_MSG( "NetworkInterface::recreateListeningSocket: "
				"couldn't create a socket\n" );
		return false;
	}

	this->dispatcher().registerFileDescriptor( socket_, pPacketReceiver_ );

	// ask endpoint to parse the interface specification into a name
	char ifname[IFNAMSIZ];
	u_int32_t ifaddr = INADDR_ANY;
	bool listeningInterfaceEmpty =
		(listeningInterface == NULL || listeningInterface[0] == 0);

	// Query bwmachined over the local interface (dev: lo) for what it
	// believes the internal interface is.
	if (listeningInterface &&
		(strcmp( listeningInterface, USE_BWMACHINED ) == 0))
	{
		INFO_MSG( "NetworkInterface::recreateListeningSocket: "
				"Querying BWMachined for interface\n" );

		if (MachineDaemon::queryForInternalInterface( ifaddr ))
		{
			WARNING_MSG( "NetworkInterface::recreateListeningSocket: "
				"No address received from machined so binding to all interfaces.\n" );
		}
	}
	else if (socket_.findIndicatedInterface( listeningInterface, ifname ) == 0)
	{
		INFO_MSG( "NetworkInterface::recreateListeningSocket: "
				"Creating on interface '%s' (= %s)\n",
			listeningInterface, ifname );
		if (socket_.getInterfaceAddress( ifname, ifaddr ) != 0)
		{
			WARNING_MSG( "NetworkInterface::recreateListeningSocket: "
				"Couldn't get addr of interface %s so using all interfaces\n",
				ifname );
		}
	}
	else if (!listeningInterfaceEmpty)
	{
		WARNING_MSG( "NetworkInterface::recreateListeningSocket: "
				"Couldn't parse interface spec '%s' so using all interfaces\n",
			listeningInterface );
	}

	// now we know where to bind, so do so
	if (socket_.bind( listeningPort, ifaddr ) != 0)
	{
		ERROR_MSG( "NetworkInterface::recreateListeningSocket: "
				"Couldn't bind the socket to %s (%s)\n",
			Address( ifaddr, listeningPort ).c_str(), strerror( errno ) );
		socket_.close();
		socket_.detach();
		return false;
	}

	// but for advertising it ask the socket for where it thinks it's bound
	socket_.getlocaladdress( (u_int16_t*)&address_.port,
		(u_int32_t*)&address_.ip );

	if (address_.ip == 0)
	{
		// we're on INADDR_ANY, report the address of the
		//  interface used by the default route then
		if (socket_.findDefaultInterface( ifname ) != 0 ||
			socket_.getInterfaceAddress( ifname,
				(u_int32_t&)address_.ip ) != 0)
		{
			ERROR_MSG( "NetworkInterface::recreateListeningSocket: "
				"Couldn't determine ip addr of default interface\n" );

			socket_.close();
			socket_.detach();
			return false;
		}

		INFO_MSG( "NetworkInterface::recreateListeningSocket: "
				"bound to all interfaces with default route "
				"interface on %s ( %s )\n",
			ifname, address_.c_str() );
	}

	INFO_MSG( "NetworkInterface::recreateListeningSocket: address %s\n",
		address_.c_str() );

	socket_.setnonblocking( true );

#if defined( unix ) && !defined( PLAYSTATION3 )
	int recverrs = true;
	setsockopt( socket_, SOL_IP, IP_RECVERR, &recverrs, sizeof(int) );
#endif

#ifdef MF_SERVER
	if (!socket_.setBufferSize( SO_RCVBUF, RECV_BUFFER_SIZE ))
	{
		WARNING_MSG( "NetworkInterface::recreateListeningSocket: "
			"Operating with a receive buffer of only %d bytes (instead of %d)\n",
			socket_.getBufferSize( SO_RCVBUF ), RECV_BUFFER_SIZE );
	}
#endif

	this->interfaceTable().registerWithMachined( this->address() );

	return true;
}


/**
 *	This method is used to register or deregister an interface with the machine
 *	guard (a.k.a. machined).
 */
Reason NetworkInterface::registerWithMachined(
		const std::string & name, int id )
{
	return this->interfaceTable().registerWithMachined( this->address(),
					name, id );
}


/**
 *	This method registers a channel with this interface.
 */
bool NetworkInterface::registerChannel( Channel & channel )
{
	MF_ASSERT( channel.addr() != Address::NONE );
	MF_ASSERT( &channel.networkInterface() == this );

	ChannelMap::iterator iter = channelMap_.find( channel.addr() );
	Channel * pExisting = iter != channelMap_.end() ? iter->second : NULL;

	// Should not register a channel twice.
	IF_NOT_MF_ASSERT_DEV( !pExisting )
	{
		return false;
	}

	channelMap_[ channel.addr() ] = &channel;

	return true;
}


/**
 *	This method deregisters a channel that has been previously registered with
 *	this interface.
 */
bool NetworkInterface::deregisterChannel( Channel & channel )
{
	const Address & addr = channel.addr();
	MF_ASSERT( addr != Address::NONE );

	if (!channelMap_.erase( addr ))
	{
		CRITICAL_MSG( "NetworkInterface::deregisterChannel: "
				"Channel not found %s!\n",
			addr.c_str() );
		return false;
	}

	if (channel.isExternal() && pMainDispatcher_)
	{
		RecentlyDeadChannels::iterator iter = recentlyDeadChannels_.begin();

		if (iter != recentlyDeadChannels_.end())
		{
			iter->second.cancel();
		}

		TimerHandle timeoutHandle = this->dispatcher().addOnceOffTimer(
				60 * 1000000, this,
				(void*)TIMEOUT_RECENTLY_DEAD_CHANNEL );

		recentlyDeadChannels_[ addr ] = timeoutHandle;
	}

	return true;
}


/**
 *  This method cleans up all internal data structures and timers related to the
 *  specified address.
 */
void NetworkInterface::onAddressDead( const Address & addr )
{
	this->onceOffSender().onAddressDead( addr, *this );

	// Clean up the anonymous channel to the dead address, if there was one.
	Channel * pDeadChannel = this->findChannel( addr );

	if (pDeadChannel && pDeadChannel->isAnonymous())
	{
		pDeadChannel->hasRemoteFailed( true );
	}
}


/**
 *	This method returns whether or not an address has recently died.
 */
bool NetworkInterface::isDead( const Address & addr ) const
{
	return recentlyDeadChannels_.find( addr ) != recentlyDeadChannels_.end();
}


/**
 *	This method handles timer events.
 */
void NetworkInterface::handleTimeout( TimerHandle handle, void * arg )
{
	if (arg == (void*)TIMEOUT_RECENTLY_DEAD_CHANNEL)
	{
		// Find the dead channel in the map
		for (RecentlyDeadChannels::iterator iter = recentlyDeadChannels_.begin();
			 iter != recentlyDeadChannels_.end(); ++iter)
		{
			if (iter->second == handle)
			{
				recentlyDeadChannels_.erase( iter );
				break;
			}
		}
	}
}


/**
 *	This method finds the channel associated with a given id.
 *
 *	@param channelID The id of the channel to find.
 *	@param srcAddr The address that the packet came from.
 *	@param pPacket The packet that's requesting to find the channel. It is
 *		possible that the registered finder will handle the packet immediately.
 *	@param rpChannel A reference to a channel pointer. This will be set to the
 *		found channel.
 *
 *	@return An enumeration indicating whether the channel was found (anything
 *		but INDEXED_CHANNEL_CORRUPTED) and whether the packet has already been
 *		handled (either INDEXED_CHANNEL_HANDLED or INDEXED_CHANNEL_NOT_HANDLED).
 */
NetworkInterface::IndexedChannelFinderResult
	NetworkInterface::findIndexedChannel( ChannelID channelID,
			const Mercury::Address & srcAddr,
			Packet * pPacket, ChannelPtr & rpChannel ) const
{
	if (pChannelFinder_ == NULL)
	{
		return INDEXED_CHANNEL_CORRUPTED;
	}

	bool hasBeenHandled = false;
	rpChannel = pChannelFinder_->find( channelID, srcAddr,
			pPacket, hasBeenHandled );

	return hasBeenHandled ?
		INDEXED_CHANNEL_HANDLED : INDEXED_CHANNEL_NOT_HANDLED;
}


/**
 *	This method finds the condemned channel with the indexed ChannelID.
 */
ChannelPtr NetworkInterface::findCondemnedChannel( ChannelID channelID ) const
{
	return pCondemnedChannels_->find( channelID );
}


/**
 *  This method sets the ChannelFinder to be used for resolving channel ids.
 */
void NetworkInterface::registerChannelFinder( ChannelFinder * pFinder )
{
	MF_ASSERT( pChannelFinder_ == NULL );
	pChannelFinder_ = pFinder;
}


/**
 *	This method returns whether there are unacked once-off packets and whether
 *	any channel has any unacked packets.
 */
bool NetworkInterface::hasUnackedPackets() const
{
	if (pOnceOffSender_->hasUnackedPackets())
	{
		return true;
	}

	// Go through registered channels and check if any have unacked packets
	ChannelMap::const_iterator iter = channelMap_.begin();

	while (iter != channelMap_.end())
	{
		const Channel & channel = *iter->second;

		if (channel.hasUnackedPackets())
		{
			return true;
		}

		++iter;
	}

	return false;
}


/**
 *	This method cleans up condemned channels that have finished.
 */
bool NetworkInterface::deleteFinishedChannels()
{
	return pCondemnedChannels_->deleteFinishedChannels();
}


/**
 *	This method returns a channel to the input address.
 *
 *	@param addr	The address of the channel to find.
 *	@param createAnonymous If set and the channel is not found, a new channel
 *		will be created and returned.
 *
 *	@return A pointer to a channel to the given address. If none is found and
 *		createAnonymous is false, NULL is returned.
 */
Channel * NetworkInterface::findChannel( const Address & addr,
		bool createAnonymous )
{
	if (addr.ip == 0)
	{
		MF_ASSERT( !createAnonymous );

		return NULL;
	}

	ChannelMap::iterator iter = channelMap_.find( addr );
	Channel * pChannel = iter != channelMap_.end() ? iter->second : NULL;

	// Indexed channels aren't supposed to be registered with the nub.
	MF_ASSERT( !pChannel || pChannel->id() == CHANNEL_ID_NULL );

	// Make a new anonymous channel if it didn't already exist and this is an
	// internal nub.
	if (!pChannel && createAnonymous)
	{
		 MF_ASSERT_DEV( !isExternal_ );

		INFO_MSG( "NetworkInterface::findChannel: "
			"Creating anonymous channel to %s\n",
			addr.c_str() );

		pChannel = new Channel( *this, addr, Channel::INTERNAL );
		pChannel->isAnonymous( true );
	}

	return pChannel;
}


/**
 *	This method is called when a channel has been condemned. Any outstanding
 *	requests are told of this failure.
 */
void NetworkInterface::onChannelGone( Channel * pChannel )
{
	this->cancelRequestsFor( pChannel );
}


/**
 *	This method cancels the requests for a given channel.
 */
void NetworkInterface::cancelRequestsFor( Channel * pChannel )
{
	if (pRequestManager_)
	{
		pRequestManager_->cancelRequestsFor( pChannel );
	}
}


/**
 *	This method cancels the requests for a given handler.
 */
void NetworkInterface::cancelRequestsFor( ReplyMessageHandler * pHandler,
	   Reason reason )
{
	if (pRequestManager_)
	{
		pRequestManager_->cancelRequestsFor( pHandler, reason );
	}
}


/**
 *	Register a channel for delayed sending.
 */
void NetworkInterface::delayedSend( Channel & channel )
{
	pDelayedChannels_->add( channel );
}


/**
 *	Sends on a channel immediately if it has been registered previously for
 *	delayed sending.
 */
void NetworkInterface::sendIfDelayed( Channel & channel )
{
	pDelayedChannels_->sendIfDelayed( channel );
}


/**
 *	This method is called when a channel times out due to inactivity.
 */
void NetworkInterface::onChannelTimeOut( Channel * pChannel )
{
	if (pChannelTimeOutHandler_)
	{
		pChannelTimeOutHandler_->onTimeOut( pChannel );
	}
	else
	{
		ERROR_MSG( "NetworkInterface::onChannelTimeOut: "
					"Channel %s timed out but no handler is registered.\n",
				pChannel->c_str() );
	}
}


/**
 *  This method condemns the anonymous channel to the specified address.
 */
void NetworkInterface::delAnonymousChannel( const Address & addr )
{
	ChannelMap::iterator iter = channelMap_.find( addr );

	if (iter != channelMap_.end())
	{
		if (iter->second->isAnonymous())
		{
			iter->second->condemn();
		}
		else
		{
			ERROR_MSG( "NetworkInterface::delAnonymousChannel: "
				"Channel to %s is not anonymous!\n",
				addr.c_str() );
		}
	}
	else
	{
		ERROR_MSG( "NetworkInterface::delAnonymousChannel: "
			"Couldn't find channel for address %s\n",
			addr.c_str() );
	}
}


/**
 *	This method sets the period that irregular channels are resent.
 */
void NetworkInterface::setIrregularChannelsResendPeriod( float seconds )
{
	pIrregularChannels_->setPeriod( seconds, this->dispatcher() );
}


/**
 *	This method returns the next sequence ID to use.
 *	It is private, and for use only within Mercury.
 */
inline SeqNum NetworkInterface::getNextSequenceID()
{
	SeqNum ret = nextSequenceID_;
	nextSequenceID_ = Channel::seqMask( nextSequenceID_ + 1 );
	return ret;
}


// -----------------------------------------------------------------------------
// Section: Interface table
// -----------------------------------------------------------------------------

/**
 *	This method returns the interface table associated with this network
 *	interface.
 */
InterfaceTable & NetworkInterface::interfaceTable()
{
	return *pInterfaceTable_;
}

/**
 *	This method returns the interface table associated with this network
 *	interface.
 */
const InterfaceTable & NetworkInterface::interfaceTable() const
{
	return *pInterfaceTable_;
}


// -----------------------------------------------------------------------------
// Section: Processing
// -----------------------------------------------------------------------------

/**
 *  This method reads data from the stream into a packet and then processes it.
 */
Reason NetworkInterface::processPacketFromStream( const Address & addr,
	BinaryIStream & data )
{
	// Set up a fresh packet from the message and feed it to the nub
	PacketPtr pPacket = new Packet();
	int len = data.remainingLength();

	memcpy( pPacket->data(), data.retrieve( len ), len );
	pPacket->msgEndOffset( len );

	return pPacketReceiver_->processPacket( addr, pPacket.get(), NULL );
}


// -----------------------------------------------------------------------------
// Section: Sending
// -----------------------------------------------------------------------------

/**
 * 	This method sends a bundle to the given address.
 *
 * 	Note: any pointers you have into the packet may become invalid after this
 * 	call (and whenever a channel calls this too).
 *
 * 	@param address	The address to send to.
 * 	@param bundle	The bundle to send
 *	@param pChannel	The Channel that is sending the bundle.
 *				(even if the bundle is not sent reliably, it is still passed
 *				through the filter associated with the channel).
 */
void NetworkInterface::send( const Address & address,
								Bundle & bundle, Channel * pChannel )
{
	MF_ASSERT( address != Address::NONE );
	MF_ASSERT( !pChannel || pChannel->addr() == address );

	MF_ASSERT( !bundle.pChannel() || (bundle.pChannel() == pChannel) );

#if ENABLE_WATCHERS
	sendingStats_.mercuryTimer().start();
#endif // ENABLE_WATCHERS

	// ok, first of all finalise the bundle
	bundle.finalise();

	if (pRequestManager_)
	{
		pRequestManager_->addReplyOrders( bundle.replyOrders_, pChannel );
	}
	else if (!bundle.replyOrders_.empty())
	{
		ERROR_MSG( "NetworkInterface::send: pRequestManager_ is NULL\n" );
	}

	// fill in all the footers that are left to us
	Packet * pFirstOverflowPacket = NULL;

	int	numPackets = bundle.sizeInPackets();
	SeqNum firstSeq = 0;
	SeqNum lastSeq = 0;

	// Write footers for each packet.
	for (Packet * pPacket = bundle.firstPacket_.get(); 
			pPacket; 
			pPacket = pPacket->next())
	{
		MF_ASSERT( pPacket->msgEndOffset() >= Packet::HEADER_SIZE );

		// Reserve space for the checksum footer if necessary
		if (shouldUseChecksums_)
		{
			MF_ASSERT( !pPacket->hasFlags( Packet::FLAG_HAS_CHECKSUM ) );
			pPacket->reserveFooter( sizeof( Packet::Checksum ) );
			pPacket->enableFlags( Packet::FLAG_HAS_CHECKSUM );
		}

		bundle.writeFlags( pPacket );

		if (pChannel)
		{
			pChannel->writeFlags( pPacket );
		}

		if ((pChannel && pChannel->isExternal()) ||  
			pPacket->hasFlags( Packet::FLAG_IS_RELIABLE ) || 
			pPacket->hasFlags( Packet::FLAG_IS_FRAGMENT )) 
		{ 
			pPacket->reserveFooter( sizeof( SeqNum ) ); 
			pPacket->enableFlags( Packet::FLAG_HAS_SEQUENCE_NUMBER ); 
		} 

		// At this point, pPacket->back() is positioned just after the message
		// data, so we advance it to the end of where the footers end, then
		// write backwards towards the message data. We check that we finish
		// up back at the message data as a sanity check.
		const int msgEndOffset = pPacket->msgEndOffset();
		pPacket->grow( pPacket->footerSize() );

		// Pack in a zero checksum.  We'll calculate it later.
		Packet::Checksum * pChecksum = NULL;

		if (pPacket->hasFlags( Packet::FLAG_HAS_CHECKSUM ))
		{
			pPacket->packFooter( Packet::Checksum( 0 ) );
			pChecksum = (Packet::Checksum*)pPacket->back();
		}

#ifdef USE_PIGGIES
		// Write piggyback info.  Note that we should only ever do this to
		// the last packet in a bundle as it should be the only packet with
		// piggybacks on it.  Piggybacks go first so that they can be
		// stripped and the rest of the packet can be dealt with as normal.
		if (pPacket->hasFlags( Packet::FLAG_HAS_PIGGYBACKS ))
		{
			MF_ASSERT( pPacket->next() == NULL );

			int16 * lastLen = NULL;

			// Remember where the end of the piggybacks is for setting up
			// the piggyFooters Field after all the piggies have been
			// streamed on
			int backPiggyOffset = pPacket->msgEndOffset();

			for (BundlePiggybacks::const_iterator it =
					bundle.piggybacks_.begin();
				 it != bundle.piggybacks_.end(); ++it)
			{
				const BundlePiggyback &pb = **it;

				// Stream on the length first
				pPacket->packFooter( pb.len_ );
				lastLen = (int16*)pPacket->back();

				// Reserve the area for the packet data
				pPacket->shrink( pb.len_ );
				char * pdata = pPacket->back();

				// Stream on the packet header
				*(Packet::Flags*)pdata = BW_HTONS( pb.flags_ );
				pdata += sizeof( Packet::Flags );

				// Stream on the reliable messages
				for (ReliableVector::const_iterator rvit = pb.rvec_.begin();
					 rvit != pb.rvec_.end(); ++rvit)
				{
					memcpy( pdata, rvit->segBegin, rvit->segLength );
					pdata += rvit->segLength;
				}

				// Stream on sequence number footer
				*(SeqNum*)pdata = BW_HTONL( pb.seq_ );
				pdata += sizeof( SeqNum );

				// Stream on any sub-piggybacks that were lost
				if (pb.flags_ & Packet::FLAG_HAS_PIGGYBACKS)
				{
					const Field & subPiggies = pb.pPacket_->piggyFooters();
					memcpy( pdata, subPiggies.beg_, subPiggies.len_ );
					pdata += subPiggies.len_;
				}

				// Sanity check
				MF_ASSERT( pdata == (char*)lastLen );

				++sendingStats_.numPiggybacks_;

				if (isVerbose_)
				{
					DEBUG_MSG( "NetworkInterface::send( %s ): "
						"Piggybacked #%u (%d bytes) onto outgoing bundle\n",
						address.c_str(), pb.seq_, pb.len_ );
				}
			}

			// One's complement the length of the last piggyback to indicate
			// that it's the last one.
			*lastLen = BW_HTONS( ~BW_NTOHS( *lastLen ) );

			// Finish setting up the piggyFooters field for this packet.
			// We'll need this if this packet is lost and we need to
			// piggyback it onto another packet later on.
			pPacket->piggyFooters().beg_ = pPacket->back();
			pPacket->piggyFooters().len_ =
				uint16( backPiggyOffset - pPacket->msgEndOffset() );
		}
#endif //USE_PIGGIES

		if (pChannel)
		{
			pChannel->writeFooter( pPacket );
		}
		else if (pPacket->hasFlags( Packet::FLAG_HAS_ACKS ))
		{
			// For once off reliable, we only ever have one ack.
			pPacket->packFooter( (Packet::AckCount) 1 );
			pPacket->packFooter( bundle.getAck() );
		}

		// Add the sequence number
		if (pPacket->hasFlags( Packet::FLAG_HAS_SEQUENCE_NUMBER ))
		{
			// If we're sending reliable traffic on a channel, use the
			// channel's sequence numbers.  Otherwise use the nub's.
			pPacket->seq() = (pChannel && pPacket->hasFlags( Packet::FLAG_IS_RELIABLE )) ?
				pChannel->useNextSequenceID() : this->getNextSequenceID();

			pPacket->packFooter( pPacket->seq() );

			if (pPacket == bundle.firstPacket_)
			{
				firstSeq = pPacket->seq();
				lastSeq = pPacket->seq() + numPackets - 1;
			}
		}

		// Add the first request offset.
		if (pPacket->hasFlags( Packet::FLAG_HAS_REQUESTS ))
		{
			pPacket->packFooter( pPacket->firstRequestOffset() );
		}

		// add the fragment info
		if (pPacket->hasFlags( Packet::FLAG_IS_FRAGMENT ))
		{
			pPacket->packFooter( lastSeq );
			pPacket->packFooter( firstSeq );
		}

		// Make sure writing all the footers brought us back to the end of
		// the message data.
		MF_ASSERT( pPacket->msgEndOffset() == msgEndOffset );

		pPacket->writeChecksum( pChecksum );

		// set up the reliable machinery
		if (pPacket->hasFlags( Packet::FLAG_IS_RELIABLE ))
		{
			if (pChannel)
			{
				const ReliableOrder *roBeg, *roEnd;

				if (pChannel->isInternal())
				{
					roBeg = roEnd = NULL;
				}
				else
				{
					bundle.reliableOrders( pPacket, roBeg, roEnd );
				}

				if (!pChannel->addResendTimer( pPacket->seq(), pPacket, 
						roBeg, roEnd ))
				{
					if (pFirstOverflowPacket == NULL)
					{
						pFirstOverflowPacket = pPacket;
					}
					// return REASON_WINDOW_OVERFLOW;
				}
				else
				{
					MF_ASSERT( pFirstOverflowPacket == NULL );
				}
			}
			else
			{
				this->onceOffSender().addOnceOffResendTimer(
					address, pPacket->seq(), pPacket, *this );
			}
		}
	}

	// Finally actually send the darned thing. Do not send overflow packets.
	for (Packet * pPacket = bundle.firstPacket_.get();
			pPacket != pFirstOverflowPacket;
			pPacket = pPacket->next())
	{
		this->sendPacket( address, pPacket, pChannel, false );
	}

#if ENABLE_WATCHERS
	sendingStats_.mercuryTimer().stop( 1 );
#endif // ENABLE_WATCHERS

	sendingStats_.numBundlesSent_++;
	sendingStats_.numMessagesSent_ += bundle.numMessages();

	// NOTE: This statistic will be incorrect on internal nubs, since reliableOrders_
	// will always be empty, but numMessagesSent_ has the correct value, since
	// all messages are reliable.
	sendingStats_.numReliableMessagesSent_ += bundle.reliableOrders_.size();
}


/**
 *	This method sends a packet. No result is returned as it cannot be trusted.
 *	The packet may never get to the other end.
 */
void NetworkInterface::sendPacket( const Address & address,
						Packet * pPacket,
						Channel * pChannel, bool isResend )
{
	if (isResend)
	{
		sendingStats_.numBytesResent_ += pPacket->totalSize();
		++sendingStats_.numPacketsResent_;
	}
	else
	{
		if (!pPacket->hasFlags( Packet::FLAG_ON_CHANNEL ))
		{
			++sendingStats_.numPacketsSentOffChannel_;
		}
	}

	// Check if we want artificial loss or latency
	if (!this->rescheduleSend( address, pPacket ))
	{
		this->sendRescheduledPacket( address, pPacket, pChannel );
	}
}


/**
 *	This method sends the packet after rescheduling has occurred.
 */
void NetworkInterface::sendRescheduledPacket( const Address & address,
						Packet * pPacket,
						Channel * pChannel )
{
	PacketFilterPtr pFilter = pChannel ? pChannel->pFilter() : NULL;

	this->onPacketOut( address, *pPacket );

	// Otherwise send as appropriate
	if (pFilter)
	{
		pFilter->send( *this, address, pPacket );
	}
	else
	{
		this->basicSendWithRetries( address, pPacket );
	}
}


/**
 *	Basic packet sending functionality that retries a few times
 *	if there are transitory errors.
 */
Reason NetworkInterface::basicSendWithRetries( const Address & addr,
		Packet * pPacket )
{
	// try sending a few times
	int retries = 0;
	Reason reason;

	while (1)
	{
		retries++;
#if ENABLE_WATCHERS
		sendingStats_.systemTimer().start();
#endif // ENABLE_WATCHERS

		reason = this->basicSendSingleTry( addr, pPacket );

#if ENABLE_WATCHERS
		sendingStats_.systemTimer().stop( 1 );
#endif // ENABLE_WATCHERS

		if (reason == REASON_SUCCESS)
			return reason;

		// If we've got an error in the queue simply send it again;
		// we'll pick up the error later.
		if (reason == REASON_NO_SUCH_PORT && retries <= 3)
		{
			continue;
		}

		// If the transmit queue is full wait 10ms for it to empty.
		if (reason == REASON_RESOURCE_UNAVAILABLE && retries <= 3)
		{
			fd_set	fds;
			struct timeval tv = { 0, 10000 };
			FD_ZERO( &fds );
			FD_SET( socket_, &fds );

			WARNING_MSG( "NetworkInterface::basicSendWithRetries: "
				"Transmit queue full, waiting for space... (%d)\n",
				retries );

#if ENABLE_WATCHERS
			sendingStats_.systemTimer().start();
#endif

			select( socket_+1, NULL, &fds, NULL, &tv );

#if ENABLE_WATCHERS
			sendingStats_.systemTimer_.stop( 1 );
#endif

			continue;
		}

		// some other error, so don't bother retrying
		break;
	}

	// flush the error queue ... or something ('tho shouldn't we do this
	// before the next retry? if not why do we bother with it at all?
	// we'd really like to get this badPorts in fact I think)

	// Could you please add a comment saying why it was necessary?
	// From the commit message:
	// Fixed problem where the baseapp process was taking 100% CPU. This was
	// occurring when there was an EHOSTUNREACH error. The error queue was not
	// being read. select was then always return > 1.

	// The following is a bit dodgy. Used to get any pending messages
	// from the error queue.

	Address badAddress;

	while (socket_.getClosedPort( badAddress ))
	{
		ERROR_MSG( "NetworkInterface::basicSendWithRetries: "
					"Bad address is %s (discarding)\n",
				badAddress.c_str() );
	}

	return reason;
}


/**
 *	Basic packet sending function that just tries to send once.
 *
 *	@return 0 on success otherwise an error code.
 */
Reason NetworkInterface::basicSendSingleTry( const Address & addr, 
		Packet * pPacket )
{
	int len = socket_.sendto( pPacket->data(), pPacket->totalSize(), 
		addr.port, addr.ip );

	if (len == pPacket->totalSize())
	{
		sendingStats_.numBytesSent_ += len + UDP_OVERHEAD;
		sendingStats_.numPacketsSent_++;

		return REASON_SUCCESS;
	}
	else
	{
		int err;
		Reason reason;

		sendingStats_.numFailedPacketSend_++;

		// We need to store the error number because it may be changed by later
		// calls (such as OutputDebugString) before we use it.
		#ifdef unix
			err = errno;

			switch (err)
			{
				case ECONNREFUSED:	reason = REASON_NO_SUCH_PORT; break;
				case EAGAIN:		reason = REASON_RESOURCE_UNAVAILABLE; break;
				case ENOBUFS:		reason = REASON_TRANSMIT_QUEUE_FULL; break;
				default:			reason = REASON_GENERAL_NETWORK; break;
			}
		#else
			err = WSAGetLastError();

			if (err == WSAEWOULDBLOCK || err == WSAEINTR)
			{
				reason = REASON_RESOURCE_UNAVAILABLE;
			}
			else
			{
				reason = REASON_GENERAL_NETWORK;
			}
		#endif

		if (len == -1)
		{
			if (reason != REASON_NO_SUCH_PORT)
			{
				ERROR_MSG( "NetworkInterface::basicSendSingleTry( %s ): "
						"Could not send packet: %s\n",
					addr.c_str(), strerror( err ) );
			}
		}
		else
		{
			WARNING_MSG( "NetworkInterface::basicSendSingleTry( %s ): "
				"Packet length %d does not match sent length %d (err = %s)\n",
				addr.c_str(), pPacket->totalSize(), len, strerror( err ) );
		}

		return reason;
	}
}


/**
 * This method reschedules a packet to be sent to the address provided
 * some short time in the future (or drop it) depending on the latency
 * settings on the nub.
 *
 * @return true if rescheduled
 */
bool NetworkInterface::rescheduleSend( const Address & addr, Packet * pPacket )
{
	// see if we drop it
	if (this->isDroppingNextSend() ||
		((artificialDropPerMillion_ != 0) &&
		rand() < int64( artificialDropPerMillion_ ) * RAND_MAX / 1000000))
	{
		if (pPacket->seq() != SEQ_NULL)
		{
			if (pPacket->channelID() != CHANNEL_ID_NULL)
			{
				DEBUG_MSG( "NetworkInterface::rescheduleSend: "
					"dropped packet #%u to %s/%d due to artificial loss\n",
					pPacket->seq(), addr.c_str(), pPacket->channelID() );
			}
			else
			{
				DEBUG_MSG( "NetworkInterface::rescheduleSend: "
					"dropped packet #%u to %s due to artificial loss\n",
					pPacket->seq(), addr.c_str() );
			}
		}
		else
		{
			// All internal messages should be reliable
			// TODO: Removing this as client currently sets isExternal_ to
			// false temporarily while logging in.
#if 0
			MF_ASSERT( isExternal_ ||
				pPacket->msgEndOffset() == Packet::HEADER_SIZE );
#endif

			DEBUG_MSG( "NetworkInterface::rescheduleSend: "
				"Dropped packet with %d ACKs to %s due to artificial loss\n",
				pPacket->nAcks(), addr.c_str() );
		}

		this->dropNextSend( false );

		return true;
	}

	// now see if we delay it
	if (artificialLatencyMax_ == 0)
		return false;

	int latency = (artificialLatencyMax_ > artificialLatencyMin_) ?
		artificialLatencyMin_ +
				rand() % (artificialLatencyMax_ - artificialLatencyMin_) :
		artificialLatencyMin_;

	if (latency < 2)
		return false;	// 2ms => send now

	// ok, we'll delay this packet then
	new RescheduledSender( *this, addr, pPacket, latency );

	return true;
}


/**
 *	This method is called when a packet has been received (and decrypted).
 */
void NetworkInterface::onPacketIn( const Address & addr, const Packet & packet )
{
	if (pPacketMonitor_)
	{
		pPacketMonitor_->packetIn( addr, packet );
	}
}


/**
 *	This method is called when a packet is about to be sent (prior to
 *	encryption).
 */
void NetworkInterface::onPacketOut( const Address & addr, const Packet & packet )
{
	if (pPacketMonitor_)
	{
		pPacketMonitor_->packetOut( addr, packet );
	}
}


/**
 *	This method returns a object containing the stats associated with receiving
 *	for this interface.
 */
const PacketReceiverStats & NetworkInterface::receivingStats() const
{
	return pPacketReceiver_->stats();
}


#if ENABLE_WATCHERS
/**
 *	This method returns a watcher that can inspect a NetworkInterface.
 */
WatcherPtr NetworkInterface::pWatcher()
{
	static DirectoryWatcherPtr pWatcher = NULL;

	if (pWatcher == NULL)
	{
		pWatcher = new DirectoryWatcher();

		NetworkInterface * pNull = NULL;

		pWatcher->addChild( "address", &Address::watcher(),
			&pNull->address_ );

		pWatcher->addChild( "interfaceByID",
				new BaseDereferenceWatcher( InterfaceTable::pWatcherByID() ),
				&pNull->pInterfaceTable_ );

		pWatcher->addChild( "interfaceByName",
				new BaseDereferenceWatcher( InterfaceTable::pWatcherByName() ),
				&pNull->pInterfaceTable_ );

		pWatcher->addChild( "artificialLoss/dropPerMillion",
			makeWatcher( pNull->artificialDropPerMillion_,
				Watcher::WT_READ_WRITE ) );

		pWatcher->addChild( "artificialLoss/minLatency",
			makeWatcher( pNull->artificialLatencyMin_,
				Watcher::WT_READ_WRITE ) );

		pWatcher->addChild( "artificialLoss/maxLatency",
			makeWatcher( pNull->artificialLatencyMax_,
				Watcher::WT_READ_WRITE ) );

		pWatcher->addChild( "receiving",
			new BaseDereferenceWatcher( PacketReceiver::pWatcher() ),
			&pNull->pPacketReceiver_ );

		pWatcher->addChild( "sending/stats",
				SendingStats::pWatcher(), &pNull->sendingStats_ );

		pWatcher->addChild( "timing",
			new BaseDereferenceWatcher( EventDispatcher::pWatcher() ),
			&pNull->pMainDispatcher_ );

	}

	return pWatcher;
}
#endif // ENABLE_WATCHERS

} // namespace Mercury

// network_interface.cpp
