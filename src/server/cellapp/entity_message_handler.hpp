/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_MESSAGE_HANDLER_HPP
#define ENTITY_MESSAGE_HANDLER_HPP

#include "cellapp_interface.hpp"

class Entity;

/**
 *  This class is a base class for entity message handler types.
 */
class EntityMessageHandler : public Mercury::InputMessageHandler
{
public:
	void handleMessage( const Mercury::Address & srcAddr,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data,
	   	EntityID entityID );

protected:
	EntityMessageHandler( EntityReality reality,
				bool shouldBufferIfTickPending = false );

	virtual void handleMessage( const Mercury::Address & srcAddr,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data );

	/**
	 *  This method actually causes the message to be fed through to the
	 *  handler, after it has passed the various checks imposed in
	 *  handleMessage().
	 */
	virtual void callHandler( const Mercury::Address & srcAddr,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data, Entity * pEntity ) = 0;

	virtual void sendFailure( const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data,
			EntityID entityID );

	EntityReality reality_;
	bool shouldBufferIfTickPending_;
};

#endif // ENTITY_MESSAGE_HANDLER_HPP
