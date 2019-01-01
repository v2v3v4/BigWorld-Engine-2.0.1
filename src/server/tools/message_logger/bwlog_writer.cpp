/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "bwlog_writer.hpp"

#include "constants.hpp"
#include "user_log_writer.hpp"
#include "mlutil.hpp"
#include "log_entry.hpp"

#include <libgen.h>


/**
 * Constructor.
 */
BWLogWriter::BWLogWriter() :
	writeToStdout_( false ),
	maxSegmentSize_( DEFAULT_SEGMENT_SIZE_MB << 20 )
{ }


/**
 * Destructor.
 */
BWLogWriter::~BWLogWriter()
{
	DEBUG_MSG( "BWLogWriter::~BWLogWriter(): shutting down\n" );

	// Ensure the reference counted objects are destroyed.
	userLogs_.clear();

	// If the current PID file is the same as our process, get rid of the
	// files we've created.
	if (pid_.getValue() == mf_getpid())
	{
		// Clean up the PID file so someone else can write here
		if (unlink( pid_.filename() ))
		{
			ERROR_MSG( "BWLogWriter::~BWLogWriter(): "
				"Unable to remove PID file '%s': %s\n",
				pid_.filename(), strerror( errno ) );
		}

		// Clean up the active_files
		activeFiles_.deleteFile();
	}
}


/**
 * Initialises the BWLogWriter instance with any available config options.
 *
 * @returns true on success, false on error.
 */
bool BWLogWriter::initFromConfig( const ConfigReader &config )
{
	bool isConfigOK = true;

	std::string tmpString;
	if (config.getValue( "message_logger", "segment_size", tmpString ))
	{
		if (sscanf( tmpString.c_str(), "%d", &maxSegmentSize_ ) != 1)
		{
			ERROR_MSG( "BWLogWriter::initFromConfig: Failed to convert "
				"'segment_size' to an integer.\n" );
			isConfigOK = false;
		}
	}

	if (config.getValue( "message_logger", "logdir", logDir_))
	{

		// If logdir begins with a slash, it is absolute, otherwise it is
		// relative to the directory the config file resides in
		if (logDir_.c_str()[0] != '/')
		{
			std::string newLogDir;

			char cwd[ 512 ], confDirBuf[ 512 ];
			char *confDir;

			memset( cwd, 0, sizeof( cwd ) );

			const char *confFilename = config.filename();
			// If the path to the config file isn't absolute...
			if (confFilename[0] != '/')
			{
				if (getcwd( cwd, sizeof( cwd ) ) == NULL)
				{
					ERROR_MSG( "BWLogWriter::init: Failed to getcwd(). %s\n",
						strerror( errno ) );
					isConfigOK = false;
				}
			}
			strcpy( confDirBuf, confFilename );
			confDir = dirname( confDirBuf );

			if (cwd[0])
			{
				newLogDir = cwd;
				newLogDir.push_back( '/' );
			}
			newLogDir += confDir;
			newLogDir .push_back( '/' );
			newLogDir += logDir_.c_str();

			logDir_ = newLogDir;
		}
	}

	return isConfigOK;
}


/**
 * Callback method invoked from BWLogCommon during initUserLogs().
 *
 * This method creates a unique UserLog instance for the newly discovered
 * user.
 */
bool BWLogWriter::onUserLogInit( uint16 uid, const std::string &username )
{
	// In write mode we actually load up the UserLog, since write mode
	// UserLogs only keep a pair of file handles open.
	return (this->createUserLog( uid, username ) != NULL);
}


/**
 * Initialises the BWLogWriter instance for use.
 *
 * @returns true on successful initialisation, false on error.
 */
bool BWLogWriter::init( const ConfigReader &config, const char *root )
{
	static const char *mode = "a+";

	// Read config in append mode only, since the Python will always pass a
	// 'root' parameter in read mode
	if (!this->initFromConfig( config ))
	{
		ERROR_MSG( "BWLogWriter::init: Failed to read config file\n" );
		return false;
	}

	// Only use logdir from the config file if none has been provided
	if (root == NULL)
	{
		root = logDir_.c_str();
	}

	this->initRootLogPath( root );

	// Make sure the root directory has the access we want
	if (!MLUtil::softMkDir( rootLogPath_.c_str() ))
	{
		ERROR_MSG( "BWLogWriter::init: Root logdir (%s) not accessible in "
			"write mode.\n", rootLogPath_.c_str() );
		return false;
	}

	// Make sure another logger isn't already logging to this directory
	if (!pid_.init( pid_.join( root, "pid" ), mode, mf_getpid() ))
	{
		ERROR_MSG( "BWLogWriter::init: Another logger seems to be writing "
			"to %s\n", root );
		return false;
	}

	// Call the parent class common initialisation
	this->initCommonFiles( mode );

	this->initUserLogs( mode );

	if (!activeFiles_.init( rootLogPath_, &userLogs_ ))
	{
		ERROR_MSG( "BWLogWriter::init: Failed to init 'active_files'\n" );
		return false;
	}

	// Now all the UserLogs have been opened, update the 'active_files'
	if (!activeFiles_.update())
	{
		ERROR_MSG( "BWLogWriter::init: Failed to touch 'active_files'\n" );
		return false;
	}

	return true;
}


/**
 * Terminates all current log segments.
 *
 * @returns true on success, false on error.
 */
bool BWLogWriter::roll()
{
	INFO_MSG( "Rolling logs\n" );

	UserLogs::iterator iter = userLogs_.begin();

	while (iter != userLogs_.end())
	{
		UserLogWriterPtr pUserLog = iter->second;

		pUserLog->rollActiveSegment();

		// Since the userlog now has no active segments, drop the object
		// entirely since it's likely we will be mlrm'ing around this time which
		// could blow away this user's directory.  If that happens, then a new
		// UserLog object must be created when the next log message arrives.
		UserLogs::iterator oldIter = iter++;
		userLogs_.erase( oldIter );
	}

	return activeFiles_.update();
}


/**
 * Finds the component associated with the provided address and sets the
 * instance ID of that component.
 *
 * This is used to update a component with information from bwmachined so we
 * know that we know which process (eg: cellapp01 or cellapp02) we are
 * are referring to.
 *
 * @returns true on success, false on error.
 */
bool BWLogWriter::setAppInstanceID( const Mercury::Address &addr, int id )
{
	LoggingComponent *pComponent = this->findLoggingComponent( addr );

	if (pComponent == NULL)
	{
		ERROR_MSG( "BWLogWriter::setAppInstanceID: "
			"Can't set app ID for unknown address %s\n", addr.c_str() );
		return false;
	}

	return pComponent->setAppInstanceID( id );
}


/**
 * This method removes a process from the list of currently logging user
 * processes.
 *
 * @returns true on success, false on failure.
 */
bool BWLogWriter::stopLoggingFromComponent( const Mercury::Address &addr )
{
	// Search through all the UserLogs and remove that component
	UserLogs::iterator it = userLogs_.begin();
	while (it != userLogs_.end())
	{
		UserLogWriterPtr pUserLog = it->second;
		if (pUserLog->removeUserComponent( addr ))
		{
			return true;
		}

		++it;
	}

	return false;
}


/**
 * This method adds an incoming message to the logs.
 *
 * It is responsible for determining which user the log message belongs to,
 * and handing off the message to the appropriate UserLog to be written.
 *
 * @returns true on success, false on error.
 */
bool BWLogWriter::addLogMessage( const LoggerComponentMessage &msg,
		const Mercury::Address &addr, MemoryIStream &is )
{
	uint16 uid = msg.uid_;

	// Stream off the header and the format string
	LoggerMessageHeader header;
	std::string format;
	int len = is.remainingLength();
	is >> header >> format;

	if (is.error())
	{
		ERROR_MSG( "BWLogWriter::addLogMessage: Log message from %s was too "
			"short (only %d bytes)\n", addr.c_str(), len );
		return false;
	}

	// Get the format string handler
	LogStringInterpolator *pHandler = formatStrings_.resolve( format );
	if (pHandler == NULL)
	{
		ERROR_MSG( "BWLogWriter::addLogMessage: Couldn't add format string to "
			"mapping. '%s'\n", format.c_str() );
		return false;
	}

	// Cache the IP address -> Hostname mapping if neccessary
	if (hostnames_.getHostByAddr( addr.ip ) == NULL)
	{
		ERROR_MSG( "BWLogWriter::addLogMessage: Error resolving host '%s'\n",
			addr.c_str() );
		return false;
	}

	// Get the user log segment
	UserLogWriterPtr pUserLog = this->getUserLog( uid );
	if (pUserLog == NULL)
	{
		std::string username;
		Mercury::Reason reason = this->resolveUID( uid, addr.ip, username );

		if (reason == Mercury::REASON_SUCCESS)
		{
			pUserLog = this->createUserLog( uid, username );
			if (pUserLog == NULL)
			{
				ERROR_MSG( "BWLogWriter::addLogMessage: Failed to create a "
							"UserLog for UID %hu\n", uid );

				return false;
			}
		}
		else
		{
			ERROR_MSG( "BWLogWriter::addLogMessage: Couldn't resolve uid %d "
				"(%s). UserLog not started.\n",
				uid, Mercury::reasonToString( reason ) );
			return false;
		}
	}

	MF_ASSERT( pUserLog != NULL );

	LoggingComponent *component = pUserLog->findLoggingComponent( msg, addr,
												componentNames_ );

	// We must have a component now, if it didn't exist, it should have been
	// created
	MF_ASSERT( component != NULL);

	LogEntry entry;
	struct timeval tv;
	gettimeofday( &tv, NULL );
	entry.time_.secs_ = tv.tv_sec;
	entry.time_.msecs_ = tv.tv_usec/1000;
	entry.componentID_ = component->getAppTypeID();
	entry.messagePriority_ = header.messagePriority_;
	entry.stringOffset_ = pHandler->fileOffset();

	// TODO: the write to STDOUT functionality may be better placed inside
	//       UserLogWriter. All the same args are being passed in?
	// Dump output to stdout if required
	if (writeToStdout_)
	{
		std::cout << pUserLog->logEntryToString( entry,
										static_cast< BWLogCommon *>( this ),
										component, *pHandler, is, msg.version_ );
	}

	if (!pUserLog->addEntry( component, entry, *pHandler, is, this, msg.version_ ))
	{
		ERROR_MSG( "BWLogWriter::addLogMessage: Failed to add entry to user log\n" );
		return false;
	}

	return true;
}


/**
 * This method forces the active files to update in case any new files have
 * been created by a UserLog.
 * 
 * @returns true on success, false on error.
 */
bool BWLogWriter::updateActiveFiles()
{
	return activeFiles_.update();
}


/**
 * This method forces the deletion of the active segments file.
 *
 * It is used only to avoid a potential race condition when adding an entry to
 * the UserLogs.
 */
void BWLogWriter::deleteActiveFiles()
{
	activeFiles_.deleteFile();
}


/**
 * This method enables or disables writing to standard output for incoming logs.
 */
void BWLogWriter::writeToStdout( bool status )
{
	writeToStdout_ = status;
}


/**
 * This method returns the maximum allowable size that UserSegments should
 * consume before rolling into a new segment file.
 *
 * @returns Maximum segment size allowable (in bytes).
 */
int BWLogWriter::getMaxSegmentSize() const
{
	return maxSegmentSize_;
}


/**
 * Creates a new UserLogWriter and adds it to the list of UserLogs.
 *
 * @returns A SmartPointer to a UserLogWriter on success, NULL on error.
 */
UserLogWriterPtr BWLogWriter::createUserLog( uint16 uid,
	const std::string &username )
{
	UserLogWriterPtr pUserLog( new UserLogWriter( uid, username ),
							UserLogWriterPtr::NEW_REFERENCE );

	if (!pUserLog->init( rootLogPath_ ) || !pUserLog->isGood())
	{
		ERROR_MSG( "BWLogWriter::createUserLog: Failed to create a UserLog "
			"for %s.\n", username.c_str() );
		return NULL;
	}

	userLogs_[ uid ] = pUserLog;

	return pUserLog;
}


/**
 * Returns the UserLog object for the requested UID.
 *
 * UserLogs should exist for all users necessary by the time this method is
 * invoked.
 *
 * @returns A SmartPointer to a UserLogWriter on success, NULL on error.
 */
UserLogWriterPtr BWLogWriter::getUserLog( uint16 uid )
{
	UserLogs::iterator it = userLogs_.find( uid );
	if (it != userLogs_.end())
	{
		return it->second;
	}

	return NULL;
}


namespace
{

/**
 * Handles receiving responses from BWMachined when querying for usernames.
 */
class UsernameHandler : public MachineGuardMessage::ReplyHandler
{
public:
	virtual ~UsernameHandler() {}

	/**
	 * This method handles receipt of a UserMessage.
	 *
	 * @returns Always returns false to avoid further message handling.
	 */
	virtual bool onUserMessage( UserMessage &um, uint32 addr )
	{
		if (um.uid_ != UserMessage::UID_NOT_FOUND)
		{
			username_ = um.username_;
		}

		return false;
	}
	std::string username_;
};

} // anonymous namespace


/**
 * Queries BWMachined in an attempt to resolve the username associated to uid.
 *
 * @param uid     The UID to query BWmachined to resolve.
 * @param addr    The address of the machine to initially query (where the
 *                 log initially came from.
 * @param result  Username that has been resolved from uid.
 *
 * @returns Mercury::REASON_SUCCESS on success, other on failure.
 */
Mercury::Reason BWLogWriter::resolveUID( uint16 uid, uint32 addr,
	std::string &result )
{
	// Hard coded a case for UID 0 which will appear when windows
	// users haven't specified a UID. This should default to the root
	// user.
	if (uid == 0)
	{
		result = "root";
		return Mercury::REASON_SUCCESS;
	}

	int reason;
	uint32 queryaddr = addr;
	UsernameHandler handler;
	UserMessage um;
	um.uid_ = uid;
	um.param_ = um.PARAM_USE_UID;

	reason = um.sendAndRecv( 0, queryaddr, &handler );
	if (reason != Mercury::REASON_SUCCESS)
	{
		ERROR_MSG( "BWLogWriter::resolveUID: UserMessage query to %s for "
					"UID %hu failed: %s\n", inet_ntoa( (in_addr&)queryaddr ),
					uid, Mercury::reasonToString( (Mercury::Reason&)reason ) );

		INFO_MSG( "BWLogWriter::resolveUID: Retrying UID query for %hu as "
					"broadcast.\n",	uid );

		queryaddr = BROADCAST;
		reason = um.sendAndRecv( 0, queryaddr, &handler );
	}

	// If we still haven't been able to resolve after trying a broadcast, fail
	if (reason != Mercury::REASON_SUCCESS)
	{
		return (Mercury::Reason&)reason;
	}

	// If we couldn't resolve the username, just use his UID as his username
	if (handler.username_.empty())
	{
		WARNING_MSG( "BWLogWriter::resolveUID: "
			"Couldn't resolve UID %hu, using UID as username\n", uid );

		char buf[ 128 ];
		bw_snprintf( buf, sizeof( buf ), "%hu", uid );
		result = buf;
	}
	else
	{
		// Now we are sure the resolving was successful, update the result
		// string
		result = handler.username_;
	}

	return (Mercury::Reason&)reason;
}


/**
 * This method retrieves the current logging component associated with the
 * specified address.
 *
 * @returns A pointer to a LoggingComponent on success, NULL if no
 *          LoggingComponent matches the specified address.
 */
LoggingComponent * BWLogWriter::findLoggingComponent(
	const Mercury::Address &addr )
{
	LoggingComponent *pComponent = NULL;

	UserLogs::iterator it = userLogs_.begin();
	while (it != userLogs_.end())
	{
		UserLogWriterPtr pUserLog = it->second;
		pComponent = pUserLog->findLoggingComponent( addr );

		if (pComponent != NULL)
		{
			return pComponent;
		}

		++it;
	}

	return NULL;
}

// bwlog_writer.cpp
