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

#include "delayed_channels.hpp"

#include "channel.hpp"
#include "event_dispatcher.hpp"

namespace Mercury
{

/**
 *	This method initialises this object.
 */
void DelayedChannels::init( EventDispatcher & dispatcher )
{
	dispatcher.addFrequentTask( this );
}


/**
 *	This method initialises this object.
 */
void DelayedChannels::fini( EventDispatcher & dispatcher )
{
	dispatcher.cancelFrequentTask( this );
}


/**
 *	This method registers a channel for delayed sending. This is used by
 *	irregular channels so that they still have an opportunity to aggregate
 *	messages. Instead of sending a packet for each method call, these are
 *	aggregated.
 */
void DelayedChannels::add( Channel & channel )
{
	channels_.insert( &channel );
}


/**
 *	This method performs a send on the given channel if that channel has been
 *	registered as a delayed channel. After sending the channel is removed from
 *	the collection of delayed channels.
 */
void DelayedChannels::sendIfDelayed( Channel & channel )
{
	if (channels_.erase( &channel ) > 0)
	{
		channel.send();
	}
}


/**
 *	This method overrides the FrequentTask method to perform the delayed
 *	sending.
 */
void DelayedChannels::doTask()
{
	Channels::iterator iter = channels_.begin();

	while (iter != channels_.end())
	{
		Channel * pChannel = iter->get();

		if (!pChannel->isDead())
		{
			pChannel->send();
		}

		++iter;
	}

	channels_.clear();
}

} // namespace Mercury

// delayed_channels.cpp
