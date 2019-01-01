/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "look_up_entity_handler.hpp"

#include "database.hpp"

#include "dbmgr_lib/db_entitydefs.hpp"

/**
 *	Constructor.
 */
LookUpEntityHandler::LookUpEntityHandler( const Mercury::Address & srcAddr,
		Mercury::ReplyID replyID, bool offChannel ) :
	replyBundle_(),
	srcAddr_( srcAddr ),
	offChannel_( offChannel )
{
	replyBundle_.startReply( replyID );
}


/**
 *	Starts the process of looking up the entity.
 */
void LookUpEntityHandler::lookUpEntity( const EntityDBKey & entityKey )
{
	Database & db = Database::instance();

	if (db.getEntityDefs().isValidEntityType( entityKey.typeID ))
	{
		db.getEntity( entityKey, NULL, true, NULL, *this );
		// When getEntity() completes, onGetEntityCompleted() is called.
	}
	else
	{
		ERROR_MSG( "LookUpEntityHandler::lookUpEntity: Invalid entity type "
				"%d\n", entityKey.typeID );
		replyBundle_ << int32(-1);

		this->sendReply();
	}
}


void LookUpEntityHandler::onGetEntityCompleted( bool isOK,
		const EntityDBKey & entityKey,
		const EntityMailBoxRef * pBaseEntityLocation )
{
	if (isOK)
	{
		if (this->isActiveMailBox( pBaseEntityLocation ))
		{
			// Entity is checked out.
			replyBundle_ << *pBaseEntityLocation;
		}
		else
		{
			// not checked out so empty message
		}
	}
	else
	{	// Entity doesn't exist
		replyBundle_ << int32(-1);
	}

	this->sendReply();
}


/**
 *	This method sends the reply and deletes this object.
 */
void LookUpEntityHandler::sendReply()
{
	if (offChannel_)
	{
		Database::instance().interface().send( srcAddr_, replyBundle_ );
	}
	else
	{
		Database::getChannel( srcAddr_ ).send( &replyBundle_ );
	}

	delete this;
}

// look_up_entity_handler.cpp
