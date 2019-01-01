/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "watcher_forwarding.hpp"

#include "cstdmf/watcher_path_request.hpp"
#include "cstdmf/watcher.hpp"
#include <limits.h>

// ForwardingWatcher static member initialisation
const std::string ForwardingWatcher::TARGET_LEAST_LOADED = "leastLoaded";
const std::string ForwardingWatcher::TARGET_ALL = "all";


/**
 * Extracts a list of component ID numbers from a provided string of comma
 * seperated IDs.
 *
 * @param targetInfo The string of comma sperated component ID's to extract.
 *
 * @returns A list of component ID's that have been extracted.
 */
ComponentIDList ForwardingWatcher::getComponentIDList(
	const std::string & targetInfo )
{
	char *targetDup = strdup( targetInfo.c_str() );
	ComponentIDList components;
	const char *TOKEN_SEP = ",";

	if (targetDup != NULL)
	{

		char *endPtr;
		long int result;
		char *curr = strtok(targetDup, TOKEN_SEP);
		while (curr != NULL)
		{
			result = strtol(curr, &endPtr, 10);
			if ((result != LONG_MIN) && (result != LONG_MAX) &&
				(*endPtr == '\0'))
			{
				// Conversion worked so add the component ID to the list.
				components.push_back( (int32)result );
			}
			else
			{
				ERROR_MSG( "ForwardingWatcher::getComponentList: strtol "
							"failed to convert component ID '%s'.\n", curr );
			}

			curr = strtok(NULL, TOKEN_SEP);
		}

		delete [] targetDup;
	}

	return components;
}


// Overridden from Watcher
bool ForwardingWatcher::setFromStream( void * base, const char * path,
		WatcherPathRequestV2 & pathRequest )
{
	// Our watcher path should now look like;
	// 'all/command/addBot'
	// 'leastLoaded/command/addBot'
	// '1,2,45,87/command/addBot'
	// 
	// Seperate on the first '/'
	std::string pathStr = path;
	size_t pos = pathStr.find( '/' );
	if (pos == (size_t)-1)
	{
		ERROR_MSG( "ForwardingWatcher::setFromStream: Appear to have received "
					"a bad destination path '%s'.\n", path );
		return false;
	}

	// If the seperator was the final thing in the string then there is no
	// remaining watcher path to use on the receiving component side.
	if ((pos+1) >= pathStr.size())
	{
		ERROR_MSG( "ForwardingWatcher::setFromStream: No destination watcher "
					"path provided '%s'.\n", path );
		return false;
	}

	// Now generate a new string containing the destination target(s)
	std::string targetInfo = pathStr.substr( 0, pos );
	std::string destWatcher = pathStr.substr( pos+1, pathStr.size() );

	// Specialised collector initialisation
	ForwardingCollector *collector = this->newCollector(
		pathRequest, destWatcher, targetInfo );

	bool status = false;
	if (collector)
	{
		status = collector->start();

		// If the collector didn't start correctly, it won't be receiving
		// any responses which will enable it to clean itself up, so do it now
		if (!status)
			delete collector;
	}

	return status;
}


// Overridden from Watcher
bool ForwardingWatcher::getAsStream( const void * base, const char * path,
		WatcherPathRequestV2 & pathRequest ) const
{
	if (isEmptyPath(path))
	{
		Watcher::Mode mode = Watcher::WT_READ_ONLY;
		watcherValueToStream( pathRequest.getResultStream(),
							  "Forwarding Watcher", mode );
		pathRequest.setResult( comment_, mode, this, base );
		return true;
	}
	else if (isDocPath( path ))
	{
		Watcher::Mode mode = Watcher::WT_READ_ONLY;
		watcherValueToStream( pathRequest.getResultStream(), comment_, mode );
		pathRequest.setResult( comment_, mode, this, base );
		return true;
	}

	return false;
}
