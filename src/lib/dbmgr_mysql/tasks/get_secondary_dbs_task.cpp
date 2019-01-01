/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "get_secondary_dbs_task.hpp"

#include "../query.hpp"
#include "../result_set.hpp"

#include <arpa/inet.h>
#include <sstream>

/**
 * 	This method executes SELECTs ip,port,appID,location FROM
 * 	bigworldSecondaryDatabases with the WHERE clause and returns the results
 * 	in entries.
 */
void getSecondaryDBEntries( MySql & connection,
		IDatabase::SecondaryDBEntries & entries,
		const std::string & condition )
{
	std::stringstream getStmtStrm;
	getStmtStrm << "SELECT ip,port,appID,location FROM "
			"bigworldSecondaryDatabases" << condition;

	Query query( getStmtStrm.str() );
	ResultSet resultSet;

	query.execute( connection, &resultSet );

	Mercury::Address addr;
	int32 appID;
	std::string location;

	while (resultSet.getResult( addr.ip, addr.port, appID, location ))
	{
		entries.push_back( IDatabase::SecondaryDBEntry(
				htonl( addr.ip ),
				htons( addr.port ),
				appID,
				location ) );
	}
}


/**
 *	Constructor.
 */
GetSecondaryDBsTask::GetSecondaryDBsTask( Handler & handler ) :
	MySqlBackgroundTask( "GetSecondaryDBsTask" ),
	handler_( handler ),
	entries_()
{
}


/**
 *
 */
void GetSecondaryDBsTask::performBackgroundTask( MySql & conn )
{
	getSecondaryDBEntries( conn, entries_ );
}


/**
 *
 */
void GetSecondaryDBsTask::performMainThreadTask( bool succeeded )
{
	handler_.onGetSecondaryDBsComplete( entries_ );
}

// get_secondary_dbs_task.cpp
