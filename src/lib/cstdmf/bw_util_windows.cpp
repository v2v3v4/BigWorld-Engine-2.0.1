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

#include "stdmf.hpp"
#include "debug.hpp"
#include "string_utils.hpp"

#include <string>


namespace BWUtil
{

void sanitisePath( std::string & path )
{
	std::replace( path.begin(), path.end(), '\\', '/' );
}


std::string getFilePath( const std::string & pathToFile )
{
	// NB: this does not behave the same way as the linux implementation
	std::string directory;
	std::string nFile = pathToFile;

    if ( pathToFile.length() >= 1 && pathToFile[pathToFile.length()-1] != '/' )
    {
    	int pos = pathToFile.find_last_not_of( " " );

        if ((0 <= pos) && (pos < (int)pathToFile.length()))
		{
            nFile = pathToFile.substr( 0, pos );
		}
    }

    int pos = nFile.find_last_of( "\\/" );

	if ((0 <= pos) && (pos < (int)nFile.length()))
	{
		directory = nFile.substr( 0, pos );
	}

	return BWUtil::formatPath( directory );
}


std::string executablePath()
{
	// get application directory
	wchar_t wbuffer[MAX_PATH];

	// TODO: not checking return value
	GetModuleFileName( NULL, wbuffer, ARRAY_SIZE( wbuffer ) );

	std::string appPath;
	bw_wtoutf8( wbuffer, appPath );

	BWUtil::sanitisePath( appPath );

	return appPath;
}

} // namespace BWUtil


FILE* bw_fopen( const char* filename, const char* mode )
{
	std::wstring wFilename;
	std::wstring wMode;

	bw_utf8tow( filename, wFilename );
	bw_utf8tow( mode, wMode );

	return _wfopen( wFilename.c_str(), wMode.c_str() );
}


long bw_fileSize( FILE* file )
{
	long currentLocation = ftell(file);
	if (currentLocation < 0) 
	{
		currentLocation = 0;
	}

	int res = fseek(file, 0, SEEK_END);
	if (res)
	{
		ERROR_MSG("bw_fileSize: fseek failed\n");
		return -1;
	}
	long length = ftell(file);
	res = fseek(file, currentLocation, SEEK_SET);
	if (res)
	{
		ERROR_MSG("bw_fileSize: fseek failed\n");
		return -1;
	}
	return length;
}


/**
 *	return a temp file name
 */
std::wstring getTempFilePathName()
{
	wchar_t tempDir[ MAX_PATH + 1 ];
	wchar_t tempFile[ MAX_PATH + 1 ];

	if (GetTempPath( MAX_PATH + 1, tempDir ) < MAX_PATH)
	{
		if (GetTempFileName( tempDir, L"BWT", 0, tempFile ))
		{
			return tempFile;
		}
	}

	return L"";
}

// bw_util_windows.cpp
