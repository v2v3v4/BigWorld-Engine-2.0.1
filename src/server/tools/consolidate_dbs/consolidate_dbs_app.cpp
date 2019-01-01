/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "Python.h"		// See http://docs.python.org/api/includes.html

#include "consolidate_dbs_app.hpp"

#include "consolidation_progress_reporter.hpp"
#include "db_file_transfer_error_monitor.hpp"
#include "file_transfer_progress_reporter.hpp"
#include "primary_database_update_queue.hpp"
#include "secondary_database.hpp"
#include "secondary_db_info.hpp"
#include "tcp_listener.hpp"
#include "transfer_db_process.hpp"

#include "cstdmf/binary_stream.hpp"

#include "dbmgr_mysql/db_config.hpp"
#include "dbmgr_mysql/locked_connection.hpp"
#include "dbmgr_mysql/query.hpp"
#include "dbmgr_mysql/result_set.hpp"
#include "dbmgr_mysql/transaction.hpp"

#include "entitydef/constants.hpp"

#include "network/event_dispatcher.hpp"

#include "pyscript/py_import_paths.hpp"
#include "pyscript/script.hpp"

#include "resmgr/bwresource.hpp"

#include "server/bwconfig.hpp"

#include "sqlite/sqlite_util.hpp"

#include "third_party/sqlite/sqlite3.h"

DECLARE_DEBUG_COMPONENT( 0 )

// -----------------------------------------------------------------------------
// Section: Link tokens
// -----------------------------------------------------------------------------

extern int UserDataObjectLinkDataType_token;
extern int ResMgr_token;

namespace // anonymous
{

int s_moduleTokens = ResMgr_token | UserDataObjectLinkDataType_token;


// -----------------------------------------------------------------------------
// Section: ConsolidateDBsApp
// -----------------------------------------------------------------------------

}

BW_SINGLETON_STORAGE( ConsolidateDBsApp )

/**
 *	Constructor.
 */
ConsolidateDBsApp::ConsolidateDBsApp( bool shouldStopOnError ) :
	DatabaseToolApp(),
	internalIP_( 0 ),
	pDBMgr_( NULL ),
	connectionInfo_(),
	consolidationDir_( "/tmp/" ),
	consolidationErrors_(),
	shouldStopOnError_( shouldStopOnError ),
	shouldAbort_( false )
{
}


/**
 *	Destructor.
 */
ConsolidateDBsApp::~ConsolidateDBsApp()
{
}


/**
 * 	This function should be called after constructor to initialise the object.
 * 	Returns true if initialisation succeeded, false if it failed.
 */
bool ConsolidateDBsApp::init( bool isVerbose, bool shouldReportToDBMgr,
		const DBConfig::ConnectionInfo & primaryDBConnectionInfo )
{
	if (!this->DatabaseToolApp::init( "ConsolidateDBs", isVerbose,
			/* shouldLock= */ true, primaryDBConnectionInfo ))
	{
		return false;
	}

	this->enableSignalHandler( SIGINT );
	this->enableSignalHandler( SIGHUP );

	internalIP_ = this->watcherNub().udpSocket().getLocalAddress().ip;

	if (shouldReportToDBMgr)
	{
		pDBMgr_.reset( new DBMgr( this->watcherNub() ) );
		if (!pDBMgr_->init())
		{
			// No DBMgr found!
			return false;
		}
	}

	TRACE_MSG( "ConsolidateDBsApp: Connected to primary database: "
			"host=%s:%d, username=%s, database=%s\n",
		primaryDBConnectionInfo.host.c_str(),
		primaryDBConnectionInfo.port,
		primaryDBConnectionInfo.username.c_str(),
		primaryDBConnectionInfo.database.c_str() );

	connectionInfo_ = primaryDBConnectionInfo;

	BWConfig::update( "dbMgr/consolidation/directory", consolidationDir_ );
	IFileSystem::FileType consolidationDirType =
			BWResource::resolveToAbsolutePath( consolidationDir_ );

	if (consolidationDirType != IFileSystem::FT_DIRECTORY)
	{
		ERROR_MSG( "ConsolidateDBsApp::init: Configuration setting "
				"dbMgr/consolidation/directory specifies a non-existent "
				"directory: %s\n", 
			consolidationDir_.c_str() );
		return false;
	}

	return true;
}


/**
 * 	After initialisation, this method starts the data consolidation process
 */
bool ConsolidateDBsApp::transferAndConsolidate()
{
	// Get the secondary DB info from primary database.
	SecondaryDBInfos secondaryDBs;

	if (!this->getSecondaryDBInfos( secondaryDBs ))
	{
		return false;
	}

	if (secondaryDBs.empty())
	{
		ERROR_MSG( "ConsolidateDBsApp::run: No secondary databases to "
				"consolidate\n" );
		return false;
	}

	FileTransferProgressReporter progressReporter( *this, 
		secondaryDBs.size() );
	FileReceiverMgr	fileReceiverMgr( this->dispatcher(), progressReporter,
		secondaryDBs, consolidationDir_ );

	if (!this->transferSecondaryDBs( secondaryDBs, fileReceiverMgr ))
	{
		return false;
	}

	// Consolidate databases
	const FileNames & dbFilePaths = fileReceiverMgr.receivedFilePaths();

	if (!this->consolidateSecondaryDBs( dbFilePaths ))
	{
		return false;
	}

	fileReceiverMgr.cleanUpRemoteFiles( consolidationErrors_ );

	this->clearSecondaryDBEntries();

	TRACE_MSG( "ConsolidateDBsApp::run: Completed successfully\n" );

	return true;
}


/**
 *	Transfer the secondary DBs from each location specified.
 */
bool ConsolidateDBsApp::transferSecondaryDBs( 
		const SecondaryDBInfos & secondaryDBs, 
		FileReceiverMgr & fileReceiverMgr )
{
	// Start listening for incoming connections
	TcpListener< FileReceiverMgr > connectionsListener( fileReceiverMgr );

	if (!connectionsListener.init( 0, internalIP_, secondaryDBs.size() ))
	{
		return false;
	}

	// Make our address:port into a string to pass to child processes
	Mercury::Address ourAddr;
	connectionsListener.getBoundAddr( ourAddr );

	// Start remote file transfer service
	for (SecondaryDBInfos::const_iterator iSecondaryDBInfo = 
				secondaryDBs.begin();
			iSecondaryDBInfo != secondaryDBs.end();
			++iSecondaryDBInfo)
	{
		TransferDBProcess transferDB( ourAddr );

		if (!transferDB.transfer( iSecondaryDBInfo->hostIP, 
				iSecondaryDBInfo->location ))
		{
			shouldAbort_ = true;
			return false;
		}
	}

	{
		DBFileTransferErrorMonitor errorMonitor( fileReceiverMgr );

		// Wait for file transfer to complete
		this->dispatcher().processUntilBreak();
	}

	return fileReceiverMgr.finished();
}


/**
 *	Consolidates the secondary databases pointed to by filePath into the
 * 	primary database.
 */
bool ConsolidateDBsApp::consolidateSecondaryDBs( const FileNames & filePaths )
{
	int numConnections =
			std::max( BWConfig::get( "dbMgr/numConnections", 5 ), 1 );
	INFO_MSG( "ConsolidateDBsApp::consolidateSecondaryDBs: "
			"Number of connections = %d.\n", 
		numConnections );

	PrimaryDatabaseUpdateQueue primaryDBQueue( connectionInfo_,
		this->entityDefs(), numConnections );
	ConsolidationProgressReporter progressReporter( *this, filePaths.size() );

	for (FileNames::const_iterator iFilePath = filePaths.begin();
			iFilePath != filePaths.end(); 
			++iFilePath)
	{
		if (!this->consolidateSecondaryDB( *iFilePath, primaryDBQueue,
				progressReporter ))
		{
			if (shouldAbort_)
			{
				WARNING_MSG( "ConsolidateDBsApp::consolidateSecondaryDBs: "
						"Data consolidation was aborted\n" );
			}
			else
			{
				WARNING_MSG( "ConsolidateDBsApp::consolidateSecondaryDBs: "
						"Some entities were not consolidated. Data "
						"consolidation must be re-run after errors have been "
						"corrected.\n" );
			}
			return false;
		}
	}

	return true;
}


/**
 * 	Get list of secondary DBs from the primary DB.
 */
bool ConsolidateDBsApp::getSecondaryDBInfos( 
		SecondaryDBInfos & secondaryDBInfos )
{
	bool isOK = true;
	try
	{
		ResultSet resultSet;
		Query query( "SELECT ip, location FROM bigworldSecondaryDatabases" );

		query.execute( this->connection(), &resultSet );

		uint32					ip;
		std::string				location;

		while (resultSet.getResult( ip, location ))
		{
			secondaryDBInfos.push_back( SecondaryDBInfo( htonl( ip ),
					location ) );
		}
	}
	catch (std::exception & e)
	{
		ERROR_MSG( "ConsolidateDBsApp::getSecondaryDBInfos: Failed to get "
				"secondary DB information from primary database: %s\n",
			e.what() );
		isOK = false;
	}

	return isOK;
}


/**
 *	Consolidates the secondary database pointed to by filePath into the
 * 	primary database.
 */
bool ConsolidateDBsApp::consolidateSecondaryDB(
		const std::string & filePath,
		PrimaryDatabaseUpdateQueue & primaryDBQueue,
		ConsolidationProgressReporter & progressReporter )
{
	SecondaryDatabase secondaryDB;

	if (!secondaryDB.init( filePath ))
	{
		return false;
	}

	INFO_MSG( "ConsolidateDBsApp::consolidateSecondaryDB: "
			"Consolidating '%s'\n",
		filePath.c_str() );

	std::string secondaryDBDigest;
	if (!secondaryDB.getChecksumDigest( secondaryDBDigest ))
	{
		return false;
	}

	if (!this->checkEntityDefsDigestMatch( secondaryDBDigest ))
	{
		ERROR_MSG( "ConsolidateDBsApp::consolidateSecondaryDB: "
				"%s failed entity digest check\n", 
			filePath.c_str() );
		return false;
	}

	if (!secondaryDB.consolidate( primaryDBQueue, progressReporter, 
			!shouldStopOnError_, shouldAbort_ ))
	{
		consolidationErrors_.addSecondaryDB( filePath );
	}

	return true;
}


/**
 *	Returns true if the given quoted MD5 digest matches the entity definition
 * 	digest that we've currently loaded
 */
bool ConsolidateDBsApp::checkEntityDefsDigestMatch(
		const std::string& quotedDigest )
{
	MD5::Digest	digest;
	if (!digest.unquote( quotedDigest ))
	{
		ERROR_MSG( "ConsolidateDBsApp::checkEntityDefsDigestMatch: "
				"Not a valid MD5 digest\n" );
		return false;
	}

	return this->entityDefs().getPersistentPropertiesDigest() == digest;
}


/**
 *	This method is used to report a string description of our progress back to
 *	the DBMgr via a watcher.
 */
void ConsolidateDBsApp::onStatus( const std::string & status )
{
	if (pDBMgr_.get() != NULL)
	{
		pDBMgr_->setStatus( status );
	}
}


/**
 *	Handle a received signal.
 */
void ConsolidateDBsApp::onSignalled( int sigNum )
{
	switch (sigNum)
	{
		case SIGINT:
		case SIGHUP:
		{
			INFO_MSG( "ConsolidateDBsApp::onSignalled: "
				"aborting consolidation\n" );
			this->abort();
			break;
		}

		default:
			break;
	}
}


/**
 * 	This method returns true if our entity definitions matches the ones used by
 *	the primary database when the system was last started.
 */
bool ConsolidateDBsApp::checkPrimaryDBEntityDefsMatch()
{
	if (!this->checkPrimaryDBEntityDefsMatchInternal())
	{
		ERROR_MSG( "ConsolidateDBsApp::init: Our entity definitions do not "
				"match the ones used by the primary database\n"
				"Database consolidation should be run before making changes to "
				"entity definitions. Changing entity definitions potentially "
				"invalidates unconsolidated data.\n"
				"Run \"consolidate_dbs --clear\" to allow the server to "
				"run without doing data consolidation. Unconsolidated data "
				"will be lost.\n" );
		return false;
	}

	return true;
}


/**
 * 	This method returns true if our entity definitions matches the ones used by
 *  the primary database when the system was last started.
 */
bool ConsolidateDBsApp::checkPrimaryDBEntityDefsMatchInternal()
{
	try
	{
		MySql & connection = this->connection();

		ResultSet resultSet;
		Query query( "SELECT checksum FROM bigworldEntityDefsChecksum" );

		query.execute( connection, &resultSet );

		std::string checkSum;

		if (resultSet.getResult( checkSum ))
		{
			return this->checkEntityDefsDigestMatch( checkSum );
		}
		else
		{
			ERROR_MSG( "ConsolidateDBsApp::checkEntityDefsMatch: "
					"Checksum table is empty\n" );
		}
	}
	catch (std::exception & e)
	{
		ERROR_MSG( "ConsolidateDBsApp::checkEntityDefsMatch: "
				"Failed to retrieve the primary database "
				"entity definition checksum: %s\n",
			e.what() );
	}

	return false;
}


/**
 *	Aborts the consolidation process.
 */
void ConsolidateDBsApp::abort()
{
	this->dispatcher().breakProcessing();
	shouldAbort_ = true;
}


/**
 * 	This method clears the secondary DB entries from the primary
 * 	database.
 */
bool ConsolidateDBsApp::clearSecondaryDBEntries( uint * pNumEntriesCleared )
{
	try
	{
		MySql & connection = this->connection();
		MySqlTransaction transaction( connection );

		connection.execute( "DELETE FROM bigworldSecondaryDatabases" );

		uint numEntriesCleared = connection.affectedRows();

		if (pNumEntriesCleared)
		{
			*pNumEntriesCleared = numEntriesCleared;
		}

		transaction.commit();

		INFO_MSG( "ConsolidateDBsApp::clearSecondaryDBEntries: "
				"Cleared %u entries from %s:%d (%s)\n",
			numEntriesCleared,
			connectionInfo_.host.c_str(),
			connectionInfo_.port,
			connectionInfo_.database.c_str() );
	}
	catch (std::exception & e)
	{
		ERROR_MSG( "ConsolidateDBsApp::clearSecondaryDBEntries: %s", e.what() );
		return false;
	}

	return true;
}

// db_consolidator.cpp
