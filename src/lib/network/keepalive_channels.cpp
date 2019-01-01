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

#include "keepalive_channels.hpp"

#include "bundle.hpp"
#include "channel.hpp"

// This value is the period at which handleTimeout() will be called, which
// is actually half the interval at which we want to send keepalives (see
// the initialisation of 'inactivityPeriod' below).
static const float KEEP_ALIVE_PING_PERIOD = 2.5f; 		// seconds

// This is the length of time where if the channel is not used, we will 
// destroy it.
static const float KEEP_ALIVE_TIMEOUT = 60.f; 	// seconds

namespace Mercury
{

/**
 *	Constructor.
 */
KeepAliveChannels::KeepAliveChannels() :
	lastTimeout_( 0 ),
	lastInvalidTimeout_( 0 )
{
}


/**
 *	This method remembers this channel for resend checking if it is irregular
 *	and is not already stored.
 */
void KeepAliveChannels::addIfNecessary( Channel & channel )
{
	// At the moment, the only channels that should be getting automatic
	// keepalive checking are anonymous channels.
	if (channel.isAnonymous())
	{
		MonitoredChannels::addIfNecessary( channel );
	}
}


KeepAliveChannels::iterator & KeepAliveChannels::channelIter(
	Channel & channel ) const
{
	return channel.keepAliveIter_;
}


/**
 *  This method returns the interval for timeouts on this object.
 */
float KeepAliveChannels::defaultPeriod() const
{
	return KEEP_ALIVE_PING_PERIOD;
}


/**
 *  This method checks for dead channels and sends keepalives as necessary.
 */
void KeepAliveChannels::handleTimeout( TimerHandle, void * )
{
	const uint64 inactivityPeriod =	uint64( 2 * period_ * ::stampsPerSecond() );
	const uint64 now = ::timestamp();
	const uint64 checkTime = now - inactivityPeriod;
	const uint64 timeoutPeriod =
		uint64( KEEP_ALIVE_TIMEOUT * ::stampsPerSecond() );
	uint64 deadTime = now - timeoutPeriod;

	// If handleTimeout has not been called for a while, do not time out other
	// channels because it is probably our fault.
	if ((lastTimeout_ != 0) && 2 * (now - lastTimeout_) > timeoutPeriod)
	{
		WARNING_MSG( "KeepAliveChannels::handleTimeout: Not called for %.2f "
						"seconds. Temporarily suspending timeouts.\n",
					(now - lastTimeout_) / ::stampsPerSecondD() );
		lastInvalidTimeout_ = now;
	}

	if (lastInvalidTimeout_ != 0)
	{
		if (lastInvalidTimeout_ > deadTime)
		{
			// Do not kill any if we paused recently
			deadTime = 0;
		}
		else
		{
			INFO_MSG( "KeepAliveChannels::handleTimeout: "
					"Timeout now restored.\n" );
			lastInvalidTimeout_ = 0;
		}
	}

	lastTimeout_ = now;

	iterator iter = channels_.begin();

	while (iter != channels_.end())
	{
		Channel & channel = **iter++;

		if (channel.lastReceivedTime() < deadTime)
		{
			ERROR_MSG( "KeepAliveChannels::check: "
				"Channel to %s has timed out (%.3fs)\n",
				channel.c_str(),
				(now - channel.lastReceivedTime()) / stampsPerSecondD() );

			this->delIfNecessary( channel );

			// Set hasRemoteFailed to true for dead channels.
			channel.hasRemoteFailed( true );
		}

		else if (channel.lastReceivedTime() < checkTime)
		{
			Bundle emptyBundle;

			// Send pings to channels that have been inactive for too long.
			emptyBundle.reliable( RELIABLE_DRIVER );
			channel.send( &emptyBundle );
		}
	}
}

} // namespace Mercury
