/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOAD_ENTITY_HANDLER_HPP
#define LOAD_ENTITY_HANDLER_HPP

#include "database.hpp" // For ICheckoutCompletionListener
#include "get_entity_handler.hpp"

/**
 *	This class is used by Database::loadEntity() to load an entity from
 *	the database and wait for the results.
 */
class LoadEntityHandler : public GetEntityHandler,
                          public IDatabase::IPutEntityHandler,
                          public Database::ICheckoutCompletionListener
{
public:
	LoadEntityHandler( const EntityDBKey & ekey,
			const Mercury::Address & srcAddr, EntityID entityID,
			Mercury::ReplyID replyID ) :
		ekey_( ekey ),
		baseRef_(),
		srcAddr_( srcAddr ),
		entityID_( entityID ),
		replyID_( replyID ),
		replyBundle_(),
		pStrmDbID_( NULL )
	{}
	virtual ~LoadEntityHandler() {}

	void loadEntity();

	// IDatabase::IGetEntityHandler/GetEntityHandler overrides
	virtual void onGetEntityCompleted( bool isOK,
		const EntityDBKey & entityKey,
		const EntityMailBoxRef * pBaseEntityLocation );

	// IDatabase::IPutEntityHandler override
	virtual void onPutEntityComplete( bool isOK, DatabaseID );

	// Database::ICheckoutCompletionListener override
	virtual void onCheckoutCompleted( const EntityMailBoxRef* pBaseRef );

private:
	void sendAlreadyCheckedOutReply( const EntityMailBoxRef& baseRef );

	EntityDBKey			ekey_;
	EntityMailBoxRef	baseRef_;

	Mercury::Address	srcAddr_;
	EntityID			entityID_;
	Mercury::ReplyID	replyID_;

	Mercury::Bundle		replyBundle_;

	DatabaseID*			pStrmDbID_;
};

#endif // LOAD_ENTITY_HANDLER_HPP
