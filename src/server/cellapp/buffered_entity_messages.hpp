/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BUFFERED_ENTITY_MESSAGES_HPP
#define BUFFERED_ENTITY_MESSAGES_HPP

#include "network/basictypes.hpp"

#include <list>

class BinaryIStream;
class BufferedEntityMessage;
class CellApp;
class EntityMessageHandler;

namespace Mercury
{
class Address;
class UnpackedMessageHeader;
}


/**
 *	Objects of this type are used to handle variable length messages destined
 *	for a cell. They simply pass all the mercury parameters to the handler
 *	function.
 */
class BufferedEntityMessages
{
public:
	void playBufferedMessages( CellApp & app );

	void add( EntityMessageHandler & handler,
		const Mercury::Address & srcAddr,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data, EntityID entityID );

	bool isEmpty() const
		{ return bufferedMessages_.empty(); }

private:
	std::list< BufferedEntityMessage * > bufferedMessages_;
};

#endif // BUFFERED_ENTITY_MESSAGES_HPP
