/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifdef _WIN32
#pragma once

#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#else
#include <stdlib.h>
#endif // _WIN32


// TODO: reference additional headers your program requires here
#include "third_party/CppUnitLite2/src/Test.h"

// TODO: Move this to cstdmf
inline void mySleep( int milliseconds )
{
#ifdef _WIN32
	Sleep( milliseconds );
#else
	usleep( milliseconds * 1000 );
#endif
}

// stdafx.h
