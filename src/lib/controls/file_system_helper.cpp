/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "file_system_helper.hpp"
#include "cstdmf/guard.hpp"

/**
 *	Fixes up the passed path so that the appropriate common root is used.
 *
 *	@param rootPath	The path that needs to be fixed up.
 *	@return	The fixed up path.
 */
/*static*/ std::string FileSystemHelper::fixCommonRootPath( std::string rootPath )
{
	BW_GUARD;

	// There are three special scenarios that need to be handled.
	//
	// 1)	rootPath is a root disk drive, e.g. c:
	//		In this case we need to append a forward slash at the end.
	// 2)	rootPath may only have a partial folder name, which may match one of
	//		the res paths, e.g.
	//		If there are two res paths defined in paths.xml:
	//			c:/mf/fantasydemo/res
	//			c:/mf_1_8/fantasydemo/res
	//		The common root would be c:/mf.  This may mistakenly resolve to
	//		using c:/mf/ as the common root which would be incorrect.
	//		Special care must be taken to remove the partial folder name
	//		so that the true root path is used, i.e c:/ from the example above.
	// 3)	rootPath has many nested folders, e.g. c:/dev/mf
	//		In this case the standard path stripping command can be used
	// ---------------------------------------------------------------------
	//
	// First check for scenario 1)
	if (rootPath.size() > 0 &&
		rootPath.at( rootPath.size() - 1 ) == ':')
	{
		rootPath += "/";
	}
	else
	{
		// Check if the last forward slash is proceeded by a colon.  If it
		// is, truncate everything after the last forward slash.
		std::string::size_type iLastForwardSlash = rootPath.rfind( "/" );
		if (iLastForwardSlash != std::string::npos &&
			iLastForwardSlash > 0 &&
			rootPath.at( iLastForwardSlash - 1 ) == ':')
		{
			rootPath = rootPath.substr( 0, iLastForwardSlash + 1 );
		}
		else
		{
			//Remove anything after the last directory slash
			rootPath = rootPath.substr( 0, iLastForwardSlash );
		}
	}

	return rootPath;
}
