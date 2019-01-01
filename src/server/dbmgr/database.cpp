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

#include "database.hpp"

#ifndef CODE_INLINE
#include "database.ipp"
#endif

#include "consolidator.hpp"
#include "custom.hpp"
#include "db_interface.hpp"
#include "dbmgr_config.hpp"
#include "delete_entity_handler.hpp"
#include "entity_auto_loader.hpp"
#include "get_entity_handler.hpp"
#include "load_entity_handler.hpp"
#include "login_app_check_status_reply_handler.hpp"
#include "login_handler.hpp"
#include "look_up_dbid_handler.hpp"
#include "look_up_entity_handler.hpp"
#include "py_billing_system.hpp"
#include "relogon_attempt_handler.hpp"
#include "write_entity_handler.hpp"

#include "common/py_network.hpp"

#include "dbmgr_lib/db_entitydefs.hpp"

#ifdef USE_MYSQL
#include "dbmgr_mysql/mysql_database_creation.hpp"
#endif

#ifdef USE_XML
#include "dbmgr_xml/xml_database.hpp"
#endif

#ifdef USE_CUSTOM_BILLING_SYSTEM
#include "custom_billing_system.hpp"
#endif

#include "baseapp/baseapp_int_interface.hpp"

#include "baseappmgr/baseappmgr_interface.hpp"

#include "common/doc_watcher.hpp"

#include "cstdmf/memory_stream.hpp"

#include "entitydef/constants.hpp"

#include "network/blocking_reply_handler.hpp"
#include "network/channel_sender.hpp"
#include "network/event_dispatcher.hpp"
#include "network/machine_guard.hpp"
#include "network/machined_utils.hpp"
#include "network/portmap.hpp"
#include "network/watcher_nub.hpp"

#include "pyscript/personality.hpp"
#include "pyscript/py_import_paths.hpp"
#include "pyscript/py_traceback.hpp"

#include "resmgr/bwresource.hpp"
#include "resmgr/dataresource.hpp"
#include "resmgr/xml_section.hpp"

#include "server/bwconfig.hpp"
#include "server/reviver_subject.hpp"
#include "server/signal_processor.hpp"
#include "server/util.hpp"
#include "server/writedb.hpp"


DECLARE_DEBUG_COMPONENT( 0 )

/// DBMgr Singleton.
BW_SINGLETON_STORAGE( Database )


extern int UserDataObjectLinkDataType_token;
extern int ResMgr_token;
extern int PyScript_token;

namespace // (anonymous)
{

int s_moduleTokens = ResMgr_token | PyScript_token | UserDataObjectLinkDataType_token;

// -----------------------------------------------------------------------------
// Section: Constants
// -----------------------------------------------------------------------------
const char * UNSPECIFIED_ERROR_STR = "Unspecified error";


// -----------------------------------------------------------------------------
// Section: Functions
// -----------------------------------------------------------------------------

bool commandShutDown( std::string & output, std::string & value )
{
	Database * pDB = Database::pInstance();
	if (pDB != NULL)
	{
		pDB->shutDown();
	}

	return true;
}

} // end anonymous namespace


// -----------------------------------------------------------------------------
// Section: Database
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
Database::Database( Mercury::EventDispatcher & mainDispatcher,
	   Mercury::NetworkInterface & interface ) :
	ServerApp( mainDispatcher, interface ),
	pEntityDefs_( NULL ),
	pDatabase_( NULL ),
	pBillingSystem_( NULL ),
	status_(),
	baseAppMgr_( interface_ ),
	shouldSendInitData_( false ),
	shouldConsolidate_( true ),
	statusCheckTimerHandle_(),
	curLoad_( 1.f ),
	anyCellAppOverloaded_( true ),
	overloadStartTime_( 0 ),
	mailboxRemapCheckCount_( 0 ),
	secondaryDBPrefix_(),
	secondaryDBIndex_( 0 ),
	pConsolidator_( NULL )
{
	// The channel to the BaseAppMgr is irregular
	baseAppMgr_.channel().isLocalRegular( false );
	baseAppMgr_.channel().isRemoteRegular( false );
}


/**
 *	Destructor.
 */
Database::~Database()
{
	interface_.cancelRequestsFor( &baseAppMgr_.channel() );

	statusCheckTimerHandle_.cancel();

	delete pBillingSystem_;
	pBillingSystem_ = NULL;

	delete pDatabase_;
	// Destroy entity descriptions before calling Script::fini() so that it
	// can clean up any PyObjects that it may have.
	delete pEntityDefs_;
	DataType::clearStaticsForReload();
	Script::fini();
	pDatabase_ = NULL;
}


/**
 *	This method initialises the application.
 */
bool Database::init( int argc, char * argv[] )
{
	if (!this->ServerApp::init( argc, argv ))
	{
		return false;
	}

	if (!interface_.isGood())
	{
		ERROR_MSG( "Database::init: Failed to create Nub on internal "
			"interface.\n");
		return false;
	}

	ReviverSubject::instance().init( &interface_, "dbMgr" );

	PyImportPaths paths;
	paths.addPath( EntityDef::Constants::databasePath() );
	paths.addPath( EntityDef::Constants::serverCommonPath() );

	if (!Script::init( paths, "database", true ))
	{
		return false;
	}

	if (!PyNetwork::init( mainDispatcher_, interface_ ))
	{
		return false;
	}

	if (Personality::import( Config::personality() ))
	{
		if (!Personality::callOnInit())
		{
			ERROR_MSG( "Database::init: BWPersonality.onInit() failed.\n" );
			return false;
		}
	}

	if (Config::allowEmptyDigest() && Config::isProduction())
	{
		WARNING_MSG( "Database::init: Production Mode: Allowing client "
			"connections with empty entity definition digests! This is "
			"a potential security risk.\n" );
	}

	BW_INIT_WATCHER_DOC( "dbmgr" );

	status_.set( DBStatus::STARTING, "Loading entity definitions" );

	pEntityDefs_ = new EntityDefs();
	if (!pEntityDefs_->init(
		BWResource::openSection( EntityDef::Constants::entitiesFile() ) ))
	{
		return false;
	}

	INFO_MSG( "Database::init: Expected digest is %s\n",
			pEntityDefs_->getDigest().quote().c_str() );

	pEntityDefs_->debugDump( Config::dumpEntityDescription() );

	// Initialise the watcher
	BW_REGISTER_WATCHER( 0, "dbmgr", "DBMgr", "dbMgr",
			mainDispatcher_, interface_.address() );

	this->addWatchers( Watcher::rootWatcher() );

	std::string databaseType = BWConfig::get( "dbMgr/type", "xml" );

	INFO_MSG( "\tDatabase layer      = %s\n", databaseType.c_str() );

#ifdef USE_XML
	if (databaseType == "xml")
	{
		pDatabase_ = new XMLDatabase();

		shouldConsolidate_ = false;
	} else
#endif

	if (databaseType == "mysql")
	{
#ifdef USE_MYSQL
		pDatabase_ = createMySqlDatabase(
				this->interface(),
				this->mainDispatcher() );

		if (pDatabase_ == NULL)
		{
			return false;
		}
#else
		ERROR_MSG( "DBMgr needs to be rebuilt with MySQL support. See "
				"the Server Installation Guide for more information\n" );
		return false;
#endif
	}
	else
	{
		ERROR_MSG( "Unknown database type: %s\n", databaseType.c_str() );

		return false;
	}

	if ((databaseType == "xml") && Config::isProduction())
	{
		ERROR_MSG(
			"The XML database is suitable for demonstrations and "
			"evaluations only.\n"
			"Please use the MySQL database for serious development and "
			"production systems.\n"
			"See the Server Operations Guide for instructions on how to switch "
			"to the MySQL database.\n" );
	}

	status_.set( DBStatus::STARTING, "Initialising database layer" );

	Mercury::MachineDaemon::registerBirthListener( interface_.address(),
			DBInterface::handleBaseAppMgrBirth, "BaseAppMgrInterface" );

	// find the BaseAppMgr interface
	Mercury::Address baseAppMgrAddr;

	if (Mercury::MachineDaemon::findInterface( "BaseAppMgrInterface", 0,
				baseAppMgrAddr ) == Mercury::REASON_SUCCESS)
	{
		baseAppMgr_.addr( baseAppMgrAddr );

		INFO_MSG( "Database::init: BaseAppMgr at %s\n",
			baseAppMgr_.c_str() );
	}
	else
	{
		INFO_MSG( "Database::init: BaseAppMgr is not ready yet.\n" );
	}

	DBInterface::registerWithInterface( interface_ );

	Mercury::MachineDaemon::registerBirthListener( interface_.address(),
			DBInterface::handleDatabaseBirth, "DBInterface" );

	bool isRecover = false;

	// We are in recovery mode if BaseAppMgr has already started
	if (baseAppMgr_.addr() != Mercury::Address::NONE)
	{
		Mercury::BlockingReplyHandlerWithResult <bool> handler( interface_ );
		Mercury::Bundle & bundle = baseAppMgr_.bundle();

		bundle.startRequest( BaseAppMgrInterface::requestHasStarted, &handler );
		baseAppMgr_.send();

		if (handler.waitForReply( &baseAppMgr_.channel() ) ==
				Mercury::REASON_SUCCESS)
		{
			isRecover = handler.get();
		}

		shouldSendInitData_ = !isRecover;
	}

	if (DBInterface::registerWithMachined( interface_, 0 ) !=
			Mercury::REASON_SUCCESS)
	{
		ERROR_MSG( "Database::init: Unable to register with interface. "
				"Is machined running?\n" );
		return false;
	}


	if (!pDatabase_->startup( this->getEntityDefs(),
				mainDispatcher_, isRecover ))
	{
		return false;
	}

	if (!this->initBillingSystem())
	{
		ERROR_MSG( "Database::init: Failed to initialise billing system\n" );
		return false;
	}

	if (shouldConsolidate_)
	{
		this->initSecondaryDBPrefix();
	}

	if (isRecover)
	{
		this->startServerBegin( true );
	}
	else if (shouldConsolidate_)
	{
		this->consolidateData();
	}
	else
	{
		status_.set( DBStatus::WAITING_FOR_APPS, "Waiting for other "
				"components to become ready" );
	}

	// A one second timer to check all sorts of things.
	statusCheckTimerHandle_ = mainDispatcher_.addTimer( 1000000, this );

	Script::initExceptionHook( &mainDispatcher_ );

	INFO_MSG( "Address = %s\n", interface_.address().c_str() );
	INFO_MSG( "Recover database = %s\n",
		isRecover ? "True" : "False" );

	return true;
}


/**
 *	This method initialises the prefix should for the secondary database files.
 */
void Database::initSecondaryDBPrefix()
{
	// Generate the run ID.
	// Theoretically, using local time will not generate a unique run ID
	// across daylight savings transitions but good enough.
	time_t epochTime = ::time( NULL );
	tm timeAndDate;
	localtime_r( &epochTime, &timeAndDate );

	// Get username for run ID
	uid_t		uid = getuid();
	passwd *	pUserDetail = getpwuid( uid );
	std::string username;

	if (pUserDetail)
	{
		username = pUserDetail->pw_name;
	}
	else
	{
		WARNING_MSG( "Database::init: Using '%hd' as the username due to uid "
				"to name lookup failure\n", uid );
		std::stringstream ss;
		ss << uid;
		username = ss.str();
	}

	// Really generate run ID
	char runIDBuf[ BUFSIZ ];
	snprintf( runIDBuf, sizeof(runIDBuf),
				"%s_%04d%02d%02d_%02d%02d%02d",
				username.c_str(),
				timeAndDate.tm_year + 1900, timeAndDate.tm_mon + 1,
				timeAndDate.tm_mday,
				timeAndDate.tm_hour, timeAndDate.tm_min, timeAndDate.tm_sec );
	secondaryDBPrefix_ = runIDBuf;

	INFO_MSG( "Database::initSecondaryDBPrefix: \"%s\"\n",
			secondaryDBPrefix_.c_str() );
}


/**
 *	This method initialises the connetion with the billing system.
 */
bool Database::initBillingSystem()
{
#ifdef USE_CUSTOM_BILLING_SYSTEM
	pBillingSystem_ = new CustomBillingSystem( *pEntityDefs_ );
#else
	// Decref'ed by call to Script::ask.
	PyObject * pFunc = NULL;

	if (Personality::instance())
	{
		pFunc = PyObject_GetAttrString( Personality::instance(),
							"connectToBillingSystem" );
	}

	if (pFunc)
	{
		PyObjectPtr pPyBillingSystem( Script::ask( pFunc, PyTuple_New( 0 ) ),
				PyObjectPtr::STEAL_REFERENCE );

		if (!pPyBillingSystem)
		{
			// Python error already printed by Script::ask.
			ERROR_MSG( "Database::initBillingSystem: "
					"Failed. See script errors\n" );
			return false;
		}

		if (pPyBillingSystem.get() != Py_None)
		{
			pBillingSystem_ = new PyBillingSystem( pPyBillingSystem.get(),
					*pEntityDefs_ );
		}
	}
	else
	{
		PyErr_Clear();
	}

	if (pBillingSystem_ == NULL)
	{
		pBillingSystem_ = pDatabase_->createBillingSystem();
	}
#endif

	return pBillingSystem_->isOkay();
}


/**
 *	This method adds the watchers associated with this object.
 */
void Database::addWatchers( Watcher & watcher )
{
	this->ServerApp::addWatchers( watcher );

	watcher.addChild( "isReady", makeWatcher( &Database::isReady ), this );

	status_.registerWatchers( watcher );

	watcher.addChild( "hasStartBegun",
		makeWatcher( MF_ACCESSORS( bool, Database, hasStartBegun ) ), this );

	watcher.addChild( "load", makeWatcher( &Database::curLoad_ ), this );

	watcher.addChild( "anyCellAppOverloaded",
			makeWatcher( &Database::anyCellAppOverloaded_ ), this );

	// Command watcher to shutdown DBMgr
	watcher.addChild( "command/shutDown",
			new NoArgFuncCallableWatcher( commandShutDown,
				CallableWatcher::LOCAL_ONLY,
				"Shuts down DBMgr" ) );
}


/**
 *	This method runs the database.
 */
bool Database::run()
{
	mainDispatcher_.processUntilBreak();

	this->finalise();

	return true;
}


/**
 *	This method performs some clean-up at the end of the shut down process.
 */
void Database::finalise()
{
	if (pDatabase_)
	{
		pDatabase_->shutDown();
	}
}


/**
 *	This method starts the data consolidation process.
 */
void Database::consolidateData()
{
	if (status_.status() <= DBStatus::STARTING)
	{
		status_.set( DBStatus::STARTUP_CONSOLIDATING, "Consolidating data" );
	}
	else if (status_.status() >= DBStatus::SHUTTING_DOWN)
	{
		status_.set( DBStatus::SHUTDOWN_CONSOLIDATING, "Consolidating data" );
	}
	else
	{
		CRITICAL_MSG( "Database::consolidateData: Not a valid state to be "
				"running data consolidation!" );
		return;
	}

	uint32 numSecondaryDBs = pDatabase_->numSecondaryDBs();
	if (numSecondaryDBs > 0)
	{
		TRACE_MSG( "Starting data consolidation\n" );
		if (!this->startConsolidationProcess())
		{
			this->onConsolidateProcessEnd( false );
		}
	}
	else
	{
		this->onConsolidateProcessEnd( true );
	}
}

/**
 *	This method runs an external command to consolidate data from secondary
 *	databases.
 */
bool Database::startConsolidationProcess()
{
	if (this->isConsolidating())
	{
		TRACE_MSG( "Database::startConsolidationProcess: Ignoring second "
				"attempt to consolidate data while data consolidation is "
				"already in progress\n" );
		return false;
	}

	pConsolidator_.reset( new Consolidator( *this ) );

	return true;
}


/**
 *	This method is called when the consolidation process exits.
 */
void Database::onConsolidateProcessEnd( bool isOK )
{
	pConsolidator_.reset();

	if (status_.status() == DBStatus::STARTUP_CONSOLIDATING)
	{
		if (isOK)
		{
			status_.set( DBStatus::WAITING_FOR_APPS,
					"Waiting for other components to become ready" );
		}
		else
		{
			// Prevent trying to consolidate again during controlled shutdown.
			shouldConsolidate_ = false;

			this->startSystemControlledShutdown();
		}
	}
	else if (status_.status() == DBStatus::SHUTDOWN_CONSOLIDATING)
	{
		this->shutDown();
	}
	else
	{
		CRITICAL_MSG( "Database::onConsolidateProcessEnd: "
				"Invalid state %d at the end of data consolidation\n", 
			status_.status() );
	}
}


/**
 *	This method handles the checkStatus messages request from the LoginApp.
 */
void Database::checkStatus( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data )
{
	Mercury::ChannelSender sender( this->baseAppMgr().channel() );

	sender.bundle().startRequest( BaseAppMgrInterface::checkStatus,
		   new LoginAppCheckStatusReplyHandler( srcAddr, header.replyID ) );
}


/**
 *	This method handles the replies from the checkStatus requests.
 */
void Database::handleStatusCheck( BinaryIStream & data )
{
	bool isOkay;
	uint32 numBaseApps = 0;
	uint32 numCellApps = 0;
	data >> isOkay >> numBaseApps >> numCellApps;
	INFO_MSG( "Database::handleStatusCheck: "
				"baseApps = %u/%u. cellApps = %u/%u\n",
			  std::max( uint32(0), numBaseApps ), Config::desiredBaseApps(),
			  std::max( uint32(0), numCellApps ), Config::desiredCellApps() );

	// Ignore other status information
	data.finish();

	if ((status_.status() <= DBStatus::WAITING_FOR_APPS) &&
			!data.error() &&
			(numBaseApps >= Config::desiredBaseApps()) &&
			(numCellApps >= Config::desiredCellApps()))
	{
		this->startServerBegin();
	}
}


/**
 *	This method handles the checkStatus request's reply.
 */
class CheckStatusReplyHandler : public Mercury::ShutdownSafeReplyMessageHandler
{
public:
	CheckStatusReplyHandler( Database & db ) : db_( db ) {}

private:
	virtual void handleMessage( const Mercury::Address & /*srcAddr*/,
			Mercury::UnpackedMessageHeader & /*header*/,
			BinaryIStream & data, void * /*arg*/ )
	{
		db_.handleStatusCheck( data );
		delete this;
	}

	virtual void handleException( const Mercury::NubException & /*ne*/,
		void * /*arg*/ )
	{
		delete this;
	}

	Database & db_;
};


/**
 *	This method handles a secondary database registration message.
 */
void Database::secondaryDBRegistration( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header, BinaryIStream & data )
{
	IDatabase::SecondaryDBEntry	secondaryDBEntry;

	data >> secondaryDBEntry.addr >> secondaryDBEntry.appID;

	data >> secondaryDBEntry.location;
	pDatabase_->addSecondaryDB( secondaryDBEntry );
}


/**
 *	This method handles an update secondary database registration message.
 *	Secondary databases registered by a BaseApp not in the provided list are
 *	deleted.
 */
void Database::updateSecondaryDBs( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header, BinaryIStream & data )
{
	BaseAppIDs ids;
	data >> ids;

	pDatabase_->updateSecondaryDBs( ids, *this );
	// updateSecondaryDBs() calls onUpdateSecondaryDBsComplete() it completes.
}


/**
 *	This method handles the request to get information for creating a new
 *	secondary database. It replies with the name of the new database.
 */
void Database::getSecondaryDBDetails( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header, BinaryIStream & data )
{
	Mercury::ChannelSender sender( Database::getChannel( srcAddr ) );
	Mercury::Bundle & bundle = sender.bundle();
	bundle.startReply( header.replyID );

	if (!secondaryDBPrefix_.empty())
	{
		char buf[ BUFSIZ ];
		++secondaryDBIndex_;
		snprintf( buf, sizeof( buf ), "%s-%d.db",
				secondaryDBPrefix_.c_str(),
				secondaryDBIndex_ );

		bundle << buf;
	}
	else
	{
		// An empty string indicates that secondary databases are disabled
		bundle << "";
	}
}


/**
 *	Deletes secondary databases whose registration have just been removed.
 */
void Database::onUpdateSecondaryDBsComplete(
		const IDatabase::SecondaryDBEntries& removedEntries )
{
	for (IDatabase::SecondaryDBEntries::const_iterator pEntry =
			removedEntries.begin(); pEntry != removedEntries.end(); ++pEntry )
	{
		if (this->sendRemoveDBCmd( pEntry->addr.ip, pEntry->location ))
		{
			TRACE_MSG( "Database::onUpdateSecondaryDBsComplete: "
					"Deleting secondary database file %s on %s\n",
					pEntry->location.c_str(), pEntry->addr.ipAsString() );
		}
		else
		{
			ERROR_MSG( "Database::onUpdateSecondaryDBsComplete: Failed to "
					"delete secondary database file %s on %s. It should be "
					"manually deleted to prevent disk space exhaustion.\n",
					pEntry->location.c_str(), pEntry->addr.ipAsString() );
		}
	}
}


/**
 *	This method sends a message to the destination BWMachined that will cause
 *	the database at dbLocation to be removed.
 */
bool Database::sendRemoveDBCmd( uint32 destIP, const std::string & dbLocation )
{
	CreateWithArgsMessage cm;
	cm.uid_ = getUserId();
#if defined _DEBUG
	cm.config_ = "Debug";
#elif defined _HYBRID
	cm.config_ = "Hybrid";
#endif
	cm.recover_ = 0;
	cm.name_ = "commands/remove_db";
	cm.fwdIp_ = 0;
	cm.fwdPort_ = 0;

	cm.args_.push_back( dbLocation );

	Endpoint ep;
	ep.socket( SOCK_DGRAM );

	return (ep.good() && (ep.bind() == 0) &&
			cm.sendto( ep, htons( PORT_MACHINED ), destIP ));
}


/**
 *	This method handles timer events. It is called every second.
 */
void Database::handleTimeout( TimerHandle handle, void * arg )
{
	// See if we should send initialisation data to BaseAppMgr
	if (shouldSendInitData_)
	{
		shouldSendInitData_ = false;
		this->sendInitData();
	}

	// See if we are ready to start.
	if (baseAppMgr_.channel().isEstablished() &&
			(status_.status() == DBStatus::WAITING_FOR_APPS))
	{
		Mercury::Bundle & bundle = baseAppMgr_.bundle();

		bundle.startRequest( BaseAppMgrInterface::checkStatus,
			   new CheckStatusReplyHandler( *this ) );

		baseAppMgr_.send();

		mainDispatcher_.clearSpareTime();
	}

	// Update our current load so we can know whether or not we are overloaded.
	if (status_.status() > DBStatus::WAITING_FOR_APPS)
	{
		uint64 spareTime = mainDispatcher_.getSpareTime();
		mainDispatcher_.clearSpareTime();

		curLoad_ = 1.f - float( double(spareTime) / stampsPerSecondD() );

		// TODO: Consider asking DB implementation if it is overloaded too...
	}

	// Check whether we should end our remapping of mailboxes for a dead BaseApp
	if (--mailboxRemapCheckCount_ == 0)
		this->endMailboxRemapping();
}


// -----------------------------------------------------------------------------
// Section: Database lifetime
// -----------------------------------------------------------------------------

/**
 *	This method is called when a new BaseAppMgr is started.
 */
void Database::handleBaseAppMgrBirth( 
		const DBInterface::handleBaseAppMgrBirthArgs & args )
{
	baseAppMgr_.addr( args.addr );

	INFO_MSG( "Database::handleBaseAppMgrBirth: BaseAppMgr is at %s\n",
		baseAppMgr_.c_str() );

	if (status_.status() < DBStatus::SHUTTING_DOWN)
	{
		shouldSendInitData_ = true;
	}
}


/**
 *	This method is called when a new DbMgr is started.
 */
void Database::handleDatabaseBirth(
		const DBInterface::handleDatabaseBirthArgs & args )
{
	if (args.addr != interface_.address())
	{
		WARNING_MSG( "Database::handleDatabaseBirth: %s\n", args.addr.c_str() );
		this->shutDown();	// Don't consolidate
	}
}


/**
 *	This method handles the shutDown message.
 */
void Database::shutDown( const DBInterface::shutDownArgs & /*args*/ )
{
	this->shutDown();
}


/**
 *	This method starts a controlled shutdown for the entire system.
 */
void Database::startSystemControlledShutdown()
{
	if (baseAppMgr_.channel().isEstablished())
	{
		BaseAppMgrInterface::controlledShutDownArgs args;
		args.stage = SHUTDOWN_TRIGGER;
		args.shutDownTime = 0;
		baseAppMgr_.bundle() << args;
		baseAppMgr_.send();
	}
	else
	{
		WARNING_MSG( "Database::startSystemControlledShutdown: "
				"No known BaseAppMgr, only shutting down self\n" );
		this->shutDownNicely();
	}
}


/**
 *	This method starts shutting down DBMgr.
 */
void Database::shutDownNicely()
{
	if (status_.status() >= DBStatus::SHUTTING_DOWN)
	{
		WARNING_MSG( "Database::shutDownNicely: Ignoring second shutdown\n" );
		return;
	}

	TRACE_MSG( "Database::shutDownNicely: Shutting down\n" );

	status_.set( DBStatus::SHUTTING_DOWN, "Shutting down" );

	interface_.processUntilChannelsEmpty();

	if (shouldConsolidate_)
	{
		this->consolidateData();
	}
	else
	{
		this->shutDown();
	}
}


/**
 *	This method shuts this process down.
 */
void Database::shutDown()
{
	TRACE_MSG( "Database::shutDown\n" );

	mainDispatcher_.breakProcessing();
}


/**
 *	This method handles telling us to shut down in a controlled manner.
 */
void Database::controlledShutDown(
		const DBInterface::controlledShutDownArgs & args )
{
	DEBUG_MSG( "Database::controlledShutDown: stage = %s\n", 
		ServerApp::shutDownStageToString( args.stage ) );

	switch (args.stage)
	{
	case SHUTDOWN_REQUEST:
	{
		// Make sure we no longer send to anonymous channels etc.
		interface_.stopPingingAnonymous();

		if (baseAppMgr_.channel().isEstablished())
		{
			BaseAppMgrInterface::controlledShutDownArgs args;
			args.stage = SHUTDOWN_REQUEST;
			args.shutDownTime = 0;
			baseAppMgr_.bundle() << args;
			baseAppMgr_.send();
		}
		else
		{
			WARNING_MSG( "Database::controlledShutDown: "
					"No BaseAppMgr. Proceeding to shutdown immediately\n" );
			this->shutDownNicely();
		}
	}
	break;

	case SHUTDOWN_PERFORM:
		this->shutDownNicely();
		break;

	default:
		ERROR_MSG( "Database::controlledShutDown: Stage %s not handled.\n",
			ServerApp::shutDownStageToString( args.stage ) );
		break;
	}
}


/**
 *	This method handles telling us that a CellApp is overloaded.
 */
void Database::cellAppOverloadStatus( 
		const DBInterface::cellAppOverloadStatusArgs & args )
{
	anyCellAppOverloaded_ = args.anyOverloaded;
}


// -----------------------------------------------------------------------------
// Section: IDatabase intercept methods
// -----------------------------------------------------------------------------

/**
 *	This method is meant to be called instead of IDatabase::getEntity() so that
 *	we can muck around with stuff before passing it to IDatabase.
 */
void Database::getEntity( const EntityDBKey & entityKey,
			BinaryOStream * pStream,
			bool shouldGetBaseEntityLocation,
			const char * pPasswordOverride,
			GetEntityHandler & handler )
{
	pDatabase_->getEntity( entityKey, pStream, shouldGetBaseEntityLocation,
			pPasswordOverride, handler );
}


/**
 *	This method is meant to be called instead of IDatabase::putEntity() so that
 *	we can muck around with stuff before passing it to IDatabase.
 */
void Database::putEntity( const EntityKey & entityKey,
		EntityID entityID,
		BinaryIStream * pStream,
		EntityMailBoxRef * pBaseMailbox,
		bool removeBaseMailbox,
		UpdateAutoLoad updateAutoLoad,
		IDatabase::IPutEntityHandler& handler )
{
	// Update mailbox for dead BaseApps.
	if (this->hasMailboxRemapping() && pBaseMailbox)
	{
		// Update mailbox for dead BaseApps.
		this->remapMailbox( *pBaseMailbox );
	}

	pDatabase_->putEntity( entityKey, entityID,
			pStream, pBaseMailbox, removeBaseMailbox, updateAutoLoad, handler );
}


/**
 *	This method is meant to be called instead of IDatabase::delEntity() so that
 *	we can muck around with stuff before passing it to IDatabase.
 */
void Database::delEntity( const EntityDBKey & ekey, EntityID entityID,
		IDatabase::IDelEntityHandler& handler )
{
	pDatabase_->delEntity( ekey, entityID, handler );
}


// -----------------------------------------------------------------------------
// Section: Entity entry database requests
// -----------------------------------------------------------------------------

/**
 *	This method handles a logOn request.
 */
void Database::logOn( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data )
{
	Mercury::Address addrForProxy;
	bool offChannel;
	LogOnParamsPtr pParams = new LogOnParams();

	data >> addrForProxy >> offChannel >> *pParams;

	const MD5::Digest & digest = pParams->digest();
	bool goodDigest = (digest == this->getEntityDefs().getDigest());

	if (!goodDigest && Config::allowEmptyDigest() && digest.isEmpty())
	{
		goodDigest = true;

		WARNING_MSG( "Database::logOn: %s logged on with empty digest.\n",
				pParams->username().c_str() );
	}

	if (!goodDigest)
	{
		ERROR_MSG( "Database::logOn: Incorrect digest\n" );
		this->sendFailure( header.replyID, srcAddr, offChannel,
			LogOnStatus::LOGIN_REJECTED_BAD_DIGEST,
			"Defs digest mismatch." );
		return;
	}

	this->logOn( srcAddr, header.replyID, pParams, addrForProxy, offChannel );
}


/**
 *	This method attempts to log on a player.
 */
void Database::logOn( const Mercury::Address & srcAddr,
		Mercury::ReplyID replyID,
		LogOnParamsPtr pParams,
		const Mercury::Address & addrForProxy,
		bool offChannel )
{
	if (status_.status() != DBStatus::RUNNING)
	{
		INFO_MSG( "Database::logOn: "
			"Login failed for %s. Server not ready.\n",
			pParams->username().c_str() );

		this->sendFailure( replyID, srcAddr, offChannel,
			LogOnStatus::LOGIN_REJECTED_SERVER_NOT_READY,
			"Server not ready." );

		return;
	}

	bool isOverloaded = curLoad_ > Config::overloadLevel();

	if (this->calculateOverloaded( isOverloaded ))
	{
		INFO_MSG( "Database::logOn: "
				"Login failed for %s. We are overloaded "
				"(load=%.02f > max=%.02f)\n",
			pParams->username().c_str(), curLoad_, Config::overloadLevel() );

		this->sendFailure( replyID, srcAddr, offChannel,
			LogOnStatus::LOGIN_REJECTED_DBMGR_OVERLOAD,
			"DBMgr is overloaded." );

		return;
	}

	if (anyCellAppOverloaded_)
	{
		INFO_MSG( "Database::logOn: "
			"Login failed for %s. At least one CellApp is overloaded.\n",
			pParams->username().c_str() );

		this->sendFailure( replyID, srcAddr, offChannel,
			LogOnStatus::LOGIN_REJECTED_CELLAPP_OVERLOAD,
			"At least one CellApp is overloaded." );

		return;
	}

	LoginHandler * pHandler =
		new LoginHandler( pParams, addrForProxy, srcAddr, offChannel, replyID );

	pHandler->login();
}


/**
 *	This method performs checks to see whether we should see ourselves as being
 *	overloaded.
 */
bool Database::calculateOverloaded( bool isOverloaded )
{
	if (!isOverloaded)
	{
		// We're not overloaded, stop the overload timer.
		overloadStartTime_ = 0;
		return false;
	}

	uint64 overloadTime;

	// Start rate limiting logins
	if (overloadStartTime_ == 0)
		overloadStartTime_ = timestamp();

	overloadTime = timestamp() - overloadStartTime_;
	INFO_MSG( "DBMgr::Overloaded for %"PRIu64"ms\n",
				overloadTime/(stampsPerSecond()/1000) );

	return (overloadTime >= Config::overloadTolerancePeriodInStamps());
}


/**
 *	This method is called when there is a log on request for an entity that is
 *	already logged on.
 */
void Database::onLogOnLoggedOnUser( EntityTypeID typeID, DatabaseID dbID,
	LogOnParamsPtr pParams,
	const Mercury::Address & clientAddr, const Mercury::Address & replyAddr,
	bool offChannel, Mercury::ReplyID replyID,
	const EntityMailBoxRef* pExistingBase )
{
	// TODO: Could add a config option to speedup denial of relogon attempts
	//       eg <shouldAttemptRelogon>

	if (this->getInProgRelogonAttempt( typeID, dbID ) != NULL)
	{
		// Another re-logon already in progress.
		INFO_MSG( "Database::logOn: %s already logged on\n",
			pParams->username().c_str() );

		this->sendFailure( replyID, replyAddr, offChannel,
				LogOnStatus::LOGIN_REJECTED_ALREADY_LOGGED_IN,
				"A relogin of same name still in progress." );
		return;
	}

	if (!GetEntityHandler::isActiveMailBox( pExistingBase ))
	{
		// Another logon still in progress.
		WARNING_MSG( "Database::logOn: %s already logging in\n",
			pParams->username().c_str() );

		this->sendFailure( replyID, replyAddr, offChannel,
			LogOnStatus::LOGIN_REJECTED_ALREADY_LOGGED_IN,
		   "Another login of same name still in progress." );
		return;
	}

	// Log on to existing base
	Mercury::ChannelSender sender(
		Database::getChannel( pExistingBase->addr ) );

	Mercury::Bundle & bundle = sender.bundle();
	bundle.startRequest( BaseAppIntInterface::logOnAttempt,
		new RelogonAttemptHandler( pExistingBase->type(), dbID,
			replyAddr, offChannel, replyID, pParams, clientAddr ) );

	bundle << pExistingBase->id;
	bundle << clientAddr;
	bundle << pParams->encryptionKey();

	bool hasPassword = this->getEntityDefs().entityTypeHasPassword( typeID );

	bundle << hasPassword;

	if (hasPassword)
	{
		bundle << pParams->password();
	}
}


/*
 *	This method creates a default entity (via createNewEntity() in custom.cpp)
 *	and serialises it into the stream.
 *
 *	@param	The type of entity to create.
 *	@param	The name of the entity (for entities with a name property)
 *	@param	The stream to serialise entity into.
 *	@param	If non-NULL, this will override the "password" property of
 *	the entity.
 *	@return	True if successful.
 */
bool Database::defaultEntityToStrm( EntityTypeID typeID,
	const std::string & name, BinaryOStream & strm,
	const std::string * pPassword ) const
{
	DataSectionPtr pSection = createNewEntity( typeID, name );
	bool isCreated = pSection.exists();
	if (isCreated)
	{
		if (pPassword)
		{
			if (this->getEntityDefs().getPropertyType( typeID, "password" ) == "BLOB")
				pSection->writeBlob( "password", *pPassword );
			else
				pSection->writeString( "password", *pPassword );
		}

		const EntityDescription& desc =
			this->getEntityDefs().getEntityDescription( typeID );
		desc.addSectionToStream( pSection, strm,
			EntityDescription::BASE_DATA | EntityDescription::CELL_DATA |
			EntityDescription::ONLY_PERSISTENT_DATA );

		if (desc.hasCellScript())
		{
			Vector3	defaultVec( 0, 0, 0 );

			strm << defaultVec;	// position
			strm << defaultVec;	// direction
			strm << SpaceID(0);	// space ID
		}
	}

	return isCreated;
}


/*
 *	This method inserts the "header" info into the bundle for a
 *	BaseAppMgrInterface::createEntity message, up till the point where entity
 *	properties should begin.
 *
 *	@return	If dbID is 0, then this function returns the position in the
 *	bundle where you should put the DatabaseID.
 */
DatabaseID* Database::prepareCreateEntityBundle( EntityTypeID typeID,
		DatabaseID dbID, const Mercury::Address & addrForProxy,
		Mercury::ReplyMessageHandler * pHandler, Mercury::Bundle & bundle,
		LogOnParamsPtr pParams )
{
	bundle.startRequest( BaseAppMgrInterface::createEntity, pHandler, 0,
		Mercury::DEFAULT_REQUEST_TIMEOUT + 1000000 ); // 1 second extra

	// This data needs to match BaseAppMgr::createBaseWithCellData.
	bundle	<< EntityID( 0 ) << typeID;

	DatabaseID *	pDbID = NULL;

	if (dbID)
	{
		bundle << dbID;
	}
	else
	{
		pDbID = reinterpret_cast< DatabaseID * >(
					bundle.reserve( sizeof( *pDbID ) ) );
	}

	// This is the client address. It is used if we are making a proxy.
	bundle << addrForProxy;

	bundle << ((pParams != NULL) ? pParams->encryptionKey() : "");

	bundle << true;		// Has persistent data only

	return pDbID;
}


/**
 *	This helper method sends a failure reply.
 */
void Database::sendFailure( Mercury::ReplyID replyID,
		const Mercury::Address & dstAddr, bool offChannel,
		LogOnStatus reason, const char * pDescription )
{
	MF_ASSERT( reason != LogOnStatus::LOGGED_ON );

	Mercury::Bundle * pBundle;

	if (offChannel)
	{
		pBundle = new Mercury::Bundle();
	}
	else
	{
		Mercury::ChannelSender sender( Database::getChannel( dstAddr ) );
		pBundle = &sender.bundle();
	}

	Mercury::Bundle & bundle = *pBundle;

	bundle.startReply( replyID );
	bundle << uint8( reason );

	if (pDescription == NULL)
	{
		pDescription = UNSPECIFIED_ERROR_STR;
	}

	bundle << pDescription;

	if (offChannel)
	{
		this->interface().send( dstAddr, bundle );
		delete pBundle;
	}
}


/**
 *	This method handles the writeEntity mercury message.
 */
void Database::writeEntity( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data )
{
	AUTO_SCOPED_PROFILE( "writeEntity" );

	int8 flags;
	data >> flags;
	// if this fails then the calling component had no need to call us
	MF_ASSERT( flags &
			(WRITE_BASE_CELL_DATA | WRITE_LOG_OFF | WRITE_AUTO_LOAD_MASK) );

	EntityDBKey	ekey( 0, 0 );
	data >> ekey.typeID >> ekey.dbID;

	// TRACE_MSG( "Database::writeEntity: %lld flags=%i\n",
	//		   ekey.dbID, flags );

	bool isOkay = this->getEntityDefs().isValidEntityType( ekey.typeID );

	if (!isOkay)
	{
		ERROR_MSG( "Database::writeEntity: Invalid entity type %d\n",
			ekey.typeID );

		if (header.flags & Mercury::Packet::FLAG_HAS_REQUESTS)
		{
			Mercury::ChannelSender sender( Database::getChannel( srcAddr ) );
			sender.bundle().startReply( header.replyID );
			sender.bundle() << isOkay << ekey.dbID;
		}
	}
	else
	{
		EntityID entityID;
		data >> entityID;

		WriteEntityHandler* pHandler =
			new WriteEntityHandler( ekey, entityID, flags,
				(header.flags & Mercury::Packet::FLAG_HAS_REQUESTS),
				header.replyID, srcAddr );

		if (flags & WRITE_DELETE_FROM_DB)
		{
			pHandler->deleteEntity();
		}
		else
		{
			pHandler->writeEntity( data, entityID );
		}
	}
}


/**
 *	This method is called when we've just logged off an entity.
 *
 *	@param	typeID The type ID of the logged off entity.
 *	@param	dbID The database ID of the logged off entity.
 */
void Database::onEntityLogOff( EntityTypeID typeID, DatabaseID dbID )
{
	// Notify any re-logon handler waiting on this entity that it has gone.
	RelogonAttemptHandler * pHandler =
			this->getInProgRelogonAttempt( typeID, dbID );

	if (pHandler)
	{
		pHandler->onEntityLogOff();
	}
}


/**
 *	This method handles a message to load an entity from the database.
 */
void Database::loadEntity( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & input )
{
	EntityDBKey	ekey( 0, 0 );
	DataSectionPtr pSection;
	bool byName;
	EntityID entityID;
	input >> ekey.typeID >> entityID >> byName;

	if (!this->getEntityDefs().isValidEntityType( ekey.typeID ))
	{
		ERROR_MSG( "Database::loadEntity: Invalid entity type %d\n",
			ekey.typeID );
		this->sendFailure( header.replyID, srcAddr, false,
			LogOnStatus::LOGIN_CUSTOM_DEFINED_ERROR,
			"Invalid entity type" );
		return;
	}

	if (byName)
	{
		input >> ekey.name;
	}
	else
	{
		input >> ekey.dbID;
	}

	LoadEntityHandler * pHandler =
		new LoadEntityHandler( ekey, srcAddr, entityID, header.replyID );
	pHandler->loadEntity();
}


/**
 *	This message deletes the specified entity if it exists and is not checked
 *	out.
 *
 *	On success, an empty reply is sent. If the entity is currently active, its
 *	mailbox is returned. If the entity did not exist, -1 is return as int32.
 */
void Database::deleteEntity( const Mercury::Address & srcAddr,
	const Mercury::UnpackedMessageHeader & header,
	const DBInterface::deleteEntityArgs & args )
{
	DeleteEntityHandler* pHandler = new DeleteEntityHandler( args.entityTypeID,
			args.dbid, srcAddr, header.replyID );
	pHandler->deleteEntity();
}


/**
 *	This message deletes the specified entity if it exists and is not
 *	checked out.
 *
 *	On success, an empty reply is sent. If the entity is currently active, its
 *	mailbox is returned. If the entity did not exist, -1 is return as int32.
 */
void Database::deleteEntityByName( const Mercury::Address & srcAddr,
	const Mercury::UnpackedMessageHeader & header,
	BinaryIStream & data )
{
	EntityTypeID	entityTypeID;
	std::string		name;
	data >> entityTypeID >> name;

	DeleteEntityHandler * pHandler = new DeleteEntityHandler( entityTypeID,
			name, srcAddr, header.replyID );
	pHandler->deleteEntity();
}


/**
 *	This message looks up the specified entity if it exists and is checked
 *	out and returns a mailbox to it. If it is not checked out it returns
 *	an empty message. If it does not exist, it returns -1 as an int32.
 */
void Database::lookupEntity( const Mercury::Address & srcAddr,
	const Mercury::UnpackedMessageHeader & header,
	const DBInterface::lookupEntityArgs & args )
{
	LookUpEntityHandler* pHandler = new LookUpEntityHandler(
		srcAddr, header.replyID, args.offChannel );
	pHandler->lookUpEntity( args.entityTypeID, args.dbid );
}


/**
 *	This message looks up the specified entity if it exists and is checked
 *	out and returns a mailbox to it. If it is not checked out it returns
 *	an empty message. If it does not exist, it returns -1 as an int32.
 */
void Database::lookupEntityByName( const Mercury::Address & srcAddr,
	const Mercury::UnpackedMessageHeader & header,
	BinaryIStream & data )
{
	EntityTypeID	entityTypeID;
	std::string		name;
	bool			offChannel;
	data >> entityTypeID >> name >> offChannel;
	LookUpEntityHandler* pHandler = new LookUpEntityHandler(
		srcAddr, header.replyID, offChannel );
	pHandler->lookUpEntity( entityTypeID, name );
}


/**
 *	This message looks up the DBID of the entity. The DBID will be 0 if the
 *	entity does not exist.
 */
void Database::lookupDBIDByName( const Mercury::Address & srcAddr,
	const Mercury::UnpackedMessageHeader & header, BinaryIStream & data )
{
	EntityTypeID	entityTypeID;
	std::string		name;
	data >> entityTypeID >> name;

	LookUpDBIDHandler* pHandler =
							new LookUpDBIDHandler( srcAddr, header.replyID );
	pHandler->lookUpDBID( entityTypeID, name );
}


// -----------------------------------------------------------------------------
// Section: Miscellaneous database requests
// -----------------------------------------------------------------------------

/**
 *	This class represents a request to execute a raw database command.
 */
class ExecuteRawCommandHandler : public IDatabase::IExecuteRawCommandHandler
{
public:
	ExecuteRawCommandHandler( Database & db,
			const Mercury::Address srcAddr,
			Mercury::ReplyID replyID ) :
		db_( db ), replyBundle_(), srcAddr_(srcAddr)
	{
		replyBundle_.startReply(replyID);
	}
	virtual ~ExecuteRawCommandHandler() {}

	void executeRawCommand( const std::string& command )
	{
		db_.getIDatabase().executeRawCommand( command, *this );
	}

	// IDatabase::IExecuteRawCommandHandler overrides
	virtual BinaryOStream& response()	{	return replyBundle_;	}
	virtual void onExecuteRawCommandComplete()
	{
		Database::getChannel( srcAddr_ ).send( &replyBundle_ );

		delete this;
	}

private:
	Database & db_;

	Mercury::Bundle		replyBundle_;
	Mercury::Address	srcAddr_;
};


/**
 *	This method executaes a raw database command specific to the present
 *	implementation of the database interface.
 */
void Database::executeRawCommand( const Mercury::Address & srcAddr,
	const Mercury::UnpackedMessageHeader & header,
	BinaryIStream & data )
{
	std::string command( (char*)data.retrieve( header.length ), header.length );
	ExecuteRawCommandHandler* pHandler =
		new ExecuteRawCommandHandler( *this, srcAddr, header.replyID );
	pHandler->executeRawCommand( command );
}

/**
 *  This method stores some previously used ID's into the database
 */
void Database::putIDs( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream& input )
{
	int numIDs = input.remainingLength() / sizeof(EntityID);
	INFO_MSG( "Database::putIDs: storing %d id's\n", numIDs);
	pDatabase_->putIDs( numIDs,
			static_cast<const EntityID*>( input.retrieve( input.remainingLength() ) ) );
}

/**
 *	This class represents a request to get IDs from the database.
 */
class GetIDsHandler : public IDatabase::IGetIDsHandler
{
public:
	GetIDsHandler( Database & db,
			const Mercury::Address & srcAddr, Mercury::ReplyID replyID ) :
		db_( db ), srcAddr_(srcAddr), replyID_( replyID ), replyBundle_()
	{
		replyBundle_.startReply( replyID );
	}
	virtual ~GetIDsHandler() {}

	void getIDs( int numIDs )
	{
		db_.getIDatabase().getIDs( numIDs, *this );
	}

	virtual BinaryOStream& idStrm()	{ return replyBundle_; }
	virtual void resetStrm()
	{
		replyBundle_.clear();
		replyBundle_.startReply( replyID_ );
	}
	virtual void onGetIDsComplete()
	{
		Database::getChannel( srcAddr_ ).send( &replyBundle_ );
		delete this;
	}

private:
	Database & db_;
	Mercury::Address	srcAddr_;
	Mercury::ReplyID	replyID_;
	Mercury::Bundle		replyBundle_;

};


/**
 *  This methods grabs some more ID's from the database
 */
void Database::getIDs( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & input )
{
	int numIDs;
	input >> numIDs;
	INFO_MSG( "Database::getIDs: fetching %d id's for %s\n",
			numIDs, srcAddr.c_str() );

	GetIDsHandler * pHandler =
		new GetIDsHandler( *this, srcAddr, header.replyID );
	pHandler->getIDs( numIDs );
}


/**
 *	This method writes information about the spaces to the database.
 */
void Database::writeSpaces( const Mercury::Address & /*srcAddr*/,
		const Mercury::UnpackedMessageHeader & /*header*/,
		BinaryIStream & data )
{
	pDatabase_->writeSpaceData( data );
}


/**
 *	This method handles a message from the BaseAppMgr informing us that a
 *	BaseApp has died.
 */
void Database::handleBaseAppDeath( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data )
{
	Mercury::Address remappingSrcAddr;
	data >> remappingSrcAddr;

	BackupHash & remappingDstAddrs = mailboxRemapInfo_[ remappingSrcAddr ];
	data >> remappingDstAddrs;

	INFO_MSG( "Database::handleBaseAppDeath: %s\n", remappingSrcAddr.c_str() );

	pDatabase_->remapEntityMailboxes( remappingSrcAddr, remappingDstAddrs);

	mailboxRemapCheckCount_ = 5;	// do remapping for 5 seconds.
}


/**
 *	This method ends the mailbox remapping for a dead BaseApp.
 */
void Database::endMailboxRemapping()
{
//	DEBUG_MSG( "Database::endMailboxRemapping: End of handleBaseAppDeath\n" );
	mailboxRemapInfo_.clear();
}


/**
 *	This method changes the address of input mailbox to cater for recent
 *	BaseApp death.
 */
void Database::remapMailbox( EntityMailBoxRef & mailbox ) const
{
	MailboxRemapInfo::const_iterator found = 
		mailboxRemapInfo_.find( mailbox.addr );
	if ( found != mailboxRemapInfo_.end() )
	{
		const Mercury::Address& newAddr =
				found->second.addressFor( mailbox.id );
		// Mercury::Address::salt must not be modified.
		mailbox.addr.ip = newAddr.ip;
		mailbox.addr.port = newAddr.port;
	}
}


/**
 *	This method writes the game time to the database.
 */
void Database::writeGameTime( const DBInterface::writeGameTimeArgs & args )
{
	pDatabase_->setGameTime( args.gameTime );
}


/**
 *	Gathers initialisation data to send to BaseAppMgr
 */
void Database::sendInitData()
{
	// NOTE: Due to the asynchronous call, if two BaseAppMgrs register in
	// quick succession then we'll end up sending the init data twice to the
	// second BaseAppMgr.
	// onGetBaseAppMgrInitDataComplete() will be called.
	pDatabase_->getBaseAppMgrInitData( *this );
}


/**
 *	Sends initialisation data to BaseAppMgr
 */
void Database::onGetBaseAppMgrInitDataComplete( GameTime gameTime,
		int32 maxSecondaryDBAppID )
{
	// __kyl__(14/8/2008) Cater for case where DB consolidation is run during
	// start-up and has not yet completed. In that case, the
	// maxSecondaryDBAppID is 0 since that's what it would be if data
	// consolidation completed successfully. If it doesn't complete
	// successfully then we'll shutdown the system so sending the "wrong"
	// value isn't that bad.
	if (status_.status() < DBStatus::RUNNING)
	{
		maxSecondaryDBAppID = 0;
	}

	Mercury::Bundle& bundle = baseAppMgr_.bundle();
	bundle.startMessage( BaseAppMgrInterface::initData );
	bundle << gameTime;
	bundle << maxSecondaryDBAppID;

	baseAppMgr_.send();
}

/**
 *	This method sets whether we have started. It is used so that the server can
 *	be started from a watcher.
 */
void Database::hasStartBegun( bool hasStartBegun )
{
	if (hasStartBegun)
	{
		if (status_.status() >= DBStatus::WAITING_FOR_APPS)
		{
			this->startServerBegin();
		}
		else
		{
			NOTICE_MSG( "Database::hasStartBegun: Server is not ready "
					"to start yet\n" );
		}
	}
}

/**
 *	This method starts the process of starting the server.
 */
void Database::startServerBegin( bool isRecover )
{
	if (status_.status() > DBStatus::WAITING_FOR_APPS)
	{
		ERROR_MSG( "Database::startServerBegin: Server already started. Cannot "
				"start again.\n" );
		return;
	}

	if (isRecover)
	{
		// Skip auto-load from DB
		this->startServerEnd( isRecover /*=true*/, 
			/*didAutoLoadEntitiesFromDB: */false );
	}
	else
	{
		status_.set( DBStatus::RESTORING_STATE, "Restoring game state" );

		// Restore game state from DB
		Mercury::Bundle& bundle = baseAppMgr_.bundle();
		bundle.startMessage( BaseAppMgrInterface::spaceDataRestore );
		if (pDatabase_->getSpacesData( bundle ))
		{
			baseAppMgr_.send();

			EntityAutoLoader * pAutoLoader = new EntityAutoLoader;
			pDatabase_->autoLoadEntities( *pAutoLoader );
			// When autoLoadEntities() finishes startServerEnd() or
			// startServerError() will be called.
		}
		else
		{
			// Something bad happened. baseAppMgr_.bundle() is probably
			// stuffed.
			// Can't do shutdown since we'll try to send stuff to BaseAppMgr.
			// this->shutdown();
			CRITICAL_MSG( "Database::startServerBegin: Failed to read game "
					"time and space data from database!" );
		}
	}
}

/**
 *	This method completes the starting process for the DBMgr and starts all of
 *	the other processes in the system.
 */
void Database::startServerEnd( bool isRecover, bool didAutoLoadEntitiesFromDB  )
{
	if (status_.status() < DBStatus::RUNNING)
	{
		status_.set( DBStatus::RUNNING, "Running" );

		if (!isRecover)
		{
			TRACE_MSG( "Database::startServerEnd: Sending startup message\n" );
			Mercury::ChannelSender sender( this->baseAppMgr().channel() );

			Mercury::Bundle & bundle = sender.bundle();
			bundle.startMessage( BaseAppMgrInterface::startup );
			bundle << didAutoLoadEntitiesFromDB;
		}
	}
	else
	{
		ERROR_MSG( "Database::startServerEnd: Already started.\n" );
	}
}

/**
 *	This method is called instead of startServerEnd() to indicate that there
 *	was an error during or after startServerBegin().
 */
void Database::startServerError()
{
	MF_ASSERT( status_.status() < DBStatus::RUNNING );
	this->startSystemControlledShutdown();
}

/**
 *	This method is the called when an entity that is being checked out has
 *	completed the checkout process. onStartEntityCheckout() should've been
 *	called to mark the start of the operation. pBaseRef is the base mailbox
 *	of the now checked out entity. pBaseRef should be NULL if the checkout
 *	operation failed.
 */
bool Database::onCompleteEntityCheckout( const EntityKey& entityID,
	const EntityMailBoxRef* pBaseRef )
{
	bool isErased = (inProgCheckouts_.erase( entityID ) > 0);
	if (isErased && (checkoutCompletionListeners_.size() > 0))
	{
		std::pair< CheckoutCompletionListeners::iterator,
				CheckoutCompletionListeners::iterator > listeners =
			checkoutCompletionListeners_.equal_range( entityID );
		for ( CheckoutCompletionListeners::const_iterator iter =
				listeners.first; iter != listeners.second; ++iter )
		{
			iter->second->onCheckoutCompleted( pBaseRef );
		}
		checkoutCompletionListeners_.erase( listeners.first,
				listeners.second );
	}

	return isErased;
}

/**
 *	This method registers listener to be called when the entity identified
 *	by typeID and dbID completes its checkout process. This function will
 *	false and not register the listener if the entity is not currently
 *	in the process of being checked out.
 */
bool Database::registerCheckoutCompletionListener( EntityTypeID typeID,
		DatabaseID dbID, Database::ICheckoutCompletionListener& listener )
{
	EntityKeySet::key_type key( typeID, dbID );
	bool isFound = (inProgCheckouts_.find( key ) != inProgCheckouts_.end());
	if (isFound)
	{
		CheckoutCompletionListeners::value_type item( key, &listener );
		checkoutCompletionListeners_.insert( item );
	}
	return isFound;
}

// database.cpp
