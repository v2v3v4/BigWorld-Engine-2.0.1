/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SECONDARY_DATABASE_HPP
#define SECONDARY_DATABASE_HPP

#include "cstdmf/stdmf.hpp"
#include "cstdmf/shared_ptr.hpp"

#include <memory>
#include <string>
#include <vector>

class ConsolidationProgressReporter;
class PrimaryDatabaseUpdateQueue;
class SecondaryDatabaseTable;
class SqliteConnection;

/**
 * 	Consolidates data from remote secondary databases.
 */
class SecondaryDatabase
{
public:
	SecondaryDatabase();
	~SecondaryDatabase();

	bool init( const std::string & dbPath );

	bool getChecksumDigest( std::string & digest );

	uint numEntities() const
		{ return numEntities_; }

	SqliteConnection & connection()
		{ return *pConnection_; }

	bool consolidate( PrimaryDatabaseUpdateQueue & primaryDBQueue,
			ConsolidationProgressReporter & progressReporter,
			bool shouldIgnoreErrors,
			bool & shouldAbort );

private:
	bool readTables();
	bool tableExists( const std::string & tableName );
	void sortTablesByAge();

	std::string							path_;

	std::auto_ptr< SqliteConnection > 	pConnection_;

	typedef std::vector< shared_ptr< SecondaryDatabaseTable > > Tables;
	Tables 								tables_;

	uint								numEntities_;
};

#endif // SECONDARY_DATABASE_HPP
