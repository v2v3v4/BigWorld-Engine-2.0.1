/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "database_tool_app.hpp"

#include "cstdmf/dprintf.hpp"

#include "entitydef/constants.hpp"

#include "dbmgr_lib/db_entitydefs.hpp"

#include "network/logger_message_forwarder.hpp"
#include "network/machined_utils.hpp"
#include "network/watcher_nub.hpp"

#include "pyscript/py_import_paths.hpp"
#include "pyscript/script.hpp"

#include "resmgr/bwresource.hpp"

#include "server/bwconfig.hpp"
#include "server/bwservice.hpp"


namespace // (anonymous)
{

class DatabaseToolAppSignalHandler : public SignalHandler
{
public:
	DatabaseToolAppSignalHandler( DatabaseToolApp & databaseToolApp ):
			databaseToolApp_( databaseToolApp )
	{}

	virtual ~DatabaseToolAppSignalHandler()
	{}

	virtual void handleSignal( int sigNum )
	{
		databaseToolApp_.onSignalled( sigNum );
	}

private:
	DatabaseToolApp & databaseToolApp_;
};

} // end namespace (anonymous)

/**
 *	Constructor.
 *
 *	@param shouldLock		Whether to lock the database for exclusive access,
 *							or not.
 */
DatabaseToolApp::DatabaseToolApp():
			eventDispatcher_(),
			pSignalHandler_( new DatabaseToolAppSignalHandler( *this ) ),
			pWatcherNub_( 0 ),
			pLoggerMessageForwarder_( 0 ),
			pLockedConn_(),
			entityDefs_()
{
	new SignalProcessor( eventDispatcher_ );
}


/**
 *	Destructor.
 */
DatabaseToolApp::~DatabaseToolApp()
{
	if (!Script::isFinalised())
	{
		Script::fini();
	}

	delete SignalProcessor::pInstance();
}



/**
 *	Default initialisation method.
 */
bool DatabaseToolApp::init( const char * appName, bool isVerbose,
		bool shouldLock/*=true*/,
		const DBConfig::ConnectionInfo & connectionInfo 
			/*=DBConfig::connectionInfo()*/ )
{
	if (!this->initLogger( appName, BWConfig::get( "loggerID", "" ), 
			isVerbose ))
	{
		ERROR_MSG( "Failed to initialise logger\n" );
		return false;
	}

	return this->initScript() &&
		this->initEntityDefs() &&
		this->connect( connectionInfo, shouldLock );
}


/**
 *	Connect to the database server.
 *
 *	@param shouldLock		If true, a named lock is required to be acquired.
 */
bool DatabaseToolApp::connect( const DBConfig::ConnectionInfo & connectionInfo, 
		bool shouldLock )
{
	pLockedConn_.reset( new MySqlLockedConnection( connectionInfo ) );

	if (!pLockedConn_->connect( shouldLock ))
	{
		ERROR_MSG( "Failed to establish a locked connection to the database. "
				"Check other processes connected to the database.\n" );
		return false;
	}
	return true;
}


/**
 *	Optional method to initialise the logger, subclasses can call this in their
 *	initialisation.
 */
bool DatabaseToolApp::initLogger( const char * appName, 
		const std::string & loggerID, bool isVerbose )
{
	// Get the internal IP
	uint32 internalIP;
	if (!Mercury::MachineDaemon::queryForInternalInterface( internalIP ))
	{
		ERROR_MSG( "DatabaseToolApp::initLogger: "
				"failed to query for internal interface\n" );
		return false;
	}

	// Set up the watcher nub and message forwarder manually
	pWatcherNub_.reset( new WatcherNub() );

	if (!pWatcherNub_->init( inet_ntoa( (struct in_addr &)internalIP ), 0 ))
	{
		pWatcherNub_.reset( NULL );
		return false;
	}

	pLoggerMessageForwarder_.reset( 
		new LoggerMessageForwarder( appName, pWatcherNub_->udpSocket(), 
			eventDispatcher_, loggerID,
			/*enabled=*/true, /*spamFilterThreshold=*/0 ) );
	
	DebugFilter::shouldWriteToConsole( true );
	if (!isVerbose)
	{
		DebugFilter::instance().filterThreshold( MESSAGE_PRIORITY_INFO );
	}
	else
	{
		DebugFilter::shouldWriteToConsole( true );
	}

	return true;
}


/**
 *	This method initialises the Python script.
 */
bool DatabaseToolApp::initScript()
{
	PyImportPaths paths;
	paths.addPath( EntityDef::Constants::databasePath() );
	paths.addPath( EntityDef::Constants::serverCommonPath() );

	if (!Script::init( paths, "database", true ))
	{
		ERROR_MSG( "DatabaseToolApp::initScript: "
				"Failed to init script system\n" );
		return false;
	}

	return true;
}


/**
 *	Read in the entity definitions.
 */
bool DatabaseToolApp::initEntityDefs()
{
	DataSectionPtr pSection =
		BWResource::openSection( EntityDef::Constants::entitiesFile() );

	if (!pSection)
	{
		ERROR_MSG( "DatabaseToolApp::initEntityDefs: Failed to open "
				"<res>/%s\n", EntityDef::Constants::entitiesFile() );
		return false;
	}

	if (!entityDefs_.init( pSection ))
	{
		ERROR_MSG( "DatabaseToolApp::initEntityDefs: "
				"failed to read in entity definitions\n" );
		return false;
	}

	return true;
}


/**
 *	Enable handling of the given signal. The virtual method onSignalled() will
 *	be called on detection of that signal.
 */
void DatabaseToolApp::enableSignalHandler( int sigNum, bool enable )
{
	if (enable)
	{
		SignalProcessor::instance().addSignalHandler( sigNum, 
			pSignalHandler_.get() );
	}
	else
	{
		SignalProcessor::instance().clearSignalHandler( sigNum, 
			pSignalHandler_.get() );
	}
}


// database_tool_app.cpp
