/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BW_UTIL_HPP
#define BW_UTIL_HPP

#include <string>
#include <algorithm>

namespace BWUtil
{

/**
 *	This method converts a path into a path that is useable by BigWorld.
 *	
 *	Primarily this is used to convert windows paths:
 *	Example
 *		c:\bigworld\fantasydemo -> c:/bigworld/fantasydemo
 *	
 *	@param path The path to sanatise.
 */
void sanitisePath( std::string & path );


/**
 *	This method makes certain that the path contains a trailing folder separator.
 *
 *	@param path 	The path string to be formatted.
 *
 *	@return The path with a trailing folder separator.
 */
std::string formatPath( const std::string & pathToFormat );


/**
 *	Retrieves the path from a file.
 *
 *	@param file file to get path of.
 *
 *	@return path string
 *
 */
std::string getFilePath( const std::string & pathToFile );


/**
 *	This method returns the directory component of the current
 *	process' executable file location.
 */
std::string executableDirectory();


/**
 *	This method returns the full path (including executable filename) of the
 *	current process' executable file.
 */
std::string executablePath();


/**
 *	This method returns if the item is inside the container
 */
template<typename Container, typename Item>
bool contains( const Container& c, const Item& i )
{
	return std::find( c.begin(), c.end(), i ) != c.end();
}

} // namespace BWUtil


#ifdef _WIN32
FILE* bw_fopen( const char* filename, const char* mode );
#define fopen bw_fopen

long bw_fileSize( FILE* file );
std::wstring getTempFilePathName();

#else
#define bw_fopen fopen //bw_fopen might be used directly
#endif


#endif // BW_UTIL_HPP
