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

#include "condemned_channels.hpp"

#include "channel.hpp"
#include "event_dispatcher.hpp"
#include "network_interface.hpp"

DECLARE_DEBUG_COMPONENT2( "Network", 0 )

namespace Mercury
{

// -----------------------------------------------------------------------------
// Section: CondemnedChannels
// -----------------------------------------------------------------------------

/**
 *	Destructor.
 */
CondemnedChannels::~CondemnedChannels()
{
	timerHandle_.cancel();
}


/**
 *	This method takes care of deleting the input channel. It will wait until
 *	there are no unacked packets before deleting the channel.
 */
void CondemnedChannels::add( Channel * pChannel )
{
	if (this->shouldDelete( pChannel, ::timestamp() ))
	{
		// delete pChannel;
		pChannel->destroy();
	}
	else
	{
		if (pChannel->isIndexed())
		{
			Channel *& rpChannel = indexedChannels_[ pChannel->id() ];

			if (rpChannel)
			{
				// delete rpChannel;
				rpChannel->destroy();
				WARNING_MSG( "CondemnedChannels::add( %s ): "
								"Already have a channel with id %d\n",
							pChannel->c_str(),
							pChannel->id() );
			}

			rpChannel = pChannel;
		}
		else
		{
			nonIndexedChannels_.push_back( pChannel );
		}

		if (!timerHandle_.isSet())
		{
			const int seconds = AGE_LIMIT;
			timerHandle_ = pChannel->networkInterface().dispatcher().addTimer(
							int( seconds * 1000000 ), this );
		}
	}
}


/**
 *	This method returns the indexed channel matching the input id.
 */
Channel * CondemnedChannels::find( ChannelID channelID ) const
{
	IndexedChannels::const_iterator iter =
		indexedChannels_.find( channelID );

	return (iter != indexedChannels_.end()) ? iter->second : NULL;
}


/**
 *	This method returns whether the condemned channel should be deleted.
 */
inline bool CondemnedChannels::shouldDelete( Channel * pChannel, uint64 now )
{
	const uint64 ageLimit = AGE_LIMIT * stampsPerSecond();

	bool shouldDelete =
		!pChannel->hasUnackedPackets() || pChannel->hasRemoteFailed();

	// We consider a channel to be timed out if we haven't sent or received
	// anything on it for a while.
	if (!shouldDelete &&
		(pChannel->lastReceivedTime() + ageLimit < now) &&
		(pChannel->lastReliableSendTime() + ageLimit < now))
	{
		WARNING_MSG( "CondemnedChannels::handleTimeout: "
						"Condemned channel %s has timed out.\n",
					pChannel->c_str() );
		shouldDelete = true;
	}

	return shouldDelete;
}


/**
 *	This method deletes any condemned channels that are now considered finished.
 *	This can be from having no more unacked packets or timing out
 *
 *	@return true if there are no more condemned channels, otherwise false.
 */
bool CondemnedChannels::deleteFinishedChannels()
{
	uint64 now = ::timestamp();

	// Consider non-indexed channels
	{
		NonIndexedChannels::iterator iter = nonIndexedChannels_.begin();

		while (iter != nonIndexedChannels_.end())
		{
			Channel * pChannel = *iter;
			NonIndexedChannels::iterator oldIter = iter;
			++iter;

			if (this->shouldDelete( pChannel, now ))
			{
				// delete pChannel;
				pChannel->destroy();
				nonIndexedChannels_.erase( oldIter );
			}
		}
	}

	// Consider indexed channels
	{
		IndexedChannels::iterator iter = indexedChannels_.begin();

		while (iter != indexedChannels_.end())
		{
			Channel * pChannel = iter->second;
			IndexedChannels::iterator oldIter = iter;
			++iter;

			if (this->shouldDelete( pChannel, now ))
			{
				// delete pChannel;
				pChannel->destroy();
				indexedChannels_.erase( oldIter );
			}
		}
	}

	bool isEmpty = nonIndexedChannels_.empty() && indexedChannels_.empty();

	if (isEmpty)
	{
		timerHandle_.cancel();
	}

	return isEmpty;
}


/**
 *  This method returns the number of condemned channels that are marked as
 *  'critical'.
 */
int CondemnedChannels::numCriticalChannels() const
{
	int count = 0;

	for (NonIndexedChannels::const_iterator iter = nonIndexedChannels_.begin();
		 iter != nonIndexedChannels_.end(); ++iter)
	{
		if ((*iter)->hasUnackedCriticals())
		{
			++count;
		}
	}

	for (IndexedChannels::const_iterator iter = indexedChannels_.begin();
		 iter != indexedChannels_.end(); ++iter)
	{
		if (iter->second->hasUnackedCriticals())
		{
			++count;
		}
	}

	return count;
}


/**
 *	This method handles the timer event. It checks whether any condemned channel
 *	can be deleted.
 */
void CondemnedChannels::handleTimeout( TimerHandle, void * )
{
	this->deleteFinishedChannels();
}

} // namespace Mercury

// condemned_channels.cpp
