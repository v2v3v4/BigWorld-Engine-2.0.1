/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BUFFERED_GHOST_MESSAGE_FACTORY_HPP
#define BUFFERED_GHOST_MESSAGE_FACTORY_HPP

#include "network/basictypes.hpp"

class BinaryIStream;
class BufferedGhostMessage;

namespace Mercury
{
class Address;
class InputMessageHandler;
class UnpackedMessageHeader;
}

namespace CellAppInterface
{
	struct ghostSetRealArgs;
}

namespace BufferedGhostMessageFactory
{

BufferedGhostMessage * createBufferedMessage(
		const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data,
		EntityID entityID,
		Mercury::InputMessageHandler * pHandler );

BufferedGhostMessage * createBufferedCreateGhostMessage(
		const Mercury::Address & srcAddr, EntityID entityID,
		SpaceID spaceID, BinaryIStream & data );

BufferedGhostMessage * createGhostSetRealMessage(
		EntityID entityID, const CellAppInterface::ghostSetRealArgs & args );

} // namespace BufferedGhostMessageFactory

#endif // BUFFERED_GHOST_MESSAGE_FACTORY_HPP
