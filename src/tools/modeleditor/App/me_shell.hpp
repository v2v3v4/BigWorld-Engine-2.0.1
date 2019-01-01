/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef ME_SHELL_HPP
#define ME_SHELL_HPP

#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "romp_harness.hpp"

class XConsole;
class TimeOfDay;


/*
 *	Interface to the BirWorld message handler
 */
struct MeShellDebugMessageCallback : public DebugMessageCallback
{
	MeShellDebugMessageCallback()
	{
	}
	~MeShellDebugMessageCallback()
	{
	}
	virtual bool handleMessage( int componentPriority,
		int messagePriority, const char * format, va_list argPtr );
};



/**
 *	This class adds solution specific routines to what already exists in
 *	appmgr/app.cpp
 */
class MeShell
{
public:
						MeShell();
						~MeShell();

	static MeShell & instance() { ASSERT(s_instance_); return *s_instance_; }

	static bool			initApp( HINSTANCE hInstance, HWND hWndApp, HWND hWndGraphics );	// so easy to pass the fn pointer around
	
	bool				init( HINSTANCE hInstance, HWND hWndApp, HWND hWndGraphics );
	void				fini();

	HINSTANCE &			hInstance() { return hInstance_; }
	HWND &				hWndApp() { return hWndApp_; }
	HWND &				hWndGraphics() { return hWndGraphics_; }

	RompHarness &		romp() { return *romp_; }

	// get time of day
	TimeOfDay* timeOfDay() { return romp_->timeOfDay(); }

	POINT				currentCursorPosition() const;


private:
	friend std::ostream& operator<<( std::ostream&, const MeShell& );
	friend struct MeShellDebugMessageCallback;

						MeShell(const MeShell&);
						MeShell& operator=(const MeShell&);

	static bool			messageHandler( int componentPriority, int messagePriority,
										const char * format, va_list argPtr );

	bool				initGraphics();
	void				finiGraphics();

	bool				initScripts();
	void				finiScripts();

	bool				initConsoles();
	bool				initErrorHandling();
	bool				initRomp();
	bool				initCamera();

	bool				initSound();

	bool				initTiming();


	MeShellDebugMessageCallback debugMessageCallback_;
	
	static MeShell *		s_instance_;		// the instance of this class (there should be only one!)

	// windows variables
	HINSTANCE					hInstance_;			// the current instance of the application
	HWND						hWndApp_;			// application window
	HWND						hWndGraphics_;		// 3D window

	RompHarness *			romp_;
};

//#ifdef CODE_INLINE
//#include "initialisation.ipp"
//#endif


#endif // ME_SHELL_HPP
