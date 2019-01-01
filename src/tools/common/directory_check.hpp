/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#pragma once

class DirectoryCheck
{
public:
	DirectoryCheck( const std::wstring& appName )
	{
		BW_GUARD;

		static const int MAX_PATH_SIZE = 8192;
		wchar_t curDir[ MAX_PATH_SIZE ];
		GetCurrentDirectory( MAX_PATH_SIZE, curDir );

		std::wstring directorPath = curDir;

		if (!directorPath.empty() && *directorPath.rbegin() != '\\')
		{
			directorPath += '\\';
		}

		directorPath = directorPath + L"resources\\scripts\\" + appName + L"Director.py";

		if( GetFileAttributes( directorPath.c_str() ) == INVALID_FILE_ATTRIBUTES )
		{// we are not running in the proper directory
			GetModuleFileName( NULL, curDir, MAX_PATH_SIZE );
			if( _tcsrchr( curDir, '\\' ) )
				*_tcsrchr( curDir, '\\' ) = 0;

			directorPath = curDir;
			directorPath = directorPath + L"\\resources\\scripts\\" + appName + L"Director.py";

			if( GetFileAttributes( directorPath.c_str() ) != INVALID_FILE_ATTRIBUTES )
			{
				SetCurrentDirectory( curDir );
			}
			else
			{
				GetCurrentDirectory( MAX_PATH_SIZE, curDir );
				MessageBox( NULL,
					Localise(L"COMMON/DIRECTORY_CHECK/DIR_CHECK" , appName, curDir, appName ),
					appName.c_str(), MB_OK | MB_ICONASTERISK );
			}
		}
	}
};