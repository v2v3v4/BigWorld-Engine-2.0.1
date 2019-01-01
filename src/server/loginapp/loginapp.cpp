/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "loginapp.hpp"

#include "database_reply_handler.hpp"
#include "login_int_interface.hpp"
#include "loginapp_config.hpp"
#include "nat_config.hpp"
#include "status_check_watcher.hpp"

#include "common/doc_watcher.hpp"

#include "dbmgr/db_interface.hpp"

#include "connection/log_on_status.hpp"
#include "connection/login_interface.hpp"
#include "connection/rsa_stream_encoder.hpp"

#include "network/encryption_filter.hpp"
#include "network/msgtypes.hpp"	// for angleToInt8
#include "network/nub_exception.hpp"
#include "network/once_off_packet.hpp"
#include "network/portmap.hpp"
#include "network/watcher_nub.hpp"

#include "resmgr/bwresource.hpp"

#include "server/bwconfig.hpp"
#include "server/util.hpp"
#include "server/writedb.hpp"

#include <pwd.h>
#include <signal.h>

#include <sys/types.h>


DECLARE_DEBUG_COMPONENT( 0 )

/// Timer period when updating login statistics
const uint32 LoginApp::UPDATE_STATS_PERIOD = 1000000;

/// LoginApp Singleton.
BW_SINGLETON_STORAGE( LoginApp )

namespace // anonymous
{
uint gNumLogins = 0;
uint gNumLoginFailures = 0;
uint gNumLoginAttempts = 0;

char g_buildID[81] = "Build ID";

// -----------------------------------------------------------------------------
// Section: Misc
// -----------------------------------------------------------------------------


bool commandStopServer( std::string & output, std::string & value )
{
	if (LoginApp::pInstance())
	{
		LoginApp::pInstance()->controlledShutDown();
	}

	return true;
}

} // namespace (anonymous)


// -----------------------------------------------------------------------------
// Section: LoginApp
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
LoginApp::LoginApp( Mercury::EventDispatcher & mainDispatcher,
		Mercury::NetworkInterface & intInterface ) :
	ServerApp( mainDispatcher, intInterface ),
	pLogOnParamsEncoder_( NULL ),
	extInterface_( &mainDispatcher,
			Mercury::NETWORK_INTERFACE_EXTERNAL,
			htons( BWConfig::get( "loginApp/externalPorts/port", PORT_LOGIN ) ),
			Config::externalInterface().c_str() ),
	netMask_(),
	externalIP_( 0 ),
	systemOverloaded_( 0 ),
	lastRateLimitCheckTime_( 0 ),
	numAllowedLoginsLeft_( 0 ),
	loginStats_()
{
	extInterface_.pExtensionData( static_cast< ServerApp * >( this ) );

	// If the initial LoginApp port failed to bind, attempt to rebind on
	// any of the specified LoginApp ports.
	if (!extInterface_.isGood())
	{
		std::string interfaceStr = Config::externalInterface();

		LoginAppPortList loginPorts;

		DataSectionPtr pPorts =
			BWConfig::getSection( "loginApp/externalPorts" );

		if (pPorts)
		{
			pPorts->readInts( "port", loginPorts );
		}

		for (uint i = 1; i < loginPorts.size(); ++i)
		{
			uint16 port = static_cast< uint16 >( loginPorts[ i ] );

			INFO_MSG( "LoginApp::init: "
					"Attempting to re-bind external interface to %s:%d\n",
				interfaceStr.c_str(), port );

			if (extInterface_.recreateListeningSocket(
					htons( port ), interfaceStr.c_str() ))
			{
				break;
			}
		}


		// NB: if shouldShutDownIfPortUsed is true, LoginApp::init, will
		//     check if the interface is bound and terminate if not.
		if (!extInterface_.isGood() && !Config::shouldShutDownIfPortUsed())
		{
			INFO_MSG( "LoginApp::init: Could not bind external interface to "
						"ports specified in bw.xml. Trying any available "
						"port\n" );

			extInterface_.recreateListeningSocket( 0, interfaceStr.c_str() );
		}
	}


	extInterface_.isVerbose( Config::verboseExternalInterface() );
}


/**
 *	Destructor.
 */
LoginApp::~LoginApp()
{
	this->extInterface().prepareForShutdown();
	statsTimer_.cancel();
}


/**
 *	This method initialises this object.
 */
bool LoginApp::init( int argc, char * argv[] )
{
	if (!this->ServerApp::init( argc, argv ))
	{
		return false;
	}

	if (!extInterface_.isGood())
	{
		ERROR_MSG( "Loginapp::init: "
			"Unable to bind external interface to any specified ports.\n" );
		return false;
	}

	INFO_MSG( "Supported client login versions: %u to %u\n", 
		OLDEST_SUPPORTED_CLIENT_LOGIN_VERSION, LOGIN_VERSION );

#ifdef USE_OPENSSL
	std::string privateKeyPath = Config::privateKey();

	if (!privateKeyPath.empty())
	{
		RSAStreamEncoder * pEncoder = 
			new RSAStreamEncoder( /* keyIsPrivate: */ true );

		if (!pEncoder->initFromKeyPath( privateKeyPath ))
		{
			delete pEncoder;
			return false;
		}
		pLogOnParamsEncoder_.reset( pEncoder ); 
	}
	else
	{
		ERROR_MSG( "LoginApp::init: "
			"You must specify a private key to use with the "
			"<loginApp/privateKey> option in bw.xml\n" );

		return false;
	}
#endif

	if (!this->intInterface().isGood())
	{
		ERROR_MSG( "Failed to create Nub on internal interface.\n" );
		return false;
	}

	if ((extInterface_.address().ip == 0) ||
			(this->intInterface().address().ip == 0))
	{
		ERROR_MSG( "LoginApp::init: Failed to open UDP ports. "
				"Maybe another process already owns it\n" );
		return false;
	}

	BW_INIT_WATCHER_DOC( "loginapp" );

	std::string netMask = NATConfig::localNetMask();
	netMask_.parse( netMask.c_str() );

	// Should use inet_aton but this does not work under Windows.
	std::string extAddr = NATConfig::externalAddress();
	externalIP_ = inet_addr( extAddr.c_str() );

	DataSectionPtr pExternalAddresses =
		BWConfig::getSection( "networkAddressTranslation/externalAddresses" );

	if (pExternalAddresses)
	{
		DataSection::iterator iter = pExternalAddresses->begin();

		while (iter != pExternalAddresses->end())
		{
			DataSectionPtr pDS = *iter;
			uint32 intIP = inet_addr( pDS->readString( "internal" ).c_str() );
			uint32 extIP = inet_addr( pDS->readString( "external" ).c_str() );

			externalIPs_[ intIP ] = extIP;

			++iter;
		}
	}

	if (netMask_.containsAddress( this->intInterface().address().ip ) ||
		netMask_.containsAddress( extInterface_.address().ip ))
	{
		INFO_MSG( "Local subnet: %s\n", netMask.c_str() );
		INFO_MSG( "NAT external addr: %s\n", extAddr.c_str() );
	}
	else
	{
		WARNING_MSG( "LoginApp::LoginApp: "
					"localNetMask %s does not match local ip %s\n",
				netMask.c_str(), this->intInterface().address().c_str() );
		INFO_MSG( "Not using localNetMask\n" );

		netMask_.clear();
	}

	MF_WATCH( "numLogins", gNumLogins );
	MF_WATCH( "numLoginFailures", gNumLoginFailures );
	MF_WATCH( "numLoginAttempts", gNumLoginAttempts );

	// ---- What used to be in loginsvr.cpp

	ReviverSubject::instance().init( &this->intInterface(), "loginApp" );

	// make sure the interface came up ok
	if (extInterface_.address().ip == 0)
	{
		CRITICAL_MSG( "login::init: Failed to start Nub.\n"
			"\tLog in port is probably in use.\n"
			"\tIs there another login server running on this machine?\n" );
		return false;
	}

	INFO_MSG( "External address = %s\n", extInterface_.address().c_str() );
	INFO_MSG( "Internal address = %s\n", this->intInterface().address().c_str() );

	if (BW_INIT_ANONYMOUS_CHANNEL_CLIENT( dbMgr_, this->intInterface(),
			LoginIntInterface, DBInterface, 0 ))
	{
		INFO_MSG( "LoginApp::init: DB addr: %s\n",
			this->dbMgr().channel().c_str() );
	}
	else
	{
		INFO_MSG( "LoginApp::init: Database not ready yet.\n" );
	}

	LoginInterface::registerWithInterface( extInterface_ );
	LoginIntInterface::registerWithInterface( this->intInterface() );

	MF_WATCH( "systemOverloaded", systemOverloaded_ );

	if (Config::allowProbe() && Config::isProduction())
	{
		ERROR_MSG( "Production Mode: bw.xml/loginApp/allowProbe is enabled. "
			"This is a development-time feature only and should be disabled "
			"during production.\n" );
	}

	// Enable latency / loss on the external interface
	extInterface_.setLatency( Config::externalLatencyMin(),
			Config::externalLatencyMax() );
	extInterface_.setLossRatio( Config::externalLossRatio() );

	if (extInterface_.hasArtificialLossOrLatency())
	{
		WARNING_MSG( "LoginApp::init: External Nub loss/latency enabled\n" );
	}

	// Set up the rate limiting parameters
	if (Config::rateLimitEnabled())
	{
		INFO_MSG( "LoginApp::init: "
				"Login rate limiting enabled: period = %u, limit = %d\n",
			Config::rateLimitDuration(), Config::loginRateLimit() );
	}

	numAllowedLoginsLeft_ = Config::loginRateLimit();
	lastRateLimitCheckTime_ = timestamp();

	Mercury::Reason reason =
		LoginIntInterface::registerWithMachined( this->intInterface(), 0 );

	if (reason != Mercury::REASON_SUCCESS)
	{
		ERROR_MSG( "LoginApp::init: Unable to register with interface. "
			"Is machined running?\n");
		return false;
	}

	if (Config::registerExternalInterface())
	{
		LoginInterface::registerWithMachined( extInterface_, 0 );
	}

	// Handle USR1 for controlled shutdown.
	this->enableSignalHandler( SIGUSR1 );

	// Start up watchers
	BW_REGISTER_WATCHER( 0, "loginapp", "LoginApp", "loginApp",
			mainDispatcher_, this->intInterface().address() );

	Watcher & root = Watcher::rootWatcher();
	this->addWatchers( root );
	root.addChild( "nubExternal", Mercury::NetworkInterface::pWatcher(), 
		&extInterface_ );

	root.addChild( "command/statusCheck", new StatusCheckWatcher() );
	root.addChild( "command/shutDownServer",
			new NoArgFuncCallableWatcher( commandStopServer,
				CallableWatcher::LOCAL_ONLY,
				"Shuts down the entire server" ) );

	// root.addChild( "dbMgr", Mercury::Channel::pWatcher(),
	//		&this->dbMgr().channel() );

	WatcherPtr pStatsWatcher = new DirectoryWatcher();
	pStatsWatcher->addChild( "rateLimited",
			makeWatcher( loginStats_, &LoginStats::rateLimited ) );
	pStatsWatcher->addChild( "repeatedForAlreadyPending",
			makeWatcher( loginStats_, &LoginStats::pending ) );
	pStatsWatcher->addChild( "failures",
			makeWatcher( loginStats_, &LoginStats::fails ) );
	pStatsWatcher->addChild( "successes",
			makeWatcher( loginStats_, &LoginStats::successes ) );
	pStatsWatcher->addChild( "all",
			makeWatcher( loginStats_, &LoginStats::all ) );

	{
		// watcher doesn't like const-ness
		static uint32 s_updateStatsPeriod = LoginApp::UPDATE_STATS_PERIOD;
		pStatsWatcher->addChild( "updatePeriod",
				makeWatcher( s_updateStatsPeriod ) );
	}

	root.addChild( "averages", pStatsWatcher );

	statsTimer_ =
		mainDispatcher_.addTimer( UPDATE_STATS_PERIOD, &loginStats_, NULL );

	if (!this->isDBReady())
	{
		INFO_MSG( "Database is not ready yet\n" );
	}

	return true;
}


/**
 *	This method performs the main loop of this application.
 */
bool LoginApp::run()
{
	mainDispatcher_.processUntilBreak();

	INFO_MSG( "LoginApp::run: Terminating normally.\n" );

	if (this->isDBReady() && Config::shutDownSystemOnExit())
	{
		Mercury::Bundle	& dbBundle = this->dbMgr().bundle();
		DBInterface::controlledShutDownArgs args;
		args.stage = SHUTDOWN_REQUEST;
		dbBundle << args;
		this->dbMgr().send();

		this->intInterface().processUntilChannelsEmpty();
	}

	return true;
}


/**
 *	This method sends a failure message back to the client.
 */
void LoginApp::sendFailure( const Mercury::Address & addr,
	Mercury::ReplyID replyID, int status, const char * pDescription,
	LogOnParamsPtr pParams )
{
	if (status == LogOnStatus::LOGIN_REJECTED_RATE_LIMITED)
	{
		loginStats_.incRateLimited();
	}
	else
	{
		loginStats_.incFails();
	}

	if (pDescription == NULL)
		pDescription = "";

	INFO_MSG( "LoginApp::sendFailure: "
		"LogOn for %s failed, LogOnStatus %d, description '%s'.\n",
			addr.c_str(), status, pDescription );

	++gNumLoginFailures;

	Mercury::Bundle bundle;

	// Replies to failed login attempts are not reliable as that would be a
	// DOS'ing vulnerability
	bundle.startReply( replyID, Mercury::RELIABLE_NO );
	bundle << (int8)status;
	bundle << pDescription;

	LoginApp & app = LoginApp::instance();
	app.extInterface_.send( addr, bundle );

	if (*pDescription == 0)
	{
		WARNING_MSG( "LoginApp::sendFailure: "
			"Sent LogOnStatus %d without a description (bad form)", status );
	}

	// Erase the cache mapping for this attempt if appropriate
	if (pParams)
	{
		app.cachedLoginMap_.erase( addr );
	}
}


/**
 *	This method is the one that actually receives the login requests.
 */
void LoginApp::login( const Mercury::Address & source,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data )
{

	if (Config::rateLimitEnabled() &&
		(timestamp() >
			lastRateLimitCheckTime_ + Config::rateLimitDurationInStamps()) )
	{
		// reset number of allowed logins per time block if we're rate limiting
		numAllowedLoginsLeft_ = Config::loginRateLimit();
		lastRateLimitCheckTime_ = timestamp();
	}

	if (!Config::allowLogin())
	{
		WARNING_MSG( "LoginApp::login: Dropping login attempt from %s because "
				"config/allowLogin is false.\n",
			source.c_str() );

		this->sendFailure( source, header.replyID,
			LogOnStatus::LOGIN_REJECTED_LOGINS_NOT_ALLOWED,
			"Logins currently not permitted" );
		data.finish();
		return;
	}
	if (!source.ip)
	{
		// spoofed address trying to login as web client!
		ERROR_MSG( "LoginApp::login: Spoofed empty address\n" );
		data.retrieve( data.remainingLength() );
		loginStats_.incFails();
		return;
	}

	bool isReattempt = (cachedLoginMap_.count( source ) != 0);
 	INFO_MSG( "LoginApp::login: %s from %s\n",
		isReattempt ? "Re-attempt" : "Attempt", source.c_str() );

	++gNumLoginAttempts;

	uint32 version = 0;
	data >> version;

	if (data.error())
	{
		ERROR_MSG( "LoginApp::login: "
			"Not enough data on stream (%d bytes total)\n",
			header.length );

		this->sendFailure( source, header.replyID,
			LogOnStatus::LOGIN_MALFORMED_REQUEST,
			"Undersized login message");

		return;
	}

	if (	(version < OLDEST_SUPPORTED_CLIENT_LOGIN_VERSION) || 
			(LOGIN_VERSION < version))
	{
		ERROR_MSG( "LoginApp::login: "
				"User at %s tried to log on with version %u. "
				"Expected between %u and %u.\n",
			source.c_str(), version, OLDEST_SUPPORTED_CLIENT_LOGIN_VERSION, 
			LOGIN_VERSION );

		char msg[BUFSIZ];

		bw_snprintf( msg, sizeof(msg), "Incorrect protocol version. "
				"Client version is %u, server supports versions "
				"between %u and %u. Your %s is out of date.", 
			version, 
			OLDEST_SUPPORTED_CLIENT_LOGIN_VERSION, LOGIN_VERSION,
		  	version < OLDEST_SUPPORTED_CLIENT_LOGIN_VERSION ? 
				"client" : "server" );

		this->sendFailure( source, header.replyID,
			LogOnStatus::LOGIN_BAD_PROTOCOL_VERSION, msg );

		data.finish();

		return;
	}

	bool isRateLimited = Config::rateLimitEnabled() &&
			(numAllowedLoginsLeft_ == 0);
	if (isRateLimited)
	{
		NOTICE_MSG( "LoginApp::login: "
				"Login from %s not allowed due to rate limiting\n",
				source.c_str() );

		this->sendFailure( source, header.replyID,
				LogOnStatus::LOGIN_REJECTED_RATE_LIMITED,
				"Logins temporarily disallowed due to rate limiting" );
		data.finish();
		return;
	}

	if (!this->isDBReady())
	{
		INFO_MSG( "LoginApp::login: "
			"Attempted login when database not yet ready.\n" );

		this->sendFailure( source, header.replyID,
			LogOnStatus::LOGIN_REJECTED_DB_NOT_READY, "DB not ready" );
		data.finish();
		return;
	}

	if (systemOverloaded_ != 0)
	{
		if (systemOverloadedTime_ + stampsPerSecond() < timestamp())
		{
			systemOverloaded_ = 0;
		}
		else
		{
			INFO_MSG( "LoginApp::login: "
				"Attempted login when system overloaded or not yet ready.\n" );
			this->sendFailure( source, header.replyID,
				systemOverloaded_, "System overloaded wait state." );
			data.finish();
			return;
		}
	}

	// Read off login parameters
	LogOnParamsPtr pParams = new LogOnParams();

	// Save the message so we can have multiple attempts to read it
	int dataLength = data.remainingLength();
	const void * pDataData = data.retrieve( dataLength );


	// First check whether this is a repeat attempt from a recent pending
	// login before attempting to decrypt and log in.
	if (this->handleResentPendingAttempt( source, header.replyID ))
	{
		// ignore this one, it's in progress
		loginStats_.incPending();
		return;
	}

	StreamEncoder * pEncoder = pLogOnParamsEncoder_.get();

	do
	{
		MemoryIStream attempt = MemoryIStream( pDataData, dataLength );

		if (pParams->readFromStream( attempt, pEncoder ))
		{
			// We are successful, move on
			break;
		}

		if (pEncoder && Config::allowUnencryptedLogins())
		{
			// If we tried using encryption, have another go without it
			pEncoder = NULL;
			continue;
		}

		// Nothing left to try, bail out
		this->sendFailure( source, header.replyID,
			LogOnStatus::LOGIN_MALFORMED_REQUEST,
			"Could not destream login parameters. Possibly caused by "
			"mis-matching LoginApp keypair." );
		return;
		// Does not reach here
	}
	while (false);

	// First check whether this is a repeat attempt from a recent
	// resolved login before attempting to log in.
	if (this->handleResentCachedAttempt( source, pParams, header.replyID ))
	{
		// ignore this one, we've seen it recently
		return;
	}

	if (Config::rateLimitEnabled())
	{
		// We've done the hard work of decrypting the logon parameters now, so
		// we count this as a login with regards to rate-limiting.
		--numAllowedLoginsLeft_;
	}

	// Check that it has encryption key if we disallow unencrypted logins
	if (pParams->encryptionKey().empty() && !Config::allowUnencryptedLogins())
	{
		this->sendFailure( source, header.replyID,
			LogOnStatus::LogOnStatus::LOGIN_MALFORMED_REQUEST,
			"No encryption key supplied, and server is not allowing "
				"unencrypted logins." );
		return;
	}


	INFO_MSG( "Logging in %s{%s} (%s)\n",
		pParams->username().c_str(),
		pParams->password().c_str(),
		source.c_str() );

	// Remember that this attempt is now in progress and discard further
	// attempts from that address for some time after it completes.
	cachedLoginMap_[ source ].reset();
	cachedLoginMap_[ source ].pParams( pParams );

	DatabaseReplyHandler * pDBHandler =
		new DatabaseReplyHandler( *this, source, header.replyID, pParams );

	Mercury::Bundle	& dbBundle = this->dbMgr().bundle();
	dbBundle.startRequest( DBInterface::logOn, pDBHandler );

	dbBundle << source << false /*off channel*/ << *pParams;

	this->dbMgr().send();
}


/**
 *
 */
uint32 LoginApp::externalIPFor( uint32 ip ) const
{
	ExternalIPs::const_iterator iter = externalIPs_.find( ip );

	if (iter != externalIPs_.end())
	{
		return iter->second;
	}

	return externalIP_;
}


// -----------------------------------------------------------------------------
// Section: ProbeHandler
// -----------------------------------------------------------------------------

/**
 *	This method handles the probe message.
 */
void LoginApp::probe( const Mercury::Address & source,
	Mercury::UnpackedMessageHeader & header,
	BinaryIStream & /*data*/ )
{
	if (Config::logProbes())
	{
		INFO_MSG( "LoginApp::probe: Got probe from %s\n", source.c_str() );
	}

	if (!Config::allowProbe() || header.length != 0) return;

	Mercury::Bundle bundle;
	bundle.startReply( header.replyID, Mercury::RELIABLE_NO );

	char buf[256];
	gethostname( buf, sizeof(buf) ); buf[sizeof(buf)-1]=0;
	bundle << PROBE_KEY_HOST_NAME << buf;

#ifndef _WIN32
	struct passwd *pwent = getpwuid( getUserId() );
	const char * username = pwent ? (pwent->pw_name ? pwent->pw_name : "") : "";
	bundle << PROBE_KEY_OWNER_NAME << username;

	if (!pwent)
	{
		ERROR_MSG( "LoginApp::probe: "
			"Process uid %d doesn't exist on this system!\n", getUserId() );
	}

#else
	DWORD bsz = sizeof(buf);
	if (!GetUserName( buf, &bsz )) buf[0]=0;
	bundle << PROBE_KEY_OWNER_NAME << buf;
#endif

	bw_snprintf( buf, sizeof(buf), "%d", gNumLogins );
	bundle << PROBE_KEY_USERS_COUNT << buf;

	bundle << PROBE_KEY_UNIVERSE_NAME << BWConfig::get( "universe" );
	bundle << PROBE_KEY_SPACE_NAME << BWConfig::get( "space" );

	bundle << PROBE_KEY_BINARY_ID << g_buildID;

	extInterface_.send( source, bundle );
}


/**
 *	This method sends a reply to a client indicating that logging in has been
 *	successful. It also caches this information so that it can be resent if
 *	necessary.
 */
void LoginApp::sendAndCacheSuccess( const Mercury::Address & addr,
		Mercury::ReplyID replyID, const LoginReplyRecord & replyRecord,
		LogOnParamsPtr pParams )
{
	this->sendSuccess( addr, replyID, replyRecord,
		pParams->encryptionKey() );

	cachedLoginMap_[ addr ].replyRecord( replyRecord );

	// Do not let the map get too big. Just check every so often to get rid of
	// old caches.

	if (cachedLoginMap_.size() > 100)
	{
		CachedLoginMap::iterator iter = cachedLoginMap_.begin();

		while (iter != cachedLoginMap_.end())
		{
			CachedLoginMap::iterator prevIter = iter;
			++iter;

			if (prevIter->second.isTooOld())
			{
				cachedLoginMap_.erase( prevIter );
			}
		}
	}
}


/**
 *	This method sends a reply to a client indicating that logging in has been
 *	successful.
 */
void LoginApp::sendSuccess( const Mercury::Address & addr,
	Mercury::ReplyID replyID, const LoginReplyRecord & replyRecord,
	const std::string & encryptionKey )
{
	Mercury::Bundle b;
	b.startReply( replyID, Mercury::RELIABLE_NO );
	b << (int8)LogOnStatus::LOGGED_ON;

#ifdef USE_OPENSSL
	if (!encryptionKey.empty())
	{
		// We have to encrypt the reply record because it contains the session key
		Mercury::EncryptionFilter filter( encryptionKey );
		MemoryOStream clearText;
		clearText << replyRecord;
		filter.encryptStream( clearText, b );
	}
	else
#endif
	{
		b << replyRecord;
	}

	loginStats_.incSuccesses();
	++gNumLogins;

	this->extInterface().send( addr, b );
}


/**
 *	This method checks whether there is a login in progress from this
 *	address.
 */
bool LoginApp::handleResentPendingAttempt( const Mercury::Address & addr,
		Mercury::ReplyID replyID )
{
	CachedLoginMap::iterator iter = cachedLoginMap_.find( addr );

	if (iter != cachedLoginMap_.end())
	{
		CachedLogin & cache = iter->second;

		if (cache.isPending())
		{
			DEBUG_MSG( "LoginApp::handleResentPendingAttempt: "
					"Ignoring repeat attempt from %s "
					"while another attempt is in progress (for '%s')\n",
				addr.c_str(),
				cache.pParams()->username().c_str() );

			return true;
		}
	}
	return false;
}


/**
 *	This method checks whether there is a cached login attempt from this
 *	address. If there is, it is assumed that the previous reply was dropped and
 *	this one is resent.
 */
bool LoginApp::handleResentCachedAttempt( const Mercury::Address & addr,
		LogOnParamsPtr pParams, Mercury::ReplyID replyID )
{
	CachedLoginMap::iterator iter = cachedLoginMap_.find( addr );

	if (iter != cachedLoginMap_.end())
	{
		CachedLogin & cache = iter->second;
		if (!cache.isTooOld() && *cache.pParams() == *pParams)
		{
			DEBUG_MSG( "%s retransmitting successful login to %s\n",
					   addr.c_str(),
					   cache.pParams()->username().c_str() );
			this->sendSuccess( addr, replyID, cache.replyRecord(),
				cache.pParams()->encryptionKey() );

			return true;
		}
	}

	return false;
}


/**
 *  Handles incoming shutdown requests.  This is basically another way of
 *  triggering a controlled system shutdown instead of sending a SIGUSR1.
 */
void LoginApp::controlledShutDown( const Mercury::Address &source,
	Mercury::UnpackedMessageHeader &header,
	BinaryIStream &data )
{
	INFO_MSG( "LoginApp::controlledShutDown: "
		"Got shutdown command from %s\n", source.c_str() );

	this->controlledShutDown();
}


void LoginApp::controlledShutDown()
{
	Config::shutDownSystemOnExit.set( true );
	mainDispatcher_.breakProcessing();
}


/**
 *	Signal handler.
 */
void LoginApp::onSignalled( int sigNum )
{
	this->ServerApp::onSignalled( sigNum );

	if (sigNum == SIGUSR1)
	{
		this->controlledShutDown();
	}
}


// -----------------------------------------------------------------------------
// Section: CachedLogin
// -----------------------------------------------------------------------------

/**
 *  This method returns true if this login is pending, i.e. we are waiting on
 *  the DBMgr to tell us whether or not the login is successful.
 */
bool LoginApp::CachedLogin::isPending() const
{
	return creationTime_ == 0;
}


/**
 *	This method returns whether or not this cache is too old to use.
 */
bool LoginApp::CachedLogin::isTooOld() const
{
	const uint64 MAX_LOGIN_DELAY = LoginAppConfig::maxLoginDelayInStamps();

	return !this->isPending() &&
		(::timestamp() - creationTime_ > MAX_LOGIN_DELAY);
}


/**
 *  This method sets the reply record into this cached object, and is called
 *  when the DBMgr replies.
 */
void LoginApp::CachedLogin::replyRecord( const LoginReplyRecord & record )
{
	replyRecord_ = record;
	creationTime_ = ::timestamp();
}



// -----------------------------------------------------------------------------
// Section: LoginStats
// -----------------------------------------------------------------------------


// Make the EMA bias equivalent to having the most recent 5 samples represent
// 86% of the total weight. This is purely arbitrary, and may be adjusted to
// increase or decrease the sensitivity of the login statistics as reported in
// the 'averages' watcher directory.
static const uint WEIGHTING_NUM_SAMPLES = 5;

/**
 * The EMA bias for the login statistics.
 */
const float LoginApp::LoginStats::BIAS = 2.f / (WEIGHTING_NUM_SAMPLES + 1);

LoginApp::LoginStats::LoginStats():
			fails_( BIAS ),
			rateLimited_( BIAS ),
			pending_( BIAS ),
			successes_( BIAS ),
			all_( BIAS )
{}

// loginapp.cpp
