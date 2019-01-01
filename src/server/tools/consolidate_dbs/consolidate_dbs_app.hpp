/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONSOLIDATE_DBS_APP_HPP
#define CONSOLIDATE_DBS_APP_HPP

#include "dbmgr.hpp"
#include "dbmgr_status_reporter.hpp"
#include "db_consolidator_errors.hpp"
#include "file_receiver_mgr.hpp"

#include "dbmgr_lib/db_entitydefs.hpp"
#include "dbmgr_lib/entity_key.hpp"

#include "dbmgr_mysql/connection_info.hpp"
#include "dbmgr_mysql/database_tool_app.hpp"

#include "cstdmf/singleton.hpp"

#include <map>
#include <memory>
#include <vector>


namespace Mercury
{
	class EventDispatcher;
}

class CommandLineParser;
class ConsolidationProgressReporter;
class FileReceiverMgr;
class MySql;
class MySqlLockedConnection;
class PrimaryDatabaseUpdateQueue;
class sqlite3_stmt;
class SqliteConnection;
class WatcherNub;

/**
 * 	Consolidates data from remote secondary databases.
 */
class ConsolidateDBsApp : public DatabaseToolApp,
		public Singleton< ConsolidateDBsApp >,
		public DBMgrStatusReporter
{
public:
	ConsolidateDBsApp( bool shouldStopOnError );
	virtual ~ConsolidateDBsApp();

	bool initLogger();

	bool init( bool isVerbose, bool shouldReportToDBMgr,
		const DBConfig::ConnectionInfo & primaryDBConnectionInfo );

	bool checkPrimaryDBEntityDefsMatch();

	bool transferAndConsolidate();
	bool consolidateSecondaryDBs( const FileNames & filePaths );
	bool clearSecondaryDBEntries( uint * pNumEntriesCleared = NULL );

	void abort();

private:
	// From DBMgrStatusReporter
	virtual void onStatus( const std::string & status );

	// From DatabaseToolApp
	virtual void onSignalled( int sigNum );

	bool checkPrimaryDBEntityDefsMatchInternal();

	bool getSecondaryDBInfos( SecondaryDBInfos & secondaryDBInfos );
	bool transferSecondaryDBs( const SecondaryDBInfos & secondaryDBInfos,
		FileReceiverMgr & fileReceiverMgr );

	bool consolidateSecondaryDB( const std::string & filePath,
		PrimaryDatabaseUpdateQueue & primaryDBQueue,
		ConsolidationProgressReporter & progressReporter );

	bool checkEntityDefsDigestMatch( const std::string & quotedDigest );


	int getNumRows( SqliteConnection & connection,
			const std::string & tblName );

// Member data

	uint32						internalIP_;

	std::auto_ptr< DBMgr >		pDBMgr_;

	DBConfig::ConnectionInfo 	connectionInfo_;

	std::string					consolidationDir_;

	DBConsolidatorErrors		consolidationErrors_;
	bool						shouldStopOnError_;

	// Flag for aborting our wait loop.
	bool						shouldAbort_;
};

#endif // CONSOLIDATE_DBS_APP_HPP
