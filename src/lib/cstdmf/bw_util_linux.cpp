/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "bw_util.hpp"

#include "debug.hpp"

#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>

#include <string>


namespace BWUtil
{

void sanitisePath( std::string & path )
{
	// This method is a NOP for linux as BigWorld uses the same format.
}


std::string getFilePath( const std::string & pathToFile )
{
	// dirname() can modify the input string so we need to make a copy
	char pathCpy[PATH_MAX + 1];
	memset( pathCpy, 0, sizeof( pathCpy ) );

	strcpy( pathCpy, pathToFile.c_str() );

	return dirname( pathCpy );
}


std::string executablePath()
{
	char path[PATH_MAX + 1];
	memset( path, 0, sizeof( path ) );

	ssize_t retval = readlink( "/proc/self/exe", path, sizeof( path ) - 1);
	if ((retval <= 0) || path[0] != '/')
	{
		ERROR_MSG( "BWUtil::executablePath: Unable to read "
				"/proc/self/exe: %s\n", strerror( errno ) );
		return "";
	}

	return path;
}

} // namespace BWUtil

// bw_linux_util.cpp
