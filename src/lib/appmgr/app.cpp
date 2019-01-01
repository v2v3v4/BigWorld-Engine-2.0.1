/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *
 * The main application class
 */

#include "pch.hpp"
#include "app.hpp"

#include "ashes/simple_gui.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/timestamp.hpp"
#include "cstdmf/profiler.hpp"
#include "cstdmf/diary.hpp"
#include "cstdmf/watcher.hpp"
#include "cstdmf/string_utils.hpp"
#include "moo/render_context.hpp"
#include "moo/animating_texture.hpp"
#include "moo/effect_visual_context.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/auto_config.hpp"
#include "romp/console_manager.hpp"
#include "romp/engine_statistics.hpp"
#include "romp/geometrics.hpp"
#include "moo/effect_visual_context.hpp"

#include "romp/flora.hpp"
#include "romp/enviro_minder.hpp"

#include "dev_menu.hpp"
#include "module_manager.hpp"
#include "options.hpp"

extern void setupTextureFeedPropertyProcessors();

static AutoConfigString s_blackTexture( "system/blackBmp" );
static AutoConfigString s_notFoundTexture( "system/notFoundBmp" );
static AutoConfigString s_notFoundModel( "system/notFoundModel" );	
static AutoConfigString s_graphicsSettingsXML("system/graphicsSettingsXML");

DECLARE_DEBUG_COMPONENT2( "App", 0 );

static DogWatch g_watchDrawConsole( "Draw Console" );
static DogWatch g_watchPreDraw( "Predraw" );

static DogWatch g_watchEndScene( "EndScene" );
static DogWatch g_watchPresent( "Present" );

// -----------------------------------------------------------------------------
// Section: Construction/Destruction
// -----------------------------------------------------------------------------

/**
 *	The constructor for the App class. App::init needs to be called to fully
 *	initialise the application.
 *
 *	@see App::init
 */
App::App() :
	hWndApp_( NULL ),
	lastTime_(0),
    paused_(false),
	maxTimeDelta_( 0.5f ),
	timeScale_( 1.f ),
	bInputFocusLastFrame_( false )
{
	// Do not use "this" in constructor list, because "this" is not a constructed
	// object yet.
	// http://msdn2.microsoft.com/en-us/library/3c594ae3.aspx
	presentThread_.init( this );
	
}


/**
 *	Destructor.
 */
App::~App()
{
}

/**
 *	This method initialises the application.
 *
 *	@param hInstance	The HINSTANCE associated with this application.
 *	@param hWndApp		The main window associated with this application.
 *	@param hWndGraphics	The window associated with the 3D device
 *
 *	@return		True if the initialisation succeeded, false otherwise.
 */
bool App::init( HINSTANCE hInstance, HWND hWndApp, HWND hWndGraphics,
			   bool ( *userInit )( HINSTANCE, HWND, HWND ) )
{
	wchar_t path[1024];

	if ( ::GetCurrentDirectory( ARRAY_SIZE( path ), path ) )
	{
		std::string npath;
		bw_wtoutf8( path, npath );
		BWResource::instance().addPath( npath );
		// Optionally enable filename case checking:
		bool checkFileCase = Options::getOptionBool("checkFileCase", false);
#if ENABLE_FILE_CASE_CHECKING
		BWResource::checkCaseOfPaths(checkFileCase);
#endif//ENABLE_FILE_CASE_CHECKING
	}
	else
	{
		CRITICAL_MSG( "Could not find working directory.  terminating\n" );
		return false;
	}

	ConsoleManager::createInstance();

	hWndApp_ = hWndApp;
	maxTimeDelta_ = Options::getOptionFloat( "app/maxTimeDelta", 
		maxTimeDelta_ );
	timeScale_ = Options::getOptionFloat( "app/timeScale", timeScale_ );
	
	bool darkBackground = Options::getOptionBool( "consoles/darkenBackground", false );
	ConsoleManager::instance().setDarkenBackground(darkBackground);

	MF_WATCH( "App/maxTimeDelta", maxTimeDelta_, Watcher::WT_READ_WRITE, "Limits the delta time passed to application tick methods." );
	MF_WATCH( "App/timeScale", timeScale_, Watcher::WT_READ_WRITE, "Multiplies the delta time passed to application tick methods." );


	// Resource glue initialisation
	if (!AutoConfig::configureAllFrom("resources.xml"))
	{
		CRITICAL_MSG( "Unable to find the required file resources.xml at the root of your resource tree.\n"
			"Check whether either the \"BW_RES_PATH\" environmental variable or the \"paths.xml\" files are incorrect." );
		return false;
	}	

	//init the texture feed instance, this registers material section
	//processors.
	setupTextureFeedPropertyProcessors();

	//Initialise the GUI
	SimpleGUI::init( NULL );

	if (userInit)
	{
		if ( !userInit( hInstance, hWndApp, hWndGraphics ) )
			return false;
	}

	if (!ModuleManager::instance().init(
		BWResource::openSection( "resources/data/modules.xml" ) ))
	{
		return false;
	}

	//Check for startup modules
	DataSectionPtr spSection =
		BWResource::instance().openSection( "resources/data/modules.xml/startup");

	std::vector< std::string > startupModules;

	if ( spSection )
	{
		spSection->readStrings( "module", startupModules );
	}

	//Now, create all startup modules.
	std::vector<std::string>::iterator it = startupModules.begin();
	std::vector<std::string>::iterator end = startupModules.end();

	while (it != end)
	{
		if (!ModuleManager::instance().push( (*it) ))
		{
			ERROR_MSG( "Unable to create module: %s",(*it).c_str() );
		}
		++it;
	}

	DataSectionPtr pSection = BWResource::instance().openSection( s_graphicsSettingsXML );

	// setup far plane options
	DataSectionPtr farPlaneOptionsSec = pSection->openSection("farPlane");
	EnviroMinderSettings::instance().init( farPlaneOptionsSec );

	// setup far flora options
	DataSectionPtr floraOptionsSec = pSection->openSection("flora");
	FloraSettings::instance().init( floraOptionsSec );

	//BUG 4252 FIX: Make sure we start up with focus to allow navigation of view.
	handleSetFocus( true );

	return true;
}


/**
 *	This method finalises the application.
 */
void App::fini()
{
	ModuleManager::instance().popAll();
	SimpleGUI::fini();	
	DogWatchManager::fini();
	presentThread_.stop();

	ConsoleManager::deleteInstance();

	Watcher::fini();
}

// -----------------------------------------------------------------------------
// Section: Framework
// -----------------------------------------------------------------------------

/**
 *	This method calculates the time between this frame and last frame.
 */
float App::calculateFrameTime()
{
	uint64 thisTime = timestamp();

	float dTime = (float)(((int64)(thisTime - lastTime_)) / stampsPerSecondD());

	if (dTime > maxTimeDelta_)
	{
		dTime = maxTimeDelta_;
	}

	dTime *= timeScale_;
	lastTime_ = thisTime;

    if (paused_)
        dTime = 0.0f;

	return dTime;
}

/**
 *  This method allows pausing.  The time between pausing and unpausing is,
 *  as far as rendering is concerned, zero.
 */
void App::pause(bool paused)
{
	static uint64 s_myDtime = 0;
    // If we are unpausing, update the current time so that there isn't a 
    // 'jump'.
    if ((paused != paused_) && !paused)
    {
        lastTime_ = timestamp() - s_myDtime;
    }
	else if ((paused != paused_) && paused)
	{
		uint64 thisTime = timestamp();
		s_myDtime = thisTime - lastTime_;
	}
    paused_ = paused;
}

/**
 *  Are we paused?
 */
bool App::isPaused() const
{
    return paused_;
}

/**
 *	This method is called once per frame to do all the application processing
 *	for that frame.
 */
void App::updateFrame( bool tick )
{
	Profiler::instance().tick();
	g_watchPreDraw.start();
	
	float dTime = this->calculateFrameTime();
	if (!tick) dTime = 0.f;

	if ( InputDevices::hasFocus() )
	{
		if ( bInputFocusLastFrame_ )
		{
			InputDevices::processEvents( this->inputHandler_ );
		}
		else
		{
			InputDevices::consumeInput();
			bInputFocusLastFrame_ = true;
		}
	}
	else bInputFocusLastFrame_ = false;

	EngineStatistics::instance().tick( dTime );
	Moo::AnimatingTexture::tick( dTime );
	Moo::Material::tick( dTime );
	Moo::EffectVisualContext::instance().tick( dTime );
	Moo::EffectManager::instance().finishEffectInits();
	
	ModulePtr pModule = ModuleManager::instance().currentModule();

	g_watchPreDraw.stop();
	
	if (pModule)
	{
        // If the device cannot be used don't bother rendering
	    if (!Moo::rc().checkDevice())
	    {
		    Sleep(100);
		    return;
	    }

		pModule->updateFrame( dTime );

		HRESULT hr = Moo::rc().beginScene();
        if (SUCCEEDED(hr))
        {
		    if (Moo::rc().mixedVertexProcessing())
			    Moo::rc().device()->SetSoftwareVertexProcessing( TRUE );
		    Moo::rc().nextFrame();

		    pModule->render( dTime );

		    g_watchDrawConsole.start();
		    if (ConsoleManager::instance().pActiveConsole() != NULL)
		    {
			    ConsoleManager::instance().draw( dTime );
		    }
		    g_watchDrawConsole.stop();
    		
		    // End rendering
		    g_watchEndScene.start();
		    Moo::rc().endScene();
		    g_watchEndScene.stop();
        }

		g_watchPresent.start();

		presentThread_.present();

		g_watchPresent.stop();
	}
	else
	{
		//TEMP! WindowsMain::shutDown();
	}
}

/**
 *	flush input queue
 */

void App::consumeInput()
{
	InputDevices::consumeInput();
}

/**
 *	This method is called when the window has resized. It is called in response
 *	to the Windows event.
 */
void App::resizeWindow()
{
	if (Moo::rc().windowed())
	{
		Moo::rc().changeMode( Moo::rc().modeIndex(), Moo::rc().windowed() );
	}
}


/**
 *	This method is called when the application gets the focus.
 */
void App::handleSetFocus( bool state )
{
	//DEBUG_MSG( "App::handleSetFocus: %d\n", state );

	bool isMouseDown = (GetAsyncKeyState( VK_LBUTTON ) < 0 ||
						GetAsyncKeyState( VK_MBUTTON ) < 0 ||
						GetAsyncKeyState( VK_RBUTTON ) < 0);
	if (state || !isMouseDown)
	{
		// Set the input focus IF we are getting the focus, OR if we are losing
		// the focus but all mouse buttons are up (not dragging anything).
		InputDevices::setFocus( state, &this->inputHandler_ );
	}
}

void App::PresentThread::ThreadFunc( LPVOID param )
{
	DX::addFakeMainThread();
	PresentThread* This = (PresentThread*)param;
	This->presentLoop();
}

void App::PresentThread::presentLoop()
{
	uint64 startTime;
	uint64 endTime;
	for(;;)
	{
		startTime = timestamp();
		pull();
		if( presenting_ )
		{
			if (Moo::rc().device()->Present( NULL, NULL, NULL, NULL ) != D3D_OK)
			{
				DEBUG_MSG( "An error occured while presenting the new frame.\n" );
			}
			/* This query forces the CPU and GPU to synchronise,
			 * this makes the framerate more constant, but more importantly
			 * it stops the mouse movement from becoming jerky.
			 * Without this, what can happen is that the present allows
			 * the cpu to get 2 or 3 frames ahead of the gpu
			 * and then pauses the application to allow it to catch up.
			 */
			std::string result;
			Watcher::Mode mode;
			Watcher::rootWatcher().getAsString( NULL, "Render/Umbra/enabled",result, mode);
			if ( result != "true" )
			{
				IDirect3DQuery9* pQuery;
				Moo::rc().device()->CreateQuery(D3DQUERYTYPE_EVENT, &pQuery);
				pQuery->Issue(D3DISSUE_END);
				while(pQuery->GetData( NULL, 0, D3DGETDATA_FLUSH ) == S_FALSE)
				{
					Sleep( 0 );
				}
				pQuery->Release(); 	
			}

			presenting_ = false;
			endTime = timestamp();
			if (startTime != endTime)
			{
				fps_ = (float)( stampsPerSecondD() / (endTime - startTime));
			}
		}
		else
		{
			break;
		}
	}
}

void App::PresentThread::present()
{
	while( presenting_ )
		Sleep( 0 ); // this gives control to the present thread
	presenting_ = true;
	push();
	app_->onPresent();
}

bool App::PresentThread::isPresenting() const
{
	return presenting_;
}

float App::PresentThread::fps() const
{
	return fps_;
}

void App::PresentThread::stop()
{
	while( presenting_ )
		Sleep( 10 );
	push();
}

void App::PresentThread::init( App* app )
{
	// SimpleThread initialisation
	SimpleThread::init( ThreadFunc, this );

	// PresentThread members
	app_		= app;
	presenting_ = false;
	fps_ = 0.f;
}


App::PresentThread::PresentThread()
	: app_( NULL ), presenting_( false ), fps_( 0.f )
{}
// app.cpp
