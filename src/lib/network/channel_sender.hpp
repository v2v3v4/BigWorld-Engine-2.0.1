/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHANNEL_SENDER_HPP
#define CHANNEL_SENDER_HPP

#include "channel.hpp"

namespace Mercury
{

/**
 *  This class is a wrapper class for a Channel that automatically sends on
 *  destruct if the channel is irregular.  Recommended for use in app code when
 *  you don't want to have to keep figuring out if channels you get with
 *  findChannel() are regular or not.
 */
class ChannelSender
{
public:
	ChannelSender( Channel & channel ) :
		channel_( channel )
	{}

	~ChannelSender()
	{
		if (!channel_.isLocalRegular())
		{
			channel_.delayedSend();
		}
	}

	Bundle & bundle() { return channel_.bundle(); }
	Channel & channel() { return channel_; }

private:
	Channel & channel_;
};

}

#endif // CHANNEL_SENDER_HPP
