/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifdef _MSC_VER 
#pragma once
#endif

#ifndef BW_WINMAIN_HPP
#define BW_WINMAIN_HPP

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


extern const char * compileTimeString;

extern bool g_bAppStarted;
extern bool g_bAppQuit;


int BWWinMain(	HINSTANCE hInstance,
				LPCTSTR lpCmdLine,
				int nCmdShow,
				LPCTSTR lpClassName,
				LPCTSTR lpWindowName );

LRESULT BWWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

bool BWProcessOutstandingMessages();


#endif // BW_WINMAIN_HPP


// bw_winmain.hpp
