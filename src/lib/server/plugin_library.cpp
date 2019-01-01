/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "plugin_library.hpp"

#include <string>
#include <set>


#ifdef unix

#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>

#endif

#include "cstdmf/debug.hpp"

DECLARE_DEBUG_COMPONENT( 0 )


// -----------------------------------------------------------------------------
// Section: PluginLibrary
// -----------------------------------------------------------------------------

typedef std::set<std::string> StringSet;

/**
 *	This function loads all plugins from the given directory name (optionally
 *	prefixed with the executing app's name) and runs their static initialisers.
 */
void PluginLibrary::loadAllFromDirRelativeToApp(
	bool prefixWithAppName, const char * partialDir )
{
#ifdef unix
	char rlbuf[512];
	int rllen = readlink( "/proc/self/exe", rlbuf, sizeof(rlbuf) );
	if (rllen <= 0 || rlbuf[0] != '/')
	{
		ERROR_MSG( "PluginLibrary: "
			"Could not read symbolic link /proc/self/exe\n" );
		return;
	}
	// Note: readlink does not append a NUL character.
	std::string dllDirName = std::string( rlbuf, rllen );
	if (!prefixWithAppName)
		dllDirName = dllDirName.substr( 0, dllDirName.find_last_of('/') );
	dllDirName = dllDirName + partialDir + "/";

	DIR * dllDir = opendir( dllDirName.c_str() );
	if (dllDir == NULL) return;

	StringSet	dllNames;

	struct dirent * entry;
	while ((entry = readdir( dllDir )) != NULL)
	{
		std::string dllName = entry->d_name;
		if (dllName.length() < 3 ||
			dllName.substr( dllName.length()-3 ) != ".so") continue;
		if (dllName[0] != '/') dllName = dllDirName + dllName;
		dllNames.insert( dllName );
	}

	for (StringSet::iterator it = dllNames.begin(); it != dllNames.end(); it++)
	{
		const std::string & dllName = *it;
		TRACE_MSG( "PluginLibrary: Loading extension %s\n", dllName.c_str() );
		void * handle = dlopen( dllName.c_str(), RTLD_LAZY | RTLD_GLOBAL );
		if (handle == NULL)
		{
			ERROR_MSG( "PluginLibrary: "
				"Could not open library: %s\n"
				"PluginLibrary: reason was: %s\n", dllName.c_str(), dlerror() );
		}
	}

	TRACE_MSG( "finish loading extensions\n" );

	closedir( dllDir );

#else // !unix

	char gmnbuf[512];
	GetModuleFileName( NULL, gmnbuf, sizeof(gmnbuf) );
	std::string dllDirName = std::string( gmnbuf, strlen(gmnbuf)-4 );
	if (!prefixWithAppName)
		dllDirName = dllDirName.substr( 0, dllDirName.find_last_of('\\') );
	dllDirName = dllDirName + partialDir + "\\*";

	WIN32_FIND_DATA findData;
	HANDLE fff = FindFirstFile( dllDirName.c_str(), &findData );
	if (fff == INVALID_HANDLE_VALUE) return;

	StringSet	dllNames;

	do
	{
		std::string dllName = findData.cFileName;
		if (dllName.length() < 4 ||
			dllName.substr( dllName.length()-4 ) != ".dle") continue;
		dllName = dllDirName.substr( 0, dllDirName.length()-1 ) + dllName;

		dllNames.insert( dllName );
	} while (FindNextFile( fff, &findData ));

	FindClose( fff );

	for (StringSet::iterator it = dllNames.begin(); it != dllNames.end(); it++)
	{
		const std::string & dllName = *it;
		TRACE_MSG( "PluginLibrary: Loading extension %s\n", dllName.c_str() );
		HMODULE handle = LoadLibraryEx( dllName.c_str(), NULL,
			LOAD_WITH_ALTERED_SEARCH_PATH );
		if (handle == NULL)
		{
			ERROR_MSG( "PluginLibrary: "
				"Could not open library: %s\n", dllName.c_str() );
		}
	}
#endif // unix
}

// plugin_library.cpp
