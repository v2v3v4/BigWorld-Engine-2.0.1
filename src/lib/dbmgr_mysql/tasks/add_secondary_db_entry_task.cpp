/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "add_secondary_db_entry_task.hpp"

#include "../query.hpp"

#include <arpa/inet.h>


/**
 *	Constructor.
 */
AddSecondaryDBEntryTask::AddSecondaryDBEntryTask(
			const IDatabase::SecondaryDBEntry & entry ) :
		MySqlBackgroundTask( "AddSecondaryDBEntryTask" ),
		entry_( entry )
{
}


namespace
{
const Query query( "INSERT INTO bigworldSecondaryDatabases "
					"(ip, port, appID, location) VALUES (?,?,?,?)" );
}


/**
 *
 */
void AddSecondaryDBEntryTask::performBackgroundTask( MySql & conn )
{
	query.execute( conn,
			ntohl( entry_.addr.ip ), ntohs( entry_.addr.port ),
			entry_.appID, entry_.location, NULL );
}


/**
 *
 */
void AddSecondaryDBEntryTask::performMainThreadTask( bool succeeded )
{
}

// add_secondary_db_entry_task.cpp
