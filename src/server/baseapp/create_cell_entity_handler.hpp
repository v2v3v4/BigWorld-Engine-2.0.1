/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CREATE_CELL_ENTITY_HANDLER_HPP
#define CREATE_CELL_ENTITY_HANDLER_HPP

#include "cstdmf/smartpointer.hpp"
#include "network/basictypes.hpp"
#include "network/interfaces.hpp"

#include <memory>

class Base;
typedef SmartPointer< Base > BasePtr;


/**
 *	This class is used by createCellEntity and createInNewSpace to handle the
 *	reply to the cell entity creation request.
 */
class CreateCellEntityHandler : public Mercury::ShutdownSafeReplyMessageHandler
{
public:
	CreateCellEntityHandler( Base * pBase );

	void handleMessage( const Mercury::Address & source,
			Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data, void * arg );

	void handleException( const Mercury::NubException & exception, void * arg );

private:
	BasePtr pBase_;
};


/**
 *	This class is used by createCellEntity and createInNewSpace to handle the
 *	reply to the cell entity creation request.
 */
class CreateCellEntityViaBaseHandler :
	public Mercury::ShutdownSafeReplyMessageHandler
{
public:
	CreateCellEntityViaBaseHandler( Base * pBase,
			Mercury::ReplyMessageHandler * pHandler,
			EntityID nearbyID );

	void handleMessage( const Mercury::Address & source,
			Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data, void * arg );

	void handleException( const Mercury::NubException & exception, void * arg );

private:
	void cellCreationFailure();

	BasePtr pBase_;
	std::auto_ptr< Mercury::ReplyMessageHandler > pHandler_;
	EntityID nearbyID_;
};

#endif // CREATE_CELL_ENTITY_HANDLER_HPP
