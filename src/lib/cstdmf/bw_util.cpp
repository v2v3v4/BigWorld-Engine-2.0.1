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
#include "bw_util.hpp"

namespace BWUtil
{


std::string formatPath( const std::string & pathToFormat )
{
	if (pathToFormat.empty())
	{
		return pathToFormat;
	}

	std::string tmpPath( pathToFormat.c_str() );
	BWUtil::sanitisePath( tmpPath );

	if (tmpPath[tmpPath.size() - 1] != '/')
	{
		tmpPath += '/';
	}

	return tmpPath;
}


std::string executableDirectory()
{
	return BWUtil::getFilePath( BWUtil::executablePath() );
}


} // namespace BWUtil

// bw_util.cpp
