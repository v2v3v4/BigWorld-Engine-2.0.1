/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef KEEPALIVE_CHANNELS_HPP
#define KEEPALIVE_CHANNELS_HPP

#include "monitored_channels.hpp"

namespace Mercury
{

/**
 *	This class is used to keep track of channels that send and receive so
 *	infrequently that we need to send keepalive traffic to ensure the peer apps
 *	haven't died.
 */
class KeepAliveChannels : public MonitoredChannels
{
public:
	KeepAliveChannels();
	void addIfNecessary( Channel & channel );

protected:
	iterator & channelIter( Channel & channel ) const;
	float defaultPeriod() const;

private:
	virtual void handleTimeout( TimerHandle, void * );

	uint64 lastTimeout_;
	uint64 lastInvalidTimeout_;
};

} // namespace Mercury

#endif // KEEPALIVE_CHANNELS_HPP
