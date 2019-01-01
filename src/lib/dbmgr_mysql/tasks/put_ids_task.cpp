/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "put_ids_task.hpp"

#include "../query.hpp"

// -----------------------------------------------------------------------------
// Section: PutIDsTask
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
PutIDsTask::PutIDsTask( int numIDs, const EntityID * ids ) :
	MySqlBackgroundTask( "PutIDsTask" ),
	ids_( ids, ids + numIDs )
{
}


/**
 *	This method puts unused IDs into the database.
 */
void PutIDsTask::performBackgroundTask( MySql & conn )
{
	static const Query query( "INSERT INTO bigworldUsedIDs (id) VALUES (?)" );

	Container::const_iterator iter = ids_.begin();

	// TODO: ugh... make this not a loop somehow!
	while (iter != ids_.end())
	{
		query.execute( conn, *iter, NULL );

		++iter;
	}
}


/**
 *	This method is called in the main thread after run() completes.
 */
void PutIDsTask::performMainThreadTask( bool succeeded )
{
}

// put_ids_task.cpp
