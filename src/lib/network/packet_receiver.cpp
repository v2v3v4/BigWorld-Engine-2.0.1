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

#include "packet_receiver.hpp"

#include "basictypes.hpp"
#include "bundle.hpp"
#include "channel.hpp"
#include "endpoint.hpp"
#include "error_reporter.hpp"
#include "event_dispatcher.hpp"
#include "network_interface.hpp"
#include "process_socket_stats_helper.hpp"

#ifdef PLAYSTATION3
#include "ps3_compatibility.hpp"
#endif

namespace Mercury
{

/**
 *	Constructor.
 */
PacketReceiver::PacketReceiver( Endpoint & socket,
	   NetworkInterface & networkInterface	) :
	socket_( socket ),
	networkInterface_( networkInterface ),
	pNextPacket_( new Packet() ),
	stats_(),
	onceOffReceiver_()
{
	onceOffReceiver_.init( this->dispatcher() );
}


/**
 *	Destructor.
 */
PacketReceiver::~PacketReceiver()
{
	onceOffReceiver_.fini();
}


/**
 *	This method is called when their is data on the socket.
 */
int PacketReceiver::handleInputNotification( int fd )
{
	if (this->processSocket( /*expectingPacket:*/true ))
	{
		while (this->processSocket( /*expectingPacket:*/false ))
		{
			/* pass */;
		}
	}

	return 0;
}


/**
 *	This method will read and process any pending data on this object's socket.
 */
bool PacketReceiver::processSocket( bool expectingPacket )
{
	stats_.updateSocketStats( socket_ );

	// Used to collect stats
	ProcessSocketStatsHelper statsHelper( stats_ );

	// try a recvfrom
	Address	srcAddr;
	int len = pNextPacket_->recvFromEndpoint( socket_, srcAddr );

	statsHelper.socketReadFinished( len );

	if (len <= 0)
	{
		return this->checkSocketErrors( len, expectingPacket );
	}

	// process it if it succeeded
	PacketPtr curPacket = pNextPacket_;
	pNextPacket_ = new Packet();

	Reason ret = this->processPacket( srcAddr, curPacket.get(),
			&statsHelper );

	if ((ret != REASON_SUCCESS) &&
			networkInterface_.isVerbose())
	{
		this->dispatcher().errorReporter().reportException( ret, srcAddr );
	}

	return true;
}


/**
 *	This method checks whether an error was received from a call to
 */
bool PacketReceiver::checkSocketErrors( int len, bool expectingPacket )
{
	// is len weird?
	if (len == 0)
	{
		WARNING_MSG( "PacketReceiver::processPendingEvents: "
			"Throwing REASON_GENERAL_NETWORK (1)- %s\n",
			strerror( errno ) );

		this->dispatcher().errorReporter().reportException(
				REASON_GENERAL_NETWORK );

		return true;
	}
		// I'm not quite sure what it means if len is 0
		// (0 => 'end of file', but with dgram sockets?)

#ifdef _WIN32
	DWORD wsaErr = WSAGetLastError();
#endif //def _WIN32

	// is the buffer empty?
	if (
#ifdef _WIN32
		wsaErr == WSAEWOULDBLOCK
#else
		errno == EAGAIN && !expectingPacket
#endif
		)
	{
		return false;
	}

#ifdef unix
	// is it telling us there's an error?
	if (errno == EAGAIN ||
		errno == ECONNREFUSED ||
		errno == EHOSTUNREACH)
	{
#if defined( PLAYSTATION3 )
		this->dispatcher().errorReporter().reportException(
				REASON_NO_SUCH_PORT );
		return true;
#else
		Mercury::Address offender;

		if (socket_.getClosedPort( offender ))
		{
			// If we got a NO_SUCH_PORT error and there is an internal
			// channel to this address, mark it as remote failed.  The logic
			// for dropping external channels that get NO_SUCH_PORT
			// exceptions is built into BaseApp::onClientNoSuchPort().
			if (errno == ECONNREFUSED)
			{
				Channel * pDeadChannel = 
					networkInterface_.findChannel( offender );

				if (pDeadChannel &&
						pDeadChannel->isInternal() &&
						!pDeadChannel->hasRemoteFailed())
				{
					INFO_MSG( "PacketReceiver::processPendingEvents: "
						"Marking channel to %s as dead (%s)\n",
						pDeadChannel->c_str(),
						reasonToString( REASON_NO_SUCH_PORT ) );

					pDeadChannel->hasRemoteFailed( true );
				}
			}

			this->dispatcher().errorReporter().reportException(
					REASON_NO_SUCH_PORT, offender );

			return true;
		}
		else
		{
			WARNING_MSG( "PacketReceiver::processPendingEvents: "
				"getClosedPort() failed\n" );
		}
#endif
	}
#else
	if (wsaErr == WSAECONNRESET)
	{
		return true;
	}
#endif // unix

	// ok, I give up, something's wrong
#ifdef _WIN32
	WARNING_MSG( "PacketReceiver::processPendingEvents: "
				"Throwing REASON_GENERAL_NETWORK - %d\n",
				wsaErr );
#else
	WARNING_MSG( "PacketReceiver::processPendingEvents: "
				"Throwing REASON_GENERAL_NETWORK - %s\n",
			strerror( errno ) );
#endif
	this->dispatcher().errorReporter().reportException(
			REASON_GENERAL_NETWORK );

	return true;
}


/**
 *	This is the entrypoint for new packets, which just gives it to the filter.
 */
Reason PacketReceiver::processPacket( const Address & addr, Packet * p,
	   ProcessSocketStatsHelper * pStatsHelper )
{
	// Packets arriving on external interface will probably be encrypted, so
	// there's no point examining their header flags right now.
	Channel * pChannel = networkInterface_.findChannel( addr,
			!networkInterface_.isExternal() && p->shouldCreateAnonymous() );

	if (pChannel != NULL)
	{
		// We update received times for addressed channels here.  Indexed
		// channels are done in processFilteredPacket().
		pChannel->onPacketReceived( p->totalSize() );

		if (pChannel->pFilter() && !pChannel->hasRemoteFailed())
		{
			// let the filter decide what to do with it
			return pChannel->pFilter()->recv( *this, addr, p, pStatsHelper );
		}
	}

	// If we're monitoring recent channel deaths, check now if this packet
	// should be dropped.
	if (networkInterface_.isExternal() &&
		networkInterface_.isDead( addr ))
	{
		if (networkInterface_.isVerbose())
		{
			DEBUG_MSG( "PacketReceiver::processPacket( %s ): "
				"Ignoring incoming packet on recently dead channel\n",
				addr.c_str() );
		}

		return REASON_SUCCESS;
	}

	return this->processFilteredPacket( addr, p, pStatsHelper );
}


/**
 *  This macro is used by PacketReceiver::processFilteredPacket() and
 *  PacketReceiver::processOrderedPacket() whenever they need to return early
 *  due to a corrupted incoming packet.
 */
#define RETURN_FOR_CORRUPTED_PACKET()										\
	++stats_.numCorruptedPacketsReceived_;									\
	return REASON_CORRUPTED_PACKET;											\


/**
 *	This function has to be very robust, if we intend to use this transport over
 *	the big bad internet. We basically have to assume it'll be complete garbage.
 */
Reason PacketReceiver::processFilteredPacket( const Address & addr,
		Packet * p, ProcessSocketStatsHelper * pStatsHelper )
{
	if (p->totalSize() < int( sizeof( Packet::Flags ) ))
	{
		if (networkInterface_.isVerbose())
		{
			WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
					"received undersized packet\n",
				addr.c_str() );
		}

		RETURN_FOR_CORRUPTED_PACKET();
	}

	networkInterface_.onPacketIn( addr, *p );

	// p->debugDump();

	// Make sure we understand all the flags
	if (p->flags() & ~Packet::KNOWN_FLAGS)
	{
		if (networkInterface_.isVerbose())
		{
			WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
				"received packet with bad flags %x\n",
				addr.c_str(), p->flags() );
		}

		RETURN_FOR_CORRUPTED_PACKET();
	}

	if (!p->hasFlags( Packet::FLAG_ON_CHANNEL ))
	{
		++stats_.numPacketsReceivedOffChannel_;
	}

	// Don't allow FLAG_CREATE_CHANNEL on external interfaces
	if (networkInterface_.isExternal() &&
			p->hasFlags( Packet::FLAG_CREATE_CHANNEL ))
	{
		WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
			"Got FLAG_CREATE_CHANNEL on external interface\n",
			addr.c_str() );

		RETURN_FOR_CORRUPTED_PACKET();
	}

	// make sure there's something in the packet
	if (p->totalSize() <= Packet::HEADER_SIZE)
	{
		WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
			"received undersize packet (%d bytes)\n",
			addr.c_str(), p->totalSize() );

		RETURN_FOR_CORRUPTED_PACKET();
	}

	// Start stripping footers
	if (!p->validateChecksum())
	{
		WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
				"Invalid checksum", addr.c_str() );
		RETURN_FOR_CORRUPTED_PACKET();
	}

	// Strip off piggybacks and process them immediately (i.e. before the
	// messages on this packet) as the piggybacks must be older than this packet
	if (!this->processPiggybacks( addr, p, pStatsHelper ))
	{
		WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
											"Corrupted piggyback packets\n",
										addr.c_str() );
		RETURN_FOR_CORRUPTED_PACKET();
	}

	ChannelPtr pChannel = NULL;	// don't bother getting channel twice
	bool shouldSendChannel = false;

	// Strip off indexed channel ID if present
	if (p->hasFlags( Packet::FLAG_INDEXED_CHANNEL ))
	{
		if (!p->stripFooter( p->channelID() ) ||
			!p->stripFooter( p->channelVersion() ))
		{
			WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
				"Not enough data for indexed channel footer (%d bytes left)\n",
				addr.c_str(), p->bodySize() );

			RETURN_FOR_CORRUPTED_PACKET();
		}

		switch (networkInterface_.findIndexedChannel( p->channelID(), addr,
				p, pChannel ))
		{
		case NetworkInterface::INDEXED_CHANNEL_CORRUPTED:
			WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
					"Got indexed channel packet with no finder registered\n",
				addr.c_str() );

			RETURN_FOR_CORRUPTED_PACKET();

		case NetworkInterface::INDEXED_CHANNEL_HANDLED:
			// If the packet has already been handled, we're done!
			// It may have been forwarded.
			return REASON_SUCCESS;

		case NetworkInterface::INDEXED_CHANNEL_NOT_HANDLED:
			// Common case, handled below
			break;

		default:
			MF_ASSERT( !"Invalid return value" );
			break;
		}

		// If we couldn't find the channel, check if it was recently condemned.
		if (!pChannel)
		{
			pChannel = networkInterface_.findCondemnedChannel( p->channelID() );
		}

		if (pChannel)
		{
			// We update received times for indexed channels here.  Addressed
			// channels are done in processPacket().
			pChannel->onPacketReceived( p->totalSize() );
		}
		else
		{
			WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
				"Could not get indexed channel for id %d\n",
				addr.c_str(), p->channelID() );

			return REASON_SUCCESS;
		}
	}

	if (!pChannel && p->hasFlags( Packet::FLAG_ON_CHANNEL ))
	{
		pChannel = networkInterface_.findChannel( addr );

		if (!pChannel)
		{
			MF_ASSERT_DEV( !p->shouldCreateAnonymous() );

			WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
				"Dropping packet due to absence of local channel\n",
				addr.c_str() );

			return REASON_GENERAL_NETWORK;
		}
	}

	if (pChannel)
	{
		if (pChannel->hasRemoteFailed())
		{
			// __glenc__ We could consider resetting the channel here if it has
			// FLAG_CREATE_CHANNEL.  This would help cope with fast restarts on
			// the same address.  Haven't particularly thought this through
			// though.  Could be issues with app code if you do this
			// underneath.  In fact I'm guessing you would need an onReset()
			// callback to inform the app code that this has happened.
			WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
				"Dropping packet due to remote process failure\n",
				pChannel->c_str() );

			return REASON_GENERAL_NETWORK;
		}
		// If the packet is out of date, drop it.
		else if (Channel::seqLessThan( p->channelVersion(),
					pChannel->creationVersion() ))
		{
			WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
					"Dropping packet from old channel %s (v%u < v%u)\n",
				pChannel->c_str(), addr.c_str(),
				p->channelVersion(), pChannel->creationVersion() );

			return REASON_SUCCESS;
		}
		else if (pChannel->wantsFirstPacket())
		{
			if (p->hasFlags( Packet::FLAG_CREATE_CHANNEL ))
			{
				pChannel->gotFirstPacket();
			}
			else
			{
				WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
					"Dropping packet on "
					"channel wanting FLAG_CREATE_CHANNEL (flags: %x)\n",
					pChannel->c_str(), p->flags() );

				return REASON_GENERAL_NETWORK;
			}
		}
	}

	if (p->hasFlags( Packet::FLAG_HAS_CUMULATIVE_ACK ))
	{
		if (!pChannel)
		{
			WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
					"Cumulative ack was not on a channel.\n",
				addr.c_str() );
			RETURN_FOR_CORRUPTED_PACKET();
		}

		SeqNum endSeq;

		if (!p->stripFooter( endSeq ))
		{
			WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
					"Not enough data for cumulative.\n",
				pChannel->c_str() );
			RETURN_FOR_CORRUPTED_PACKET();
		}

		if (!pChannel->handleCumulativeAck( endSeq ))
		{
			RETURN_FOR_CORRUPTED_PACKET();
		}
	}

	// Strip and handle ACKs
	if (p->hasFlags( Packet::FLAG_HAS_ACKS ))
	{
		if (!p->stripFooter( p->nAcks() ))
		{
			WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
				"Not enough data for ack count footer (%d bytes left)\n",
				addr.c_str(), p->bodySize() );

			RETURN_FOR_CORRUPTED_PACKET();
		}

		if (p->nAcks() == 0)
		{
			WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
				"Packet with FLAG_HAS_ACKS had 0 acks\n",
				addr.c_str() );

			RETURN_FOR_CORRUPTED_PACKET();
		}

		// The total size of all the ACKs on this packet
		int ackSize = p->nAcks() * sizeof( SeqNum );

		// check that we have enough footers to account for all of the
		// acks the packet claims to have (thanks go to netease)
		if (p->bodySize() < ackSize)
		{
			WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
				"Not enough footers for %d acks "
				"(have %d bytes but need %d)\n",
				addr.c_str(), p->nAcks(), p->bodySize(), ackSize );

			RETURN_FOR_CORRUPTED_PACKET();
		}

		// For each ACK that we receive, we no longer need to store the
		// corresponding packet.
		if (pChannel)
		{
			for (uint i=0; i < p->nAcks(); i++)
			{
				SeqNum seq;
				p->stripFooter( seq );

				if (!pChannel->handleAck( seq ))
				{
					WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
						"handleAck() failed for #%u\n",
						addr.c_str(), seq );

					RETURN_FOR_CORRUPTED_PACKET();
				}
			}
		}
		else if (!p->hasFlags( Packet::FLAG_ON_CHANNEL ))
		{
			for (uint i=0; i < p->nAcks(); i++)
			{
				SeqNum seq;
				p->stripFooter( seq );
				networkInterface_.onceOffSender().delOnceOffResendTimer( addr,
						seq, networkInterface_ );
			}
		}
		else
		{
			p->shrink( ackSize );

			WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
				"Got %d acks without a channel\n",
				addr.c_str(), p->nAcks() );
		}
	}

	// Strip sequence number
	if (p->hasFlags( Packet::FLAG_HAS_SEQUENCE_NUMBER ))
	{
		if (!p->stripFooter( p->seq() ))
		{
			WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
				"Not enough data for sequence number footer (%d bytes left)\n",
				addr.c_str(), p->bodySize() );

			RETURN_FOR_CORRUPTED_PACKET();
		}
	}

	// now do something if it's reliable
	if (p->hasFlags( Packet::FLAG_IS_RELIABLE ))
	{
		// first make sure it has a sequence number, so we can address it
		if (p->seq() == SEQ_NULL)
		{
			WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
					"Dropping packet due to illegal request for reliability "
					"without related sequence number\n", 
				addr.c_str() );

			RETURN_FOR_CORRUPTED_PACKET();
		}

		// should we be looking in a channel
		if (pChannel)
		{
			Channel::AddToReceiveWindowResult result =
				pChannel->addToReceiveWindow( p, addr, stats_ );

			if (!pChannel->isLocalRegular())
			{
				shouldSendChannel = true;
			}

			if (result != Channel::SHOULD_PROCESS)
			{
				// The packet is not corrupted, and has either already been
				// received, or is too early and has been buffered. In either
				// case, we send the ACK immediately, as long as the channel is
				// established and is irregular.
				if (result != Channel::PACKET_IS_CORRUPT)
				{
					if (pChannel->isEstablished() && shouldSendChannel)
					{
						Bundle emptyBundle;
						pChannel->send( &emptyBundle );
					}

					return REASON_SUCCESS;
				}

				// The packet has an invalid sequence number.
				else
				{
					RETURN_FOR_CORRUPTED_PACKET();
				}
			}
		}

		// If the packet is not on a channel (i.e. FLAG_ON_CHANNEL is not
		// set), it must be once-off reliable.
		else
		{
			// Don't allow incoming once-off-reliable traffic on external
			// interfaces since this is a potential DOS vulnerability.
			if (networkInterface_.isExternal())
			{
				NOTICE_MSG( "PacketReceiver::processFilteredPacket( %s ): "
					"Dropping deprecated once-off-reliable packet\n",
					addr.c_str() );

				return REASON_SUCCESS;
			}

			// send back the ack for this packet
			Bundle backBundle;
			backBundle.addAck( p->seq() );
			backBundle.send( addr, networkInterface_ );

			if (onceOffReceiver_.onReliableReceived(
						this->dispatcher(), addr, p->seq() ))
			{
				return REASON_SUCCESS;
			}
		}
	}
	else // if !RELIABLE
	{
		// If the packet is unreliable, confirm that the sequence number
		// is incrementing as a sanity check against malicious parties.
		if ((pChannel != NULL) && (pChannel->isExternal()))
		{
			if (!pChannel->validateUnreliableSeqNum( p->seq() ))
			{
				WARNING_MSG( "PacketReceiver::processFilteredPacket( %s ): "
					"Dropping packet due to invalid unreliable seqNum\n",
					addr.c_str() );
				RETURN_FOR_CORRUPTED_PACKET();
			}
		}

	}

	Reason oret = REASON_SUCCESS;
	PacketPtr pCurrPacket = p;
	PacketPtr pNextPacket = NULL;

	// push this packet chain (frequently one) through processOrderedPacket

	// NOTE: We check isCondemned on the channel and not isDead. If a channel
	// has isDestroyed set to true but isCondemned false, we still want to
	// process remaining messages. This can occur if there is a message that
	// causes the entity to teleport. Any remaining messages are still
	// processed and will likely be forwarded from the ghost entity to the
	// recently teleported real entity.

	// TODO: It would be nice to display a message if the channel is condemned
	// but there are messages on it.

	while (pCurrPacket &&
		((pChannel == NULL) || !pChannel->isCondemned()))
	{
		// processOrderedPacket expects packets not to be chained, since
		// chaining is used for grouping fragments into bundles.  The packet
		// chain we've set up doesn't have anything to do with bundles, so we
		// break the packet linkage before passing the packets into
		// processOrderedPacket.  This can mean that packets that aren't the one
		// just received drop their last reference, hence the need for
		// pCurrPacket and pNextPacket.
		pNextPacket = pCurrPacket->next();
		pCurrPacket->chain( NULL );

		// Make sure they are actually packets with consecutive sequences.
		MF_ASSERT( pNextPacket.get() == NULL || 
			Channel::seqMask( pCurrPacket->seq() + 1 ) == 
				pNextPacket->seq() );

		// At this point, the only footers left on the packet should be the
		// request and fragment footers.
		Reason ret = this->processOrderedPacket( addr, pCurrPacket.get(),
				pChannel.get(), pStatsHelper );

		if (oret == REASON_SUCCESS)
		{
			oret = ret;
		}

		pCurrPacket = pNextPacket;
	}

	// If this bundle was delivered to a channel and there are still ACKs to
	// send, do it now.
	if (pChannel &&
		!pChannel->isDead() &&
		shouldSendChannel &&
		pChannel->isEstablished() &&
		(pChannel->hasAcks()))
	{
		Bundle emptyBundle;
		pChannel->send( &emptyBundle );
	}

	return oret;
}

namespace
{

/**
 *	This class is used by processPiggybacks to visit and process the piggyback
 *	packets on a packet.
 */
class PiggybackProcessor : public PacketVisitor
{
public:
	PiggybackProcessor( PacketReceiver & receiver,
			const Address & addr,
			ProcessSocketStatsHelper * pStatsHelper ) :
		receiver_( receiver ),
		addr_( addr ),
		pStatsHelper_( pStatsHelper )
	{
	}

private:
	bool onPacket( PacketPtr pPacket )
	{
		Reason reason = receiver_.processFilteredPacket( addr_,
				pPacket.get(), pStatsHelper_ );
		return reason != REASON_CORRUPTED_PACKET;
	}

	PacketReceiver & receiver_;
	const Address & addr_;
	ProcessSocketStatsHelper * pStatsHelper_;
};

} // anonymous namespace


/**
 *	This method processes any piggyback packets that may be on the packet
 *	passed in.
 */
bool PacketReceiver::processPiggybacks( const Address & addr,
		Packet * pPacket, ProcessSocketStatsHelper * pStatsHelper )
{
	PiggybackProcessor processor( *this, addr, pStatsHelper );

	return pPacket->processPiggybackPackets( processor );
}


/**
 * Process a packet after any ordering guaranteed by reliable channels
 * has been imposed (further ordering guaranteed by fragmented bundles
 * is still to be imposed)
 */
Reason PacketReceiver::processOrderedPacket( const Address & addr, Packet * p,
	Channel * pChannel, ProcessSocketStatsHelper * pStatsHelper )
{
	// Label to use in debug output
#	define SOURCE_STR (pChannel ? pChannel->c_str() : addr.c_str())

	// Strip first request offset.
	if (p->hasFlags( Packet::FLAG_HAS_REQUESTS ))
	{
		if (!p->stripFooter( p->firstRequestOffset() ))
		{
			WARNING_MSG( "PacketReceiver::processOrderedPacket( %s ): "
				"Not enough data for first request offset (%d bytes left)\n",
				SOURCE_STR, p->bodySize() );

			RETURN_FOR_CORRUPTED_PACKET();
		}
	}

	// Smartpointer required to keep the packet chain alive until the
	// outputBundle is constructed in the event a fragment chain completes.
	PacketPtr pChain = NULL;
	bool isOnChannel = pChannel && p->hasFlags( Packet::FLAG_IS_RELIABLE );

	// Strip fragment footers
	if (!p->hasFlags( Packet::FLAG_IS_FRAGMENT ) &&
			p->hasFlags( Packet::FLAG_HAS_SEQUENCE_NUMBER ) &&
			(isOnChannel && pChannel->pFragments() != NULL))
	{
		// Make sure that for incoming non-fragment packets with sequence
		// numbers, we are not expecting them to be fragments. 
		if (networkInterface_.isExternal())
		{
			WARNING_MSG( "Nub::processOrderedPacket( %s ): "
					"got non-fragment packet, when expecting fragment "
					"(#%u)\n",
				pChannel->c_str(), p->seq() );
		}
		else
		{
			CRITICAL_MSG( "Nub::processOrderedPacket( %s ): "
					"got non-fragment packet, when expecting fragment "
					"(#%u)\n",
				pChannel->c_str(), p->seq() );
		}
		RETURN_FOR_CORRUPTED_PACKET();
	}


	if (p->hasFlags( Packet::FLAG_IS_FRAGMENT ))
	{
		if (!p->stripFragInfo())
		{
			WARNING_MSG( "PacketReceiver::processOrderedPacket( %s ): "
					"Invalid fragment footers\n", SOURCE_STR );

			RETURN_FOR_CORRUPTED_PACKET();
		}

		FragmentedBundle::Key key( addr, p->fragBegin() );

		// Find the existing packet chain for this bundle, if any.
		FragmentedBundlePtr pFragments = NULL;
		FragmentedBundles & fragmentedBundles =
			onceOffReceiver_.fragmentedBundles();
		FragmentedBundles::iterator fragIter = fragmentedBundles.end();

		// Channels are easy, they just maintain their own fragment chain, which
		// makes lookup really cheap.  Note that only reliable packets use the
		// channel's sequence numbers.
		if (isOnChannel)
		{
			pFragments = pChannel->pFragments();
		}

		// We need to look up off-channel fragments in fragmentedBundles
		else
		{
			fragIter = fragmentedBundles.find( key );
			pFragments = (fragIter != fragmentedBundles.end()) ?
				fragIter->second : NULL;
		}

		// If the previous fragment is really old, then this must be some
		// failed bundle that has not been resent and has been given up on, so
		// we get rid of it now.
		if (pFragments && pFragments->isOld() && !isOnChannel)
		{
			WARNING_MSG( "PacketReceiver::processOrderedPacket( %s ): "
					"Discarding abandoned stale overlapping fragmented bundle "
					"from seq %u to %u\n",
				SOURCE_STR, p->fragBegin(), pFragments->lastFragment() );

			pFragments = NULL;

			fragmentedBundles.erase( fragIter );
		}

		if (pFragments == NULL)
		{
			// If this is on a channel, it must be the first packet in the
			// bundle, since channel traffic is ordered.
			if (pChannel && p->seq() != p->fragBegin())
			{
				ERROR_MSG( "PacketReceiver::processOrderedPacket( %s ): "
						"Bundle (#%u,#%u) is missing packets before #%u\n",
					SOURCE_STR, p->fragBegin(), p->fragEnd(), p->seq() );

				RETURN_FOR_CORRUPTED_PACKET();
			}

			// This is the first fragment from this bundle we've seen, so make
			// a new element and bail out.
			pFragments = new FragmentedBundle( p );

			if (isOnChannel)
			{
				pChannel->pFragments( pFragments );
			}
			else
			{
				fragmentedBundles[ key ] = pFragments;
			}

			return REASON_SUCCESS;
		}

		if (!pFragments->addPacket( p,
					networkInterface_.isExternal(), SOURCE_STR ))
		{
			RETURN_FOR_CORRUPTED_PACKET();
		}

		// If the bundle is still incomplete, stop processing now.
		if (!pFragments->isComplete())
		{
			return REASON_SUCCESS;
		}

		// The bundle is now complete, so set p to the start of the chain and
		// we'll process the whole bundle below.
		else
		{
			// We need to acquire a reference to the fragment chain here to keep
			// it alive until we construct the outputBundle below.
			pChain = pFragments->pChain();
			p = pChain.get();

			if (isOnChannel)
			{
				pChannel->pFragments( NULL );
			}
			else
			{
				fragmentedBundles.erase( fragIter );
			}
		}
	}

	// We have a complete packet chain.  We can drop the reference in pChain now
	// since the Bundle owns it.
	Bundle outputBundle( p );
	pChain = NULL;

	Reason reason = outputBundle.dispatchMessages(
			networkInterface_.interfaceTable(),
			addr,
			pChannel,
			networkInterface_,
			pStatsHelper );

	if (reason == REASON_CORRUPTED_PACKET)
	{
		RETURN_FOR_CORRUPTED_PACKET();
	}

	return reason;

#	undef SOURCE_STR
}


#if ENABLE_WATCHERS
/**
 *	This method returns a Watcher that can be used to inspect PacketReceiver
 *	instances.
 */
WatcherPtr PacketReceiver::pWatcher()
{
	WatcherPtr pWatcher = new DirectoryWatcher();
	PacketReceiver * pNull = NULL;

	// TODO:TN Needs to be named correctly.
	pWatcher->addChild( "stats",
			PacketReceiverStats::pWatcher(), &pNull->stats_ );

	return pWatcher;
}
#endif


/**
*	This method returns the dispatcher that is used by this receiver.
*/
EventDispatcher & PacketReceiver::dispatcher()
{
	return networkInterface_.dispatcher();
}

} // namespace Mercury

// packet_receiver.cpp
