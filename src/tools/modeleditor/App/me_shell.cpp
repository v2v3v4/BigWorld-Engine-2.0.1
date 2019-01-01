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

#include "me_shell.hpp"
#include "me_scripter.hpp"

#include "appmgr/application_input.hpp"
#include "appmgr/module_manager.hpp"
#include "appmgr/options.hpp"
#include "resmgr/auto_config.hpp"
#include "ashes/simple_gui.hpp"
#include "chunk/chunk_item_amortise_delete.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"
#include "tools_camera.hpp"
#include "gizmo/tool.hpp"
#include "gizmo/general_editor.hpp"
#include "input/input.hpp"
#include "moo/init.hpp"
#include "moo/render_context.hpp"
#include "moo/visual_channels.hpp"
#include "terrain/manager.hpp"
#include "terrain/terrain_settings.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/resource_cache.hpp"
#include "romp/console_manager.hpp"
#include "romp/console.hpp"
#include "romp/engine_statistics.hpp"
#include "romp/resource_statistics.hpp"
#include "romp/font_manager.hpp"
#include "romp/flora.hpp"
#include "romp/lens_effect_manager.hpp"
#include "common/dxenum.hpp"
#include "common/material_properties.hpp"
#include "post_processing/manager.hpp"

#include "guimanager/gui_input_handler.hpp"
#include "resource_loader.hpp"
#include "splash_dialog.hpp"
#include "material_preview.hpp"
#include "cstdmf/processor_affinity.hpp"

#include "pyscript/pyobject_plus.hpp"

#include "cstdmf/diary.hpp"


DECLARE_DEBUG_COMPONENT2( "Shell", 0 )

static AutoConfigString s_engineConfigXML("system/engineConfigXML");
static AutoConfigString s_floraXML( "environment/floraXML" );
static AutoConfigString s_defaultSpace( "environment/defaultEditorSpace" );
static AutoConfigString s_dxEnumPath( "system/dxenum" );

MeShell * MeShell::s_instance_ = NULL;

// need to define some things for the libs we link to..
// for ChunkManager
std::string g_specialConsoleString = "";
// for network
const char * compileTimeString = __TIME__ " " __DATE__;
// for gizmos
void findRelevantChunks(class SmartPointer<class Tool>, float buffer = 0.f) { ;}

MeShell::MeShell()
	: hInstance_(NULL)
	, hWndApp_(NULL)
	, hWndGraphics_(NULL)
	, romp_(NULL)
{
	BW_GUARD;

	ASSERT(s_instance_ == NULL);
	s_instance_ = this;
}

MeShell::~MeShell()
{
	BW_GUARD;

	ASSERT(s_instance_);
	DebugFilter::instance().deleteMessageCallback( &debugMessageCallback_ );
	s_instance_ = NULL;
}


bool MeShell::initApp( HINSTANCE hInstance, HWND hWndApp, HWND hWndGraphics )
{
	BW_GUARD;

	return MeShell::instance().init( hInstance, hWndApp, hWndGraphics );
}


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
MeShell::init( HINSTANCE hInstance, HWND hWndApp, HWND hWndGraphics )
{
	BW_GUARD;

	hInstance_ = hInstance;
	hWndApp_ = hWndApp;
	hWndGraphics_ = hWndGraphics;

	initErrorHandling();

	// Create Direct Input devices
	InputDevices * pInputDevices = new InputDevices();

	if (!InputDevices::instance().init( hInstance, hWndGraphics ))
	{
		CRITICAL_MSG( "MeShell::initApp - Init inputDevices FAILED\n" );
		return false;
	}

	if (!initTiming())
	{
		return false;
	}

	if (!initGraphics())
	{
		return false;
	}

	if (!initScripts())
	{
		return false;
	}

	if (!initConsoles())
	{
		return false;
	}

    initSound(); // No need to exit if this fails

	// Init the terrain
	Terrain::Manager::init();

	// Init the MaterialPreview singleton. No need to store the pointer in a
	// member variable because we'll use the singleton's pInstance to delete
	// it.
	MaterialPreview* pMaterialPreview = new MaterialPreview();

	// romp needs chunky
	{
		new DXEnum( s_dxEnumPath );

		new AmortiseChunkItemDelete();

		ChunkManager::instance().init();

		ResourceCache::instance().init();

		// Precompile effects?
		if ( Options::getOptionInt( "precompileEffects", 1 ) )
		{
			std::vector<ISplashVisibilityControl*> SVCs;
			if (CSplashDlg::getSVC())
				SVCs.push_back(CSplashDlg::getSVC());
			ResourceLoader::instance().precompileEffects( SVCs );
		}

		ChunkSpacePtr space = new ChunkSpace(1);

		std::string spacePath = Options::getOptionString( "space", s_defaultSpace );
		Matrix& nonConstIdentity = const_cast<Matrix&>( Matrix::identity );
		GeometryMapping* mapping = space->addMapping( SpaceEntryID(), nonConstIdentity, spacePath );
		if (!mapping)
		{
			ERROR_MSG( "Couldn't map path \"%s\" as a space.\n", spacePath.c_str() );
			// To let it work without mapping for s_defaultSpace, used in "Terrain" mode.
		}

		ChunkManager::instance().camera( Matrix::identity, space );

		// Call tick to give it a chance to load the outdoor seed chunk, before
		// we ask it to load the dirty chunks
		ChunkManager::instance().tick( 0.f );
	}

	if (!initRomp())
	{
		return false;
	}

	ApplicationInput::disableModeSwitch();

	return true;
}

bool MeShell::initSound()
{
	BW_GUARD;

#if FMOD_SUPPORT
	//Load the "engine_config.xml" file...
	DataSectionPtr configRoot = BWResource::instance().openSection( s_engineConfigXML.value() );
	
	if (!configRoot)
	{
		ERROR_MSG( "MeShell::initSound: Couldn't open \"%s\"\n", s_engineConfigXML.value().c_str() );
		return false;
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
			ERROR_MSG( "MeShell::initSound: Failed to initialise sound\n" );
			return false;
		}
	}
	else
	{
		ERROR_MSG( "MeShell::initSound: No <soundMgr> config section found, sound support is disabled\n" );
		return false;
	}
#endif // FMOD_SUPPORT
	
	return true;
}

bool MeShell::initRomp()
{
	BW_GUARD;

	if ( !romp_ )
	{
		romp_ = new RompHarness;

		if ( !romp_->init() )
		{
			ERROR_MSG( "MeShell::initRomp: init romp FAILED\n" );
			Py_DECREF( romp_ );
			romp_ = NULL;
			return false;
		}

		// set it into the ModelEditor module
		PyObject * pMod = PyImport_AddModule( "ModelEditor" );	// borrowed
		PyObject_SetAttrString( pMod, "romp", romp_ );

		romp_->enviroMinder().activate();
	}
	return true;
}


/**
 *	This method finalises the application. All stuff done in initApp is undone.
 *	@todo make a better sentence
 */
void MeShell::fini()
{
	BW_GUARD;

	if ( romp_ )
		romp_->enviroMinder().deactivate();

	ResourceLoader::fini();

	ChunkManager::instance().fini();

	ResourceCache::instance().fini();

	if ( romp_ )
	{
		PyObject * pMod = PyImport_AddModule( "ModelEditor" );
		PyObject_DelAttrString( pMod, "romp" );

		Py_DECREF( romp_ );
		romp_ = NULL;
	}

	delete AmortiseChunkItemDelete::pInstance();

	delete MaterialPreview::pInstance();

	Terrain::Manager::fini();

	MeShell::finiGraphics();

	MeShell::finiScripts();

	delete InputDevices::pInstance();

	BWResource::instance().purgeAll();

	// Kill winsock
	//WSACleanup();

	Diary::fini();

	DebugMsgHelper::fini();

	PropManager::fini();

	ModuleManager::fini();

	delete DXEnum::pInstance();
}


bool MeShellDebugMessageCallback::handleMessage( int componentPriority,
												int messagePriority,
												const char * format,
												va_list argPtr )
{
	BW_GUARD;

	return MeShell::instance().messageHandler( componentPriority,
												messagePriority,
												format,
												argPtr );
}


/**
 *	This method sets up error handling, by routing error msgs
 *	to our own static function
 *
 *	@see messageHandler
 */
bool MeShell::initErrorHandling()
{
	BW_GUARD;

	MF_WATCH( "debug/filterThreshold",
				DebugFilter::instance(),
				MF_ACCESSORS( int, DebugFilter, filterThreshold ) );

	DebugFilter::instance().addMessageCallback( &debugMessageCallback_ );
	return true;
}


// -----------------------------------------------------------------------------
// Section: Graphics
// -----------------------------------------------------------------------------

/**
 *	This method initialises the graphics subsystem.
 *
 *	@param hInstance	The HINSTANCE variable for the application.
 *	@param hWnd			The HWND variable for the application's 3D view window.
 *
 * @return true if initialisation went well
 */
bool MeShell::initGraphics()
{
	BW_GUARD;

	if ( !Moo::init() )
		return false;

	// Read render surface options.
	uint32 width = Options::getOptionInt( "graphics/width", 1024 );
	uint32 height = Options::getOptionInt( "graphics/height", 768 );
	uint32 bpp = Options::getOptionInt( "graphics/bpp", 32 );
	bool fullScreen = Options::getOptionBool( "graphics/fullScreen", false );

	const Moo::DeviceInfo& di = Moo::rc().deviceInfo( 0 );

	uint32 modeIndex = 0;
	bool foundMode = false;

	// Go through available modes and try to match a mode from the options file.
	while( foundMode != true &&
		modeIndex != di.displayModes_.size() )
	{
		if( di.displayModes_[ modeIndex ].Width == width &&
			di.displayModes_[ modeIndex ].Height == height )
		{
			if( bpp == 32 &&
				( di.displayModes_[ modeIndex ].Format == D3DFMT_R8G8B8 ||
				di.displayModes_[ modeIndex ].Format == D3DFMT_X8R8G8B8 ) )
			{
				foundMode = true;
			}
			else if( bpp == 16 &&
				( di.displayModes_[ modeIndex ].Format == D3DFMT_R5G6B5 ||
				di.displayModes_[ modeIndex ].Format == D3DFMT_X1R5G5B5 ) )
			{
				foundMode = true;
			}
		}
		if (!foundMode)
			modeIndex++;
	}

	// If the mode could not be found. Set windowed and mode 0.
	if (!foundMode)
	{
		modeIndex = 0;
		fullScreen = false;
	}

	// Read shadow options.
	bool useShadows = Options::getOptionBool( "graphics/shadows", false );

	// Initialise the directx device with the settings from the options file.
	if (!Moo::rc().createDevice( hWndGraphics_, 0, modeIndex, !fullScreen, useShadows, Vector2(0, 0), false ))
	{
		CRITICAL_MSG( "MeShell:initApp: Moo::rc().createDevice() FAILED\n" );
		return false;
	}

	if ( Moo::rc().device() )
	{
		Moo::VisualChannel::initChannels();
		::ShowCursor( true );
	}
	else
	{
		CRITICAL_MSG( "MeShell:initApp: Moo::rc().device() FAILED\n" );
		return false;
	}

	//Use no fogging...
	Moo::rc().fogNear( 5000.f );
	Moo::rc().fogFar( 10000.f );

	//Load Ashes
    SimpleGUI::instance().hwnd( hWndGraphics_ );

	TextureFeeds::init();

	PostProcessing::Manager::init();

	//We need to call this so that we can set material properties
	MaterialProperties::runtimeInitMaterialProperties();

	// Hide the 3D window to avoid it turning black from the clear device in
	// the following method
	::ShowWindow( hWndGraphics_, SW_HIDE );

	FontManager::init();
	LensEffectManager::init();

	::ShowWindow( hWndGraphics_, SW_SHOW );

	return true;
}


/**
 *	This method finalises the graphics sub system.
 */
void MeShell::finiGraphics()
{
	BW_GUARD;

	LensEffectManager::fini();
	FontManager::fini();
	PostProcessing::Manager::fini(); 
	TextureFeeds::fini(); 

	Moo::VertexDeclaration::fini();

	Moo::rc().releaseDevice();

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
bool MeShell::initScripts()
{
	BW_GUARD;

	if (!Scripter::init( BWResource::instance().rootSection() ))
	{
		CRITICAL_MSG( "MeShell::initScripts: failed\n" );
		return false;
	}

	return true;
}


/**
 * This method finalises the scripting environment
 */
void MeShell::finiScripts()
{
	BW_GUARD;

	Scripter::fini();
}

POINT MeShell::currentCursorPosition() const
{
	BW_GUARD;

	POINT pt;
	::GetCursorPos( &pt );
	::ScreenToClient( hWndGraphics_, &pt );

	return pt;
}


// -----------------------------------------------------------------------------
// Section: Consoles
// -----------------------------------------------------------------------------

/**
 *	This method initialises the consoles.
 */
bool MeShell::initConsoles()
{
	BW_GUARD;

	// Initialise the consoles
	ConsoleManager & mgr = ConsoleManager::instance();

	XConsole * pStatusConsole = new XConsole();
	XConsole * pStatisticsConsole =
		new StatisticsConsole( &EngineStatistics::instance() );

	mgr.add( pStatisticsConsole,											"Default",	KeyCode::KEY_F5, MODIFIER_CTRL );
	mgr.add( new ResourceUsageConsole( &ResourceStatistics::instance() ),	"Resource",	KeyCode::KEY_F5, MODIFIER_CTRL | MODIFIER_SHIFT );
	mgr.add( new DebugConsole(),											"Debug",	KeyCode::KEY_F7, MODIFIER_CTRL );
	mgr.add( new PythonConsole(),											"Python",	KeyCode::KEY_P, MODIFIER_CTRL );
	mgr.add( pStatusConsole,												"Status" );

	pStatusConsole->setConsoleColour( 0xFF26D1C7 );
	pStatusConsole->setScrolling( true );
	pStatusConsole->setCursor( 0, 20 );

	return true;
}

// -----------------------------------------------------------------------------
// Section: Timing
// -----------------------------------------------------------------------------

/**
 *	This method inits the timing code, it also sets up the
 *	
 */
bool MeShell::initTiming()
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
// Section: Miscellaneous
// -----------------------------------------------------------------------------


/**
 *	This static function implements the callback that will be called for each
 *	*_MSG.
 */
bool MeShell::messageHandler( int componentPriority,
		int messagePriority,
		const char * format, va_list argPtr )
{
	// This is commented out to stop ModelEditor from spitting out
	// an error box for every message, it still appears in 3D view
	// and messages panel.
	//if ( componentPriority >= DebugFilter::instance().filterThreshold() &&
	//	messagePriority == MESSAGE_PRIORITY_ERROR )
	//{
	//	bool fullScreen = !Moo::rc().windowed();

	//	//don't display message boxes in full screen mode.
	//	char buf[2*BUFSIZ];
	//	bw_vsnprintf( buf, sizeof(buf), format, argPtr );
	//	buf[sizeof(buf)-1] = '\0';

	//	if ( DebugMsgHelper::showErrorDialogs() && !fullScreen )
	//	{
	//		::MessageBox( AfxGetApp()->m_pMainWnd->GetSafeHwnd(),
	//			buf, "Error", MB_ICONERROR | MB_OK );
	//	}
	//}
	return false;
}

/**
 *	Output streaming operator for MeShell.
 */
std::ostream& operator<<(std::ostream& o, const MeShell& t)
{
	BW_GUARD;

	o << "MeShell\n";
	return o;
}
