/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "bwlog_reader.hpp"

#include "mlutil.hpp"

#include <sys/types.h>



// ----------------------------------------------------------------------------
// Section: BWLogReader
// ----------------------------------------------------------------------------

/**
 * Destructor.
 */
BWLogReader::~BWLogReader()
{
	userLogs_.clear();
}


/**
 * Callback method invoked from BWLogCommon during initUserLogs().
 *
 * This method updates the map of UID -> username with the current entry.
 */
bool BWLogReader::onUserLogInit( uint16 uid, const std::string &username )
{
	// In read mode we just make a record of the uid -> username mapping
	usernames_[ uid ] = username;

	return true;
}


/**
 * Initialises the BWLogReader instance for use.
 *
 * @returns true on successful initialisation, false on error.
 */
bool BWLogReader::init( const char *root )
{
	static const char *mode = "r";

	if (root == NULL)
	{
		ERROR_MSG( "BWLogReader::init: No root log path specified.\n" );
		return false;
	}
	this->initRootLogPath( root );

	// Make sure the root directory has the access we want
	if (!MLUtil::isPathAccessible( rootLogPath_.c_str() ))
	{
		ERROR_MSG( "BWLogReader::init: Root logdir (%s) not accessible in "
			"read mode.\n", rootLogPath_.c_str() );
		return false;
	}

	// Call the parent class common initialisation
	if (!this->initCommonFiles( mode ))
	{
		return false;
	}

	if (!this->initUserLogs( mode ))
	{
		return false;
	}

	return true;
}


/**
 * Returns the full path to the logs being reference by this BWLogReader.
 *
 * @returns Absolute path of the log directory being used.
 */
const char *BWLogReader::getLogDirectory() const
{
	return rootLogPath_.c_str();
}


/**
 * Retrieves the component names of all logging components calling visitor
 * for each component.
 *
 * @returns true on success, false on error.
 */
bool BWLogReader::getComponentNames( LogComponentsVisitor &visitor ) const
{
	return componentNames_.visitAllWith( visitor );
}


/**
 * Retrieves the network address and hostname of each known machine in the
 * logs, calling visitor for each known host.
 *
 * @returns true on success, false on error.
 */
bool BWLogReader::getHostnames( HostnameVisitor &visitor ) const
{
	return hostnames_.visitAllWith( visitor );
}


/**
 * Returns a list of all known format strings in the log directory.
 *
 * @returns An std::vector of std::strings.
 */
FormatStringList BWLogReader::getFormatStrings() const
{
	return formatStrings_.getFormatStrings();
}


/**
 * Obtains a dictionary of usernames and UIDs of all users seen in the logs.
 *
 * @returns std::map of unix UID as uint16 and unix username as an std::string.
 */
const UsernamesMap & BWLogReader::getUsernames() const
{
	return usernames_;
}


/**
 * Obtains a UserLog for the specified unix UID.
 *
 * If a UserLog already exists for this user it is returned, otherwise it will
 * be created and loaded.
 *
 * @returns A SmartPointer to a UserLogReader on success, NULL on error.
 */
UserLogReaderPtr BWLogReader::getUserLog( uint16 uid )
{
	UserLogs::iterator it = userLogs_.find( uid );
	if (it != userLogs_.end())
	{
		return it->second;
	}

	UsernamesMap::iterator usernameIter = usernames_.find( uid );
	if (usernameIter == usernames_.end())
	{
		// NB: This has been made an error message to assist in track down
		//     potential issues. Can be disabled if it is seen in syslog a lot.
		ERROR_MSG( "BWLogReader::getUserLog: Unable to find a UserLog for "
			"UID %hu.\n", uid );
		return NULL;
	}

	// If we've found the UID in our list of known user logs, load the
	// UserLog for use.
	UserLogReaderPtr pUserLog( new UserLogReader( uid, usernameIter->second ),
						UserLogReaderPtr::NEW_REFERENCE );

	if (!pUserLog->init( rootLogPath_ ) || !pUserLog->isGood())
	{
		ERROR_MSG( "BWLogReader::getUserLog: Failed to create a UserLog "
			"for %s.\n", usernameIter->second.c_str() );
		return NULL;
	}

	userLogs_[ uid ] = pUserLog;

	return pUserLog;
}


/**
 * Obtains the IP address associated with the provided hostname.
 *
 * @returns The IP address of the requested host on success, 0 on error.
 */
uint32 BWLogReader::getAddressFromHost( const char *hostname ) const
{
	return hostnames_.getHostByName( hostname );
}


/**
 * Obtains a handler for the requested log entry, to allow presentation.
 *
 * @returns A pointer to a log handler on success, NULL on error.
 */
const LogStringInterpolator *BWLogReader::getHandlerForLogEntry(
	const LogEntry &entry )
{
	return formatStrings_.getHandlerForLogEntry( entry );
}


/**
 * Refreshes the main log files if they are being written to and have been
 * modified since we last looked at them.
 *
 * @returns true on success, false on failure.
 */
bool BWLogReader::refreshFileMaps()
{
	if (formatStrings_.isDirty() && !formatStrings_.refresh())
	{
		return false;
	}

	if (hostnames_.isDirty() && !hostnames_.refresh())
	{
		return false;
	}

	if (componentNames_.isDirty() && !componentNames_.refresh())
	{
		return false;
	}

	return true;
}

// bwlog_reader.cpp
