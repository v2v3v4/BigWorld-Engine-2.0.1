/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHANNEL_OWNER_HPP
#define CHANNEL_OWNER_HPP

#include "channel.hpp"

namespace Mercury
{

/**
 *	This class is a simple base class for classes that want to own a channel.
 */
class ChannelOwner
{
public:
	ChannelOwner( NetworkInterface & networkInterface,
			const Address & address = Address::NONE,
			Channel::Traits traits = Channel::INTERNAL ) :
		pChannel_( traits == Channel::INTERNAL ?
			Channel::get( networkInterface, address ) :
			new Channel( networkInterface, address, traits ) )
	{
	}

	~ChannelOwner()
	{
		pChannel_->condemn();
		pChannel_ = NULL;
	}

	Bundle & bundle()				{ return pChannel_->bundle(); }
	const Address & addr() const	{ return pChannel_->addr(); }
	const char * c_str() const		{ return pChannel_->c_str(); }
	void send( Bundle * pBundle = NULL ) { pChannel_->send( pBundle ); }

	Channel & channel()					{ return *pChannel_; }
	const Channel & channel() const		{ return *pChannel_; }

	void addr( const Address & addr );

#if ENABLE_WATCHERS
	static WatcherPtr pWatcher();
#endif

private:
	Channel * pChannel_;
};

} // namespace Mercury

#endif // CHANNEL_OWNER_HPP
