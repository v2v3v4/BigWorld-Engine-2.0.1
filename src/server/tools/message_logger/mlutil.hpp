/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ML_UTIL_HPP
#define ML_UTIL_HPP

#include <string>

namespace MLUtil
{

bool determinePathToConfig( std::string & configFile );

bool isPathAccessible( const char *path );

bool softMkDir( const char *path );
};

#endif // ML_UTIL_HPP
