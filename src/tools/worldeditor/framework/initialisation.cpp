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
#include "worldeditor/framework/initialisation.hpp"
#ifndef CODE_INLINE
#include "worldeditor/framework/initialisation.ipp"
#endif
#include "worldeditor/scripting/world_editor_script.hpp"
#include "worldeditor/world/world_manager.hpp"
#include "appmgr/module_manager.hpp"
#include "appmgr/options.hpp"
#include "input/input.hpp"
#include "moo/init.hpp"
#include "moo/render_context.hpp"
#include "moo/visual_channels.hpp"
#include "moo/graphics_settings.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/auto_config.hpp"
#include "romp/console_manager.hpp"
#include "romp/console.hpp"
#include "romp/engine_statistics.hpp"
#include "romp/font_manager.hpp"
#include "romp/resource_statistics.hpp"
#include "romp/sky_light_map.hpp"
#include "romp/frame_logger.hpp"
#include "romp/lens_effect_manager.hpp"
#include "post_processing/manager.hpp"
#include "ashes/simple_gui.hpp"
#include "xactsnd/soundmgr.hpp"
#include "fmodsound/sound_manager.hpp"
#include "cstdmf/processor_affinity.hpp"
#include "network/endpoint.hpp"
#include "cstdmf/diary.hpp"
#include <WINSOCK.H>

#include "entitydef/data_description.hpp"


DECLARE_DEBUG_COMPONENT2( "App", 0 )

static AutoConfigString s_engineConfigXML("system/engineConfigXML");

HINSTANCE Initialisation::s_hInstance = NULL;
HWND Initialisation::s_hWndApp = NULL;
HWND Initialisation::s_hWndGraphics = NULL;


/**
 *	This method intialises global resources for the application.
 *
 *	@param hInstance	The HINSTANCE variable for the application.
 *	@param hWndApp		The HWND variable for the application's main window.
 *	@param hWndGraphics	The HWND variable the 3D device will use.
 *
 *	@return		True if initialisation succeeded, otherwise false.
 */
bool
Initialisation::initApp( HINSTANCE hInstance, HWND hWndApp, HWND hWndGraphics )
{
	BW_GUARD;

	s_hInstance = hInstance;
	s_hWndApp = hWndApp;
	s_hWndGraphics = hWndGraphics;

	initErrorHandling();

	// Initialise the timestamps
	initTiming();

	// Create input devices
	InputDevices * pInputDevices = new InputDevices();

	if (!InputDevices::instance().init( hInstance, hWndGraphics ))
	{
		ERROR_MSG( "Initialisation::initApp: Init inputDevices FAILED\n" );
		return false;
	}

	if (!Initialisation::initGraphics(hInstance, hWndGraphics))
	{
		return false;
	}
	// Init winsock
	initNetwork();

	if (!Initialisation::initScripts())
	{
		return false;
	}

	if (!Initialisation::initConsoles())
	{
		return false;
	}

	Initialisation::initSound();

	if ( !WorldManager::instance().init( s_hInstance, s_hWndApp, s_hWndGraphics ) )
	{
		return false;
	}

	return true;
}


/**
 *	This method finalises the application. All stuff done in initApp is undone.
 *	@todo make a better sentence
 */
void Initialisation::finiApp()
{
	BW_GUARD;

	WorldManager::instance().fini();

	Initialisation::finaliseScripts();

	Initialisation::finaliseGraphics();

	delete InputDevices::pInstance();

	Diary::fini();
	DogWatchManager::fini();

	// Kill winsock
	WSACleanup();

	DataType::fini();
}


/**
 *	This method sets up error handling, by routing error msgs
 *	to our own static function
 *
 *	@see messageHandler
 */
bool Initialisation::initErrorHandling()
{
	BW_GUARD;

	MF_WATCH( "debug/filterThreshold",
				DebugFilter::instance(),
				MF_ACCESSORS( int, DebugFilter, filterThreshold ) );

	DebugFilter::instance().addMessageCallback( WorldManager::instance().getDebugMessageCallback() );
	DebugFilter::instance().addCriticalCallback( WorldManager::instance().getCriticalMessageCallback() );
	return true;
}

/**
 *	This method set up the high frequency timing used by bigworld
 */
bool Initialisation::initTiming()
{
	BW_GUARD;

	// Update the processor affinity, this is so that the timing stays in sync on the cpu we run on.
	uint32 affinity = Options::getOptionInt( "app/mainThreadCpuAffinity", ProcessorAffinity::get() );
	ProcessorAffinity::set( affinity );
	
	// this initialises the internal timers for stamps per second, this can take up to a second
	double d = stampsPerSecondD();

	return true;
}


// -----------------------------------------------------------------------------
// Section: Graphics
// -----------------------------------------------------------------------------

/**
 *	This method initialises the graphics subsystem.
 *
 *	@param hInstance	The HINSTANCE variable for the application.
 *	@param hWnd			The HWND variable for the application's main window.
 *
 * @return true if initialisation went well
 */
bool Initialisation::initGraphics( HINSTANCE hInstance, HWND hWnd )
{
	BW_GUARD;

	// Initialise Moo library.
	if ( !Moo::init() )
		return false;

	// Read render surface options.
	bool fullScreen = false;
	uint32 modeIndex = 0;

	// Read shadow options.
	bool useShadows = Options::getOptionBool( "graphics/shadows", false );

	// initialise graphics settings
	DataSectionPtr graphicsPrefDS = BWResource::openSection(
		Options::getOptionString(
			"graphics/graphicsPreferencesXML",
			"resources/graphics_preferences.xml" ) );
	if ( graphicsPrefDS != NULL )
		Moo::GraphicsSetting::init( graphicsPrefDS );

	// Look for Nvidia's PerfHUD device, and use that if available.
	uint32 deviceIndex = 0;

	// Uncomment this to enable
/*
	for (uint32 i = 0; i < Moo::rc().nDevices(); i++)
	{
		std::string description 
			= Moo::rc().deviceInfo(i).identifier_.Description;
		
		if ( description.find("PerfHUD") != std::string::npos )
		{
			deviceIndex		= i;
			break;
		}
	}
*/
	// Initialise the directx device with the settings from the options file.
	if (!Moo::rc().createDevice( hWnd, deviceIndex, modeIndex, !fullScreen, useShadows, Vector2(0, 0), false ))
	{
		CRITICAL_MSG( "Initialisation:initApp: Moo::rc().createDevice() FAILED\n" );
		return false;
	}
	Moo::VisualChannel::initChannels();

	//We don't need no fog man!
	Moo::rc().fogNear( 5000.f );
	Moo::rc().fogFar( 10000.f );

	//ASHES
    SimpleGUI::instance().hwnd( hWnd );

	TextureFeeds::init();

	PostProcessing::Manager::init();

	// Hide the 3D window to avoid it turning black from the clear device in
	// the following method
	::ShowWindow( hWnd, SW_HIDE );

	FontManager::init();
	LensEffectManager::init();

	::ShowWindow( hWnd, SW_SHOW );

	return true;
}


/**
 *	This method finalises the graphics sub system.
 */
void Initialisation::finaliseGraphics()
{
	BW_GUARD;

	LensEffectManager::fini();
	FontManager::fini();

	PostProcessing::Manager::fini();
	TextureFeeds::fini();

	Moo::VertexDeclaration::fini();

	Moo::VisualChannel::finiChannels();

	Moo::rc().releaseDevice();	

	// save graphics settings
	DataSectionPtr graphicsPrefDS = BWResource::openSection(
		Options::getOptionString(
			"graphics/graphicsPreferencesXML",
			"resources/graphics_preferences.xml" ), true );
	if ( graphicsPrefDS != NULL )
	{
		Moo::GraphicsSetting::write( graphicsPrefDS );
		graphicsPrefDS->save();
	}

	// Finalise the Moo library
	Moo::fini();
}



// -----------------------------------------------------------------------------
// Section: Scripts
// -----------------------------------------------------------------------------

/**
 * This method initialises the scripting environment
 *
 * @param hInstance the HINSTANCE variable for the application
 * @param hWnd the HWND variable for the application's main window
 *
 * @return true if initialisation went well
 */
bool Initialisation::initScripts()
{
	BW_GUARD;

	// as a test, nuke the PYTHONPATH
	_putenv( "PYTHONPATH=" );

	if (!WorldEditorScript::init( BWResource::instance().rootSection() ))
	{
		CRITICAL_MSG( "Initialisation::initScripts: WorldEditorScript::init() failed\n" );
		return false;
	}

	return true;
}


/**
 * This method finalises the scripting environment
 */
void Initialisation::finaliseScripts()
{
	BW_GUARD;

	WorldEditorScript::fini();
}


// -----------------------------------------------------------------------------
// Section: Consoles
// -----------------------------------------------------------------------------

/**
 *	This method initialises the consoles.
 */
bool Initialisation::initConsoles()
{
	BW_GUARD;

	// Initialise the consoles
	ConsoleManager & mgr = ConsoleManager::instance();

	XConsole * pStatusConsole = new XConsole();
	XConsole * pStatisticsConsole = new StatisticsConsole( &EngineStatistics::instance() );
	XConsole * pResourceConsole = new ResourceUsageConsole( &ResourceStatistics::instance() );

	mgr.add( new HistogramConsole(),	"Histogram",	KeyCode::KEY_F11 );
	mgr.add( pStatisticsConsole,		"Default",		KeyCode::KEY_F5, MODIFIER_CTRL );
	mgr.add( pResourceConsole,			"Resource",		KeyCode::KEY_F5, MODIFIER_CTRL | MODIFIER_SHIFT );
	mgr.add( new DebugConsole(),		"Debug",		KeyCode::KEY_F7, MODIFIER_CTRL );
	mgr.add( new PythonConsole(),		"Python",		KeyCode::KEY_P, MODIFIER_CTRL );
	mgr.add( pStatusConsole,			"Status" );

	pStatusConsole->setConsoleColour( 0xFF26D1C7 );
	pStatusConsole->setScrolling( true );
	pStatusConsole->setCursor( 0, 20 );

	FrameLogger::init();

	return true;
}


// -----------------------------------------------------------------------------
// Section: Miscellaneous
// -----------------------------------------------------------------------------

void Initialisation::initSound()
{
#if FMOD_SUPPORT
	BW_GUARD;

	//Load the "engine_config.xml" file...
	DataSectionPtr configRoot = BWResource::instance().openSection( s_engineConfigXML.value() );
	
	if (!configRoot)
	{
		ERROR_MSG( "Initialisation::initSound: Couldn't open \"%s\"\n", s_engineConfigXML.value().c_str() );
		return;
	}

	if (! configRoot->readBool( "soundMgr/enabled", true ) )
	{
		WARNING_MSG( "Sounds are disabled for the client (set in \"engine_config.xml/soundMrg/enabled\").\n" );
	}

	DataSectionPtr dsp = configRoot->openSection( "soundMgr" );

	if (dsp)
	{
		if (!SoundManager::instance().initialise( dsp ))
		{
			ERROR_MSG( "PeShell::initSound: Failed to initialise sound\n" );
		}
	}
	else
	{
		ERROR_MSG( "PeShell::initSound: "
			"No <soundMgr> config section found, sound support is "
			"disabled\n" );
	}
#endif // FMOD_SUPPORT
}

/**
 *	Output streaming operator for Initialisation.
 */
std::ostream& operator<<(std::ostream& o, const Initialisation& t)
{
	BW_GUARD;

	o << "Initialisation\n";
	return o;
}

// initialisation.cpp
