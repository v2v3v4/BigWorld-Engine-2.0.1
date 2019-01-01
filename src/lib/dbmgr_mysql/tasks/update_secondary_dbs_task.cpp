/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "update_secondary_dbs_task.hpp"

#include "get_secondary_dbs_task.hpp" // For getSecondaryDBEntries

#include "../query.hpp"

#include <sstream>

/**
 *	Constructor.
 */
UpdateSecondaryDBsTask::UpdateSecondaryDBsTask( const BaseAppIDs & ids,
			IDatabase::IUpdateSecondaryDBshandler & handler ) :
	MySqlBackgroundTask( "UpdateSecondaryDBsTask" ),
	handler_( handler ),
	entries_()
{
	// Create condition from ids.
	if (!ids.empty())
	{
		std::stringstream conditionStrm;
		conditionStrm << " WHERE appID NOT IN (";
		BaseAppIDs::const_iterator iter = ids.begin();
		conditionStrm << *iter;

		while (iter != ids.end())
		{
			conditionStrm << "," << *iter;

			++iter;
		}

		conditionStrm << ')';
		condition_ = conditionStrm.str();
	}
}


/**
 *
 */
void UpdateSecondaryDBsTask::performBackgroundTask( MySql & conn )
{
	// Get the entries that we're going to delete.
	getSecondaryDBEntries( conn, entries_, condition_ );

	// Delete the entries
	std::stringstream delStmtStrm_;
	delStmtStrm_ << "DELETE FROM bigworldSecondaryDatabases" << condition_;

	Query query( delStmtStrm_.str() );
	query.execute( conn, NULL );
}


/**
 *
 */
void UpdateSecondaryDBsTask::performMainThreadTask( bool succeeded )
{
	handler_.onUpdateSecondaryDBsComplete( entries_ );
}

// update_secondary_dbs_task.cpp
