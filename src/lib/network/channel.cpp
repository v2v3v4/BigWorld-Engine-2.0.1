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

#include "channel.hpp"

#include "basictypes.hpp"
#include "bundle.hpp"
#include "condemned_channels.hpp"
#include "delayed_channels.hpp"
#include "event_dispatcher.hpp"
#include "network_interface.hpp"
#include "packet_receiver_stats.hpp"
#include "unacked_packet.hpp"

#include "cstdmf/memory_tracker.hpp"

#ifndef CODE_INLINE
#include "channel.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "Network", 0 )

namespace Mercury
{

const int EXTERNAL_CHANNEL_SIZE = 256;
const int INTERNAL_CHANNEL_SIZE = 4096;
const int INDEXED_CHANNEL_SIZE = 512;


// Maximum number of overflow packets per channel type
// maximum size calculated by max overflow packets * packet size (MTU)
uint Channel::s_maxOverflowPackets_[] =
	{ 1024, // External channel.
	  8192, // Internal channel
	  4096  // Indexed channel (ie: entity channel).
	};

bool Channel::s_assertOnMaxOverflowPackets = false;

int Channel::s_sendWindowWarnThresholds_[] =
	{ INTERNAL_CHANNEL_SIZE / 4, INDEXED_CHANNEL_SIZE / 4 };

int Channel::s_sendWindowCallbackThreshold_ = INDEXED_CHANNEL_SIZE/2;
Channel::SendWindowCallback Channel::s_pSendWindowCallback_ = NULL;

namespace
{

class WatcherIniter
{
public:
	WatcherIniter()
	{
		Channel::staticInit();
	}
};

WatcherIniter s_watcherIniter_;

} // anonymous namespace

// -----------------------------------------------------------------------------
// Section: Channel
// -----------------------------------------------------------------------------

/**
 * 	This is the constructor.
 *
 * 	@param networkInterface	The interface on which to send and receive messages
 * 	@param address	The address of our peer or Address::NONE for indexed
 *						channel.
 * 	@param traits	The traits of network this channel spans.
 *	@param minInactivityResendDelay  The minimum delay in seconds before
 *									packets are resent.
 *  @param pFilter	The packet filter to use for sending and receiving.
 *  @param id		The ID for indexed channels (if provided).
 */
Channel::Channel( NetworkInterface & networkInterface,
		const Address & address, Traits traits,
		float minInactivityResendDelay,
		PacketFilterPtr pFilter, ChannelID id ):
	pNetworkInterface_( &networkInterface ),
	traits_( traits ),
	id_( id ),
	inactivityTimerHandle_(),
	inactivityExceptionPeriod_( 0 ),
	version_( 0 ),
	creationVersion_( 0 ),
	lastReceivedTime_( 0 ),
	pFilter_( pFilter ),
	addr_( Address::NONE ),
	pBundle_( NULL ),
	windowSize_(	(traits != INTERNAL)    ? EXTERNAL_CHANNEL_SIZE :
					(id == CHANNEL_ID_NULL) ? INTERNAL_CHANNEL_SIZE :
											  INDEXED_CHANNEL_SIZE ),
	smallOutSeqAt_( 0 ),
	largeOutSeqAt_( 0 ),
	oldestUnackedSeq_( SEQ_NULL ),
	lastReliableSendTime_( 0 ),
	lastReliableResendTime_( 0 ),
	roundTripTime_( (traits == INTERNAL) ?
		stampsPerSecond() / 10 : stampsPerSecond() ),
	minInactivityResendDelay_(
		uint64( minInactivityResendDelay * stampsPerSecond() ) ),
	unreliableInSeqAt_( SEQ_NULL ),
	unackedPackets_( windowSize_ ),
	hasSeenOverflowWarning_( false ),
	inSeqAt_( 0 ),
	bufferedReceives_( windowSize_ ),
	numBufferedReceives_( 0 ),
	pFragments_( NULL ),
	highestAck_( seqMask( smallOutSeqAt_ - 1 ) ),
	irregularIter_( networkInterface.irregularChannels().end() ),
	keepAliveIter_( networkInterface.keepAliveChannels().end() ),
	isLocalRegular_( true ),
	isRemoteRegular_( true ),
	isCondemned_( false ),
	isDestroyed_( false ),
	pBundlePrimer_( NULL ),
	hasRemoteFailed_( false ),
	isAnonymous_( false ),
	unackedCriticalSeq_( SEQ_NULL ),
	pushUnsentAcksThreshold_( 0 ),
	shouldAutoSwitchToSrcAddr_( false ),
	wantsFirstPacket_( false ),
	shouldDropNextSend_( false ),

	// Stats
	numPacketsSent_( 0 ),
	numPacketsReceived_( 0 ),
	numBytesSent_( 0 ),
	numBytesReceived_( 0 ),
	numPacketsResent_( 0 ),
	numReliablePacketsSent_( 0 ),

	// Message filter
	pMessageFilter_( NULL )
{
	// This corresponds to the decRef in Channel::destroy.
	this->incRef();

	if (pFilter_ && id_ != CHANNEL_ID_NULL)
	{
		CRITICAL_MSG( "Channel::Channel: "
			"PacketFilters are not supported on indexed channels (id:%d)\n",
			id_ );
	}

	// Initialise the bundle
	this->clearBundle();

	// This registers non-indexed channels
	this->addr( address );
}


/**
 *  Static initialisation for watchers etc.
 */
void Channel::staticInit()
{
	// TODO: Update the thresholds from bw.xml, probably in a global <network>
	// section.

#ifdef MF_SERVER
	// This is only interesting on the server.

	MF_WATCH( "network/internalSendWindowSizeThreshold",
		s_sendWindowWarnThresholds_[ 0 ] );

	MF_WATCH( "network/indexedSendWindowSizeThreshold",
		s_sendWindowWarnThresholds_[ 1 ] );
#endif

}


/**
 *  This static method will look in the provided interface for an existing anonymous
 *  channel to the specified address, and if found will mark it as no longer
 *  being anonymous and return it.  If not found, the regular constructor is
 *  called and a new channel is returned.
 */
Channel * Channel::get( NetworkInterface & networkInterface,
		const Address & address )
{
	Channel * pChannel = networkInterface.findChannel( address );

	if (pChannel)
	{
		MF_ASSERT( pChannel->isAnonymous() );

		// This brings the channel back in sync with the state it would have
		// been in from a normal (explicit) construction.
		pChannel->isAnonymous( false );

		INFO_MSG( "Channel::get: "
			"Claimed anonymous channel to %s\n",
			pChannel->c_str() );

		if (pChannel->isCondemned())
		{
			WARNING_MSG( "Channel::get: "
				"Returned condemned channel to %s\n",
				pChannel->c_str() );
		}
	}
	else
	{
		pChannel = new Channel( networkInterface, address, INTERNAL );
	}

	return pChannel;
}


/**
 *	This method sets the address of this channel. If necessary, it is registered
 *	with the interface.
 */
void Channel::addr( const Address & addr )
{
	if (addr_ != addr)
	{
		lastReceivedTime_ = ::timestamp();

		if (!this->isIndexed())
		{
			if (addr_ != Address::NONE)
			{
				MF_VERIFY( pNetworkInterface_->deregisterChannel( *this ) );
			}

			addr_ = addr;

			if (addr_ != Address::NONE)
			{
				MF_VERIFY( pNetworkInterface_->registerChannel( *this ) );
			}
		}
		else
		{
			addr_ = addr;
		}
	}
}


/**
 * 	Destructor.
 */
Channel::~Channel()
{
	pNetworkInterface_->onChannelGone( this );

	this->clearState();

	delete pBundle_;
}


/**
 *	This method clears out any objects that this Channel is responsible for.
 */
void Channel::clearState( bool warnOnDiscard /*=false*/ )
{
	// Clear unacked sends
	if (this->hasUnackedPackets())
	{
		int numUnacked = 0;

		for (uint i = 0; i < unackedPackets_.size(); i++)
		{
			if (unackedPackets_[i])
			{
				// if (warnOnDiscard)
				// {
				// 	unackedPackets_[i]->pPacket_->debugDump();
				// }
				++numUnacked;
				delete unackedPackets_[i];
				unackedPackets_[i] = NULL;
			}
		}

		if (warnOnDiscard && numUnacked > 0)
		{
			WARNING_MSG( "Channel::clearState( %s ): "
				"Forgetting %d unacked packet(s)\n",
				this->c_str(), numUnacked );
		}
	}

	// Clear buffered receives
	if (numBufferedReceives_ > 0)
	{
		if (warnOnDiscard)
		{
			WARNING_MSG( "Channel::clearState( %s ): "
				"Discarding %u buffered packet(s)\n",
				this->c_str(), numBufferedReceives_ );
		}

		for (uint i=0;
			 i < bufferedReceives_.size() && numBufferedReceives_ > 0;
			 i++)
		{
			if (bufferedReceives_[i] != NULL)
			{
				bufferedReceives_[i] = NULL;
				--numBufferedReceives_;
			}
		}
	}

	// Clear any chained fragments.
	if (pFragments_)
	{
		if (warnOnDiscard)
		{
			WARNING_MSG( "Channel::clearState( %s ): "
				"Forgetting %d unprocessed packets in the fragment chain\n",
				this->c_str(), pFragments_->chainLength() );
		}

		pFragments_ = NULL;
	}

	// Reset fields.
	inSeqAt_ = 0;
	smallOutSeqAt_ = 0;
	largeOutSeqAt_ = 0;
	lastReceivedTime_ = ::timestamp();
	highestAck_ = seqMask( smallOutSeqAt_ - 1 );
	oldestUnackedSeq_ = SEQ_NULL;
	lastReliableSendTime_ = 0;
	lastReliableResendTime_ = 0;
	roundTripTime_ =
		this->isInternal() ? stampsPerSecond() / 10 : stampsPerSecond();
	hasRemoteFailed_ = false;
	unackedCriticalSeq_ = SEQ_NULL;
	wantsFirstPacket_ = false;
	shouldDropNextSend_ = false;
	numPacketsSent_ = 0;
	numPacketsReceived_ = 0;
	numBytesSent_ = 0;
	numBytesReceived_ = 0;
	numPacketsResent_ = 0;
	numReliablePacketsSent_ = 0;

	acksToSend_.clear();

	// Increment the version, since we're not going to be talking to the same
	// channel on the other side anymore.
	if (this->isIndexed())
	{
		version_ = seqMask( version_ + 1 );
		creationVersion_ = version_;
	}

	// Clear this channel from any monitoring collections.
	pNetworkInterface_->irregularChannels().delIfNecessary( *this );
	pNetworkInterface_->keepAliveChannels().delIfNecessary( *this );

	// not sure about cancelling the inactivity timer... but it is not
	// expected to be used on the channels we are resetting, and it is
	// the right thing to do anyway as inactivity is to be expected
	// (or is it? channel can't really stay around for too long in a
	// half-created state... which reset doesn't do anyway -
	// it just resets .... hmmm)
	inactivityTimerHandle_.cancel();

	pNetworkInterface_->cancelRequestsFor( this );

	// This will make sure that the channel is deregistered.
	this->addr( Address::NONE );

	// Note: id_ is left unchanged
}


/**
 *	This method schedules this channel for deletion once all of its packets have
 *	been acked.
 */
void Channel::condemn()
{
	if (this->isCondemned())
	{
		WARNING_MSG( "Channel::condemn( %s ): Already condemned.\n",
			   this->c_str() );
		return;
	}

	// Send any unsent traffic that may have accumulated here.
	if (this->hasUnsentData())
	{
		if (this->isEstablished())
		{
			this->send();
		}
		else
		{
			WARNING_MSG( "Channel::condemn( %s ): "
				"Unsent data was lost because channel not established\n",
				this->c_str() );
		}
	}

	// Since you aren't going to be actively sending on this channel anymore, it
	// must be marked as irregular.
	this->isLocalRegular( false );
	this->isRemoteRegular( false );

	isCondemned_ = true;

	// Note: This call may delete this channel.
	pNetworkInterface_->condemnedChannels().add( this );
}


/**
 *	This method "destroys" this channel. It should be considered similar to
 *	delete pChannel except that there may be other references remaining.
 */
void Channel::destroy()
{
	IF_NOT_MF_ASSERT_DEV( !isDestroyed_ )
	{
		return;
	}

	isDestroyed_ = true;

	this->decRef();
}


/**
 *	This method checks for overflow errors, checking how large the overflow has
 *	become. It warns if the overflow is starting to get large, and assert if it
 *	has exceeded MAX_OVERFLOW_PACKETS.
 */
void Channel::checkOverflowErrors()
{
	const uint maxOverflowPackets = this->getMaxOverflowPackets();
	const SeqNum numOverflowPackets =
		seqMask( largeOutSeqAt_ - smallOutSeqAt_ );

	if (maxOverflowPackets != 0)
	{
		// Only assert if we're explicitly told to
		MF_ASSERT( !s_assertOnMaxOverflowPackets ||
					(numOverflowPackets < maxOverflowPackets) );

		// Warn if the overflow size has grown to 1/2 of the MAX size
		if (numOverflowPackets > (maxOverflowPackets / 2))
		{
			if (!hasSeenOverflowWarning_)
			{
				WARNING_MSG( "Channel::checkOverflowErrors(%s): "
						"Overflow packet list size (%u) exceeding "
						"safety threshold (%u).\n",
					this->c_str(), numOverflowPackets, 
					(maxOverflowPackets / 2) );

				hasSeenOverflowWarning_ = true;
			}
		}
		else if (hasSeenOverflowWarning_)
		{
			if (numOverflowPackets < (maxOverflowPackets / 3))
			{
				hasSeenOverflowWarning_ = false;
			}
		}
	}
}


/**
 *	This method reconstructs this channel from streamed data. It is used for
 *	streaming the entity channel when the real cell entity is offloaded.
 *
 *	This assumes that this object was constructed with the same arguments as the
 *	source channel.
 */
void Channel::initFromStream( BinaryIStream & data,
	   const Address & addr )
{
	uint64 timeNow = timestamp();
	lastReceivedTime_ = timeNow;
	addr_ = addr;

	data >> version_;
	data >> smallOutSeqAt_;
	data >> largeOutSeqAt_;
	data >> oldestUnackedSeq_;

	uint32 count = (oldestUnackedSeq_ == SEQ_NULL) ?
					0 : seqMask( largeOutSeqAt_ - oldestUnackedSeq_ );

	highestAck_ = (oldestUnackedSeq_ != SEQ_NULL) ?
		seqMask( oldestUnackedSeq_ - 1 ) : seqMask( smallOutSeqAt_ - 1 );

	unackedPackets_.inflateToAtLeast( count );

	// This loop destreams the unacked sends (i.e. fills unackedPackets_).
	for (uint32 i = 0; i < count; ++i)
	{
		MF_ASSERT( i < unackedPackets_.size() );

		SeqNum currSeq = seqMask( oldestUnackedSeq_ + i );

		UnackedPacket * pUnacked = UnackedPacket::initFromStream( data, timeNow );

		if (pUnacked)
		{
			unackedPackets_[ currSeq ] = pUnacked;
		}
		else
		{
			// Each time we hit a slot that has been acked, it is the new highestAck_.
			highestAck_ = currSeq;
		}
	}

	// Start debugging
	uint32			highestAck;

	data >> highestAck;

	MF_ASSERT( highestAck == highestAck_ );
	// End debugging

	lastReliableSendTime_ = timeNow;
	lastReliableResendTime_ = timeNow;

	roundTripTime_ = minInactivityResendDelay_ / 2;

	// Now we destream the buffered receives.
	data >> inSeqAt_;
	data >> numBufferedReceives_;
	int numToReceive = numBufferedReceives_;

	uint32 receiveWindowSize;
	data >> receiveWindowSize;
	bufferedReceives_.inflateToAtLeast( receiveWindowSize );

	for (uint32 i = 1; numToReceive > 0; ++i)
	{
		PacketPtr pPacket =
			Packet::createFromStream( data, Packet::BUFFERED_RECEIVE );

		bufferedReceives_[ inSeqAt_ + i ] = pPacket;

		if (pPacket)
		{
			--numToReceive;
		}
	}

	pFragments_ = FragmentedBundle::createFromStream( data );

	data >> unackedCriticalSeq_ >> wantsFirstPacket_;

	// If this channel is irregular, make sure its resends will be tracked.
	// Without this, no resends will happen until the next time this channel
	// sends.
	pNetworkInterface_->irregularChannels().addIfNecessary( *this );

	data >> numPacketsSent_;
	data >> numPacketsReceived_;
	data >> numBytesSent_;
	data >> numBytesReceived_;
	data >> numPacketsResent_;
}


/**
 *  This method writes this channel's state to the provided stream so that it
 *  can be reconstructed with initFromStream().
 */
void Channel::addToStream( BinaryOStream & data )
{
	// Avoid having to stream this with the channel.
	if (this->hasUnsentData())
	{
		this->send();
	}

	// Increment version number for peer
	data << seqMask( version_ + 1 );

	data << smallOutSeqAt_;
	data << largeOutSeqAt_;
	data << oldestUnackedSeq_;

	uint32 count = this->sendWindowUsage();

	MF_ASSERT( (count == 0) || unackedPackets_[ oldestUnackedSeq_ ] );

	if (oldestUnackedSeq_ != SEQ_NULL)
	{
		for (uint32 i = oldestUnackedSeq_; seqLessThan(i, largeOutSeqAt_);
			 i = seqMask(i + 1))
		{
			UnackedPacket::addToStream( unackedPackets_[ i ], data );
		}
	}

	data << highestAck_;

	data << inSeqAt_;

	data << numBufferedReceives_;
	int numToSend = numBufferedReceives_;

	data << uint32( bufferedReceives_.size() );

	for (uint32 i = 1; numToSend > 0; ++i)
	{
		MF_ASSERT( i <= bufferedReceives_.size() );

		const Packet * pPacket = bufferedReceives_[ inSeqAt_ + i ].getObject();
		Packet::addToStream( data, pPacket, Packet::BUFFERED_RECEIVE );

		if (pPacket)
		{
			--numToSend;
		}
	}

	FragmentedBundle::addToStream( pFragments_, data );

	data << unackedCriticalSeq_ << wantsFirstPacket_;

	MF_ASSERT( !hasRemoteFailed_ );

	data << numPacketsSent_;
	data << numPacketsReceived_;
	data << numBytesSent_;
	data << numBytesReceived_;
	data << numPacketsResent_;
}


/**
 *	This method returns the bundle associated with this channel.
 */
Bundle & Channel::bundle()
{
	return *pBundle_;
}


/**
 *	This method returns the bundle associated with this channel.
 */
const Bundle & Channel::bundle() const
{
	return *pBundle_;
}


/**
 *  This method returns true if this channel's bundle has any unsent data on it,
 *  excluding messages that may have been put there by the BundlePrimer.
 */
bool Channel::hasUnsentData() const
{
	// Unreliable messages written by the bundle primer are not counted here.
	const int primeMessages =
		pBundlePrimer_ ? pBundlePrimer_->numUnreliableMessages() : 0;

	return pBundle_->numMessages() > primeMessages ||
		pBundle_->hasDataFooters() ||
		pBundle_->isReliable() || !acksToSend_.empty();
}


/**
 *	This method sends a bundle on this channel and resends unacked packets as
 *	necessary.  By default it sends the channel's own bundle, however it can
 *	also send a bundle passed in from the outside.
 */
void Channel::send( Bundle * pBundle )
{
	if (this->isDestroyed())
	{
		ERROR_MSG( "Channel::send( %s ): Channel is destroyed.", this->c_str() );
		// TODO: Should we return here?
	}

	// Don't do anything if the remote process has failed
	if (hasRemoteFailed_)
	{
		WARNING_MSG( "Channel::send( %s ): "
			"Not doing anything due to remote process failure\n",
			this->c_str() );

		return;
	}

	bool isSendingOwnBundle = (pBundle == NULL);

	// If we are not sending the channel's bundle, then we basically want to
	// make sure that the bundle is modified the same way the channel's own
	// bundle is in clearBundle().
	if (!isSendingOwnBundle)
	{
		// If for some reason we start sending external bundles on indexed
		// channels, it's probably OK to just enable the flag here instead of
		// asserting.  Can't see why we would need to interleave bundles on an
		// indexed channel like that though. Unless we are just pushing acks
		// which will use an empty bundle.
		MF_ASSERT( !this->isIndexed()  || pBundle->numMessages() == 0 );

		// If this channel uses a bundle primer, then the external bundle won't have
		// been set up correctly.  We don't support sending external bundles on
		// channels with bundle primers yet. Unless we are just pushing acks.
		MF_ASSERT( !pBundlePrimer_  || pBundle->numMessages() == 0 );
	}
	else
	{
		pBundle = pBundle_;
	}

	IF_NOT_MF_ASSERT_DEV( addr_.ip != 0 )
	{
		WARNING_MSG( "Channel::send(%s): "
				"no address to send to, dropping bundle\n",
			this->c_str() );
		pBundle->cancelRequests();

		// Clear the bundle
		if (isSendingOwnBundle)
		{
			this->clearBundle();
		}
		else
		{
			pBundle->clear();
		}

		return;
	}

	// All internal traffic must be marked as reliable by the startMessage calls.
	MF_ASSERT( this->isExternal() ||
		pBundle->numMessages() == 0 ||
		pBundle->isReliable() );

	this->checkResendTimers();

	// If we're sending the channel's bundle and it's empty, just don't do it.
	// It's important to do this after the call to checkResendTimers() so that
	// channels that are marked as regular but don't have any actual data to
	// send will still check their resends when they call this method.
	if (isSendingOwnBundle && !this->hasUnsentData())
	{
		return;
	}

	// Enable artificial loss if required.
	if (shouldDropNextSend_)
	{
		pNetworkInterface_->dropNextSend();
		shouldDropNextSend_ = false;
	}

	// pNetworkInterface_->nub().send( addr_, *pBundle, this );
	pBundle->send( addr_, *pNetworkInterface_, this );

	// Update our stats
	++numPacketsSent_;
	numBytesSent_ += pBundle->size();

	if (pBundle->isReliable())
	{
		++numReliablePacketsSent_;
	}

	// Channels that do not send regularly are added to a collection to do their
	// resend checking periodically.
	pNetworkInterface_->irregularChannels().addIfNecessary( *this );

	// If the bundle that was just sent was critical, the sequence number of its
	// last packet is the new unackedCriticalSeq_.
	if (pBundle->isCritical())
	{
		unackedCriticalSeq_ =
			pBundle->firstPacket_->seq() + pBundle->sizeInPackets() - 1;
	}

	// Clear the bundle
	if (isSendingOwnBundle)
	{
		this->clearBundle();
	}
	else
	{
		pBundle->clear();
	}
}


/**
 *	This method schedules this channel to send at the next available sending opportunity.
 */
void Channel::delayedSend()
{
	if (!this->isLocalRegular())
	{
		this->networkInterface().delayedSend( *this );
	}
}


/**
 *	This method calls send on this channel if it has not sent for a while and
 *	is getting close to causing resends.
 */
void Channel::sendIfIdle()
{
	if (this->isEstablished())
	{
		if (this->lastReliableSendOrResendTime() <
				::timestamp() - minInactivityResendDelay_/2)
		{
			this->send();
		}
	}
}


/**
 *	This method records a packet that may need to be resent later if it is not
 *	acknowledged. It is called when a packet is sent on our behalf.
 *
 *	@return false if the window size was exceeded.
 */
bool Channel::addResendTimer( SeqNum seq, Packet * p,
		const ReliableOrder * roBeg, const ReliableOrder * roEnd )
{
	MF_ASSERT( (oldestUnackedSeq_ == SEQ_NULL) ||
			unackedPackets_[ oldestUnackedSeq_ ] );
	MF_ASSERT( seq == p->seq() );

	UnackedPacket * pUnackedPacket = new UnackedPacket( p );

	// If this channel has no unacked packets, record this as the oldest.
	if (oldestUnackedSeq_ == SEQ_NULL)
	{
		oldestUnackedSeq_ = seq;
	}

	// Fill it in
	pUnackedPacket->lastSentAtOutSeq_ = seq;

	uint64 now = ::timestamp();
	pUnackedPacket->lastSentTime_ = now;
	lastReliableSendTime_ = now;

	pUnackedPacket->wasResent_ = false;

	if (roBeg != roEnd)
	{
		pUnackedPacket->reliableOrders_.assign( roBeg, roEnd );
	}

	// Grow the unackedPackets_ array, if necessary.
	if (seqMask( seq - oldestUnackedSeq_ + 1 ) > unackedPackets_.size())
	{
		unackedPackets_.doubleSize( oldestUnackedSeq_ );
		INFO_MSG( "Channel::addResendTimer( %s ): "
				"Doubled send buffer size to %u\n",
			this->c_str(),
			unackedPackets_.size() );
	}

	MF_ASSERT( unackedPackets_[ seq ] == NULL );
	unackedPackets_[ seq ] = pUnackedPacket;

	MF_ASSERT( (oldestUnackedSeq_ == SEQ_NULL) ||
			unackedPackets_[ oldestUnackedSeq_ ] );

	if (seqMask( largeOutSeqAt_ - oldestUnackedSeq_ ) >= windowSize_)
	{
		// Make sure that we at least send occasionally.
		UnackedPacket * pPrevUnackedPacket =
			unackedPackets_[ seqMask( smallOutSeqAt_ - 1 ) ];

		if ((pPrevUnackedPacket == NULL) ||
			(pPrevUnackedPacket->lastSentTime_ + minInactivityResendDelay_ <
				 now))
		{
			this->sendUnacked( *unackedPackets_[ smallOutSeqAt_ ] );
			smallOutSeqAt_ = seqMask( smallOutSeqAt_ + 1 );
		}

		this->checkOverflowErrors();
		//We shouldn't send now
		return false;
	}
	else
	{
		//We should send now
		smallOutSeqAt_ = largeOutSeqAt_;
		return true;
	}
}


/**
 *	This method handles a cumulative ACK. This indicates that all packets
 *	BEFORE a sequence number have been received by the remote end.
 *
 *  @return False on error, true otherwise.
 */
bool Channel::handleCumulativeAck( SeqNum endSeq )
{
	// Make sure the sequence number is valid
	if (seqMask( endSeq ) != endSeq)
	{
		ERROR_MSG( "Channel::handleCumulativeAck( %s ): "
			"Got out-of-range endSeq #%u (outseq: #%u)\n",
			this->c_str(), endSeq, smallOutSeqAt_ );

		return false;
	}

	if (!this->hasUnackedPackets())
	{
		return true;
	}

	// Check that the ACK is not in the future.
	// Note: endSeq is first seqNum after what's been received.
	if (Channel::seqLessThan( smallOutSeqAt_, endSeq ))
	{
		if (this->isExternal())
		{
			ERROR_MSG( "Channel::handleCumulativeAck( %s ): "
						"Received ACK for packets not yet sent. "
						"endSeq: #%u outSeq: #%u\n",
					this->c_str(), endSeq, smallOutSeqAt_ );
		}
		else
		{
			CRITICAL_MSG( "Channel::handleCumulativeAck( %s ): "
						"Received ACK for packets not yet sent. "
						"endSeq: #%u outSeq: #%u\n",
					this->c_str(), endSeq, smallOutSeqAt_ );
		}

		return false;
	}

	SeqNum seq = oldestUnackedSeq_;

	// Note: Up to but not including endSeq
	while (Channel::seqLessThan( seq, endSeq ))
	{
		this->handleAck( seq );
		seq = seqMask( seq + 1 );
	}

	return true;
}


/**
 *	This method removes a packet from the collection of packets that have been
 *	sent but not acknowledged. It is called when an acknowledgement to a packet
 *	on this channel is received.
 *
 *  Returns false on error, true otherwise.
 */
bool Channel::handleAck( SeqNum seq )
{
	MF_ASSERT( (oldestUnackedSeq_ == SEQ_NULL) ||
			unackedPackets_[ oldestUnackedSeq_ ] );

	// Make sure the sequence number is valid
	if (seqMask( seq ) != seq)
	{
		ERROR_MSG( "Channel::handleAck( %s ): "
			"Got out-of-range seq #%u (outseq: #%u)\n",
			this->c_str(), seq, smallOutSeqAt_ );

		return false;
	}

	if (!this->isInSentWindow( seq ))
	{
		return true;
	}

	// now make sure there's actually a packet there
	UnackedPacket * pUnackedPacket = unackedPackets_[ seq ];
	if (pUnackedPacket == NULL)
	{
		return true;
	}

	// Update the average RTT for this channel, if this packet hadn't already
	// been resent.
	if (!pUnackedPacket->wasResent_)
	{
		const uint64 RTT_AVERAGE_DENOM = 10;

		roundTripTime_ = ((roundTripTime_ * (RTT_AVERAGE_DENOM - 1)) +
			(timestamp() - pUnackedPacket->lastSentTime_)) / RTT_AVERAGE_DENOM;
	}

	// If this packet was the critical one, we're no longer in a critical state!
	if (unackedCriticalSeq_ == seq)
	{
		unackedCriticalSeq_ = SEQ_NULL;
	}

	// If we released the oldest unacked packet, figure out the new one
	if (seq == oldestUnackedSeq_)
	{
		oldestUnackedSeq_ = SEQ_NULL;
		for (uint i = seqMask( seq+1 );
			 i != largeOutSeqAt_;
			 i = seqMask( i+1 ))
		{
			if (unackedPackets_[ i ])
			{
				oldestUnackedSeq_ = i;
				break;
			}
		}
	}

	// If the incoming seq is after the last ack, then it is the new last ack
	if (seqLessThan( highestAck_, seq ))
	{
		highestAck_ = seq;
	}

	// Now we can release the unacked packet
	delete pUnackedPacket;
	unackedPackets_[ seq ] = NULL;

	MF_ASSERT( oldestUnackedSeq_ == SEQ_NULL ||
			unackedPackets_[ oldestUnackedSeq_ ] );

	while (seqMask(smallOutSeqAt_ - oldestUnackedSeq_) < windowSize_ &&
		   unackedPackets_[ smallOutSeqAt_ ])
	{
		this->sendUnacked( *unackedPackets_[ smallOutSeqAt_ ] );
		smallOutSeqAt_ = seqMask( smallOutSeqAt_ + 1 );
	}

	return true;
}


/**
 *	This method resends any unacked packets as appropriate. This can be because
 *	of time since last sent, receiving later acks before earlier ones.
 */
void Channel::checkResendTimers()
{
	// There are no un-acked packets
	if (oldestUnackedSeq_ == SEQ_NULL)
	{
		return;
	}

	// Don't do anything if the remote process has failed
	if (hasRemoteFailed_)
	{
		WARNING_MSG( "Channel::checkResendTimers( %s ): "
			"Not doing anything due to remote process failure\n",
			this->c_str() );

		return;
	}

	// If we have unacked packets that are getting a bit old, then resend the
	// ones that are older than we'd like.  Anything that has taken more than
	// twice the RTT on the channel to come back is considered to be too old.
	uint64 now = timestamp();
	uint64 thresh = now - std::max( roundTripTime_*2, minInactivityResendDelay_ );
	uint64 lastReliableSendTime = this->lastReliableSendOrResendTime();

	const bool isIrregular = !this->isRemoteRegular();
	const SeqNum endSeq = isIrregular ? smallOutSeqAt_ : highestAck_;

	int numResends = 0;

	// TODO: 8 is a magic number and would be nice to be more scientific.
	// The idea is to throttle the resends a little in extreme situations. We
	// want to send enough so that no (or not too many) packets are lost but
	// still be able to send more when the RTT is large.
	const int MAX_RESENDS = windowSize_/8;

	for (SeqNum seq = oldestUnackedSeq_;
		seqLessThan( seq, endSeq ) && numResends < MAX_RESENDS;
		seq = seqMask( seq + 1 ))
	{
		UnackedPacket * pUnacked = unackedPackets_[ seq ];

		// Send if the packet is old, or we have a later ack
		if (pUnacked != NULL)
		{
			const bool hasNewerAck =
				 seqLessThan( pUnacked->lastSentAtOutSeq_, highestAck_);

			const bool shouldResend = hasNewerAck ||
				(isIrregular && (pUnacked->lastSentTime_ < thresh));

			const SeqNum prevLastSentAtOutSeq = pUnacked->lastSentAtOutSeq_;
			const uint64 prevLastSentTime = pUnacked->lastSentTime_;

			if (shouldResend)
			{
				this->resend( seq );
				++numResends;
			}

			if (shouldResend && this->networkInterface().isVerbose())
			{
				if (hasNewerAck)
				{
					WARNING_MSG( "Channel::checkResendTimers( %s ): "
							"Resent unacked packet #%u due to newer ack "
							"(Last sent #%u, Latest ack #%u)\n",
						this->c_str(),
						pUnacked->pPacket_->seq(),
						prevLastSentAtOutSeq,
						highestAck_ );
					// pUnacked->pPacket_->debugDump();
				}
				else
				{
					WARNING_MSG( "Channel::checkResendTimers( %s ): "
							"Resent unacked packet #%u due to inactivity "
							"(Packet %.3fs, Channel %.3fs, RTT %.3fs)\n",
						this->c_str(),
						pUnacked->pPacket_->seq(),
						(now - prevLastSentTime) / stampsPerSecondD(),
						(now - lastReliableSendTime) / stampsPerSecondD(),
						roundTripTime_ / stampsPerSecondD() );
					// pUnacked->pPacket_->debugDump();
				}
			}
		}
	}
}


/**
 *  Resends an un-acked packet by the most sensible method available.
 *
 *  @return Returns true if the packet was piggybacked and not actually sent.
 */
void Channel::resend( SeqNum seq )
{
	++numPacketsResent_;

	UnackedPacket & unacked = *unackedPackets_[ seq ];

#ifdef USE_PIGGIES
	// If possible, piggypack this packet onto the next outgoing bundle
	if (this->isExternal() &&
		!unacked.pPacket_->hasFlags( Packet::FLAG_IS_FRAGMENT ) &&
		(unackedPackets_[ smallOutSeqAt_ ] == NULL)) // Not going to overflow
	{
		if (this->bundle().piggyback(
				seq, unacked.reliableOrders_, unacked.pPacket_.getObject() ))
		{
			this->handleAck( seq );
			return;
		}
	}
#endif

	// Otherwise just send as normal.
	if (this->isInternal())
	{
		/*WARNING_MSG( "Channel::resend( %s ): Resending #%d (outSeqAt #%d)\n",
		  this->c_str(), seq, smallOutSeqAt_ );*/
	}

	// If there are any acks on this packet, then they will be resent too, but
	// it does no harm.
	this->sendUnacked( unacked );
}


/**
 *  Resends an un-acked packet by the most sensible method available.
 */
void Channel::sendUnacked( UnackedPacket & unacked )
{
	unacked.pPacket_->updateChannelVersion( version_, id_ );

	pNetworkInterface_->sendPacket( addr_, unacked.pPacket_.get(), this, true );

	unacked.lastSentAtOutSeq_ = smallOutSeqAt_;
	unacked.wasResent_ = true;

	uint64 now = timestamp();
	unacked.lastSentTime_ = now;
	lastReliableResendTime_ = now;
}


/**
 *	This method is called when a packet is received. It is responsible for
 *	adding the packet to the receive window and queueing an ACK to the next
 *	outgoing bundle on this channel.
 */
Channel::AddToReceiveWindowResult Channel::addToReceiveWindow( Packet * p,
	const Address & srcAddr, PacketReceiverStats & stats )
{
	const SeqNum seq = p->seq();

	// Make sure the sequence number is valid
	if (seqMask( seq ) != seq)
	{
		ERROR_MSG( "Channel::addToReceiveWindow( %s ): "
			"Got out-of-range incoming seq #%u (inSeqAt: #%u)\n",
			this->c_str(), seq, inSeqAt_ );

		return PACKET_IS_CORRUPT;
	}

	if (shouldAutoSwitchToSrcAddr_)
	{
		// We switch address if the version number is acceptable. We switch on
		// equal version numbers because the first packet from a cell entity
		// sets the address and is version 0.

		if (!seqLessThan( p->channelVersion(), version_ ))
		{
			version_ = p->channelVersion();
			this->addr( srcAddr );
		}
	}
	else if (addr_ != srcAddr)
	{
		ERROR_MSG( "Channel::addToReceiveWindow( %s ): "
				"Got packet #%u from wrong source address: %s\n",
			this->c_str(), seq, srcAddr.c_str() );

		return PACKET_IS_CORRUPT;
	}

	if (!p->isPiggyback())
	{
		// No need to ACK piggybacks as they are implicitly ACKed by the 
		// containing packet's ACK.

		// Always add the ACKs for others even though a cumulative ack may mean
		// it is not sent. This still shows that the channel is not empty.
		acksToSend_.insert( seq );
	}

	// Push the outgoing bundle immediately if required
	if (pushUnsentAcksThreshold_ &&
		(acksToSend_.size() >= pushUnsentAcksThreshold_))
	{
		if (this->networkInterface().isVerbose())
		{
			DEBUG_MSG( "Channel::addToReceiveWindow( %s ): "
					"Pushing %"PRIzu" unsent ACKs due to inactivity\n",
				this->c_str(), acksToSend_.size());
		}

		this->send();
	}

	// check the good case first
	if (seq == inSeqAt_)
	{
		inSeqAt_ = seqMask( inSeqAt_ + 1 );

		Packet * pPrev = p;
		Packet * pBufferedPacket = bufferedReceives_[ inSeqAt_ ].getObject();

		// Attach as many buffered packets as possible to this one.
		while (pBufferedPacket != NULL)
		{
			// Link it to the prev packet then remove it from the buffer.
			pPrev->chain( pBufferedPacket );
			bufferedReceives_[ inSeqAt_ ] = NULL;
			--numBufferedReceives_;

			// Advance to the next buffered packet.
			pPrev = pBufferedPacket;
			inSeqAt_ = seqMask( inSeqAt_ + 1 );
			pBufferedPacket = bufferedReceives_[ inSeqAt_ ].getObject();
		}

		return SHOULD_PROCESS;
	}

	// see if we've got this one before. We have if seq < inSeqAt_.
	if (seqLessThan( seq, inSeqAt_ ))
	{
		if (this->networkInterface().isVerbose())
		{
			DEBUG_MSG( "Channel::addToReceiveWindow( %s ): "
					"Discarding already-seen packet #%u below inSeqAt #%u\n",
				this->c_str(), seq, inSeqAt_ );
		}

		stats.incDuplicatePackets();

		return SHOULD_NOT_PROCESS;
	}

	// make sure it's in range
	uint32 requiredWindowSize = seqMask(seq - inSeqAt_);

	// Note that we store packets between
	//  [inSeqAt_ + 1, inSeqAt_ + bufferedReceives_.size()] 
	// in bufferedReceives. This is why we have > instead of >= in the below
	// conditions.

	if ((requiredWindowSize > 2 * bufferedReceives_.size()) ||
			(requiredWindowSize > this->maxWindowSize()))
	{
		if (this->networkInterface().isVerbose())
		{
			WARNING_MSG( "Channel::addToReceiveWindow( %s ): "
					"Sequence number #%u is way out of window #%u!\n",
				this->c_str(), seq, inSeqAt_ );
		}

		return SHOULD_NOT_PROCESS;
	}
	else if (requiredWindowSize > bufferedReceives_.size())
	{
		// Copy buffered receives from the start of our buffered receives,
		// the first starting at inSeqAt_ + 1.
		bufferedReceives_.doubleSize( inSeqAt_ + 1 );
		INFO_MSG( "Channel::addToReceiveWindow( %s ): "
				"Doubled receive buffer size to %u\n",
			this->c_str(),
			bufferedReceives_.size() );
	}

	// ok - we'll buffer this packet then....
	PacketPtr & rpBufferedPacket = bufferedReceives_[ seq ];

	// ... but only if we don't already have it
	if (rpBufferedPacket != NULL)
	{
		if (rpBufferedPacket->seq() == seq)
		{

			if (this->networkInterface().isVerbose())
			{
				DEBUG_MSG( "Channel::addToReceiveWindow( %s ): "
						"Discarding already-buffered packet #%u\n",
					this->c_str(), seq );
			}
			stats.incDuplicatePackets();
		}
		else
		{
			CRITICAL_MSG( "Channel::addToReceiveWindow( %s ): "
					"Packet %u in %u's buffered receive slot\n",
				this->c_str(), rpBufferedPacket->seq(), seq );
		}
	}
	else
	{
		rpBufferedPacket = p;
		++numBufferedReceives_;

		if (this->networkInterface().isVerbose())
		{
			DEBUG_MSG( "Channel::addToReceiveWindow( %s ): "
					"Buffering packet #%u above #%u\n",
				this->c_str(), seq, inSeqAt_ );
		}
	}

	return SHOULD_NOT_PROCESS;
}


/**
 *  This method sets the anonymous state for this channel.
 */
void Channel::isAnonymous( bool anonymous )
{
	isAnonymous_ = anonymous;
	KeepAliveChannels & channels = pNetworkInterface_->keepAliveChannels();

	// Anonymity means we need keepalive checking (and vice versa).
	if (isAnonymous_)
	{
		channels.addIfNecessary( *this );
	}
	else
	{
		channels.delIfNecessary( *this );
	}

	// Anonymity means irregularity too.
	if (isAnonymous_)
	{
		this->isLocalRegular( false );
		this->isRemoteRegular( false );
	}
}


/**
 *  This method resends all unacked packets on this Channel, up to and including
 *  the critical packet with the highest sequence number.
 */
void Channel::resendCriticals()
{
	if (unackedCriticalSeq_ == SEQ_NULL)
	{
		WARNING_MSG( "Channel::resendCriticals( %s ): "
			"Called with no unacked criticals!\n",
			this->c_str() );

		return;
	}

	// Resend all unacked sends up to the highest critical.
	for (SeqNum seq = oldestUnackedSeq_;
		 seq != seqMask( unackedCriticalSeq_ + 1 );
		 seq = seqMask( seq + 1 ))
	{
		if (unackedPackets_[ seq ])
		{
			this->resend( seq );
		}
	}
}


/**
 *  This method configures this channel to auto switch its address to the
 *  source address of incoming packets.  Enabling this is only allowed for
 *  indexed channels.
 */
void Channel::shouldAutoSwitchToSrcAddr( bool b )
{
	shouldAutoSwitchToSrcAddr_ = b;
	MF_ASSERT( !shouldAutoSwitchToSrcAddr_ || this->isIndexed() );
}


/**
 *	This method returns a string representation of this channel which is useful
 *	in output messages.
 *
 *	Note: a static string is returned so this cannot be called twice in
 *	succession.
 */
const char * Channel::c_str() const
{
	static char dodgyString[ 40 ];

	int length = addr_.writeToString( dodgyString, sizeof( dodgyString ) );

	if (this->isIndexed())
	{
		length += bw_snprintf( dodgyString + length,
			sizeof( dodgyString ) - length,	"/%d", id_ );
	}

	// Annotate condemned channels with an exclamation mark.
	if (isCondemned_)
	{
		length += bw_snprintf( dodgyString + length,
			sizeof( dodgyString ) - length,	"!" );
	}

	return dodgyString;
}


/**
 *  This method clears the bundle on this channel and gets it ready to have a
 *  new set of messages added to it.
 */
void Channel::clearBundle()
{
	if (!pBundle_)
	{
		pBundle_ = new Bundle( pFilter_ ? pFilter_->maxSpareSize() : 0, this );
	}
	else
	{
		pBundle_->clear();
	}

	// If we have a bundle primer, now's the time to call it!
	if (pBundlePrimer_)
	{
		pBundlePrimer_->primeBundle( *pBundle_ );
	}
}


/**
 *  This method sets the BundlePrimer object for this channel.  If the channel's
 *  bundle is empty, it will be primed.
 */
void Channel::bundlePrimer( BundlePrimer & primer )
{
	pBundlePrimer_ = &primer;

	if (pBundle_->numMessages() == 0)
	{
		primer.primeBundle( *pBundle_ );
	}
}


/**
 *  This method will write the flags on a packet fitting for one that will ride
 *  on this channel. It will also reserve enough space for the footer.
 */
void Channel::writeFlags( Packet * p )
{
	p->enableFlags( Packet::FLAG_ON_CHANNEL );

	if (this->isIndexed())
	{
		p->enableFlags( Packet::FLAG_INDEXED_CHANNEL );
		p->channelID() = id_;
		p->channelVersion() = version_;
		p->reserveFooter( sizeof( ChannelID ) + sizeof( ChannelVersion ) );
	}

	// Add a cumulative ACK. This indicates that all packets BEFORE a given seq
	// have been received.
	if (p->freeSpace() >= int( sizeof( SeqNum )))
	{
		p->enableFlags( Packet::FLAG_HAS_CUMULATIVE_ACK );
		p->reserveFooter( sizeof( SeqNum ) );

		Acks::iterator iter = acksToSend_.begin();

		while (iter != acksToSend_.end())
		{
			// Need to go through all due to wrap-around case.
			if (Channel::seqLessThan( *iter, inSeqAt_ ) )
			{
				acksToSend_.erase( iter++ );
			}
			else
			{
				++iter;
			}
		}
	}

	// Put on as many acks as we can.
	if (!acksToSend_.empty() &&
		p->freeSpace() >= int(sizeof( Packet::AckCount ) + sizeof( SeqNum )))
	{
		//Required to make GCC link this, something to do with templates
		const size_t MAX_ACKS = Packet::MAX_ACKS;
		p->enableFlags( Packet::FLAG_HAS_ACKS );
		p->reserveFooter( sizeof( Packet::AckCount ) );
		p->nAcks() = std::min( std::min( p->freeSpace() / sizeof( SeqNum ),
										 acksToSend_.size() ), 
							   MAX_ACKS );
		p->reserveFooter( sizeof( SeqNum ) * p->nAcks() );
	}

	// If this is the first reliable outbound packet, flag it.
	if (p->hasFlags(Packet::FLAG_IS_RELIABLE) && this->isInternal() && 
		largeOutSeqAt_ == 0 && numReliablePacketsSent_ == 0)
	{
		p->enableFlags( Packet::FLAG_CREATE_CHANNEL );
	}
}


/**
 *  This method will write the appropriate flags on a packet to indicate that
 *  it is on this channel. It must be called after writeFlags.
 */
void Channel::writeFooter( Packet * p )
{
	if (p->hasFlags( Packet::FLAG_INDEXED_CHANNEL ))
	{
		p->packFooter( p->channelID() );
		p->packFooter( p->channelVersion() );
	}

	if (p->hasFlags( Packet::FLAG_HAS_CUMULATIVE_ACK ))
	{
		p->packFooter( inSeqAt_ );
	}

	if (p->hasFlags( Packet::FLAG_HAS_ACKS ))
	{
		// Note: Technically we should start at inSeqAt_ since sequence numbers
		// wrap around but this is rare enough not to worry about (since it
		// still works but is less efficient).
		p->packFooter( (Packet::AckCount)p->nAcks() );
		uint acksAdded = 0;
		while (!acksToSend_.empty() && acksAdded < p->nAcks())
		{
			p->packFooter( *acksToSend_.begin() );
			acksToSend_.erase( acksToSend_.begin() );
			++acksAdded;
		}
	}
}


/**
 *	This method handles the channel's timer events.
 */
void Channel::handleTimeout( TimerHandle, void * arg )
{
	switch (reinterpret_cast<uintptr>( arg ))
	{
		case TIMEOUT_INACTIVITY_CHECK:
		{
			if (timestamp() - lastReceivedTime_ > inactivityExceptionPeriod_)
			{
				this->networkInterface().onChannelTimeOut( this );
			}
			break;
		}
	}
}


/**
 *	This method resets this channel to be as if it had just been constructed. It
 *	will deregister the channel (but does not clear the index).
 */
void Channel::reset( const Address & newAddr, bool warnOnDiscard )
{
	// Don't do anything if the address hasn't changed.
	if (newAddr == addr_)
	{
		return;
	}

	// Send it now if the network interface has it registered for delayed
	// sending. 
	pNetworkInterface_->sendIfDelayed( *this );

	// Clear owned objects
	this->clearState( warnOnDiscard );


	// If this channel was previously established, we will wait for a packet
	// with FLAG_CREATE_CHANNEL, since we don't want to accept any packets from
	// the old connection.
	if (this->isEstablished())
	{
		wantsFirstPacket_ = true;
	}

	// This handles registering this channel (deregistering done in
	// clearState above).
	this->addr( newAddr );

	// If we're establishing this channel, call the bundle primer, since
	// we just cleared the bundle.
	if (this->isEstablished() && pBundlePrimer_)
	{
		this->bundlePrimer( *pBundlePrimer_ );
	}
}


/**
 *  This method copies configuration settings from one channel to another.
 */
void Channel::configureFrom( const Channel & other )
{
	this->isLocalRegular( other.isLocalRegular() );
	this->isRemoteRegular( other.isRemoteRegular() );

	this->shouldAutoSwitchToSrcAddr( other.shouldAutoSwitchToSrcAddr() );
	this->pushUnsentAcksThreshold( other.pushUnsentAcksThreshold() );

	// We don't support setting this fields post-construction, so for now, just
	// make sure the channels match.
	MF_ASSERT( traits_ == other.traits_ );
	MF_ASSERT( minInactivityResendDelay_ == other.minInactivityResendDelay_ );
}


/**
 *  This method transfers this Channel to a different interface.
 */
void Channel::switchInterface( NetworkInterface * pDestInterface )
{
	pNetworkInterface_->irregularChannels().delIfNecessary( *this );
	pNetworkInterface_->keepAliveChannels().delIfNecessary( *this );

	pNetworkInterface_->deregisterChannel( *this );
	pNetworkInterface_ = pDestInterface;
	pNetworkInterface_->registerChannel( *this );

	irregularIter_ = pNetworkInterface_->irregularChannels().end();
	pNetworkInterface_->irregularChannels().addIfNecessary( *this );

	keepAliveIter_ = pNetworkInterface_->keepAliveChannels().end();
	pNetworkInterface_->keepAliveChannels().addIfNecessary( *this );
}


/**
 *	This method starts detection of inactivity on this channel. If nothing is
 *	received for the input period amount of time, an INACTIVITY error is
 *	generated.
 *
 *	@param period	The number of seconds without receiving a packet before
 *		the channel is considered inactive.
 *	@param checkPeriod The number of seconds between checking for inactivity.
 */
void Channel::startInactivityDetection( float period, float checkPeriod )
{
	inactivityTimerHandle_.cancel();

	inactivityExceptionPeriod_ = uint64( period * stampsPerSecond() );
	lastReceivedTime_ = timestamp();

	inactivityTimerHandle_ =
		this->dispatcher().addTimer( int( checkPeriod * 1000000 ),
									this, (void *)TIMEOUT_INACTIVITY_CHECK );
}


/**
 *	This method sets whether this channel sends irregularly and indicates that
 *	its resends are managed globally.
 */
void Channel::isLocalRegular( bool isLocalRegular )
{
	isLocalRegular_ = isLocalRegular;

	// Channels that do not send regularly are added to a collection to do their
	// resend checking periodically.
	pNetworkInterface_->irregularChannels().addIfNecessary( *this );
}


/**
 *	This method sets whether the remote side of this channel sends regularly.
 *	If the remote side sends regularly, resends can be done via NACKS instead
 *	of being timer based.
 */
void Channel::isRemoteRegular( bool isRemoteRegular )
{
	isRemoteRegular_ = isRemoteRegular;
}


/**
 * 	This method returns the next sequence ID, and then increments it.
 *
 * 	@return The next sequence ID.
 */
SeqNum Channel::useNextSequenceID()
{
	SeqNum	retSeq = largeOutSeqAt_;
	largeOutSeqAt_ = seqMask( largeOutSeqAt_ + 1 );

	if (this->isInternal())
	{
		int usage = this->sendWindowUsage();
		int & threshold = this->sendWindowWarnThreshold();

		if (usage > threshold)
		{
			WARNING_MSG( "Channel::useNextSequenceID( %s ): "
							"Send window backlog is now %d packets, "
							"exceeded previous max of %d, "
							"critical size is %u\n",
						this->c_str(), usage, threshold, windowSize_ );

			threshold = usage;
		}

		if (this->isIndexed() &&
				(s_pSendWindowCallback_ != NULL) &&
				(usage > s_sendWindowCallbackThreshold_))
		{
			(*s_pSendWindowCallback_)( *this );
		}
	}

	return retSeq;
}


/**
 * Validates whether the provided sequence number from an unreliable
 * packet looks to be valid. That is, is it larger that the previous
 * sequence number seen, within the window size considered valid.
 */
bool Channel::validateUnreliableSeqNum( const SeqNum seqNum )
{
	if (seqNum != seqMask( seqNum ))
	{
		WARNING_MSG( "Channel:validateUnreliableSeqNum: "
			"Invalid sequence number (%d).\n", seqNum );
		return false;
	}

	if (Channel::seqLessThan( seqNum, unreliableInSeqAt_ ) &&
			(unreliableInSeqAt_ != SEQ_NULL))
	{
		WARNING_MSG( "Channel:validateUnreliableSeqNum: Received an invalid "
			"seqNum (%d) on an unreliable channel. Last valid seqNum (%d)\n",
			seqNum, unreliableInSeqAt_ );
		return false;
	}

	// Only store the new seqNum if it has been completely validated.
	unreliableInSeqAt_ = seqNum;
	return true;
}


/**
 *	This method sets whether the remote process has failed.
 */
void Channel::hasRemoteFailed( bool v )
{
	hasRemoteFailed_ = v;

	// If this channel is anonymous, then no-one else is going to clean it up.
	if (isAnonymous_)
	{
		INFO_MSG( "Channel::hasRemoteFailed: "
				"Cleaning up dead anonymous channel to %s\n",
			this->c_str() );

		pNetworkInterface_->delAnonymousChannel( addr_ );
	}
}


/**
 *	This method is called to indicate that a packet associated with this channel
 *	has been received.
 */
void Channel::onPacketReceived( int bytes )
{
	lastReceivedTime_ = timestamp();
	++numPacketsReceived_;
	numBytesReceived_ += bytes;
}


#if ENABLE_WATCHERS
/**
 *	This static function returns a watcher that can be used to watch Channels.
 */
WatcherPtr Channel::pWatcher()
{
	static DirectoryWatcherPtr pWatcher = NULL;

	if (pWatcher == NULL)
	{
		pWatcher = new DirectoryWatcher();

#define ADD_WATCHER( PATH, MEMBER )		\
		pWatcher->addChild( #PATH, makeWatcher( &Channel::MEMBER ) );

		ADD_WATCHER( addr,				addr_ );
		ADD_WATCHER( packetsSent,		numPacketsSent_ );
		ADD_WATCHER( packetsReceived,	numPacketsReceived_ );
		ADD_WATCHER( bytesSent,			numBytesSent_ );
		ADD_WATCHER( bytesReceived,		numBytesReceived_ );
		ADD_WATCHER( packetsResent,		numPacketsResent_ );
		ADD_WATCHER( reliablePacketsResent,		numReliablePacketsSent_ );

		ADD_WATCHER( isLocalRegular,		isLocalRegular_ );
		ADD_WATCHER( isRemoteRegular,		isRemoteRegular_ );

		pWatcher->addChild( "roundTripTime",
				makeWatcher( &Channel::roundTripTimeInSeconds ) );
	}

	return pWatcher;
}
#endif


/**
 *	This static method sets the callback associated with the send window usage
 *	for an internal, indexed channel exceeding the sendWindowCallbackThreshold.
 */
void Channel::setSendWindowCallback( SendWindowCallback callback )
{
	s_pSendWindowCallback_ = callback;
}


/**
 *	This static method sets the threshold for when to call the send-window
 *	callback. If an internal, indexed channel's send-window get larger than
 *	this number of packets, the callback set in setSendWindowCallback is called.
 */
void Channel::sendWindowCallbackThreshold( float threshold )
{
	s_sendWindowCallbackThreshold_ = int( threshold * INDEXED_CHANNEL_SIZE );
}


/**
 *	This static method returns the threshold for when the send-window callback
 *	is called.
 */
float Channel::sendWindowCallbackThreshold()
{
	return float(s_sendWindowCallbackThreshold_)/INDEXED_CHANNEL_SIZE;
}


/**
 *
 */
EventDispatcher & Channel::dispatcher()
{
	return pNetworkInterface_->mainDispatcher();
}


} // namespace Mercury

// channel.cpp
