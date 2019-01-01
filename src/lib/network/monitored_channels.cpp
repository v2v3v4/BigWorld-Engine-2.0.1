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

#include "monitored_channels.hpp"

#include "channel.hpp"
#include "network_interface.hpp"
#include "event_dispatcher.hpp"

namespace Mercury
{

// -----------------------------------------------------------------------------
// Section: MonitoredChannels
// -----------------------------------------------------------------------------

/**
 *	Destructor
 */
MonitoredChannels::~MonitoredChannels()
{
	timerHandle_.cancel();
}


/**
 *	This method sets the monitoring period for channels in this collection.
 */
void MonitoredChannels::setPeriod( float seconds, EventDispatcher & dispatcher )
{
	timerHandle_.cancel();

	if (seconds > 0.f)
	{
		timerHandle_ = dispatcher.addTimer( int( seconds * 1000000 ), this );
	}
	period_ = seconds;
}


/**
 *	This methods stops the monitoring of the channels.
 */
void MonitoredChannels::stopMonitoring( EventDispatcher & dispatcher )
{
	this->setPeriod( 0.f, dispatcher );
}


/**
 *	This method remembers this channel for checking if it isn't in this
 *	collection already.
 */
void MonitoredChannels::addIfNecessary( Channel & channel )
{
	iterator & iter = this->channelIter( channel );

	if (iter == channels_.end())
	{
		iter = channels_.insert( channels_.end(), &channel );

		if (!timerHandle_.isSet())
		{
			this->setPeriod( this->defaultPeriod(),
						channel.networkInterface().dispatcher() );
		}
	}
}


/**
 *	This method forgets this channel for resend checking, if necessary.
 */
void MonitoredChannels::delIfNecessary( Channel & channel )
{
	iterator & iter = this->channelIter( channel );

	if (iter != channels_.end())
	{
		channels_.erase( iter );
		iter = channels_.end();
	}
}

} // namespace Mercury

// monitored_channels.cpp
