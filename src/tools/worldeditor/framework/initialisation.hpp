/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WE_INITIALISATION_HPP
#define WE_INITIALISATION_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include <iostream>


/**
 *	This class exists purely to move code out of app.cpp
 */
class Initialisation
{
public:
	static bool initApp( HINSTANCE hInstance, HWND hWndApp, HWND hWndGraphics );
	static void finiApp();

	static HINSTANCE s_hInstance;
	static HWND s_hWndApp;
	static HWND s_hWndGraphics;

private:
	static bool initGraphics( HINSTANCE hInstance, HWND hWnd );
	static void finaliseGraphics();

	static bool initScripts();
	static void finaliseScripts();

	static bool initTiming();
	static bool initConsoles();
	static void initSound();
	static bool initErrorHandling();

	Initialisation(const Initialisation&);
	Initialisation& operator=(const Initialisation&);

	friend std::ostream& operator<<( std::ostream&, const Initialisation& );
};


#ifdef CODE_INLINE
#include "initialisation.ipp"
#endif


#endif // WE_INITIALISATION_HPP
