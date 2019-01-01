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
#ifndef _XBOX360
	#include <windows.h>
#endif
#include <vector>
#include <sstream>
#include "restart.hpp"
#include "cstdmf/string_utils.hpp"

#ifndef _XBOX360

static std::wstring s_commandLine = GetCommandLine();
static const std::wstring s_commandLineToken = L"/wait_for_process:";


/*********************************************************************
 * This function will return immediately if the application is not
 * started by calling restart. Otherwise it will wait until the previous
 * application is ended.
*********************************************************************/
void waitForRestarting()
{
	std::string::size_type off = s_commandLine.find( s_commandLineToken.c_str() );
	if (off != s_commandLine.npos)
	{
		DWORD pid = (DWORD)_wtoi64( s_commandLine.c_str() + off + s_commandLineToken.size() );
		HANDLE ph = OpenProcess( SYNCHRONIZE, FALSE, pid );
		if (ph)
		{
			WaitForSingleObject( ph, INFINITE );
			CloseHandle( ph );
		}
		s_commandLine.resize( off );
	}
}


/*********************************************************************
 * This function start a new instance of this application with the
 * exact paramter and environment. This function won't terminate the
 * current process and it is the caller's responsibility to do that.
*********************************************************************/
void startNewInstance()
{
	std::vector<wchar_t> exeName( 1024 );
	while (GetModuleFileName( NULL, &exeName[0], exeName.size() ) + 1 > exeName.size())
	{
		exeName.resize( exeName.size() * 2 );
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	GetStartupInfo( &si );

	std::wostringstream oss;
	oss << s_commandLine << ' ' << s_commandLineToken << GetCurrentProcessId();

	std::wstring str = oss.str();
	std::vector<wchar_t> commandLine( str.begin(), str.end() );
	commandLine.push_back( 0 );

	CreateProcess( NULL, &commandLine[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi );
}


#endif
