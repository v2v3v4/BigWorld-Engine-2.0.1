/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "delete_entity_handler.hpp"

#include "database.hpp"

#include "dbmgr_lib/db_entitydefs.hpp"


/**
 *	Constructor. For deleting entity by database ID.
 */
DeleteEntityHandler::DeleteEntityHandler( EntityTypeID typeID, DatabaseID dbID,
		const Mercury::Address& srcAddr, Mercury::ReplyID replyID ) :
	replyBundle_(),
	srcAddr_(srcAddr),
	ekey_( typeID, dbID )
{
	replyBundle_.startReply(replyID);
}


/**
 *	Constructor. For deleting entity by name.
 */
DeleteEntityHandler::DeleteEntityHandler( EntityTypeID typeID,
		const std::string& name, const Mercury::Address& srcAddr,
		Mercury::ReplyID replyID ) :
	replyBundle_(),
	srcAddr_( srcAddr ),
	ekey_( typeID, 0, name )
{
	replyBundle_.startReply( replyID );
}


/**
 *	Starts the process of deleting the entity.
 */
void DeleteEntityHandler::deleteEntity()
{
	if (Database::instance().getEntityDefs().isValidEntityType( ekey_.typeID ))
	{
		// See if it is checked out
		Database::instance().getEntity( ekey_, NULL, true, NULL, *this );
		// When getEntity() completes, onGetEntityCompleted() is called.
	}
	else
	{
		ERROR_MSG( "DeleteEntityHandler::deleteEntity: Invalid entity type "
				"%d\n", int(ekey_.typeID) );
		this->addFailure();

		this->sendReply();
	}
}


/**
 *	GetEntityHandler overrides
 */
void DeleteEntityHandler::onGetEntityCompleted( bool isOK,
		const EntityDBKey & entityKey,
		const EntityMailBoxRef * pBaseEntityLocation )
{
	ekey_ = entityKey;

	if (isOK)
	{
		if (this->isActiveMailBox( pBaseEntityLocation ))
		{
			TRACE_MSG( "Database::deleteEntity: entity checked out\n" );
			// tell the caller where to find it
			replyBundle_ << *pBaseEntityLocation;
		}
		else
		{	// __kyl__ TODO: Is it a problem if we delete the entity when it's awaiting creation?
			Database::instance().delEntity( ekey_, 0, *this );
			// When delEntity() completes, onDelEntityComplete() is called.
			return;	// Don't send reply just yet.
		}
	}
	else
	{	// Entity doesn't exist
		TRACE_MSG( "Database::deleteEntity: no such entity\n" );
		this->addFailure();
	}

	this->sendReply();
}


/**
 *	IDatabase::IDelEntityHandler overrides
 */
void DeleteEntityHandler::onDelEntityComplete( bool isOK )
{
	if (isOK)
	{
		TRACE_MSG( "Database::deleteEntity: succeeded\n" );
	}
	else
	{
		ERROR_MSG( "Database::deleteEntity: Failed to delete entity '%s' "
					"(%"FMT_DBID") of type %d\n",
					ekey_.name.c_str(), ekey_.dbID, ekey_.typeID );
		this->addFailure();
	}

	this->sendReply();
}


/**
 *	This method adds failure data to the reply bundle.
 */
void DeleteEntityHandler::addFailure()
{
	// Just needs to be anything that's a different size than a mailbox.
	replyBundle_ << int32(-1);
}


/**
 *	This method sends the reply.
 */
void DeleteEntityHandler::sendReply()
{
	Database::getChannel( srcAddr_ ).send( &replyBundle_ );
	delete this;
}

// delete_entity_handler.cpp
