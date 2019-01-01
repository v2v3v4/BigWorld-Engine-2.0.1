/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONSOLIDATE_DBS__SECONDARY_DATABASE_TABLE_HPP
#define CONSOLIDATE_DBS__SECONDARY_DATABASE_TABLE_HPP

#include "network/basictypes.hpp"

#include <memory>
#include <string>

class ConsolidationProgressReporter;
class PrimaryDatabaseUpdateQueue;
class SecondaryDatabase;
class SqliteStatement;

/**
 *	Class to keep track of entity data stored in a table in a secondary
 *	database.
 */
class SecondaryDatabaseTable
{
public:
	SecondaryDatabaseTable( SecondaryDatabase & database, 
			const std::string & tableName );
	bool init();

	~SecondaryDatabaseTable();

	const std::string & tableName() const
		{ return tableName_; }

	GameTime firstGameTime();

	int numRows();

	bool consolidate( PrimaryDatabaseUpdateQueue & primaryQueue,
			ConsolidationProgressReporter & progressReporter,
			bool & shouldAbort );

private:
	SecondaryDatabase & 				database_;
	std::string 						tableName_;
	std::auto_ptr< SqliteStatement > 	pGetDataQuery_;
	std::auto_ptr< SqliteStatement > 	pGetNumRowsQuery_;
};

#endif // CONSOLIDATE_DBS__SECONDARY_DATABASE_TABLE_HPP
