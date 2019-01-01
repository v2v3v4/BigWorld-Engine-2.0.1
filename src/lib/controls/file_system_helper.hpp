/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _FILE_SYSTEM_HELPER_HPP_
#define _FILE_SYSTEM_HELPER_HPP_

#include <string>

/**
 *	Utility class of file system helper functions.
 */
class FileSystemHelper
{
public:
	static std::string fixCommonRootPath( std::string rootPath );
};


#endif // !define ( file_system_helper.hpp )