/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHANNEL_FINDER_HPP
#define CHANNEL_FINDER_HPP

#include "misc.hpp"

namespace Mercury
{

class Channel;
class Packet;

/**
 *  A functor used to resolve ChannelIDs to Channels.  Used when a packet is
 *  received with FLAG_INDEXED_CHANNEL to figure out which channel to deliver it
 *  to.
 */
class ChannelFinder
{
public:
	virtual ~ChannelFinder() {};

	/**
	 *  Resolve the provided id to a Channel.  This will be called when an
	 *  indexed channel packet is received, before any messages are processed, so
	 *  this function should also set any context necessary for processing the
	 *  messages on the packet.
	 *
	 *  Callers should pass the bool parameter in as 'false', and the
	 *  ChannelFinder should set it to true if the ChannelFinder has dealt with
	 *  the packet and it should not be processed any further.
	 *
	 *  Should return NULL if the id cannot be resolved to a Channel.
	 */
	virtual Channel* find( ChannelID id, const Mercury::Address & srcAddr,
			const Packet * pPacket, bool & rHasBeenHandled ) = 0;
};

} // namespace Mercury

#endif // CHANNEL_FINDER_HPP
