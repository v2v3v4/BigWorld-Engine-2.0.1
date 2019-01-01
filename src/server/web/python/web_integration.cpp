/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "web_integration.hpp"

#include "autotrace.hpp"
#include "backup_hash_chain_request_handler.hpp"
#include "blocking_db_logon_handler.hpp"
#include "blocking_db_reply_handler.hpp"
#include "mailbox.hpp"

#include "baseappmgr/baseappmgr_interface.hpp"

#include "connection/log_on_params.hpp"

#include "cstdmf/debug.hpp"

#include "dbmgr/db_interface.hpp"

#include "entitydef/constants.hpp"
#include "entitydef/entity_description_map.hpp"
#include "entitydef/mailbox_base.hpp"

#include "network/basictypes.hpp"
#include "network/blocking_reply_handler.hpp"
#include "network/bundle.hpp"
#include "network/event_dispatcher.hpp"
#include "network/machined_utils.hpp"

#include "pyscript/script.hpp"

#include "resmgr/bwresource.hpp"

#include "server/backup_hash_chain.hpp"

/// Web Integration module Singleton.
BW_SINGLETON_STORAGE( WebIntegration )

namespace // anonymous
{
// ----------------------------------------------------------------------------
// Section: Helper method implementations
// ----------------------------------------------------------------------------

/**
 *	Returns the appropriate exception object for a given logon status value.
 *
 *	@param status the LogOnStatus value
 *	@return the appropriate Python exception object
 */
PyObject * getLogOnStatusException( const LogOnStatus& status )
{
	PyObject* exception = NULL;
	switch (status)
	{
		case LogOnStatus::CONNECTION_FAILED:
		case LogOnStatus::DNS_LOOKUP_FAILED:
		case LogOnStatus::LOGIN_REJECTED_ILLEGAL_CHARACTERS:
			exception = PyExc_IOError;
			break;

		case LogOnStatus::UNKNOWN_ERROR:
		case LogOnStatus::PUBLIC_KEY_LOOKUP_FAILED:
		case LogOnStatus::LOGIN_REJECTED_BAD_DIGEST:
		case LogOnStatus::LOGIN_REJECTED_DB_GENERAL_FAILURE:
		case LogOnStatus::LOGIN_REJECTED_DB_NOT_READY:
		case LogOnStatus::LOGIN_REJECTED_SERVER_NOT_READY:
		case LogOnStatus::LOGIN_REJECTED_NO_BASEAPPS:
		case LogOnStatus::LOGIN_REJECTED_BASEAPP_OVERLOAD:
		case LogOnStatus::LOGIN_REJECTED_CELLAPP_OVERLOAD:
		case LogOnStatus::LOGIN_REJECTED_BASEAPP_TIMEOUT:
		case LogOnStatus::LOGIN_REJECTED_BASEAPPMGR_TIMEOUT:
		case LogOnStatus::LOGIN_REJECTED_DBMGR_OVERLOAD:
			exception = PyExc_SystemError;
			break;
		case LogOnStatus::LOGIN_REJECTED_NO_SUCH_USER:
		case LogOnStatus::LOGIN_REJECTED_INVALID_PASSWORD:
		case LogOnStatus::CANCELLED:
		case LogOnStatus::LOGIN_REJECTED_ALREADY_LOGGED_IN:
			exception = PyExc_ValueError;
			break;

		default:
			exception = PyExc_RuntimeError;

	}
	return exception;
}

} // namespace anonymous


// ----------------------------------------------------------------------------
// Section: WebIntegration
// ----------------------------------------------------------------------------

/**
 *	Constructor.
 */
WebIntegration::WebIntegration():
		pInterface_( NULL ),
			// pInterface_ is created on-demand, and createPid_ is used to
			// track whether we need to recreate because we've been forked, see
			// definition of WebIntegration::networkInterface()
		pDispatcher_( new Mercury::EventDispatcher() ),
		createPid_( 0 ),
		dbMgrAddr_( 0, 0 ),
		pEntityDescriptions_( NULL ),
		hasInited_( false ),
		pBackupHashChain_( new BackupHashChain() ),
		loggerSocket_(),
		loggerMessageForwarder_( "Web", loggerSocket_ )
{
	// Create the initial interface
	this->interface();
}


/**
 *	Destructor.
 */
WebIntegration::~WebIntegration()
{
	pInterface_->detach();

	if (pEntityDescriptions_)
	{
		delete pEntityDescriptions_;
	}
}


/**
 *	Initialise the web integration singleton. Returns false and sets an
 *	appropriate Python exception on initialisation failure.
 *
 *	@return true if initialisation success, otherwise
 */
bool WebIntegration::init()
{
	if (hasInited_)
	{
		PyErr_SetString( PyExc_EnvironmentError,
			"web integration module already initialised" );
		return false;
	}

	pEntityDescriptions_ = new EntityDescriptionMap();

	DataSectionPtr pEntityDefData = BWResource::openSection(
		EntityDef::Constants::entitiesFile() );

	if (!pEntityDefData)
	{
		ERROR_MSG( "WebIntegration::init: "
			"Could not open %s to parse entity definitions\n",
			EntityDef::Constants::entitiesFile() );
		PyErr_Format( PyExc_EnvironmentError,
			"Could not open %s to parse entity definitions\n",
			EntityDef::Constants::entitiesFile() );
		return false;
	}

	if (!pEntityDescriptions_->parse( pEntityDefData ))
	{
		ERROR_MSG( "WebIntegration::init: "
			"Failed to parse entity definitions\n" );
		PyErr_SetString( PyExc_EnvironmentError,
			"Failed to parse entity definitions\n" );
		return false;
	}

	WebEntityMailBox::initMailboxFactory();

	hasInited_ = true;

	return true;
}


/**
 *	Sets the network interface port, and invalidates the local copy of the
 *	DBMgr's address.
 *
 *	@param port the port, or 0 for a random port.
 *
 */
void WebIntegration::resetNetworkInterface( uint16 port )
{
	pInterface_.reset( new Mercury::NetworkInterface( pDispatcher_.get(),
		Mercury::NETWORK_INTERFACE_INTERNAL, port ) );

	// also reset addresses
	dbMgrAddr_ = Mercury::Address::NONE;

}


/**
 *	Return the last known address for the DBMgr component. If forget is true,
 *	then (re)-query the machine daemon for the DBMgr address, store, then
 *	return the retrieved address.
 *
 *	@param forget		whether to forget the last known address, and retrieve
 *						the DBMgr by querying the machine daemon.
 *
 *	@return the last known DBMgr address, or Address::NONE if no DBMgr exists
 *	that could be looked up.
 */
const Mercury::Address & WebIntegration::getDBMgrAddr( bool forget )
{
	if (forget || dbMgrAddr_ == Mercury::Address::NONE)
	{
		// do the look up
		int res = Mercury::MachineDaemon::findInterface( "DBInterface", 0,
														 dbMgrAddr_ );
		if (res != Mercury::REASON_SUCCESS)
		{
			ERROR_MSG( "Could not get DBMgr interface address\n" );
			dbMgrAddr_ = Mercury::Address::NONE;
		}
	}
	return dbMgrAddr_;
}


/**
 *	This method authenticates a user, identified by the given username and
 *	password, and checks out the corresponding user entity if it has not
 *	already been - the mailbox can then be retrieved via the
 *	BigWorld.lookUpEntityByName() call.
 *
 *	Already checked out user entities have the BaseApp script
 *	callback Base.onLogOnAttempt() called on.
 *
 *	@param username					the user name of the entity being logged
 *									on
 *	@param password					the password of the entity being logged on
 *	@param allowAlreadyLoggedOn		whether to throw an exception if the user
 *									entity already exists at the time of the
 *									log on
 */
int WebIntegration::logOn( const std::string & username,
		const std::string & password, bool allowAlreadyLoggedOn )
{
	if (!this->checkDBMgr())
	{
		return -1;
	}

	BlockingDBLogonHandler logonHandler( this->interface() );

	Mercury::Bundle request;

	request.startRequest( DBInterface::logOn, &logonHandler );

	// supply a blank address for a non-client proxy instance
	Mercury::Address blank( 0, 0 );
	request << blank;

	request << true; // off-channel

	// logon params
	std::string encryptionKey; // empty encryption key
	LogOnParamsPtr pParams = new LogOnParams(
		username, password, encryptionKey );

	// calculate digest
	MD5 md5;
	pEntityDescriptions_->addToMD5( md5 );
	MD5::Digest digest;
	md5.getDigest( digest );
	pParams->digest( digest );

	request << *pParams;

	this->interface().send( this->dbMgrAddr(), request );

	Mercury::Reason err = logonHandler.waitForReply();

	if (err != Mercury::REASON_SUCCESS)
	{
		PyErr_SetString( PyExc_IOError,
			Mercury::reasonToString(
				( Mercury::Reason ) err ) );
		return -1;
	}

	if (logonHandler.status() == LogOnStatus::LOGGED_ON)
	{
		return 0;
	}
	else if (allowAlreadyLoggedOn &&
			logonHandler.status() ==
				LogOnStatus::LOGIN_REJECTED_ALREADY_LOGGED_IN)
	{
		return 0;
	}
	else
	{
		PyObject *exception = getLogOnStatusException( logonHandler.status() );
		std::string errString;
		if (logonHandler.errString().size())
		{
			errString = logonHandler.errString();
		}
		else
		{
			errString = logonHandler.statusToString();
		}
		PyErr_SetString( exception, errString.c_str() );

		return -1;
	}
}


/**
 *	This method looks up a checked out entity by its entity type and
 *	identifier string.
 *
 *	@param entityTypeName	the name of the entity type
 *	@param entityName		the value of the identifier property string
 *
 *	@return the mailbox if such an entity exists and has been checked out, the
 *	Python object for True if such an entity exists but has not been checked
 *	out, the Python object for False if such an entity does not exist in the
 *	database, or NULL if an exception occurred.
 */
PyObject * WebIntegration::lookUpEntityByName(
		const std::string & entityTypeName, const std::string & entityName )
{
	EntityTypeID entityTypeID;

	if (!this->getEntityTypeByName( entityTypeName, entityTypeID ))
	{
		return NULL;
	}

	BlockingDBLookUpHandler handler( this->interface() );

	Mercury::Bundle b;
	b.startRequest( DBInterface::lookupEntityByName, &handler );
	b << entityTypeID << std::string( entityName ) << true /*offChannel*/;

	return this->lookUpEntityComplete( handler, b );
}


/**
 *	This method looks up a checked out entity by its entity type and
 *	database ID.
 *
 *	@param entityTypeName	the name of the entity type
 *	@param dbID				the value of the database ID
 *
 *	@return the mailbox if such an entity exists and has been checked out, the
 *	Python object for True if such an entity exists but has not been checked
 *	out, the Python object for False if such an entity does not exist in the
 *	database, or NULL if an exception occurred.
 */
PyObject * WebIntegration::lookUpEntityByDBID(
		const std::string & entityTypeName, DatabaseID dbID )
{
	EntityTypeID entityTypeID;

	if (!this->getEntityTypeByName( entityTypeName, entityTypeID ))
	{
		return NULL;
	}

	BlockingDBLookUpHandler handler( this->interface() );
	Mercury::Bundle b;

	DBInterface::lookupEntityArgs & lea = lea.startRequest(
		b, &handler );
	lea.entityTypeID = entityTypeID;
	lea.dbid = dbID;
	lea.offChannel = true;

	return this->lookUpEntityComplete( handler, b );
}


/**
 *	This method completes the lookup operation after the given bundle has had
 *	the appropriate request streamed onto it.
 *
 *	@param handler	the request reply handler for the look up request
 *	@param bundle	the bundle holding the streamed request
 *
 *	@return	the Python object to return back to script, or NULL if an exception
 *	has occurred.
 */
PyObject * WebIntegration::lookUpEntityComplete(
		BlockingDBLookUpHandler & handler, Mercury::Bundle & bundle )
{
	if (!this->checkDBMgr())
	{
		return NULL;
	}

	Mercury::Address dbMgrAddr = this->dbMgrAddr();
	this->interface().send( dbMgrAddr, bundle );

	Mercury::Reason reason = handler.waitForReply();

	if (reason != Mercury::REASON_SUCCESS)
	{
		PyErr_Format( PyExc_IOError, "database communication error: %s",
			Mercury::reasonToString( reason ) );
		return NULL;
	}

	// check the result
	BlockingDBLookUpHandler::Result result = handler.result();

	if (result == BlockingDBLookUpHandler::OK)
	{
		const EntityMailBoxRef & mbox = handler.mailbox();
		TRACE_MSG( "Mailbox: %s/id=%u,type=%d,component=%d\n",
			mbox.addr.c_str(),
			mbox.id,
			mbox.type(),
			mbox.component()
		);


		PyObject * mboxObj = Script::getData( mbox );
		if (mboxObj == Py_None)
		{
			ERROR_MSG( "Script::getData() returned None object\n" );
		}

		return mboxObj;
	}

	// we don't have a mailbox, try and find out why
	if (result == BlockingDBLookUpHandler::NOT_CHECKED_OUT)
	{
		Py_RETURN_TRUE;
	}

	if (result == BlockingDBLookUpHandler::DOES_NOT_EXIST)
	{
		Py_RETURN_FALSE;
	}

	if (result == BlockingDBLookUpHandler::TIMEOUT)
	{
		PyErr_SetString( PyExc_IOError,
			BlockingDBLookUpHandler::getResultString( result ) );
		return NULL;
	}

	if (result == BlockingDBLookUpHandler::PENDING)
	{
		PyErr_SetString( PyExc_RuntimeError, "handler is still pending" );
		return NULL;
	}

	if (result == BlockingDBLookUpHandler::GENERAL_ERROR)
	{
		PyErr_SetString( PyExc_SystemError,
			BlockingDBLookUpHandler::getResultString( result ) );
		return NULL;
	}

	PyErr_SetString( PyExc_SystemError, "unknown error" );
	return NULL;
}


/**
 *	Retrieves the database manager address, returns false and sets a Python
 *	exception if none is running, otherwise returns true.
 */
bool WebIntegration::checkDBMgr()
{
	const Mercury::Address& dbMgrAddr = this->dbMgrAddr();

	if (dbMgrAddr == Mercury::Address::NONE)
	{
		// Attempt to retrieve it.
		this->getDBMgrAddr( true );
	}

	if (dbMgrAddr == Mercury::Address::NONE)
	{
		PyErr_SetString( PyExc_IOError, "Server not running" );
		return false;
	}

	return true;
}


/**
 *	This method looks up the entity type ID of the given entity type.
 *	If it fails, it returns false.
 *
 *	@param name		the entity type name string
 *	@param id		if successful, this will be filled with the corresponding
 *					entity type ID value
 *	@return			success or failure
 */
bool WebIntegration::getEntityTypeByName(
		const std::string & name, EntityTypeID & id )
{
	if (!pEntityDescriptions_->nameToIndex( name,
			id ))
	{
		PyErr_Format( PyExc_Exception, "No such entity type: %s",
			name.c_str() );
		return false;
	}
	return true;
}


bool WebIntegration::checkBaseAppHashHistory()
{
	if (!this->updateBaseAppHashHistory())
	{
		// Reset our manager addresses, the server has probably gone down.
		dbMgrAddr_ = Mercury::Address::NONE;

		return false;
	}
	return true;
}


/**
 *	Update the BaseApp hash history and remap any base mailboxes as required.
 */ 
bool WebIntegration::updateBaseAppHashHistory()
{
	Mercury::Address baseAppMgrAddr;

	TRACE_MSG( "WebIntegration::updateBaseAppHashHistory: "
			"Started query to BaseAppMgr\n" );

	Mercury::Reason res = Mercury::MachineDaemon::findInterface( 
		"BaseAppMgrInterface", 0, baseAppMgrAddr );

	if (res != Mercury::REASON_SUCCESS)
	{
		ERROR_MSG( "WebIntegration::updateBaseAppHashHistory: "
				"Could not look up BaseAppMgr, server may have gone away (%s)\n",
			Mercury::reasonToString( res ) );
		return false;
	}

	// Send the request, and wait for it.
	BackupHashChainRequestHandler handler( *pBackupHashChain_ );
	Mercury::BlockingReplyHandler blockingHandler( this->interface(),  
		&handler );

	Mercury::Bundle bundle;
	bundle.startRequest( BaseAppMgrInterface::requestBackupHashChain, 
		&blockingHandler );

	this->interface().send( baseAppMgrAddr, bundle );

	Mercury::Reason reason = blockingHandler.waitForReply();

	if (reason != Mercury::REASON_SUCCESS)
	{
		ERROR_MSG( "WebIntegration::updateBaseAppHashHistory: "
				"Could not successfully request backup hash chain: %s\n", 
			Mercury::reasonToString( reason ) );
		return false;
	}

	if (!handler.isOK())
	{
		ERROR_MSG( "WebIntegration::updateBaseAppHashHistory: "
				"Error occurred while waiting for backup hash chain reply\n" );
		return false;
	}

	TRACE_MSG( "WebIntegration::updateBaseAppHashHistory: "
			"Got the BaseApp hash history successfully\n" );

	WebEntityMailBox::remapMailBoxes( *pBackupHashChain_ );

	return true;
}


/**
 *	Return the network interface used for this component.
 *
 *	This network interface is created on demand to cater for Apache forking.
 *
 *	@return the network interface
 */
Mercury::NetworkInterface & WebIntegration::interface()
{
	// Because Apache preforks processes, we don't want to use the network
	// interface that was created in the parent process. We create the network
	// interface on-demand in the child Apache processes here.

	if (!createPid_ || createPid_ != getpid())
	{
		pInterface_.reset( new Mercury::NetworkInterface( pDispatcher_.get(),
			Mercury::NETWORK_INTERFACE_INTERNAL ) );
		INFO_MSG( "WebIntegration::interface: "
				"(re-)creating network interface: %s\n",
			pInterface_->address().c_str() );
		createPid_ = getpid();
	}
	return *pInterface_;
}


/**
 *	Constructor for LoggerEndpoint.
 */
WebIntegration::LoggerEndpoint::LoggerEndpoint() :
		Endpoint()
{
	this->switchSocket();
}


/**
 *	This method switches the socket it uses to recreated socket.
 */
bool WebIntegration::LoggerEndpoint::switchSocket()
{
	if (this->good())
	{
		this->close();
	}

	this->socket( SOCK_DGRAM );

	if (!this->good())
	{
		ERROR_MSG( "AnyEndpoint::switchSocket: socket() failed\n" );
		return true;
	}

	if (this->setnonblocking( true ))	// make it nonblocking
	{
		ERROR_MSG( "AnyEndpoint::switchSocket: fcntl(O_NONBLOCK) failed\n" );
		return true;
	}

	if (this->bind( 0, INADDR_ANY ))
	{
		ERROR_MSG( "AnyEndpoint::switchSocket: bind() failed\n" );
		this->close();
		return true;
	}

	return false;
}


// web_integration.cpp
