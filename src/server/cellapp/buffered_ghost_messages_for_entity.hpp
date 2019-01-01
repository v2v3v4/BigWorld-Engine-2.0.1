/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BUFFERED_GHOST_MESSAGES_FOR_ENTITY_HPP
#define BUFFERED_GHOST_MESSAGES_FOR_ENTITY_HPP

#include "buffered_ghost_message_queue.hpp"

#include <map>


/**
 *  This class collects all buffered ghost message for an entity. It keeps
 *	these messages in queues from different CellApps. Only the ordering within
 *	these queues is guaranteed. This class works out the ordering between these
 *	queues.
 *
 *	The last message from a real entity to a ghost before being offloaded
 *	(ghostSetNextReal) informs us which is the next queue to process from.
 */
class BufferedGhostMessagesForEntity
{
public:
	BufferedGhostMessagesForEntity() {}

	void add( const Mercury::Address & addr,
		BufferedGhostMessage * pMessage );

	bool playSubsequence( const Mercury::Address & srcAddr );
	bool playNewLifespan();

	void delaySubsequence( const Mercury::Address & addr,
			BufferedGhostMessage * pFirstMessage );

	bool hasMessagesFrom( const Mercury::Address & addr ) const;
	bool isDelayingMessagesFor( const Mercury::Address & addr ) const;

private:
	BufferedGhostMessageQueuePtr find( const Mercury::Address & addr ) const;
	BufferedGhostMessageQueuePtr findOrCreate( const Mercury::Address & addr );

	typedef std::map< Mercury::Address, BufferedGhostMessageQueuePtr > Queues;
	Queues queues_;
};

#endif // BUFFERED_GHOST_MESSAGES_FOR_ENTITY_HPP
