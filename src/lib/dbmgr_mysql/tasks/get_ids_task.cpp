/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "get_ids_task.hpp"

#include "../query.hpp"
#include "../result_set.hpp"

// -----------------------------------------------------------------------------
// Section: GetIDsTask
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
GetIDsTask::GetIDsTask( int numIDs, IDatabase::IGetIDsHandler & handler ) :
	MySqlBackgroundTask( "GetIDsTask" ),
	numIDs_( numIDs ),
	handler_( handler )
{
}

namespace
{
// For returned ids
const Query getIDsQuery( "SELECT id FROM bigworldUsedIDs LIMIT ? FOR UPDATE" );
const Query delIDsQuery( "DELETE FROM bigworldUsedIDs LIMIT ?" );

// For new ids
const Query incIDQuery( "UPDATE bigworldNewID SET id=id+?" );
const Query getIDQuery( "SELECT id FROM bigworldNewID LIMIT 1 FOR UPDATE" );
}


/**
 *	This method gets some unused IDs from the database. May be executed in a
 *	separate thread.
 */
void GetIDsTask::performBackgroundTask( MySql & conn )
{
	BinaryOStream & stream = handler_.idStrm();

	int numRemaining = this->getUsedIDs( conn, numIDs_, stream );

	if (numRemaining > 0)
	{
		this->getNewIDs( conn, numRemaining, stream );
	}
}


/**
 *	This gets ids from the pool of previously used ids.
 *
 *	@return The number of ids that it failed to get.
 */
int GetIDsTask::getUsedIDs( MySql & conn, int numIDs, BinaryOStream & stream )
{
	// Reuse any id's we can get our hands on
	ResultSet resultSet;
	getIDsQuery.execute( conn, numIDs, &resultSet );

	int numIDsRetrieved = 0;

	EntityID entityID;

	while (resultSet.getResult( entityID ))
	{
		stream << entityID;
		++numIDsRetrieved;
	}

	if (numIDsRetrieved > 0)
	{
		delIDsQuery.execute( conn, numIDsRetrieved, NULL );
	}

	return numIDs - numIDsRetrieved;
}


/**
 *	This gets ids from the pool of new ids.
 */
void GetIDsTask::getNewIDs( MySql & conn, int numIDs, BinaryOStream & stream )
{
	ResultSet resultSet;
	getIDQuery.execute( conn, &resultSet );
	incIDQuery.execute( conn, numIDs, NULL );

	EntityID newID;
	MF_VERIFY( resultSet.getResult( newID ) );

	for (int i = 0; i < numIDs; ++i)
	{
		stream << newID++;
	}
}


/**
 *
 */
void GetIDsTask::onRetry()
{
	handler_.resetStrm();
}


/**
 *	This method is called in the main thread after run() completes.
 */
void GetIDsTask::performMainThreadTask( bool succeeded )
{
	handler_.onGetIDsComplete();
}

// get_ids_task.cpp
