/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "main_app.hpp"

#include "client_app.hpp"
#include "entity.hpp"
#include "patrol_graph.hpp"
#include "py_bots.hpp"

#include "connection/rsa_stream_encoder.hpp"

#include "entitydef/constants.hpp"

#include "network/machined_utils.hpp"
#include "network/misc.hpp"
#include "network/watcher_nub.hpp"

#include "pyscript/py_import_paths.hpp"
#include "pyscript/py_traceback.hpp"

#include "server/bwconfig.hpp"
#include "server/python_server.hpp"

#ifdef unix
#include <signal.h>
#include <dlfcn.h>
#endif

#include <memory>

DECLARE_DEBUG_COMPONENT2( "Bots", 0 )

extern int Math_token;
extern int ResMgr_token;
extern int PyScript_token;

extern int PyUserDataObject_token;
extern int UserDataObjectDescriptionMap_Token;

namespace
{
// These options are related to splitting the sends up over each tick.
const int TICK_FRAGMENTS = 1; // Currently not used.
const int TICK_FREQUENCY = 10;
const int TICK_TIMEOUT = 1000000/TICK_FREQUENCY/TICK_FRAGMENTS;
const float TICK_PERIOD = 1.f/TICK_FREQUENCY;
const float MINI_TICK_PERIOD = 0.1f / TICK_FRAGMENTS;

int s_moduleTokens =
	Math_token | ResMgr_token | PyScript_token;
int s_udoTokens = PyUserDataObject_token | UserDataObjectDescriptionMap_Token;
}

// -----------------------------------------------------------------------------
// Section: Static data
// -----------------------------------------------------------------------------

/// Bots Application Singleton.
BW_SINGLETON_STORAGE( MainApp )

// -----------------------------------------------------------------------------
// Section: class MainApp
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
MainApp::MainApp( Mercury::EventDispatcher & mainDispatcher, 
			Mercury::NetworkInterface & networkInterface ) :
		ServerApp( mainDispatcher, networkInterface ),
		pLogOnParamsEncoder_( NULL ), // initialised below in the constructor
		localTime_( 0.0 ),
		timerHandle_(),
		sendTimeReportThreshold_( 10.0 ),
		pPythonServer_( NULL ),
		clientTickIndex_( bots_.end() )
{
	srand( (unsigned int)timestamp() );
}


/**
 *	Destructor.
 */
MainApp::~MainApp()
{
	timerHandle_.cancel();

	bots_.clear();
	Py_XDECREF( pPythonServer_ );
	pPythonServer_ = NULL;

	Script::fini();
}


/**
 *	This method initialises the application.
 *
 *	@return True on success, otherwise false.
 */
bool MainApp::init( int argc, char * argv[] )
{
	if (!this->ServerApp::init( argc, argv ))
	{
		return false;
	}

#ifdef USE_OPENSSL
	std::auto_ptr< RSAStreamEncoder > pEncoder( 
		new RSAStreamEncoder( /* keyIsPrivate */ false ) );

	if (pEncoder->initFromKeyPath( BotsConfig::publicKey() ))
	{
		pLogOnParamsEncoder_.reset( pEncoder.release() );
	}
	else
	{
		ERROR_MSG( "MainApp::MainApp: "
				"Could not initialise LoginApp public key from %s, "
				"using unencrypted logins\n", 
			BotsConfig::publicKey().c_str() );
	}
#endif

	loginDigest_.unquote( BotsConfig::loginMD5Digest() );

	INFO_MSG( "MainApp::MainApp: Using digest %s\n",
			loginDigest_.quote().c_str() );

	this->parseCommandLine( argc, argv );

	if (BotsConfig::serverName().empty())
	{
		Mercury::Address addr;
		Mercury::Reason lookupStatus = Mercury::MachineDaemon::findInterface(
											"LoginInterface", 0, addr, 4 );
		if (lookupStatus == Mercury::REASON_SUCCESS)
		{
			BotsConfig::serverName.set( addr.c_str() );
			/* ignore port_ config as we've got that from LoginInterface */
			BotsConfig::port.set( 0 );
			INFO_MSG( "Found LoginInterface via bwmachined at %s.",
				BotsConfig::serverName().c_str() );
		}
		else
		{
			ERROR_MSG( "MainApp::init: Failed to find LoginApp Interface "								"(%s)\n", Mercury::reasonToString( lookupStatus ) );
			return false;
		}
	}

	if (BotsConfig::serverName().empty())
	{
		char inputServerName[128];
		std::cout << "Input server name: ";
		if (scanf( "%128s", inputServerName ) > 0)
		{
			BotsConfig::serverName.set( inputServerName );
		}
	}

	if (BotsConfig::port() != 0)
	{
		char portNum[10];
		snprintf( portNum, 10, ":%d", BotsConfig::port() );
		std::string newServerName = BotsConfig::serverName() + 
			std::string( portNum );
		
		BotsConfig::serverName.set( newServerName );
	}

	timerHandle_ = this->mainDispatcher().addTimer( TICK_TIMEOUT, this );

	if (!this->initScript())
	{
		return false;
	}
	
	this->initWatchers();

	Script::initExceptionHook( &(this->mainDispatcher()) );

	return true;
}


/**
 *	This method adds another simulated client to this application.
 */
void MainApp::addBot()
{
	std::string bname = BotsConfig::username();
	std::string bpass = BotsConfig::password();
	if (BotsConfig::shouldUseRandomName())
	{
		char randName[10];
		bw_snprintf( randName, sizeof( randName ), "_%08x", rand() );
		bname.append( randName );
	}

	ClientApp * pNewApp = new ClientApp( bname, bpass, BotsConfig::tag() );
	pNewApp->connectionSendTimeReportThreshold( sendTimeReportThreshold_ ); 
	bots_.push_back( pNewApp ); 
}


/**
 *	This method adds a number of simulated clients to this application.
 */
void MainApp::addBots( int num )
{
	if (num <= 0)
		return;

	for (int i = 0; i < num; ++i)
	{
		this->addBot();
	}
}

void MainApp::addBotsWithName( PyObjectPtr logInfoData )
{
	if (!logInfoData || logInfoData == Py_None)
	{
		PyErr_SetString( PyExc_TypeError,
			"Bots::addBotsWithName: empty log info. "
			"Argument must be list of tuple." );
		return;
	}
	if (!PyList_Check( logInfoData.getObject() ))
	{
		PyErr_SetString( PyExc_TypeError,
			"Bots::addBotsWithName: Argument must be list of tuple." );
		return;
	}
	Py_ssize_t listSize = PyList_Size( logInfoData.getObject() );
	for (Py_ssize_t i = 0; i < listSize; ++i)
	{
		PyObject * loginItem = PyList_GetItem( logInfoData.getObject(), i );
		if (!PyTuple_Check( loginItem ) || PyTuple_Size( loginItem ) != 2)
		{
			PyErr_Format( PyExc_TypeError,
				"Bots::addBotsWithName: Argument list item %"PRIzd" must "
				"be tuple of two string.", i );
			return;
		}
		PyObject * nameInfo = PyTuple_GetItem( loginItem, 0 );
		PyObject * passwdInfo = PyTuple_GetItem( loginItem, 1 );
		if (!PyString_Check( nameInfo ) || !PyString_Check( passwdInfo ))
		{
			PyErr_Format( PyExc_TypeError,
				"Bots::addBotsWithName: Argument list item %"PRIzd" must "
				"be tuple of two string.", i );
			return;
		}
		bots_.push_back( 
			new ClientApp( std::string( PyString_AsString( nameInfo ) ),
			std::string( PyString_AsString( passwdInfo ) ),
			BotsConfig::tag() ) );
	}
}

/**
 *	This method removes a number of simulated clients from this application.
 */
void MainApp::delBots( int num )
{
	while (num-- > 0 && !bots_.empty())
	{
		Bots::iterator iter = bots_.begin();
		if (iter == clientTickIndex_)
		{
			++clientTickIndex_;
		}
		Py_DECREF( iter->get() );
		bots_.pop_front();
	}
}


/**
 *	This method updates the movement controllers of all bots matching the input
 *	tag based on the current default values.
 *
 *	If the input tag is empty, all bots are changed.
 */
void MainApp::updateMovement( std::string tag )
{
	Bots::iterator iter = bots_.begin();

	while (iter != bots_.end())
	{
		if (tag.empty() || ((*iter)->tag() == tag))
		{
			bool ok = (*iter)->setMovementController( 
				BotsConfig::controllerType(),
				BotsConfig::controllerData() );
			if (!ok)
			{
				PyErr_Print();
			}
		}
		++iter;
	}
}


/**
 *	This method runs the input string.
 */
void MainApp::runPython( std::string command )
{
	if (PyRun_SimpleString( command.c_str() ) != 0)
	{
		ERROR_MSG( "MainApp::runPython: Couldn't execute '%s'\n",
				command.c_str() );
		PyErr_Print();
	}
}

void MainApp::loginMD5Digest( std::string quoteDigest )
{
	if (!loginDigest_.unquote( quoteDigest ))
	{
		PyErr_SetString( PyExc_ValueError,
			"Bots::setLoginMD5Digest: Login MD5 digest in text format "
			"should be 32 character long" );
		PyErr_Print();
	}
}
/**
 *	This method deletes tagged entities.
 */
void MainApp::delTaggedEntities( std::string tag )
{
	Bots::iterator iter = bots_.begin();
	Bots condemnedBots; //Call destructors when going out of scope

	while (iter != bots_.end())
	{
		Bots::iterator oldIter = iter++;
		if ((*oldIter)->tag() == tag)
		{
			if (oldIter == clientTickIndex_)
			{
				++clientTickIndex_;
			}
			condemnedBots.push_back(*oldIter);
			Py_DECREF( oldIter->get() );
			bots_.erase( oldIter );
		}
	}
}


/*
 *	Override from TimerExpiryHandler.
 */
void MainApp::handleTimeout( TimerHandle, void * )
{
	static bool inTick = false;

	if (inTick)
	{
		// This can occur because the tick method of ClientApp can processInput
		// on the ServerConnection.
		WARNING_MSG( "MainApp::handleTimeout: Called recursively\n" );
		return;
	}

	inTick = true;

	// give Bots personality script a chance to handle
	Script::call( 
		PyObject_GetAttrString( this->getPersonalityModule().get(), "onTick" ),
		PyTuple_New( 0 ), "BWPersonality.onTick", true );

	static int remainder = 0;
	int numberToUpdate = (bots_.size() + remainder) / TICK_FRAGMENTS;
	remainder = (bots_.size() + remainder) % TICK_FRAGMENTS;

	localTime_ += MINI_TICK_PERIOD;

	while (numberToUpdate-- > 0 && !bots_.empty())
	{
		if (clientTickIndex_ == bots_.end())
		{
			clientTickIndex_ = bots_.begin();	
		}
		Bots::iterator iter = clientTickIndex_++;

		if (!(*iter)->tick( TICK_PERIOD ))
		{
			bots_.erase( iter );
		}
	}

	inTick = false;
}


/**
 *	Thie method returns personality module
 */
PyObjectPtr MainApp::getPersonalityModule() const
{
	std::string moduleName = BWConfig::get( "personality", "BWPersonality" );

	PyObjectPtr pModule(
		PyImport_ImportModule( (char*)moduleName.c_str() ),
		PyObjectPtr::STEAL_REFERENCE );

	if (!pModule)
	{
		PyObject * pErrType = PyErr_Occurred();

		if (pErrType == PyExc_ImportError)
		{
			INFO_MSG( "No personality module - '%s'\n", moduleName.c_str() );
			PyErr_Clear();
		}
		else
		{
			ERROR_MSG( "MainApp::getPersonalityModule: "
						"Failed to import personality module '%s'\n",
					moduleName.c_str() );
			PyErr_Print();
		}
	}

	return pModule;
}


/** 
 *  Set the send time report threshold for new connections and existing 
 *  connections. If a client app takes longer than this threshold for 
 *  successive sends to the server, it will emit a warning message. 
 * 
 *  @param threshold    the new send time reporting threshold 
*/ 
void MainApp::sendTimeReportThreshold( double threshold ) 
{ 
	// Set it here for new connections 
	sendTimeReportThreshold_ = threshold; 
	// Also set it for any client apps we already have 
	Bots::iterator iBot = bots_.begin(); 
	while (iBot != bots_.end()) 
	{ 
		(*iBot)->connectionSendTimeReportThreshold( threshold ); 
		++iBot; 
	} 
}


namespace
{
typedef std::map< std::string, MovementFactory * > MovementFactories;
MovementFactories * s_pMovementFactories;
}

/**
 *	This method returns a default movement controller instance.
 */
MovementController * MainApp::createDefaultMovementController(
	float & speed, Vector3 & position )
{
	return this->createMovementController( speed, position,
		BotsConfig::controllerType(), BotsConfig::controllerData() );
}


/**
 *	This method creates a movement controller corresponding to the input
 *	arguments.
 */
MovementController *
	MainApp::createMovementController( float & speed, Vector3 & position,
							   const std::string & controllerTypeIn,
							   const std::string & controllerData )
{
	std::string controllerType = controllerTypeIn;

	if (controllerType == "None") return NULL;

#ifdef unix
	uint soPos = controllerType.find( ".so:" );
   	if (soPos < controllerType.length())
	{
		std::string soName = controllerType.substr( 0, soPos+3 );
		static std::set<std::string> loadedLibs;
		if (loadedLibs.find( soName ) == loadedLibs.end())
		{
			loadedLibs.insert( soName );
			std::string soPath = "bots-extensions/"+soName;
			void * handle = dlopen( soPath.c_str(), RTLD_LAZY | RTLD_GLOBAL );
			if (handle == NULL)
			{
				ERROR_MSG( "MainApp::createMovementController: "
					"Failed to load dyn lib '%s' since %s\n",
					soName.c_str(), dlerror() );
			}
			else
			{
				INFO_MSG( "MainApp::createMovementController: "
					"Loaded dyn lib '%s'\n", soName.c_str() );
			}
		}

		controllerType = controllerType.substr( soPos+4 );
	}
#endif

	if (s_pMovementFactories != NULL)
	{
		MovementFactories::iterator iter =
				s_pMovementFactories->find( controllerType );

		if (iter != s_pMovementFactories->end())
		{
			return iter->second->create( controllerData, speed,
						position );
		}
	}

	PyErr_Format( PyExc_TypeError, "No such controller type '%s'",
		controllerType.c_str() );
	return NULL;
}


/**
 *	This static method registers a MovementFactory.
 */
void MainApp::addFactory( const std::string & name, MovementFactory & factory )
{
	if (s_pMovementFactories == NULL)
	{
		s_pMovementFactories = new MovementFactories;
	}

	(*s_pMovementFactories)[ name ] = &factory;
}


// -----------------------------------------------------------------------------
// Section: Script related
// -----------------------------------------------------------------------------

/**
 *	This method returns the client application with the input id.
 */
ClientApp * MainApp::findApp( EntityID id ) const
{
	// This is inefficient. Could look at making a map of these but it should
	// not be used this way very often.(??)
	Bots::const_iterator iter = bots_.begin();

	while (iter != bots_.end())
	{
		ClientApp * pApp = iter->get();

		// When deleting multiple bots, there may be holes in the bots_ vector
		// temporarily, so jump over any NULL pointers we encounter
		if (pApp && pApp->id() == id)
		{
			Py_INCREF( pApp );
			return pApp;
		}

		++iter;
	}

	return NULL;
}


/**
 *	This method populates a list with the IDs of available apps.
 */
void MainApp::appsKeys( PyObject * pList ) const
{
	Bots::const_iterator iter = bots_.begin();

	while (iter != bots_.end())
	{
		PyObject * pInt = PyInt_FromLong( (*iter)->id() );
		PyList_Append( pList, pInt );
		Py_DECREF( pInt );

		++iter;
	}
}


/**
 *	This method populates a list with available apps.
 */
void MainApp::appsValues( PyObject * pList ) const
{
	Bots::const_iterator iter = bots_.begin();

	while (iter != bots_.end())
	{
		PyList_Append( pList, iter->get() );

		++iter;
	}
}


/**
 *	This method populates a list with id, value pairs of the available apps.
 */
void MainApp::appsItems( PyObject * pList ) const
{
	Bots::const_iterator iter = bots_.begin();

	while (iter != bots_.end())
	{
		PyObject * pTuple = PyTuple_New( 2 );
		PyTuple_SetItem( pTuple, 0, PyInt_FromLong( (*iter)->id() ) );
		Py_INCREF( (*iter).get() );
		PyTuple_SetItem( pTuple, 1, iter->get() );
		PyList_Append( pList, pTuple );
		Py_DECREF( pTuple );

		++iter;
	}
}


/**
 *	This method parses the command line for options used by the bots process.
 */
void MainApp::parseCommandLine( int argc, char * argv[] )
{
	// Get any command line arguments
	for (int i = 0; i < argc; i++)
	{
		if (strcmp( "-serverName", argv[i] ) == 0)
		{
			i++;
			BotsConfig::serverName.set( ( i < argc ) ? argv[ i ] : "" );
			INFO_MSG( "Server name is %s\n", BotsConfig::serverName().c_str() );
		}
		else if (strcmp( "-username", argv[i] ) == 0)
		{
			i++;
			BotsConfig::username.set( 
				( i < argc ) ? argv[ i ] : BotsConfig::username() );
			INFO_MSG( "Username is %s\n", BotsConfig::username().c_str() );
		}
		else if (strcmp( "-password", argv[i] ) == 0)
		{
			i++;
			BotsConfig::password.set( 
				( i < argc ) ? argv[ i ] : BotsConfig::password() );
		}
		else if (strcmp( "-port", argv[i] ) == 0)
		{
			i++;
			BotsConfig::port.set( 
				( i < argc ) ? atoi(argv[ i ]) : BotsConfig::port() );
		}
		else if (strcmp( "-randomName", argv[i] ) == 0)
		{
			BotsConfig::shouldUseRandomName.set( true );
		}
		else if (strcmp( "-scripts", argv[i] ) == 0)
		{
			BotsConfig::shouldUseScripts.set( true );
		}
	}
}


/**
 *	Initialise Python scripting.
 */
bool MainApp::initScript()
{
	PyImportPaths paths;
	paths.addPath( EntityDef::Constants::botScriptsPath() );

	Script::init( paths, "bot", true );

	PyObject * pModule = PyImport_AddModule( "BigWorld" );

	if (pModule)
	{
		PyObject * pBots = new PyBots;

		if (PyObject_SetAttrString( pModule, "bots", pBots ) == -1)
		{
			ERROR_MSG( "MainApp::init: Failed to set BigWorld.bots\n" );
			PyErr_Clear();
		}
		Py_DECREF( pBots );
	}

	// Initialise the entity descriptions.
	// Read entities scripts anyway as we need to create player entities
	// for logoff purpose
	if (EntityType::init( BotsConfig::standinEntity() ) == -1)
	{
		ERROR_MSG( "MainApp::init: Could not initialise entity data.Abort!\n" );
		return false;
	}

	Script::call(
		PyObject_GetAttrString( this->getPersonalityModule().get(), 
			"onBotsReady" ),
		PyTuple_New( 0 ), "onBotsReady", true );

	pPythonServer_ = new PythonServer( "Welcome to the Bot process" );
	pPythonServer_->startup( mainDispatcher_, 0,
							 interface_.address().port, 0 );
	PyRun_SimpleString( "import BigWorld" );

	INFO_MSG( "Python Server Port is %d\n", pPythonServer_->port() );

	return true;
}


void MainApp::initWatchers()
{
	// Register the watcher
	BW_REGISTER_WATCHER( 0, "bots", "Bot App", "bots", this->mainDispatcher(),
						 interface_.address() );

	Watcher & root = Watcher::rootWatcher();
	this->ServerApp::addWatchers( root );

	MF_WATCH( "command/addBots", *this,
			MF_WRITE_ACCESSOR( int, MainApp, addBots ) );
	MF_WATCH( "command/delBots", *this,
			MF_WRITE_ACCESSOR( int, MainApp, delBots ) );

	MF_WATCH( "tag", BotsConfig::tag.getRef() );
	MF_WATCH( "command/delTaggedEntities", *this,
			MF_WRITE_ACCESSOR( std::string, MainApp, delTaggedEntities ) );

	MF_WATCH( "numBots", bots_, &Bots::size );

	MF_WATCH( "pythonServerPort", *pPythonServer_, &PythonServer::port );

	/* */
	MF_WATCH( "defaultControllerType", BotsConfig::controllerType.getRef() );
			// MF_ACCESSORS( std::string, MainApp, controllerType ) );
	MF_WATCH( "defaultControllerData", BotsConfig::controllerData.getRef() );
			// MF_ACCESSORS( std::string, MainApp, controllerData ) );
	MF_WATCH( "defaultStandinEntity", BotsConfig::standinEntity.getRef() );
	MF_WATCH( "loginMD5Digest", *this,
			MF_ACCESSORS( std::string, MainApp, loginMD5Digest ) );
	MF_WATCH( "command/updateMovement", *this,
			MF_WRITE_ACCESSOR( std::string, MainApp, updateMovement ) );
	MF_WATCH( "command/runPython", *this,
			MF_WRITE_ACCESSOR( std::string, MainApp, runPython ) );

	MF_WATCH( "sendTimeReportThreshold", *this, 
			MF_ACCESSORS( double, MainApp, sendTimeReportThreshold ) ); 
}


// -----------------------------------------------------------------------------
// Section: BigWorld script functions
// -----------------------------------------------------------------------------

namespace // anonymous
{


void addBots( int count )
{
	MainApp::instance().addBots( count );
}
PY_AUTO_MODULE_FUNCTION( RETVOID, addBots, ARG( int, END ), BigWorld )


void addBotsWithName( PyObjectPtr logInfoData )
{
	// validate input log info. it should be a list of duple of
	// user name and password.
	MainApp::instance().addBotsWithName( logInfoData );
	if (PyErr_Occurred())
	{
		PyErr_Print();
	}
}
PY_AUTO_MODULE_FUNCTION( RETVOID, addBotsWithName, ARG( PyObjectPtr, END ), 
	BigWorld )


void setLoginMD5Digest( std::string quoteDigest )
{
	MainApp::instance().loginMD5Digest( quoteDigest );
}
PY_AUTO_MODULE_FUNCTION( RETVOID, setLoginMD5Digest, ARG( std::string, END ), BigWorld )


void delBots( int count )
{
	MainApp::instance().delBots( count );
}
PY_AUTO_MODULE_FUNCTION( RETVOID, delBots, ARG( int, END ), BigWorld )


#define DEFAULT_ACCESSOR( N1, N2 )											\
std::string getDefault##N2()												\
{																			\
	return BotsConfig::N1();												\
}																			\
PY_AUTO_MODULE_FUNCTION( RETDATA, getDefault##N2, END, BigWorld )			\
																			\
void setDefault##N2( const std::string & v )								\
{																			\
	BotsConfig::N1.set( v );												\
}																			\
																			\
PY_AUTO_MODULE_FUNCTION( RETVOID,											\
		setDefault##N2, ARG( std::string, END ), BigWorld )					\

DEFAULT_ACCESSOR( serverName, Server )
DEFAULT_ACCESSOR( username, Username )
DEFAULT_ACCESSOR( password, Password )
DEFAULT_ACCESSOR( tag, Tag )
DEFAULT_ACCESSOR( controllerType, ControllerType )
DEFAULT_ACCESSOR( controllerData, ControllerData )


class BotAdder : public TimerHandler
{
public:
	/**
	 *	Constructor.
	 *	@param total 	Number of bots to add.
	 *	@param period 	The tick period to add bots.
	 *	@param perTick	How many bots to add each tick.
	 */
	BotAdder( int total, float period, int perTick ) :
		remaining_( total ),
		perTick_( perTick )
	{
		timer_ = MainApp::instance().mainDispatcher().addTimer(
				int(period * 1000000), this );
	}

	/**
	 *	Destructor.
	 */
	virtual ~BotAdder()
	{
		timer_.cancel();
	}

	/**
	 *	Override from TimerHandler.
	 */
	virtual void handleTimeout( TimerHandle handle, void * )
	{
		MainApp::instance().addBots( std::min( remaining_, perTick_ ) );
		remaining_ -= perTick_;

		if (remaining_ <= 0)
		{
			timer_.cancel();
			delete this;
		}
	}

private:
	int remaining_;
	int perTick_;

	TimerHandle timer_;
};

void addBotsSlowly( int count, float period, int perTick )
{
	new BotAdder( count, period, perTick );
}
PY_AUTO_MODULE_FUNCTION( RETVOID, addBotsSlowly,
		ARG( int, OPTARG( float, 1.f, OPTARG( int, 1, END ) ) ), BigWorld )


} // namespace (anonymous)

/*~ function BigWorld.addBots
 *	@components{ bots }
 *
 *	This function immediately creates a specified number of simulated clients. 
 *	These clients will log into an existing BigWorld server known to the 
 *	Bots application.
 *
 *	@param	num		Number of simulated clients to be added.
 */
/*~ function BigWorld.addBotsSlowly
 *	@components{ bots }
 *
 *	This function creates a specified number of simulated clients (similar to 
 *	BigWorld.addBots) in a slower and controlled fashion (in groups) to 
 *	prevent sudden surge of load that would overwhelm the server.
 *
 *	@param	num		Number of simulated clients to be added (will be broken up 
 *					into groups of size groupSize).
 *	@param	delay	a delay (in seconds) between groups
 *	@param	groupSize	group size. default is 1.
 *
 *	For example:
 *	@{
 *	BigWorld.addBotsSlowly( 200, 5, 20 )
 *	@}
 *	This example will add 200 clients in total in groups of 20 with 5
 *	seconds intervals between adding groups.
 */

/*~ function BigWorld.addBotsWithName
 *	@components{ bots }
 *
 *	This function creates a number of simulated clients with specific
 *	login names and passwords.
 *
 *	@param	PyObject	list of tuples of login name and password
 *
 *	For example:
 *	@{
 *	BigWorld.addBotsWithName( [('tester01', 'tp1'),('tester02', 'tp2'),('tester03', 'tp3')] )
 *	@}
 */
/*~ function BigWorld.delBots
 *	@components{ bots }
 *
 *	This function immediately removes a specified number of simulated
 *	clients from the Bots application. These clients will be logged
 *	off the server gracefully.
 *
 *	@param	num		Number of simulated clients to be removed.
 */
/*~ function BigWorld.setLoginMD5Digest
 *	@components{ bots }
 *
 *	This function set the specific MD5 digest (in readable hex format)
 *	required for simulated clients log into a BigWorld server. The
 *	digest must be 32 characters long. A corrupted input digest will
 *	set the digest to null.
 *
 *	@param	digest		string.
 */
/*~ function BigWorld.setDefaultServer
 *	@components{ bots }
 *
 *	This function sets the default login server for the simulated
 *	clients.
 *
 *	@param	host		string in format of 'host:port'.
 */
/*~ function BigWorld.getDefaultServer
 *	@components{ bots }
 *
 *	This function returns the name of the default login server for 
 *	the simulated clients.
 *
 *	@return	host		string in format of 'host:port'.
 */
/*~ function BigWorld.setDefaultUsername
 *	@components{ bots }
 *
 *	This function sets the default login user name for 
 *	the simulated clients to use.
 *
 *	@param	username		string.
 */
/*~ function BigWorld.getDefaultUsername
 *	@components{ bots }
 *
 *	This function returns the default login user name for 
 *	the simulated clients to use.
 *
 *	@return	username		string.
 */
/*~ function BigWorld.setDefaultPassword
 *	@components{ bots }
 *
 *	This function sets the default login password for 
 *	the simulated clients to use.
 *
 *	@param	password		string.
 */
/*~ function BigWorld.getDefaultPassword
 *	@components{ bots }
 *
 *	This function returns the default login password for 
 *	the simulated clients to use.
 *
 *	@return	password		string.
 */
/*~ function BigWorld.setDefaultTag
 *	@components{ bots }
 *
 *	This function sets the default tag name for simulated clients.
 *
 *	@param	tag		string.
 */
/*~ function BigWorld.getDefaultTag
 *	@components{ bots }
 *
 *	This function returns the default tag name for the simulated
 *	clients.
 *
 *	@return	tag		string.
 */
/*~ function BigWorld.setDefaultControllerType
 *	@components{ bots }
 *
 *	This function sets the default movement controller to be used by
 *	the simulated clients. The input parameter should be the name of a
 *	compiled shared object file (with .so extension) containing a
 *	movement controller, residing under the bot-extensions
 *	subdirectory of your Bots installation.  Note: we recommend you to
 *	use the builtin default movement controller and create custom data
 *	for it instead of creating your own movement controller.
 *
 *	@param	controllerType		string.
 */
/*~ function BigWorld.getDefaultControllerType
 *	@components{ bots }
 *
 *	This function returns the default movement controller to
 *	be used by the simulated clients.
 *
 *	@return	controllerType		string.
 */
/*~ function BigWorld.setDefaultControllerData
 *	@components{ bots }
 *
 *	This function sets the default data file used by the current
 *	default movement controller. The input parameter should be the
 *	relative path (with respect to the BigWorld res path) to a data
 *	file. Make sure the data file is can be understood by your
 *	movement controller, otherwise the movement controller may cause
 *	an error and bring down the Bots application.
 *
 *	@param	controllerData		string.
 */
/*~ function BigWorld.getDefaultControllerData
 *	@components{ bots }
 *
 *	This function returns the default movement controller to
 *	be used by the simulated clients.
 *
 *	@return	controllerData		string.
 */
/*~ attribute BigWorld bots @components{ bots } bots contains a list
 *	of all simulated clients currently instanced on the Bots
 *	application.  
 *
 *  @type Read-only PyBots
 */
// main_app.cpp
