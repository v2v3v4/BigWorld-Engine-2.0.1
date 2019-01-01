/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "set_entity_key_for_account_task.hpp"

#include "../query.hpp"

// -----------------------------------------------------------------------------
// Section: SetEntityKeyForAccountTask
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
SetEntityKeyForAccountTask::SetEntityKeyForAccountTask( const std::string & username,
			const std::string & password,
			const EntityKey & ekey ) :
	MySqlBackgroundTask( "SetEntityKeyForAccountTask" ),
	username_( username ),
	password_( password ),
	typeID_( ekey.typeID ),
	entityDatabaseID_( ekey.dbID )
{
}


/**
 *	This method writes the log on mapping into the table.
 */
void SetEntityKeyForAccountTask::performBackgroundTask( MySql & conn )
{
	static const Query query(
		"REPLACE INTO bigworldLogOnMapping "
				"(logOnName, password, entityType, entityID) "
			"VALUES (?, "
				"IF( (SELECT isPasswordHashed FROM bigworldInfo), "
						"MD5( CONCAT( ?, logOnName ) ), "
						"? ), "
				"(SELECT typeID FROM bigworldEntityTypes WHERE bigworldID=?), "
				"?)" );

	query.execute( conn,
		username_, password_, password_, typeID_, entityDatabaseID_,
		/* pResultSet:*/ NULL );
}


/**
 *	This method is called in the main thread after run() is complete.
 */
void SetEntityKeyForAccountTask::performMainThreadTask( bool succeeded )
{
	// Nothing required
}

// set_entity_key_for_account.cpp
