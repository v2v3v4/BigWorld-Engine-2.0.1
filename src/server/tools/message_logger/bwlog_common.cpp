/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "bwlog_common.hpp"

#include "constants.hpp"

#include <dirent.h>

/**
 * Converts a root path to from a relative path to an absolute path if
 * necessary.
 *
 * @returns true on success, false on error.
 */
bool BWLogCommon::initRootLogPath( const char *rootPath )
{
	if (rootPath[0] != '/')
	{
		char buf[ 512 ];
		if (getcwd( buf, sizeof( buf )) == NULL)
		{
			ERROR_MSG( "BWLogCommon::init: Failed to getcwd(). %s\n",
				strerror( errno ) );
			return false;
		}
		rootLogPath_ = buf;
		rootLogPath_ += '/';
		rootLogPath_ += rootPath;
	}
	else
	{
		rootLogPath_ = rootPath;
	}

	return true;
}


/**
 * Initialises the top level log directory files such as format strings,
 * hostname map and process component types.
 *
 * @returns true on success, false on error.
 */
bool BWLogCommon::initCommonFiles( const char *mode )
{
	const char *versionFile = version_.join( rootLogPath_.c_str(), "version" );
	if (!version_.init( versionFile, mode, LOG_FORMAT_VERSION ))
	{
		ERROR_MSG( "BWLogCommon::init: Couldn't init version file\n" );
		return false;
	}

	if (!componentNames_.init( rootLogPath_.c_str(), mode ))
	{
		ERROR_MSG( "BWLogCommon::init: Couldn't init component names "
			"mapping\n" );
		return false;
	}

	if (!hostnames_.init( rootLogPath_.c_str(), mode ))
	{
		ERROR_MSG( "BWLogCommon::init: Couldn't init hostnames mapping\n" );
		return false;
	}

	if (!formatStrings_.init( rootLogPath_.c_str(), mode ))
	{
		ERROR_MSG( "BWLogCommon::init: Couldn't init strings mapping\n" );
		return false;
	}

	return true;
}


/**
 * This method scans the log directory attempting to find all subdirectories
 * containing logs for each unix user.
 *
 * This method invokes onUserLogInit() when a valid user's log directory is
 * found.
 *
 * @returns true on success, false on error.
 */
bool BWLogCommon::initUserLogs( const char *mode )
{
	DIR *rootdir = opendir( rootLogPath_.c_str() );
	struct dirent *dirent;
	while ((dirent = readdir( rootdir )) != NULL)
	{
		char fname[ 1024 ];
		struct stat statinfo;

		bw_snprintf( fname, sizeof( fname ), "%s/%s/uid",
			rootLogPath_.c_str() , dirent->d_name );
		if (stat( fname, &statinfo ))
			continue;

		UnaryIntegerFile uidfile;
		if (!uidfile.init( fname, mode ) || uidfile.getValue() == -1)
		{
			ERROR_MSG( "BWLogCommon::initUserLogs: Skipping user directory "
						"with invalid uid file %s\n", fname );
			continue;
		}

		uint16 uid = uidfile.getValue();
		std::string username = dirent->d_name;

		// Call into the implementing sub-class
		if (!this->onUserLogInit( uid, username ))
		{
			ERROR_MSG( "BWLogCommon::initUserLogs: "
						"Failed to initialise log for user '%s'.\n",
					username.c_str() );
			return false;
		}
	}
	closedir( rootdir );

	return true;
}


/**
 * This method returns the hostname associated with the specified IP.
 *
 * It is possible that if the hostname can't be resolved and the IP address
 * may be returned as a string.
 *
 * @returns Hostname associated with IP on success, NULL on error.
 */
const char * BWLogCommon::getHostByAddr( uint32 addr )
{
	return hostnames_.getHostByAddr( addr );
}


/**
 * This method returns the name of the component type with the specified ID.
 *
 * @returns Component name on success, NULL on error.
 */
const char * BWLogCommon::getComponentByID( int typeID ) const
{
	return componentNames_.getNameFromID( typeID );
}
