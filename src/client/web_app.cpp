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

#include "web_app.hpp"

#include "app.hpp"
#include "app_config.hpp"

#include "web_render/mozilla_web_page.hpp"

// Static vars
WebApp WebApp::instance;

int WebApp_token = 1;


/**
 *	Constructor.
 */
WebApp::WebApp()
{
	BW_GUARD;
	MainLoopTasks::root().add( this, "Web/App", NULL );
}


/**
 *	Destructor.
 */
WebApp::~WebApp()
{

}


/**
 * Init the web stuff
 */
bool WebApp::init()
{
	// The mozilla interface needs the application window
	WebPage::init();
	MozillaWebPageManager::setApplicationWindow(App::instance().hWnd());
	MozillaWebPageManager::s_run(AppConfig::instance().pRoot());
	return true;
}


/**
 * Fini the web stuff
 */
void WebApp::fini()
{
	MozillaWebPageManager::s_fini(false);
	WebPage::fini();
}


/**
 *	Override from MainLoopTasks::tick().
 */
void WebApp::tick( float dTime )
{
	MozillaWebPageManager::s_tick(dTime);
}


// web_app.cpp
