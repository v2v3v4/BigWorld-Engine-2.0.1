/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "look_up_dbid_handler.hpp"

#include "database.hpp"
#include "dbmgr_lib/db_entitydefs.hpp"


/**
 *	Starts the process of looking up the DBID.
 */
void LookUpDBIDHandler::lookUpDBID( EntityTypeID typeID,
		const std::string & name )
{
	if (Database::instance().getEntityDefs().isValidEntityType( typeID ))
	{
		EntityDBKey entityKey( typeID, 0, name );
		Database::instance().getEntity( entityKey, NULL, false, NULL, *this );
		// When getEntity() completes, onGetEntityCompleted() is called.
	}
	else
	{
		ERROR_MSG( "LookUpDBIDHandler::lookUpDBID: Invalid entity type %d\n",
				typeID );
		replyBundle_ << DatabaseID( 0 );
		Database::getChannel( srcAddr_ ).send( &replyBundle_ );
		delete this;
	}
}


/**
 *	GetEntityHandler override.
 */
void LookUpDBIDHandler::onGetEntityCompleted( bool isOK,
		const EntityDBKey & entityKey,
   		const EntityMailBoxRef * pBaseEntityLocation )
{
	replyBundle_ << entityKey.dbID;
	Database::getChannel( srcAddr_ ).send( &replyBundle_ );

	delete this;
}

// look_up_dbid_handler.cpp
