/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_CHANNEL_FINDER_HPP
#define ENTITY_CHANNEL_FINDER_HPP

#include "network/channel_finder.hpp"

class EntityChannelFinder : public Mercury::ChannelFinder
{
public:
	virtual Mercury::Channel * find( Mercury::ChannelID id,
		const Mercury::Address & srcAddr,
		const Mercury::Packet * pPacket,
		bool & rHasBeenHandled );
};

#endif // ENTITY_CHANNEL_FINDER_HPP
