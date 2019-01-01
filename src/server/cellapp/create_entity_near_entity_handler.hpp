/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CREATE_ENTITY_NEAR_ENTITY_HANDLER_HPP
#define CREATE_ENTITY_NEAR_ENTITY_HANDLER_HPP

class BufferedCreateEntityMessage;
class CellApp;

/**
 *	Objects of this type are used to handle variable length messages destined
 *	for a cell. They simply pass all the mercury parameters to the handler
 *	function.
 */
class CreateEntityNearEntityHandler : public Mercury::InputMessageHandler
{
public:
	/**
	 *	This type is the function pointer type that handles the incoming
	 *	message.
	 */
	typedef void (Cell::*Handler)(
		const Mercury::Address & addr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & stream,
		EntityPtr pNearbyEntity );

	CreateEntityNearEntityHandler( Handler handler );

	void playBufferedMessages( CellApp & app );

	void handle( const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data, EntityID entityID );

private:
	// Override
	virtual void handleMessage( const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	void sendFailure( const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data,
			EntityID entityID );

	void buffer( const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data, EntityID entityID );

	std::list< BufferedCreateEntityMessage * > bufferedMessages_;
	Handler handler_;
};

#endif // CREATE_ENTITY_NEAR_ENTITY_HANDLER_HPP
