/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BUFFERED_GHOST_MESSAGES_HPP
#define BUFFERED_GHOST_MESSAGES_HPP

#include "buffered_ghost_messages_for_entity.hpp"

#include "network/basictypes.hpp"

#include <map>

namespace Mercury
{
class Address;
}

class BufferedGhostMessage;


/**
 *	This class is used to buffer ghost messages that have arrived out of order.
 *	Messages from a single CellApp are ordered but if the real entity is
 *	offload, some ghost messages may arrive too soon.
 *
 *	To get around this problem, we consider the lifespan of a ghost as being
 *	split into subsequence of messages from each CellApp that the real visits.
 *	The Real entity tells ghosts of the next address before offloading. This
 *	allows the ghost to chain these subsequences together and play them in the
 *	correct order.
 */
class BufferedGhostMessages
{
public:
	void playSubsequenceFor( EntityID id, const Mercury::Address & srcAddr );
	void playNewLifespanFor( EntityID id );

	bool hasMessagesFor( EntityID entityID, 
		const Mercury::Address & addr ) const;

	bool isDelayingMessagesFor( EntityID entityID,
			const Mercury::Address & addr ) const;

	void delaySubsequence( EntityID entityID,
			const Mercury::Address & srcAddr,
			BufferedGhostMessage * pFirstMessage );

	void add( EntityID entityID, const Mercury::Address & srcAddr,
				   BufferedGhostMessage * pMessage );

	bool isEmpty() const	{ return map_.empty(); }

	static BufferedGhostMessages & instance();

private:
	typedef std::map< EntityID, BufferedGhostMessagesForEntity > Map;

	Map map_;
};

#endif // BUFFERED_GHOST_MESSAGES_HPP
