/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BUFFERED_GHOST_MESSAGE_QUEUE_HPP
#define BUFFERED_GHOST_MESSAGE_QUEUE_HPP

#include "cstdmf/smartpointer.hpp"
#include "network/basictypes.hpp"

#include <list>

class BufferedGhostMessage;

/**
 *  This class represents a stream of buffered real->ghost messages from a
 *  single CellApp destined for a single entity. It owns the
 *	BufferedGhostMessages that it contains.
 *
 *	BufferedGhostMessagesForEntity contains a collection of these mapped by
 *	source address.
 */
class BufferedGhostMessageQueue: public ReferenceCount
{
public:
	BufferedGhostMessageQueue();
	~BufferedGhostMessageQueue();

	void add( BufferedGhostMessage * pMessage );

	bool isFrontCreateGhost() const;
	bool isEmpty() const				{ return messages_.empty(); }
	bool isDelaying() const;

	bool playSubsequence();

	void delaySubsequence( BufferedGhostMessage * pFirstMessage );
	void promoteDelayedSubsequences();

private:
	bool playFront();
	BufferedGhostMessage * popFront();

	bool isFrontSubsequenceStart() const;
	bool isFrontSubsequenceEnd() const;

	typedef std::list< BufferedGhostMessage* > Messages;
	typedef Messages::iterator iterator;

	Messages messages_;
	Messages * pDelayedMessages_;
};

typedef SmartPointer<BufferedGhostMessageQueue> BufferedGhostMessageQueuePtr;

#endif // BUFFERED_GHOST_MESSAGE_QUEUE_HPP
