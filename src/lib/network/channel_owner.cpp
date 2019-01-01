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

#include "channel_owner.hpp"

#include "cstdmf/watcher.hpp"

namespace Mercury
{

// -----------------------------------------------------------------------------
// Section: ChannelOwner
// -----------------------------------------------------------------------------

/**
 *  This method switches this ChannelOwner to a different address.  We can't
 *  simply call through to Channel::addr() because there might already be an
 *  anonymous channel to that address.  We need to look it up and claim the
 *  anonymous one if it already exists.
 */
void ChannelOwner::addr( const Address & addr )
{
	MF_ASSERT( pChannel_ );

	// Don't do anything if it's already on the right address
	if (this->addr() == addr)
	{
		return;
	}

	// Get a new channel to the right address.
	Channel * pNewChannel =
		Channel::get( pChannel_->networkInterface(), addr );

	// Configure the new channel like the old one, and then get rid of it.
	pNewChannel->configureFrom( *pChannel_ );
	pChannel_->condemn();

	// Put the new channel in its place.
	pChannel_ = pNewChannel;
}


#if ENABLE_WATCHERS
/**
 *	This static method returns the watcher associated with this class.
 */
WatcherPtr ChannelOwner::pWatcher()
{
	static WatcherPtr pWatcher =
		new BaseDereferenceWatcher( Channel::pWatcher() );

	return pWatcher;
}
#endif // ENABLE_WATCHERS

} // namespace Mercury

// channel_owner.cpp
