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

#define TIMESTAMP_UNRELIABLE	// for broken laptops

#include "action_matcher.hpp"	// for the 'debug_' static
#include "adaptive_lod_controller.hpp"
#include "app.hpp"
#include "app_config.hpp"
#include "bw_winmain.hpp"
#include "canvas_app.hpp"
#include "client_camera.hpp"
#include "connection_control.hpp"
#include "debug_app.hpp"
#include "device_app.hpp"
#include "entity.hpp"
#include "entity_flare_collider.hpp"
#include "entity_manager.hpp"
#include "entity_picker.hpp"
#include "entity_type.hpp"
#include "filter.hpp"
#include "finale_app.hpp"
#include "physics.hpp"
#include "player.hpp"
#include "py_server.hpp"
#include "python_server.hpp"
#include "resource.h"
#include "script_bigworld.hpp"
#include "shadow_manager.hpp"
#include "version_info.hpp"
#include "web_app.hpp"
#include "world.hpp"
#include "world_app.hpp"

#include "ashes/alpha_gui_shader.hpp"
#include "ashes/clip_gui_shader.hpp"
#include "ashes/frame_gui_component.hpp"
#include "ashes/simple_gui.hpp"
#include "ashes/text_gui_component.hpp"

#include "camera/annal.hpp"
#include "camera/camera_control.hpp"
#include "camera/direction_cursor.hpp"
#include "camera/free_camera.hpp"
#include "camera/projection_access.hpp"

#include "chunk/chunk.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_obstacle.hpp"
#include "chunk/chunk_space.hpp"
#if UMBRA_ENABLE
#include "chunk/chunk_umbra.hpp"
#endif

#include "cstdmf/base64.h"
#include "cstdmf/config.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/dogwatch.hpp"
#include "cstdmf/main_loop_task.hpp"
#include "cstdmf/memory_stream.hpp"
#include "cstdmf/memory_trace.hpp"
#include "cstdmf/profile.hpp"
#include "cstdmf/profiler.hpp"
#include "cstdmf/string_utils.hpp"
#include "cstdmf/timestamp.hpp"
#include "cstdmf/watcher.hpp"

#include "connection/server_connection.hpp"

#include "duplo/foot_trigger.hpp"
#include "duplo/py_loft.hpp"
#include "duplo/py_splodge.hpp"
#include "duplo/pymodel.hpp"

#include "input/py_input.hpp"
#include "input/input_cursor.hpp"
#include "input/ime.hpp"

#include "math/blend_transform.hpp"
#include "math/colour.hpp"

#include "model/super_model.hpp"

#include "moo/animation_manager.hpp"
#include "moo/animating_texture.hpp"
#include "moo/effect_material.hpp"
#include "moo/effect_visual_context.hpp"
#include "moo/interpolated_animation_channel.hpp"
#include "moo/texture_manager.hpp"
#include "moo/vertex_declaration.hpp"
#include "moo/visual_channels.hpp"
#include "moo/visual_manager.hpp"
#include "moo/visual_compound.hpp"

#include "particle/particle_system_manager.hpp"

#include "network/remote_stepper.hpp"
#if ENABLE_WATCHERS
#include "network/watcher_glue.hpp"
#else
#include <WINSOCK.H>	// for gethostname
#endif

#include "pyscript/personality.hpp"
#include "pyscript/py_callback.hpp"
#include "pyscript/py_data_section.hpp"
#include "pyscript/py_output_writer.hpp"

#include "resmgr/access_monitor.hpp"
#include "resmgr/auto_config.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/win_file_system.hpp"

#include "romp/console.hpp"
#include "romp/console_manager.hpp"
#include "romp/diary_display.hpp"
#include "romp/fog_controller.hpp"
#include "romp/engine_statistics.hpp"
#include "romp/enviro_minder.hpp"
#include "romp/frame_logger.hpp"
#include "romp/geometrics.hpp"
#include "romp/gui_progress.hpp"
#include "romp/histogram_provider.hpp"
#include "romp/flash_bang_effect.hpp"
#include "romp/flora.hpp"
#include "romp/lens_effect_manager.hpp"
#include "romp/py_chunk_light.hpp"
#include "romp/py_chunk_spot_light.hpp"
#include "romp/rain.hpp"
#include "romp/sea.hpp"
#include "romp/sky_gradient_dome.hpp"
#include "romp/sky_light_map.hpp"
#include "romp/sun_and_moon.hpp"
#include "romp/star_dome.hpp"
#include "romp/terrain_occluder.hpp"
#include "romp/texture_renderer.hpp"
#include "romp/time_of_day.hpp"
#include "romp/water.hpp"
#include "romp/weather.hpp"
#include "romp/z_buffer_occluder.hpp"

#include "waypoint/waypoint_stats.hpp"

#include "scaleform/config.hpp"
#if SCALEFORM_SUPPORT
	#include "scaleform/manager.hpp"
	#include "scaleform/util.hpp"
#endif

#include "zip/zlib.h"

#include <algorithm>

#ifndef TIMESTAMP_UNRELIABLE
#define frameTimerSetup() stampsPerSecond()
#define frameTimerValue() timestamp()
#define frameTimerFreq() stampsPerSecondD()
#define FRAME_TIMER_TYPE uint64
#else
#include <mmsystem.h>
#define frameTimerSetup() timeBeginPeriod(1)
#define frameTimerValue() timeGetTime()
#define frameTimerFreq() 1000.0
#define FRAME_TIMER_TYPE DWORD
#endif

class BackBufferEffect;

extern void setupTextureFeedPropertyProcessors();


#ifndef CODE_INLINE
#include "app.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "App", 0 );

PROFILER_DECLARE( Sys_Sleep, "Sys Sleep" );

PROFILER_DECLARE( App_Tick, "App Tick" );
PROFILER_DECLARE( App_Draw, "App Draw" );


// -----------------------------------------------------------------------------
// Section: Config string
// -----------------------------------------------------------------------------

#if   defined( _DEBUG )
#define CONFIG_STRING "DEBUG"
#elif defined( _INSTRUMENTED )
#define CONFIG_STRING "INSTRUMENTED"
#elif defined( _HYBRID )
  #if defined( _EVALUATION )
    #define CONFIG_STRING "EVALUATION"
  #else
    #define CONFIG_STRING "HYBRID"
  #endif
#elif defined(_RELEASE)
#define CONFIG_STRING "RELEASE"
#else
#define CONFIG_STRING "UNKNOWN"
#endif

const char * configString		= CONFIG_STRING;

#define SAFE_DELETE(pointer)\
	if (pointer != NULL)    \
	{                       \
		delete pointer;     \
		pointer = NULL;     \
	}

// -----------------------------------------------------------------------------
// Section: Statics and globals
// -----------------------------------------------------------------------------

App * App::pInstance_ = NULL;

static const float FAR_DISTANCE  = 10000.0f;
//there are 4 stages to progress;
//app startup, shader compilation, preloads + personality script init.
//in C++ the total progress goes to 100%, but the gui progress bar script
//rescales this value for display if it needs some leeway at the end of the
//progress bar for personality script initialisation.
static const float PROGRESS_TOTAL = 3.f;
static const float APP_PROGRESS_STEP = 1.f / 10.f;	//app startup has 10 steps

static DogWatch g_splodgeWatch( "Splodge" );

static DogWatch g_floraWatch( "Flora" );


bool g_drawWireframe = false;

/// Adjusting this allows run-time adjustment of the discrete level of detail.
float CLODPower = 10.f;

// reference the ChunkInhabitants that we want to be able to load.
extern int ChunkAttachment_token;
extern int ChunkModel_token;
extern int ChunkLight_token;
extern int ChunkTerrain_token;
extern int ChunkFlare_token;
extern int ChunkWater_token;
extern int ChunkSound_token;
extern int ChunkEntity_token;
extern int ChunkParticles_token;
extern int ChunkTree_token;
extern int ChunkStationNode_token;
extern int ChunkUserDataObject_token;
static int s_chunkTokenSet = ChunkAttachment_token | ChunkModel_token | ChunkLight_token |
	ChunkTerrain_token | ChunkFlare_token | ChunkWater_token |
	ChunkSound_token | ChunkEntity_token |
	ChunkParticles_token | ChunkTree_token | 
	ChunkStationNode_token| ChunkUserDataObject_token;

extern int PyMetaParticleSystem_token;
extern int PyParticleSystem_token;
static int PS_tokenSet = PyMetaParticleSystem_token | PyParticleSystem_token;

extern int Tracker_token;
extern int PyMorphControl_token;
static int fashionTokenSet = Tracker_token | PyMorphControl_token;

extern int FootTrigger_token;
extern int PySplodge_token;
extern int PyLoft_token;
static int attachmentTokenSet = FootTrigger_token | PySplodge_token | PyLoft_token;

extern int PyModelObstacle_token;
static int embodimentTokenSet = PyModelObstacle_token;

extern int AvatarFilter_token;
extern int AvatarDropFilter_token;
extern int BoidsFilter_token;
extern int DumbFilter_token;
extern int PlayerAvatarFilter_token;
static int filterTokenSet =	AvatarFilter_token |
							AvatarDropFilter_token |
							BoidsFilter_token |
							DumbFilter_token |
							PlayerAvatarFilter_token;

extern int Decal_token;
extern int PyModelRenderer_token;
extern int PySceneRenderer_token;
extern int PyResourceRefs_token;
extern int TextureFeeds_token;
extern int Oscillator_token;
extern int Servo_token;
extern int PyEntities_token;
extern int PyChunk_token;
extern int Homer_token;
extern int Bouncer_token;
extern int Propellor_token;
extern int LinearHomer_token;
extern int Orbitor_token;
extern int BoxAttachment_token;
extern int SkeletonCollider_token;
extern int PyGraphicsSetting_token;
extern int PyPhysics2_token;
extern int PyVOIP_token;
extern int PyWebPageProvider_token;
extern int ServerDiscovery_token;
extern int Pot_token;
extern int PyMaterial_token;
extern int PyRenderTarget_token;
extern int PyShimmerCountProvider_token;

static int miscTokenSet = PyModelRenderer_token | PySceneRenderer_token |
	PyEntities_token | PyChunk_token | Oscillator_token | Homer_token | Bouncer_token |
	Propellor_token | ServerDiscovery_token | Pot_token | TextureFeeds_token |
	Servo_token | LinearHomer_token | Orbitor_token | BoxAttachment_token |
	SkeletonCollider_token | Decal_token | PyPhysics2_token |
	PyVOIP_token | PyResourceRefs_token | PyWebPageProvider_token |
	PyMaterial_token | PyRenderTarget_token | PyShimmerCountProvider_token |
	PyGraphicsSetting_token;

namespace PostProcessing
{
	extern int tokenSet;
	extern int PlayerFader_token;
	static int ppTokenSet = tokenSet | PlayerFader_token;
}

extern int LatencyGUIComponent_token;
extern int Minimap_token;
static int guiTokenSet = LatencyGUIComponent_token | Minimap_token;

extern int EntityDirProvider_token;
extern int DiffDirProvider_token;
extern int ScanDirProvider_token;
extern int InvViewMatrixProvider_token;
static int dirProvTokenSet = EntityDirProvider_token | DiffDirProvider_token |
	ScanDirProvider_token | InvViewMatrixProvider_token;


extern int CameraApp_token;
extern int CanvasApp_token;
extern int DebugApp_token;
extern int DeviceApp_token;
extern int FacadeApp_token;
extern int FinalApp_token;
extern int GUIApp_token;
extern int LensApp_token;
extern int ProfilerApp_token;
extern int ScriptApp_token;
extern int VOIPApp_token;
extern int WebApp_token;
extern int WorldApp_token;
static int mainLoopTaskTokenSet =	CameraApp_token |
									CanvasApp_token |
									DebugApp_token |
									DeviceApp_token |
									FacadeApp_token |
									FinalApp_token |
									GUIApp_token |
									LensApp_token |
									ProfilerApp_token |
									ScriptApp_token |
									VOIPApp_token |
									WebApp_token | 
									WorldApp_token;


bool gWorldDrawEnabled = true;
const char * const gWorldDrawLoopTasks[] = {	"Canvas",
												"World",
												"Flora",
												"Facade",
												"Lens"		};

#ifdef _DEBUG

extern uint32 heapSize();


#define MEM_SIGNATURE	'mfmm'
/*
void* operator new(size_t amount)
{
	BW_GUARD;
	//TRACE_MSG("operator new:\n");

	_totalMemUsedBytes += amount;
	_totalMemUsed = _totalMemUsedBytes / 1024;
	uint32 *mem = (uint32*) malloc(amount + (sizeof(uint32)*2));
	*mem++ = MEM_SIGNATURE;
	*mem++ = amount;
	return mem;
}



void operator delete(void *memory)
{
	BW_GUARD;
	//TRACE_MSG("operator delete:\n");

	if (!memory)
		return;

	uint32 *mem = (uint32 *)memory;
	uint32 sz = *--mem;
	uint32 sig = *--mem;

	if (sig != MEM_SIGNATURE) {
		OutputDebugString("delete: bad sig on free **********\n");
		ENTER_DEBUGGER();
	} else {
		_totalMemUsedBytes -= sz;
		_totalMemUsed = _totalMemUsedBytes / 1024;
		free(mem);
	}
}


void* operator new[](size_t amount)
{
	BW_GUARD;
	//TRACE_MSG("operator new[]:\n");
	return (operator new)(amount);
}


void operator delete[](void *memory)
{
	BW_GUARD;
	//TRACE_MSG("operator delete[]:\n");
	(operator delete)(memory);
}
*/

#endif // _DEBUG

AutoConfigString s_engineConfigXML("system/engineConfigXML");
AutoConfigString s_scriptsConfigXML("system/scriptsConfigXML");
AutoConfigString loadingScreenName("system/loadingScreen");
AutoConfigString loadingScreenGUI("system/loadingScreenGUI");
AutoConfigString s_graphicsSettingsXML("system/graphicsSettingsXML");
AutoConfigString s_floraXML("environment/floraXML");
AutoConfigString s_shadowsXML("system/shadowsXML");
AutoConfigString s_blackTexture("system/blackBmp");
int  s_framesCounter              = -1;
bool s_usingDeprecatedBigWorldXML = false;
//static bool displayLoadingScreen();
//static void freeLoadingScreen();
//static void loadingText( const std::string & s );
DataSectionPtr s_scriptsPreferences = NULL;
std::string s_configFileName( "" );

namespace 
{
	/**
	*	This function returns the total game time elapsed, used by callbacks from
	*	lower level modules, so they do not create circular dependencies back to
	*	the bwclient lib.
	*/
	double getGameTotalTime()
	{
		BW_GUARD;
		return App::instance().getTime();
	}
}

// -----------------------------------------------------------------------------
// Section: Initialisation and Termination
// -----------------------------------------------------------------------------

/**
 *	The constructor for the App class. App::init needs to be called to fully
 *	initialise the application.
 *
 *  @param configFilename		top level configuration file, eg "bigworld.xml".
 *  @param compileTime			build time stamp to display, if required.
 *
 *	@see App::init
 */
App::App( const std::string &	configFilename,
		  const char *			compileTime	) :
	hWnd_( NULL ),
	isEmbedded_( true ),
	dTime_(0.0f),
	lastTime_(0),
	lastFrameEndTime_(0),
	minFrameTime_(0),
	totalTime_( 0.f ),
	minimumFrameRate_( 8.f ),
#if !ENABLE_DEBUG_KEY_HANDLER
	debugKeyEnable_( false ),
#else
	debugKeyEnable_( true ),
#endif
	activeCursor_( NULL ),
	handleKeyEventDepth_( -1 ),
	sleepTime_( 1 )
{
	BW_GUARD;

	// Init some singletons
	InputDevices * pInputDevice = new InputDevices();
	WaypointStats * pWaypointStats = new WaypointStats();


	// If specified, copy in compile time string.
	if ( compileTime )
		compileTime_ = compileTime;

	frameTimerSetup();
	lastTime_ = frameTimerValue();

	// Set callback for PyScript so it can know total game time
	Script::setTotalGameTimeFn( getGameTotalTime );

	// Make sure that this is the only instance of the app.
	MF_ASSERT_DEV( pInstance_ == NULL );
	pInstance_ = this;

	// Clear key routing tables
	ZeroMemory( &keyRouting_, sizeof( keyRouting_ ) );

	// Run things that configure themselves from a config file
	if (!AutoConfig::configureAllFrom( AutoConfig::s_resourcesXML ))
	{
		criticalInitError(
			"Could not find resources.xml, which should "
			"contain the location of system resources!" );

		throw InitError( "Could not load resources XML" );
	}

	// Load engine_config.xml
	std::string filename = s_engineConfigXML.value();
	if(!configFilename.empty())
	{
		INFO_MSG("Loading engine configuration file '%s' from command line.\n", configFilename);
		filename = configFilename;
	}

	DataSectionPtr configRoot =
		BWResource::instance().openSection(filename);

	if (AppConfig::instance().init(configRoot))
	{
		s_usingDeprecatedBigWorldXML = false;
		s_configFileName = s_engineConfigXML.value();
	}
	else
	{
		criticalInitError( "Could not load config file: %s!",
					filename.c_str() );

		throw InitError( "Could not load config file" );
	}

	DataSectionPtr configSection = AppConfig::instance().pRoot();

	BW_INIT_WATCHER_DOC( "client" );

	// Read in the debug keys
	DataSectionPtr debugKeysSection = configSection->openSection( "debugKeys" );
	if ( debugKeysSection )
	{
		std::vector< DataSectionPtr> combinations;
		debugKeysSection->openSections( "combination", combinations );

		for (size_t i = 0; i < combinations.size(); i++)
		{
			KeyCode::KeyArray debugKeyArray;

			std::vector< DataSectionPtr> keys;
			combinations[i]->openSections( "key", keys );

			for(size_t k = 0; k < keys.size(); k++)
			{
				std::string keyName = keys[k]->asString();
				KeyCode::Key key = KeyCode::stringToKey( keyName );
				if (key != KeyCode::KEY_NOT_FOUND)
				{
					debugKeyArray.push_back( key );
				}				
			}

			if ( !debugKeyArray.empty() )
			{
				debugKeys_.push_back( debugKeyArray );
			}
		}
	}
	
	// If we didn't get any valid debug keys, add a default.
	if ( debugKeys_.empty() )
	{
		KeyCode::KeyArray debugKeyArray;
		debugKeyArray.push_back( KeyCode::KEY_GRAVE );
		debugKeys_.push_back( debugKeyArray );
	}

	keyUpChar_.reset();

	lastFrameEndTime_ = frameTimerValue();
	int frameRate = configSection->readInt( "renderer/maxFrameRate", 0 );
	minFrameTime_ = frameRate != 0 ? uint64(frameTimerFreq() / frameRate) : 0;

	s_framesCounter = configSection->readInt( "debug/framesCount", 0 );

	// Initialise Access Monitoring.
	AccessMonitor::instance().active( configSection->readBool(
		"accessMonitor", false ) );

	// Check filenames:
#if ENABLE_FILE_CASE_CHECKING
	bool checkFilesCase = configSection->readBool( "debug/checkFileCase", false );
	BWResource::checkCaseOfPaths(checkFilesCase);
#endif // ENABLE_FILE_CASE_CHECKING

	// If there's only one core we need a small sleep every frame,
	// otherwise chunks would never get loaded.  If there's more than one
	// core, we don't need this
	if ( getNumCores() > 1 )
	{
		sleepTime_ = 0;
	}
}


/**
 *	Destructor for the App class.
 */
App::~App()
{
	BW_GUARD;
	BWResource::watchAccessFromCallingThread(false);
	fini();
}


/*~	function BigWorld.setCursor
 *
 *	Sets the active cursor. The active cursor will get all mouse,
 *	keyboard and axis events forwarded to it. Only one cursor can
 *	be active at a time.
 *
 *	@param	cursor	the new active cursor, or None to simply
 *					deactivate the current one.
 */
PyObject * App::py_setCursor( PyObject * args )
{
	BW_GUARD;
	PyObject* pyCursor = NULL;
	if (!PyArg_ParseTuple( args, "O", &pyCursor ))
	{
		PyErr_SetString( PyExc_TypeError, "py_setCursor: Argument parsing error." );
		return NULL;
	}

	// set to null
	InputCursor * cursor = NULL;
	if (pyCursor != Py_None)
	{
		// type check
		if (!InputCursor::Check( pyCursor ))
		{
			PyErr_SetString( PyExc_TypeError, "py_setCursor: Expected a Cursor." );
			return NULL;
		}

		cursor = static_cast< InputCursor * >( pyCursor );
	}

	App::instance().activeCursor( cursor );
	Py_Return;
}
PY_MODULE_STATIC_METHOD( App, setCursor, BigWorld )

extern void initNetwork();
extern void reloadChunks(); //script_bigworld.cpp

#ifdef TRACK_MEMORY_BLOCKS

namespace MemoryTrackingSocket
{
	void send( const void * data, uint32 size );

	enum { CALL_STACK_DEPTH = 32 };

	static THREADLOCAL( bool ) handling = false;

	static void sendAllocationMessage( void * addr )
	{
		BW_GUARD;
		if (handling) return;
		handling = true;

		uint32 msg[3+CALL_STACK_DEPTH];
		msg[0] = (uint32)addr;
		msg[1] = 0;

		uint32 blocks = allocatedSizeBase( addr ) >> 4;
		if (blocks < 65536)
			msg[2] = blocks<<16;
		else
			msg[2] = (blocks|128)<<8;/* = ((blocks>>8)<<16) | (1<<15);*/


		DmCaptureStackBackTrace( CALL_STACK_DEPTH, (void**)(msg+3) );
		uint32 frames = 0;
		while (frames < CALL_STACK_DEPTH && (msg+3)[frames++] != 0) ; //scan
		msg[2] |= frames;

		MemoryTrackingSocket::send( msg, (3 + frames)*sizeof(uint32) );
		handling = false;
	}

	static void sendDeallocationMessage( void * addr )
	{
		BW_GUARD;
		if (handling) return;
		handling = true;

		uint32 msg[3];
		msg[0] = (uint32)addr;
		msg[1] = uint32(-1);
		msg[2] = 0;
		MemoryTrackingSocket::send( msg, sizeof(msg) );

		handling = false;
	}

	typedef uint32 (WINAPI *TellDebuggerFn)(
		unsigned long ulCode, void * pvData );
	static TellDebuggerFn s_TellDebuggerDefault = NULL;

	struct DebuggerIF
	{
		uint32			pSomething0_;
		uint32			pSomething1_;
		uint32			filler_;
		uint32			pSomething3_;
		uint32			pSomething4_;
		TellDebuggerFn	pTellDebugger_;
	};

	enum DebugMsgCodes
	{
		DMSG_ALLOC = 0x0A,
		DMSG_FREE = 0x0B
	};

	/**
	*	This function intercepts a call to the debugger from the runtime
	*	environment. We are interested in memory alloction messages.
	*/
	static uint32 WINAPI tellDebuggerInterloper(
		unsigned long ulCode, void * pvData )
	{
		BW_GUARD;
		if (ulCode == DMSG_ALLOC)
		{
			uint32 addr = ((uint32*)pvData)[0];
			uint32 size = ((uint32*)pvData)[1];
			//dprintf( "Allocated %d at 0x%08X\n", size, addr );

			sendAllocationMessage( (void*)addr );
		}
		else if (ulCode == DMSG_FREE)
		{
			uint32 addr = ((uint32*)pvData)[0];
			//dprintf( "Freed 0x%08X\n", addr );

			sendDeallocationMessage( (void*)addr );
		}
		else
		{
			//dprintf( "Telling debugger %d 0x%08X\n", ulCode, pvData );
		}
		return (*s_TellDebuggerDefault)( ulCode, pvData );
	}


	static DebuggerIF * get_g_dmi()
	{
		__asm
		{
			mov eax, DWORD PTR fs:0x20
			mov eax, DWORD PTR 0x250[eax]
		}
	}


	extern "C"
	{
		extern __declspec(dllimport) HRESULT __stdcall
			MmAllocateContiguousMemory( DWORD NumberOfBytes );
		extern __declspec(dllimport) HRESULT __stdcall
			MmAllocateContiguousMemoryEx(
				DWORD NumberOfBytes, DWORD LowestAcceptableAddress,
				DWORD HighestAcceptableAddress, DWORD Alignment, DWORD Protect );
		extern __declspec(dllimport) HRESULT __stdcall
			MmFreeContiguousMemory( PVOID BaseAddress );
	}

	const uint8 & irqLevel = *(uint8*)0x8004C5D0;

	static HRESULT (__stdcall *s_AllocateContiguousMemoryDefault)( DWORD NumberOfBytes );
	static HRESULT __stdcall AllocateContiguousMemoryInterloper( DWORD NumberOfBytes )
	{
		BW_GUARD;
		HRESULT res = (*s_AllocateContiguousMemoryDefault)( NumberOfBytes );
		if (irqLevel == 0)
			sendAllocationMessage( (void*)res );
		return res;
	}

	static HRESULT (__stdcall *s_AllocateContiguousMemoryExDefault)(
		DWORD, DWORD, DWORD, DWORD, DWORD );
	static HRESULT __stdcall AllocateContiguousMemoryExInterloper(
		DWORD NumberOfBytes, DWORD LowestAcceptableAddress,
		DWORD HighestAcceptableAddress, DWORD Alignment, DWORD Protect )
	{
		BW_GUARD;
		HRESULT res = (*s_AllocateContiguousMemoryExDefault)(
			NumberOfBytes, LowestAcceptableAddress,
			HighestAcceptableAddress, Alignment, Protect );
		if (irqLevel == 0)
			sendAllocationMessage( (void*)res );
		return res;
	}

	static HRESULT (__stdcall *s_FreeContiguousMemoryDefault)( PVOID BaseAddress );
	static HRESULT __stdcall FreeContiguousMemoryInterloper( PVOID BaseAddress )
	{
		BW_GUARD;
		if (irqLevel == 0)
			sendDeallocationMessage( BaseAddress );
		HRESULT res = (*s_FreeContiguousMemoryDefault)( BaseAddress );
		return res;
	}

#define GET_SYMBOL_ADDRESS( ADDRVAR, SYMBOL )	\
		__asm { lea eax, SYMBOL };				\
		void ** ADDRVAR = emptyFunc()			\

	static inline void ** emptyFunc() { __asm mov eax, eax };

	static void setValBypass( void *& loc, void * val )
	{
		BW_GUARD;
		MEMORY_BASIC_INFORMATION curProt;
		VirtualQuery( &loc, &curProt, sizeof(curProt) );
		if (!(curProt.Protect & PAGE_READWRITE))
		{
			DWORD newProt = PAGE_READWRITE;
			DWORD oldProt = 0;
			VirtualProtect( &loc, sizeof(void*), newProt, &oldProt );
		}

		loc = val;	// all that work for this!

		if (!(curProt.Protect & PAGE_READWRITE))
		{
			DWORD newProt = curProt.Protect;
			DWORD oldProt = 0;
			VirtualProtect( &loc, sizeof(void*), newProt, &oldProt );
		}
	}

	static SimpleMutex * oneAnything = NULL;
	static Endpoint * s_socket = NULL;

	void init()
	{
		BW_GUARD;
		MF_ASSERT_DEV( s_socket == NULL );

		if (oneAnything == NULL)	// allocate before catching allocs!
			oneAnything = new SimpleMutex();

		Endpoint listener;
		listener.socket( SOCK_STREAM );
		listener.bind( htons(37645) );
		listener.listen( 1 );

		INFO_MSG( "MemoryTrackingSocket::init(): "
			"Waiting for memory tracking connection on port %d\n", 37645 );

		s_socket = listener.accept();

		INFO_MSG( "MemoryTrackingSocket::init(): "
			"Accepted memory tracking connection\n" );

		// reroute debugger messages to our function
		DebuggerIF * g_dmi = get_g_dmi();
		s_TellDebuggerDefault = g_dmi->pTellDebugger_;
		g_dmi->pTellDebugger_ = tellDebuggerInterloper;

		// change the address of contiguous memory allocations and
		// deallocations to go through our functions too
		s_AllocateContiguousMemoryDefault = MmAllocateContiguousMemory;
		GET_SYMBOL_ADDRESS( pMmACM, MmAllocateContiguousMemory );
		setValBypass( *pMmACM, (void*)AllocateContiguousMemoryInterloper );
		s_AllocateContiguousMemoryExDefault = MmAllocateContiguousMemoryEx;
		GET_SYMBOL_ADDRESS( pMmACMEx, MmAllocateContiguousMemoryEx );
		setValBypass( *pMmACMEx, (void*)AllocateContiguousMemoryExInterloper );
		s_FreeContiguousMemoryDefault = MmFreeContiguousMemory;
		GET_SYMBOL_ADDRESS( pMmFCM, MmFreeContiguousMemory );
		setValBypass( *pMmFCM, (void*)FreeContiguousMemoryInterloper );
	}

	struct AtomicInt
	{
		inline int __fastcall inc()
		{
			__asm mov eax, 1
			__asm lock xadd [ecx], eax
		}

		inline int __fastcall add( int amt )
		{
			__asm mov eax, edx
			__asm lock xadd [ecx], eax
		}

		int val_;
	};

	void send( const void * data, uint32 size )
	{
		BW_GUARD;
		// static initialisers can register allocations with their static
		// memory counter before it has been constructed
		if (s_socket == NULL) return;

		oneAnything->grab();

		static AtomicInt nsent = { 0 };
		static bool waitingAck = false;
		static volatile int nkilos = 0;

		// send this message
		s_socket->send( data, size );

		// see if we've got an ack back
		if (waitingAck)
		{
			// TODO: fix race condition here if we remove oneAnything mutex
			bool shouldBlock = nsent.val_ > 512;
			if (!shouldBlock)
				s_socket->setnonblocking( true );

			int ack = -1;
			int myks = nkilos;
			if (s_socket->recv( &ack, 4 ) == 4 || shouldBlock)
			{
				if (ack != myks)
				{
					ERROR_MSG( "MTS: Got wrong ack: %d instead of %d\n",
						ack, myks );
					// but there's not much we can do about it...
					delete s_socket;
					s_socket = NULL;
				}
				else
				{
					nkilos++;
					waitingAck = false;
				}
			}

			if (!shouldBlock && s_socket != NULL)
				s_socket->setnonblocking( false );
		}

		// see if we expect another ack
		if (nsent.inc() == 1024-1)
		{
			nsent.add( -1024 );
			waitingAck = true;

			//static SimpleMutex oneReceiver;
/*
			int myks = nkilos++;
			int ack = -1;

			//INFO_MSG( "MTS: Waiting for ack for %d kilos\n", myks );
			//oneReceiver.grab();
			if (s_socket->recv( &ack, 4 ) != 4 || ack != myks)
			{
				ERROR_MSG( "MTS: Got wrong ack: %d instead of %d\n",
					ack, myks );
				// but there's not much we can do about it...
				delete s_socket;
				s_socket = NULL;
			}
			else
			{
				//INFO_MSG( "MTS: Got ack\n" );
			}
*/
			//oneReceiver.give();
		}
		oneAnything->give();
	}
};
#endif


#if ENABLE_WATCHERS
uint32 memUsed();

uint32 memoryAccountedFor();
int32 memoryUnclaimed();
#endif




static DogWatch	g_watchTick("Tick");
static DogWatch	g_watchUpdate("Update");
static DogWatch	g_watchOutput("Output");

typedef SmartPointer<PyModel> PyModelPtr;



HINSTANCE DeviceApp::s_hInstance_ = NULL;
HWND DeviceApp::s_hWnd_ = NULL;
ProgressDisplay * DeviceApp::s_pProgress_ = NULL;
GUIProgressDisplay * DeviceApp::s_pGUIProgress_ = NULL;
ProgressTask * DeviceApp::s_pStartupProgTask_ = NULL;

std::vector< PyModelPtr > DeviceApp::updateModels_;

/**
 *	This function returns the estimated server time.
 */
double getServerTime()
{
	BW_GUARD;
	ServerConnection * pSC = EntityManager::instance().pServer();

	if (pSC)
	{
		return pSC->serverTime( App::instance().getTime() );
	}

	return -1.0;
}

/*~ function BigWorld.flashBangAnimation
 *	@components{ client }
 *
 *	This function adds a vector4 provider for the flash bang animation.
 *	The output of the flashbanganimation is modulated with the previous
 *	frame of the game to create a saturated frame buffer. There can be
 *	any number of flash bang animations running at the same time.
 *
 *	@param p the Vector4Provider to add
 */
void flashBangAnimation( Vector4ProviderPtr p ) { CanvasApp::instance.flashBangAnimations_.push_back( p ); };
PY_AUTO_MODULE_FUNCTION( RETVOID, flashBangAnimation, ARG( Vector4ProviderPtr, END ), BigWorld )

/*~ function BigWorld.removeFlashBangAnimation
 *	@components{ client }
 *
 *	This function removes a Vector4Provider from the list of flash bang
 *	animations
 *
 *	@param p the Vector4Provider to remove
 */
void removeFlashBangAnimation( Vector4ProviderPtr p )
{	std::vector< Vector4ProviderPtr >& fba = CanvasApp::instance.flashBangAnimations_;
	std::vector< Vector4ProviderPtr >::iterator it = std::find( fba.begin(), fba.end(), p );
	if (it != fba.end())
		fba.erase( it );
};
PY_AUTO_MODULE_FUNCTION( RETVOID, removeFlashBangAnimation, ARG( Vector4ProviderPtr, END ), BigWorld )

/*~ function BigWorld.addAlwaysUpdateModel
 *	@components{ client }
 *
 *	This function adds a PyModel to a list of models that have their nodes updated
 *	even when they are not visible.
 *	@param pModel the model to always update
 */
void addAlwaysUpdateModel( PyModelPtr pModel )
{
	BW_GUARD;
	DeviceApp::updateModels_.push_back( pModel );
}
PY_AUTO_MODULE_FUNCTION( RETVOID, addAlwaysUpdateModel, ARG( PyModelPtr, END ), BigWorld )

/*~ function BigWorld.delAlwaysUpdateModel
 *	@components{ client }
 *
 *	This function removes a model from the update list.
 *	@param pModel the model to remove from the update list
 */
void delAlwaysUpdateModel( PyModelPtr pModel )
{
	BW_GUARD;
	std::vector< PyModelPtr >::iterator it = std::find(
		DeviceApp::updateModels_.begin(), DeviceApp::updateModels_.end(), pModel );
	if (it != DeviceApp::updateModels_.end())
	{
		DeviceApp::updateModels_.erase( it );
	}
}
PY_AUTO_MODULE_FUNCTION( RETVOID, delAlwaysUpdateModel, ARG( PyModelPtr, END ), BigWorld )




int menuChoice( const std::string & prompt, std::vector<std::string> & items )
{
	// TODO: Bring up a dialog box and ask!
	return 0;
}






/*~	function BigWorld.worldDrawEnabled
 *	Sets and gets the value of a flag used to control if the world is drawn.
 *	Note: the value of this flag will also be used to turn the watching of
 *	files being loaded in the main thread on or off. That is, if enabled,
 *	warnings will be issued into the debug output whenever a file is
 *	accessed from the main thread.
 *	@param	newValue	(optional) True enables the drawing of the world, False disables.
 *	@return Bool		If the no parameters are passed the current value of the flag is returned.
 */
static PyObject * py_worldDrawEnabled( PyObject * args )
{
	BW_GUARD;
	if ( PyTuple_Size( args ) == 1 )
	{
		int newDrawEnabled;

		if (!PyArg_ParseTuple( args, "i:BigWorld.worldDrawEnabled",
				&newDrawEnabled ))
		{
			return NULL;
		}
		::gWorldDrawEnabled = (newDrawEnabled != 0);

		for( uint i=0; i<sizeof(gWorldDrawLoopTasks)/sizeof(const char*); i++ )
		{
			MainLoopTask * task = MainLoopTasks::root().getMainLoopTask( gWorldDrawLoopTasks[i] );
			if( task )
				task->enableDraw = ::gWorldDrawEnabled;
		}

		// grab/start a diary entry if turning off world draw,
		// and stop the entry when turning world draw back on.
		if (!gWorldDrawEnabled)		
		{
			Diary::instance().add( "Global Disable World Draw" )->stop();
		}
		else
		{
			Diary::instance().add( "Global Enable World Draw" )->stop();
		}

		// when turning world draw enabled off, turn fs
		// access watching off straight away to prevent
		// warning of files being accessed in this frame.
		if (!gWorldDrawEnabled)
		{
			BWResource::watchAccessFromCallingThread(false);
		}

		Py_Return;
	}
	else if ( PyTuple_Size( args ) == 0 )
	{
		PyObject * pyId = PyBool_FromLong(static_cast<long>(::gWorldDrawEnabled));
		return pyId;
	}
	else
	{
		PyErr_SetString(
			PyExc_TypeError,
			"BigWorld.worldDrawEnabled expects one boolean or no arguments." );
		return NULL;
	}
}
PY_MODULE_FUNCTION( worldDrawEnabled, BigWorld )

class PythonServer;

#if ENABLE_WATCHERS
class SumWV : public WatcherVisitor
{
public:

	virtual bool visit( Watcher::Mode type,
		const std::string & label,
		const std::string & desc,
		const std::string & valueStr )
	{
		BW_GUARD;
		if (label.substr( label.size()-4 ) == "Size")
		{
			//dprintf( "\t%s is %s\n", label.c_str(), valueStr.c_str() );
			sum_ += atoi( valueStr.c_str() );
		}
		return true;
	}
	uint32 sum_;
};
static SumWV s_sumWV;
static bool s_memoryAccountedFor_running = false;
uint32 memoryAccountedFor()
{
	BW_GUARD;
	if (s_memoryAccountedFor_running) return 0;
	s_memoryAccountedFor_running = true;
	s_sumWV.sum_ = 0;
#if ENABLE_WATCHERS
	Watcher::rootWatcher().visitChildren( NULL, "Memory/", s_sumWV );
#endif
	s_memoryAccountedFor_running = false;
	return s_sumWV.sum_ / 1024;
}
int32 memoryUnclaimed()
{
	BW_GUARD;
	// we are zero if memoryAccountedFor is running, even if
	// we didn't run it (since we are not accounted for!)
	if (s_memoryAccountedFor_running) return 0;
	memoryAccountedFor();
	return (memUsed() * 1024) - s_sumWV.sum_;
}
#endif

extern void registerAccountBudget( const std::string & account, uint32 budget );
extern void registerAccountContributor( const std::string & account,
	const std::string & name, uint32 & c );
extern void registerAccountContributor( const std::string & account,
	const std::string & name, uint32 (*c)() );


/**
 *	Shows a critical initialisation error message box.
 *	Except in release builds, explanations about what can
 *	be wrong and where to look for more information on running
 *	the client will be displayed in the message box.
 */
void criticalInitError( const char * format, ... )
{
	BW_GUARD;
	// build basic error message
	char buffer1[ BUFSIZ * 2 ];

	va_list argPtr;
	va_start( argPtr, format );
	bw_vsnprintf( buffer1, sizeof(buffer1), format, argPtr );
	buffer1[sizeof(buffer1)-1] = '\0';
	va_end( argPtr );

	// add additional explanations
	const char pathMsg[] =
#ifndef _RELEASE
		"%s\n\nThe most probable causes for this error are running "
		"the game executable from the wrong working directory or "
		"having a wrong BW_RES_PATH environment variable. For more "
		"information on how to correctly setup and run BigWorld "
		"client, please refer to the Client Installation Guide, "
		"in bigworld/doc directory.\n";
#else
		"%s";
#endif

	char buffer2[ BUFSIZ * 2 ];
	bw_snprintf( buffer2, sizeof(buffer2), pathMsg, buffer1 );

	CRITICAL_MSG( buffer2 );
}

// memory currently used in KB
uint32 memUsed()
{
	BW_GUARD;
	VersionInfo	* pVI = DebugApp::instance.pVersionInfo_;
	if (pVI == NULL) return 0;
	return pVI->workingSetRefetched();
}


/*~	function BigWorld.totalPhysicalMemory
 *	@components{ client }
 *
 *	Return the amount of physical RAM in the machine.  This can
 *	help when a game automatically chooses graphics settings for
 *	a user.  There are a few graphics settings that affect the
 *	amount of system RAM used.  Two examples are:
 *	- texture detail (managed textures are used, and they are
 *	mirrored in RAM)
 *	- the far plane distance directly affects how many chunks are
 *	loaded and thus how many unique assets will be loaded at any time.
 *
 *	@return Integer	The amount of physical RAM, in bytes.
 */
uint32 totalPhysicalMemory()
{
	BW_GUARD;
	VersionInfo	* pVI = DebugApp::instance.pVersionInfo_;
	if (pVI == NULL) return 0;
	return pVI->totalPhysicalMemory();
}
PY_AUTO_MODULE_FUNCTION( RETDATA, totalPhysicalMemory, END, BigWorld );


/*~	function BigWorld.totalVirtualMemory
 *	@components{ client }
 *
 *	Return the amount of virtual memory, a.k.a the amount of address
 *	space available to the process.  Used in conjunction with the
 *	totalPhysicalMemory function, you should be able to determine whether
 *	or not the user running a 64 bit client and therefore whether or not
 *	your game has access to the full amount of physical RAM installed.
 *
 *	This can help a game automatically choose graphics settings for
 *	a user.  There are a few graphics settings that affect the
 *	amount of system RAM used.  Two examples are:
 *	- texture detail (managed textures are used, and they are
 *	mirrored in RAM)
 *	- the far plane distance directly affects how many chunks are
 *	loaded and thus how many unique assets will be loaded at any time.
 *
 *	@return Integer	The amount of virtual RAM, in bytes.
 */
uint32 totalVirtualMemory()
{
	BW_GUARD;
	VersionInfo	* pVI = DebugApp::instance.pVersionInfo_;
	if (pVI == NULL) return 0;
	return pVI->totalVirtualMemory();
}
PY_AUTO_MODULE_FUNCTION( RETDATA, totalVirtualMemory, END, BigWorld );


/**
 *  Device callback object to provide Personality.onRecreateDevice() hook.
 */
class RecreateDeviceCallback : public Moo::DeviceCallback
{
public:
	static void createInstance() 
	{ 
		if (s_instance_== NULL)
			s_instance_ = new RecreateDeviceCallback();
	}
	static void deleteInstance()
	{
		delete s_instance_;
		s_instance_ = NULL;
	}


	/*~ callback Personality.onRecreateDevice
	 *
	 *	This callback method is called on the personality script when
	 *	the Direct3D device has asked the engine to recreate all
	 *	resources not managed by Direct3D itself.
	 */
	void createUnmanagedObjects()
	{
		BW_GUARD;
		PyObject *pPersonality = Personality::instance();

		if (pPersonality)
		{
			PyObject * pfn = PyObject_GetAttrString( pPersonality,
				"onRecreateDevice" );

			if (pfn)
			{
				Script::callNextFrame(
					pfn, PyTuple_New(0),
					"RecreateDeviceCallback::createUnmanagedObjects: " );
			}
			else
			{
				PyErr_Clear();
			}
		}
	}

	static RecreateDeviceCallback* s_instance_;
};

RecreateDeviceCallback* RecreateDeviceCallback::s_instance_ = NULL;


#if ENABLE_WATCHERS
/**
 *	WatcherVisitor that prints memory counters
 */
static class MemoryWV : public WatcherVisitor
{
	virtual bool visit( Watcher::Mode type,
		const std::string & label,
		const std::string & desc,
		const std::string & valueStr )
	{
		BW_GUARD;
		dprintf( "%s\t%s\n", label.c_str(), valueStr.c_str() );
		return true;
	}
} s_mwv;

/*~ function BigWorld.dumpMemCounters
 *	@components{ client }
 *
 *	This debugging function prints out the current value of memory watchers
 *	found in "Memory/" watcher directory.
 */
void dumpMemCounters()
{
	BW_GUARD;
	Watcher::rootWatcher().visitChildren( NULL, "Memory/", s_mwv );
}
PY_AUTO_MODULE_FUNCTION( RETVOID, dumpMemCounters, END, BigWorld );
#endif


#ifdef USE_MEMORY_TRACER
/**
 *	This class traverses the memory trace tree, and writes to the output window
 */
class MemoryTraceDebugStringWalker : public ResourceMemoryTrace::TreeWalker
{
public:
	MemoryTraceDebugStringWalker( uint32 maxDepth, uint32 minKB ):
	  maxDepth_( maxDepth ),
	  minKB_( minKB )
	{
	}

	bool atLeaf( const std::string& id, uint32 memUsed, uint32 depth )
	{
		BW_GUARD;
		if ( minKB_ > memUsed )
			return false;

		for ( uint32 i=0; i<depth; i++ )
		{
			buf_[i] = '.';
		}
		buf_[depth]=(char)0;
		DEBUG_MSG( "%s%s %d\n", buf_, id.c_str(), memUsed );
		return ( maxDepth_ > depth );
	}

	char buf_[256];
	uint32 maxDepth_;
	uint32 minKB_;
};

void outputMemoryTrace( uint32 maxDepth = 1, uint32 minKB = 50 )
{
	MemoryTraceDebugStringWalker w( maxDepth, minKB );
	ResourceMemoryTrace::instance().traverse( w );
}
PY_AUTO_MODULE_FUNCTION(
	RETVOID, outputMemoryTrace, OPTARG( uint32, 1, OPTARG( uint32, 50, END ) ), BigWorld )

#endif


/**
 *	This method initialises the application.
 *
 *	@param hInstance	The HINSTANCE associated with this application.
 *	@param hWnd			The main window associated with this application.
 *
 *	@return		True if the initialisation succeeded, false otherwise.
 */
bool App::init( HINSTANCE hInstance, HWND hWnd )
{
	BW_GUARD;
	hWnd_ = hWnd;
	isEmbedded_ = !!GetParent( hWnd_ );

	// Pass some parameters
	DeviceApp::s_hInstance_ = hInstance;
	DeviceApp::s_hWnd_ = hWnd;
	ConsoleManager::createInstance();

	// Compress animations on load so we'll only save the compressed versions
    Moo::InterpolatedAnimationChannel::inhibitCompression( false );

	// Set up the MainLoopTask groups and dependencies
	MainLoopTasks::root().add( NULL, "Device", NULL );
	MainLoopTasks::root().add( NULL, "VOIP",   ">Device", NULL );
	MainLoopTasks::root().add( NULL, "Web",   ">VOIP", NULL );
	MainLoopTasks::root().add( NULL, "Script", ">Web",   NULL );
	MainLoopTasks::root().add( NULL, "Camera", ">Script", NULL );
	MainLoopTasks::root().add( NULL, "Canvas", ">Camera", NULL );
	MainLoopTasks::root().add( NULL, "World",  ">Canvas", NULL );
	MainLoopTasks::root().add( NULL, "Flora",  ">World",  NULL );
	MainLoopTasks::root().add( NULL, "Facade", ">Flora",  NULL );
	MainLoopTasks::root().add( NULL, "Lens",   ">Facade", NULL );
	MainLoopTasks::root().add( NULL, "GUI",    ">Lens",   NULL );
	MainLoopTasks::root().add( NULL, "Debug",  ">GUI",    NULL );
	MainLoopTasks::root().add( NULL, "Finale", ">Debug",  NULL );

	// And initialise them all!
	bool ok = MainLoopTasks::root().init();

	RecreateDeviceCallback::createInstance();

	MF_WATCH( "Debug/debugKeyEnable",
		debugKeyEnable_,
		Watcher::WT_READ_WRITE,
		"Toggle use of the debug key" );

	MF_WATCH( "Debug/activeConsole", *this, MF_ACCESSORS( std::string, App, activeConsole ) );

	MF_WATCH( "Debug/Sleep time (ms)",
		sleepTime_,
		Watcher::WT_READ_WRITE,
		"Number of milliseconds to pause (yield) in-between frames." );

	// Only set a default cursor if the personality script didn't set one
	if ( this->activeCursor_ == NULL )
		this->activeCursor( &DirectionCursor::instance() );

	if ( ok )
	{
		// Unload the loading screen
		freeLoadingScreen();

		// Make sure we setup the effect visual context constants here to make
		// sure that a space with only terrain will still render correctly
		Moo::EffectVisualContext::instance().initConstants();
	}

#ifdef USE_MEMORY_TRACER
	outputMemoryTrace();
#endif

	// resetting lastTime so that the
	// first frame's dTime doesn't account
	// for the initialisation time
	lastTime_ = frameTimerValue();

	return ok;
}


/**
 *	This method finialises the application.
 *
 */
void App::fini()
{
	BW_GUARD;
	if (pInstance_ != NULL)
	{
		Moo::rc().releaseUnmanaged();

		RecreateDeviceCallback::deleteInstance();		

		MainLoopTasks::finiAll();

		ConsoleManager::deleteInstance();
		pInstance_ = NULL;
	}

	DogWatchManager::fini();

	s_scriptsPreferences = NULL;
}


/**
 *	This class stores loading screen information.
 *	This class is only used by the method App::loadingScreen()
 *
 *	@see App::loadingScreen
 */
class LoadingScreenInfo
{
public:
	LoadingScreenInfo( std::string & name, bool fullScreen ) :
		name_( name ),
		fullScreen_( fullScreen )
	{
	}

	typedef std::vector< LoadingScreenInfo >	Vector;

	std::string & name( void )	{ return name_; }
	bool fullScreen( void )		{ return fullScreen_; }
private:
	std::string name_;
	bool		fullScreen_;
};



/**
 *	This method displays the loading screen.
 *	Assumes beginscene has already been called.
 */
static std::string lastUniverse = "/";
static Moo::Material loadingMat;

bool displayLoadingScreen()
{
	BW_GUARD;
	if (!BWProcessOutstandingMessages())
		return false;

	if (DeviceApp::s_pGUIProgress_)
	{
		return true;
	}

	static bool fullScreen = true;
	static CustomMesh<Moo::VertexTLUV> mesh( D3DPT_TRIANGLESTRIP );
	static bool s_inited = false;

	//HACK_MSG( "displayLoadingScreen: heap size is %d\n", heapSize() );

	if (s_inited)
	{
		if ( loadingScreenName.value() != "" )
		{
			loadingMat.set();
			mesh.draw();
		}

		// and we draw the status console here too...
		XConsole * pStatCon = ConsoleManager::instance().find( "Status" );
		if (pStatCon != NULL) pStatCon->draw( 1.f );

		return true;
	}

	Moo::BaseTexturePtr	loadingBack = Moo::TextureManager::instance()->get( loadingScreenName );

	loadingMat = Moo::Material();
	Moo::TextureStage ts;
	ts.pTexture( loadingBack );
	ts.useMipMapping( false );
	ts.colourOperation( Moo::TextureStage::SELECTARG1 );
	loadingMat.addTextureStage( ts );
	Moo::TextureStage ts2;
	ts2.colourOperation( Moo::TextureStage::DISABLE );
	ts2.alphaOperation( Moo::TextureStage::DISABLE );
	loadingMat.addTextureStage( ts2 );
	loadingMat.fogged( false );

	Moo::VertexTLUV vert;
	mesh.clear();

	vert.colour_ = 0xffffffff;
	vert.pos_.z = 0;
	vert.pos_.w = 1;

	vert.pos_.x = 0;
	vert.pos_.y = 0;
	vert.uv_.set( 0,0 );
	mesh.push_back( vert );

	vert.pos_.x = Moo::rc().screenWidth();
	vert.pos_.y = 0;
	vert.uv_.set( 1,0 );
	mesh.push_back( vert );

	vert.pos_.x = 0;
	vert.pos_.y = Moo::rc().screenHeight();
	vert.uv_.set( 0,1 );
	mesh.push_back( vert );

	vert.pos_.x = Moo::rc().screenWidth();
	vert.pos_.y = Moo::rc().screenHeight();
	vert.uv_.set( 1,1 );
	mesh.push_back( vert );

	//fix texel alignment
	for ( int i = 0; i < 4; i++ )
	{
		mesh[i].pos_.x -= 0.5f;
		mesh[i].pos_.y -= 0.5f;
	}

	s_inited = true;

	// call ourselves to draw now that we're set up
	return displayLoadingScreen();
}


/**
 *	This method ensures the resources we used just for loading are freed up.
 */
static void freeLoadingScreen()
{
	lastUniverse = "/";
	loadingMat = Moo::Material();
}

/**
 *	Draw this loading text message. They appear beneath the progress bars
 */
void loadingText( const std::string& s )
{
	ConsoleManager::instance().find( "Status" )->print( s + "\n" );

	if (DeviceApp::s_pProgress_ != NULL)
	{
		DeviceApp::s_pProgress_->draw( true );
	}

	INFO_MSG( "%s\n", s.c_str() );
}

// -----------------------------------------------------------------------------
// Section: Update and Draw functions
// -----------------------------------------------------------------------------



extern DogWatch	g_watchSound;	// declared in soundmgr.cpp

/**
 *	This method is called once per frame to do all the application processing
 *	for that frame.
 */
bool App::updateFrame(bool active)
{
	BW_GUARD;
	Profiler::instance().tick();

	DiaryScribe deAll(Diary::instance(), "Frame" );

	// Timing
	this->calculateFrameTime();

	// Mouse clipping
	MouseCursor::updateMouseClipping();

	// Only tick and draw if some time has passed, this fixes an issue with 
	// minimising on intel cpus.
	if (dTime_ > 0.f)
	{
		if (active)
		{
			// Now tick (and input)
			{
				PROFILER_SCOPED( App_Tick );
				g_watchTick.start();
				DiaryEntryPtr deTick = Diary::instance().add( "Tick" );
				MainLoopTasks::root().tick( dTime_ );
				deTick->stop();
				g_watchTick.stop();
			}

			// And draw
			if (Moo::rc().checkDevice())
			{
				PROFILER_SCOPED( App_Draw );
				g_watchOutput.start();
				DiaryEntryPtr deDraw = Diary::instance().add( "Draw" );
				MainLoopTasks::root().draw();
				deDraw->stop();
				g_watchOutput.stop();
			}
		}
		else
		{
			MainLoopTasks::root().inactiveTick( dTime_ );
		}

		int sleepTime = 0;

		uint64 frameEndTime = frameTimerValue();
		if (minFrameTime_ > 0 && frameEndTime < lastFrameEndTime_ + minFrameTime_)
		{
			sleepTime = uint32((lastFrameEndTime_ + minFrameTime_ - frameEndTime) * 1000 / frameTimerFreq());
		}

		sleepTime = max( sleepTime, sleepTime_ );

		if ( sleepTime )
		{
			PROFILER_SCOPED( Sys_Sleep );
			::Sleep( sleepTime );
		}

		lastFrameEndTime_ = frameTimerValue();

		if (s_framesCounter <= 0)
		{
			return true;
		}
		else	
		{
			--s_framesCounter;
			if (s_framesCounter % 100 == 0)
			{
				DEBUG_MSG("s_framesCounter: %d\n", s_framesCounter);
			}
			return  s_framesCounter > 0;
		}
	}
	return true;
}





/**
 *	This method is called once a frame by updateFrame to update the scene.
 *
 *	@param dTime	The amount of time that has elapsed since the last update.
 */
void App::updateScene( float dTime )
{
	BW_GUARD;	
}

/**
 *	This method updates the pose of any cameras in the scene.
 */
void App::updateCameras( float dTime )
{
	BW_GUARD;	
}

/**
 *	This method renders a frame using the current camera and current scene.
 */
void App::renderFrame()
{
	BW_GUARD;	
}

/**
 *	This method draws the 3D world, i.e. anything that uses the Z buffer
 */
void App::drawWorld()
{
	BW_GUARD;	
}


/**
 *	This method draws what is considered to be the scene, i.e. everything
 *	that is placed at a definite transform.
 */
void App::drawScene()
{
	BW_GUARD;	
}

/**
 *	This method is called to start quitting the application.
 *	You may optionally restart the application.
 */
void App::quit( bool restart )
{
	BW_GUARD;
	if (restart)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );
		::CreateProcess( NULL, ::GetCommandLine(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi );
	}

	INFO_MSG("Quitting app");
	PostMessage( hWnd_, WM_CLOSE, 0, 0 );
}


/**
 *	This method overrides the InputHandler method to handle keyboard events. 
 *
 *	@param event	The key event.
 *
 *	@return			True if the event was handled, false otherwise.
 */
bool App::handleKeyEvent(const KeyEvent & event)
{
	BW_GUARD;
	HandleKeyEventHolder holder(*this);

	// Create debug key event if the debug key is enabled.
	if ( debugKeyEnable_ && handleKeyEventDepth_ == 0 && 
		 event.key() != KeyCode::KEY_DEBUG && 
		 !event.isRepeatedEvent() && 
		 checkDebugKeysState( event.key() ) )
	{
		// checkDebugKeysState will pass if the OTHER keys in the debug key combination
		// are also pressed. It doesn't check the state of event.key, so we can get keyup
		// events from the debug key.
		KeyEvent thisEvent = KeyEvent::make( KeyCode::KEY_DEBUG, event.isKeyDown(), event.modifiers(), event.cursorPosition() );
		InputDevices::keyStates().setKeyStateNoRepeat( KeyCode::KEY_DEBUG, event.isKeyDown() );
		this->handleKeyEvent( thisEvent );
	}

	bool handled = false;

	// check if this is a key up event
	eEventDestination keySunk = EVENT_SINK_NONE;
	if (event.isKeyUp())
	{
		// post off a char event on keyup if there is one associated with it.
		if ( keyUpChar_.key() != KeyCode::KEY_NONE )
		{
			handleKeyEvent( KeyEvent::make(	keyUpChar_.key(), 
											InputDevices::keyRepeatCount(keyUpChar_.key())+1, 
											keyUpChar_.utf16Char(),
											InputDevices::modifiers(),
											keyUpChar_.cursorPosition() ) );
			keyUpChar_.reset();
		}

		// disallow it when there has been no recorded key down.
		// this can happen when events are lost
		if (keyRouting_[event.key()] == EVENT_SINK_NONE) return true;

		// otherwise cache where it when to
		keySunk = keyRouting_[event.key()];

		// and clear it in advance
		keyRouting_[event.key()] = EVENT_SINK_NONE;
	}

	// If the key associated with the character is a debug key, then only
	// send the auto-repeats. We will send the first char event event if
	// no repeats occured on the key-up.
	if ( debugKeyEnable_ && 
		 event.key() != KeyCode::KEY_DEBUG &&
		 wcscmp( event.utf16Char(), L"" ) != 0 &&
		 event.isKeyDown() && isDebugKey( event.key() ) )
	{
		if( event.repeatCount() == 0 && keyUpChar_.key() == KeyCode::KEY_NONE )
		{
			keyUpChar_.fill( event );
			handled = true;
		}
		else
		{
			keyUpChar_.reset();
		}
	}

	// disable IME while the debug key is held down.
	if (IME::pInstance() && event.key() == KeyCode::KEY_DEBUG)
	{
		IME::instance().allowEnable( event.isKeyUp() && 
			ConsoleManager::instance().pActiveConsole() == NULL );
	}

	// only consider debug keys if it is down
	if (!handled && event.isKeyDown() &&
		InputDevices::isKeyDown( KeyCode::KEY_DEBUG ))
	{
		handled = this->handleDebugKeyDown( event );
		if (handled)
		{
			keyRouting_[event.key()] = EVENT_SINK_DEBUG;
			keyUpChar_.reset();
		}
	}

	// consume keyups corresponding to EVENT_SINK_DEBUG keydowns
	if (keySunk == EVENT_SINK_DEBUG) handled = true;


	// give the active console and console manager a go
	if (!handled)
	{
		handled = ConsoleManager::instance().handleKeyEvent( event );
		if (handled && event.isKeyDown())
		{
			keyRouting_[event.key()] = EVENT_SINK_CONSOLE;
		}
	}

	// consume keyups corresponding to EVENT_SINK_CONSOLE keydowns
	if (keySunk == EVENT_SINK_CONSOLE) handled = true;

	// post off to the script global hooks
	if ( !handled )
	{
		InputEvent ievent;
		ievent.type_ = InputEvent::KEY;
		ievent.key_ = event;

		handled = BigWorldClientScript::sinkKeyboardEvent(ievent);
	}

#if SCALEFORM_SUPPORT
	if (!handled)
	{
		handled = Scaleform::Manager::instance().onKeyEvent( event );
	}
#endif

	// now give the personality script its go
	if (!handled )
	{
		PyObject * ret = Script::ask(
			PyObject_GetAttrString( Personality::instance(), "handleKeyEvent" ),
			Py_BuildValue("(N)", Script::getData(event) ),
			"Personality handleKeyEvent: " );
		Script::setAnswer( ret, handled, "Personality handleKeyEvent retval" );

		if (handled && event.isKeyDown())
		{
			keyRouting_[event.key()] = EVENT_SINK_PERSONALITY;
		}
	}

	// consume keyups corresponding to EVENT_SINK_PERSONALITY keydowns
	if (keySunk == EVENT_SINK_PERSONALITY) handled = true;


	// give the app its chance (it only wants keydowns)
	if (!handled && event.isKeyDown())
	{
		handled = this->handleKeyDown( event );
		if (handled && event.isKeyDown())
		{
			keyRouting_[event.key()] = EVENT_SINK_APP;
		}
	}

	// consume keyups corresponding to EVENT_SINK_APP keydowns
	if (keySunk == EVENT_SINK_APP) handled = true;


	// finally let the script have the crumbs
	//  (has to be last since we don't extract 'handled' from it)
	if (!handled)
	{
		if ( Player::entity() != NULL)
		{
			Script::call(
				PyObject_GetAttrString( Player::entity(), "handleKeyEvent" ),
				Py_BuildValue("(N)", Script::getData(event) ),
				"Player handleKeyEvent: " );
		}

		// TODO: Look at return value and set 'handled'

		// For now, sink all key presses in scripts
		if (event.isKeyDown())
		{
			keyRouting_[event.key()] = EVENT_SINK_SCRIPT;
			handled = true;
		}
		else if (event.isKeyUp())
		{
			keyRouting_[event.key()] = EVENT_SINK_NONE;
			handled = true;
		}
	}


	// and for sanity make sure the key routing entry is cleared
	//  if we got a key up (it should already have been)
	if (keySunk != EVENT_SINK_NONE && !handled)
	{
		WARNING_MSG( "KeyUp for 0x%02x routed to %d was unclaimed!\n",
			int(event.key()), int(keySunk) );
	}

	return handled;
}

/**
 *	This method overrides the InputHandler method to handle the even raised
 *	when the user changes the current input language.
 *
 *	@return			True if the event was handled, false otherwise.
 */
bool App::handleInputLangChangeEvent()
{
	bool handled = false;

	// Post it directly to the personality script if the function is defined.
	PyObject* fn = PyObject_GetAttrString( Personality::instance(), "handleInputLangChangeEvent" );
	if ( fn != NULL )
	{
		PyObject * ret = Script::ask(
			fn, Py_BuildValue("()" ),
			"Personality handleInputLangChangeEvent: " );
		Script::setAnswer( ret, handled, "Personality handleInputLangChangeEvent retval" );
	}

	return handled;
}

/**
 *	This method overrides the InputHandler method to handle IME related events. 
 *	IME events occur when the active Input Method Editor has changed state.
 *
 *	@param event	The current IME event.
 *
 *	@return			True if the event was handled, false otherwise.
 */
bool App::handleIMEEvent( const IMEEvent & event )
{
	bool handled = false;

	// Post it directly to the personality script if the function is defined.
	PyObject* fn = PyObject_GetAttrString( Personality::instance(), "handleIMEEvent" );
	if ( fn != NULL )
	{
		PyObject * ret = Script::ask(
			fn, Py_BuildValue("(N)", Script::getData(event) ),
			"Personality handleIMEEvent: " );
		Script::setAnswer( ret, handled, "Personality handleIMEEvent retval" );
	}

	return handled;
}

void App::clientChatMsg( const std::string & msg )
{
	BW_GUARD;
	if (!Py_IsInitialized())
	{
		return;
	}

	PyObject * args = PyTuple_New( 2 );
	PyTuple_SetItem( args, 0, PyInt_FromLong( -1 ) );
	PyTuple_SetItem( args, 1, PyString_FromString( msg.c_str() ) );
	PyObject * function = PyObject_GetAttrString( Personality::instance(), "addChatMsg" );
	if(function == NULL)
	{
		DEBUG_MSG("Personality script does not have 'addChatMsg' method to display output message.\n");
		return ;
	}
	Script::call( function, args );
}


/**
 *	This method triggers a personality script callback for the memory critical
 *	method.
 */
void App::memoryCriticalCallback()
{
	BW_GUARD;
	PyObject *pPersonality = Personality::instance();
	if (pPersonality)
	{
		PyObject * pfn = PyObject_GetAttrString( pPersonality,
			"onMemoryCritical" );

		if (pfn)
		{
			Script::call( pfn, PyTuple_New(0) );
		}
		else
		{
			App::instance().clientChatMsg( "WARNING: Memory load critical, adjust your detail settings.\n" );
			PyErr_Clear();
		}
	}
}

/**
 *	This function handles key and button down events.
 *	It is called by App::handleKeyEvent above.
 */
bool App::handleKeyDown( const KeyEvent & event )
{
	BW_GUARD;
	bool handled = true;

	if ( event.isRepeatedEvent() )
	{
		return false;
	}

	switch (event.key())
	{

	case KeyCode::KEY_F4:
		if (event.isAltDown() && !isEmbedded_)
		{
			this->quit();
		}
		else
		{
			handled = false;
		}
		break;

	case KeyCode::KEY_SYSRQ:
		{

// The super-shot functionality will only work it watchers are enabled
#if ENABLE_WATCHERS

			if (event.isCtrlDown()) //Toggle super screenshot settings in engine_config.xml/superShot.
			{
				static bool s_superShotEnabled = false;

				static std::string s_backBufferWidthXML = AppConfig::instance().pRoot()->readString("superShot/hRes", "2048");
				static std::string s_farPlaneDistXML = AppConfig::instance().pRoot()->readString("superShot/farPlaneDist", "1500");
				static uint32 s_floraVBSizeXML = AppConfig::instance().pRoot()->readInt("superShot/floraVBSize", 16000000);

				static std::string s_backBufferWidth;
				static std::string s_farPlaneDist;
				static uint32 s_floraVBSize;
				static bool isWindowed = Moo::rc().windowed();
				s_superShotEnabled = !s_superShotEnabled;

				if (s_superShotEnabled) //Store current settings and apply new settings.
				{
					//Store current settings.
					Watcher::Mode wMode;
					Watcher::rootWatcher().getAsString(NULL, "Render/backBufferWidthOverride", s_backBufferWidth, wMode);
					Watcher::rootWatcher().getAsString(NULL, "Render/Far Plane", s_farPlaneDist, wMode);
					s_floraVBSize = ChunkManager::instance().cameraSpace()->enviro().flora()->vbSize();
					isWindowed = Moo::rc().windowed();

					if(!isWindowed) //set to windowed mode
					{
						Moo::rc().changeMode( Moo::rc().modeIndex(), true );
					}

					//Apply settings from resources.xml.
					Watcher::rootWatcher().setFromString(NULL, "Render/backBufferWidthOverride", s_backBufferWidthXML.c_str());
					Watcher::rootWatcher().setFromString(NULL, "Render/Far Plane", s_farPlaneDistXML.c_str());
					ChunkManager::instance().cameraSpace()->enviro().flora()->vbSize(s_floraVBSizeXML);
					if(AppConfig::instance().pRoot()->readBool("superShot/disableGUI"))
					{
						SimpleGUI::instance().setUpdateEnabled(false);
					}
				}
				else
				{
					//Restore previous settings.
					Watcher::rootWatcher().setFromString(NULL, "Render/backBufferWidthOverride", s_backBufferWidth.c_str());
					Watcher::rootWatcher().setFromString(NULL, "Render/Far Plane", s_farPlaneDist.c_str());
					ChunkManager::instance().cameraSpace()->enviro().flora()->vbSize(s_floraVBSize);
					if(AppConfig::instance().pRoot()->readBool("superShot/disableGUI"))
					{
						SimpleGUI::instance().setUpdateEnabled(true);
					}
					if(!isWindowed)
					{
						Moo::rc().changeMode( Moo::rc().modeIndex(), false );
					}
				}

				//Notify user know of toggle.
				if (s_superShotEnabled)
					clientChatMsg( "High Quality Screenshot Settings Enabled" );
				else
					clientChatMsg( "High Quality Screenshot Settings Disabled" );
			}
			else

#endif //ENABLE_WATCHERS

			{
				DataSectionPtr rootDS = AppConfig::instance().pRoot();

				PathedFilename pathedFile( 
								rootDS->openSection( "screenShot/path" ),
								"", PathedFilename::BASE_EXE_PATH );

				std::string fullName = 
					pathedFile.resolveName() + "/" + 
					rootDS->readString( "screenShot/name", "shot" );

				BWResource::ensureAbsolutePathExists( fullName );

				std::string fileName = Moo::rc().screenShot( 
					rootDS->readString("screenShot/extension", "bmp"),
					fullName );
				if( !fileName.empty() )
				{
					clientChatMsg( "Screenshot saved: " + BWResource::getFilename( fileName ) );
				}
			}
		}
		break;

	case KeyCode::KEY_RETURN:
		if (InputDevices::isAltDown( ))
		{
			if( Moo::rc().device() && !isEmbedded_ )
				Moo::rc().changeMode( Moo::rc().modeIndex(), !Moo::rc().windowed() );
		}
		else
		{
			handled = false;
		}
		break;

	case KeyCode::KEY_TAB:
		if (!event.isAltDown())
		{
			handled = false;
		}
		break;

	default:
		handled = false;
	}

	return handled;
}


/**
 *	Handle a debugging key. Only gets called if debug key is pressed.
 */
bool App::handleDebugKeyDown( const KeyEvent & event )
{
	BW_GUARD;	
#if !ENABLE_DEBUG_KEY_HANDLER

	return false;

#else // ENABLE_DEBUG_KEY_HANDLER

	if ( event.isRepeatedEvent() )
	{
		return false;
	}


	bool handled = true;

	switch (event.key())
	{

	case KeyCode::KEY_F2:
	{
		RECT clientRect;
		GetClientRect( hWnd_, &clientRect );

		const int numSizes = 4;
		POINT sizes[numSizes] = {
			{ 512, 384 }, { 640, 480 },
			{ 800, 600 }, { 1024, 768 } };

		int i = 0;
		int width = this->windowSize().x;
		for (; i < numSizes; ++i)
		{
			if (sizes[i].x == width)
			{
				break;
			}
		}
		i = (i + (event.isShiftDown() ? numSizes-1 : 1)) % numSizes;
		this->resizeWindow(sizes[i].x, sizes[i].y);
		std::stringstream s;
		s << "Resolution: " << sizes[i].x << " x " << sizes[i].y << std::endl; 
		clientChatMsg(s.str()); 
		break;
	}

	case KeyCode::KEY_F4:
		if( !( event.isCtrlDown() || event.isAltDown() ) )
		{
			ConsoleManager::instance().toggle( "Histogram" );
		}
		break;

	case KeyCode::KEY_F6:
	{
		int modsum =
				( event.isCtrlDown()  ? EnviroMinder::DrawSelection::skyGradient : 0 ) |
				( event.isAltDown()   ? EnviroMinder::DrawSelection::skyBoxes      : 0 ) |
				( event.isShiftDown() ?
					EnviroMinder::DrawSelection::sunAndMoon +
					EnviroMinder::DrawSelection::sunFlare : 0 );

		if (!modsum)
		{
			// toggle all sky drawing options
			CanvasApp::instance.drawSkyCtrl_ =
				CanvasApp::instance.drawSkyCtrl_ ? 0 : EnviroMinder::DrawSelection::all;
			clientChatMsg( "toggle: all sky drawing options" );
		}
		else
		{
			// toggle the option formed by adding the modifier's binary values
			CanvasApp::instance.drawSkyCtrl_ = CanvasApp::instance.drawSkyCtrl_ ^ modsum;

			std::string s = "toggle: ";
			s.append(event.isCtrlDown() ? "'Sky Gradient' " : "");
			s.append(event.isAltDown() ? "'Sky Boxes' " : "");
			s.append(event.isShiftDown() ? "'Sun & Moon Flare'" : "");
			clientChatMsg(s);
			// as in other areas (such as watcher menus),
			// shift is worth one, ctrl is worth 2, and alt is worth 4.
		}
		break;
	}


	case KeyCode::KEY_F8:
		{
			WorldApp::instance.wireFrameStatus_++;
			std::stringstream s;
			s << "wireframe status: " << WorldApp::instance.wireFrameStatus_ << std::endl;
			clientChatMsg(s.str());
		}
		break;


	case KeyCode::KEY_F9:
		{
			bool darkBG = ConsoleManager::instance().darkenBackground();
			ConsoleManager::instance().setDarkenBackground(!darkBG);
		}
		break;

	case KeyCode::KEY_F10:
		{
			Moo::Camera cam = Moo::rc().camera();
			cam.ortho( !cam.ortho() );
			Moo::rc().camera( cam );
		}
		break;

	case KeyCode::KEY_F11:
	{
		DEBUG_MSG( "App::handleKeyDown: Reloading entity script classes...\n" );
		XConsole * pConsole = ConsoleManager::instance().find( "Python" );

		if (EntityType::reload())
		{
			DEBUG_MSG( "App::handleKeyDown: reload successful!\n" );
#if 0
			::soundMgr().playFx( "loadGun" );
#endif
			clientChatMsg( "App: Script reload succeeded." );
		}
		else
		{
			DEBUG_MSG( "App::handleKeyDown: reload failed.\n" );
#if 0
			::soundMgr().playFx( "players/hurt" );
#endif
			clientChatMsg( "App: Script reload failed." );

			if (PyErr_Occurred())
			{
				PyErr_PrintEx(0);
				PyErr_Clear();
			}
		}
		break;
	}
#if UMBRA_ENABLE
	case KeyCode::KEY_O:
	{
		// Toggle umbra occlusion culling on/off using the watchers
		if (UmbraHelper::instance().occlusionCulling())
		{
			UmbraHelper::instance().occlusionCulling(false);
			clientChatMsg( "Umbra occlusion culling disabled" );
		}
		else
		{
			UmbraHelper::instance().occlusionCulling(true);
			clientChatMsg( "Umbra occlusion culling enabled" );
		}
		break;
	}
#endif

	case KeyCode::KEY_5:
	case KeyCode::KEY_H:
		{
			Filter::isActive( !Filter::isActive() );
			std::string s = "App: Filter is ";
			s.append(Filter::isActive() ? "on" : "off");
			clientChatMsg(s);
		}
		break;

	case KeyCode::KEY_I:
		{
			// Invert mouse verticals for DirectionCursor.
			CameraControl::isMouseInverted(
				!CameraControl::isMouseInverted() );
			std::string s = "App: Mouse vertical movement ";
			s.append(CameraControl::isMouseInverted() ? "Inverted" : "Normal");
			clientChatMsg(s);
		}
		break;

	case KeyCode::KEY_N:
		EntityManager::instance().displayIDs(
			!EntityManager::instance().displayIDs() );
		break;

	case KeyCode::KEY_P:
		if (!event.isCtrlDown())
		{
			// Toggle python console
			ConsoleManager::instance().toggle( "Python" );
		}
		break;

	case KeyCode::KEY_L:
		{
			// Toggle the static sky dome
			CanvasApp::instance.drawSkyCtrl_ =
				CanvasApp::instance.drawSkyCtrl_ ^
				EnviroMinder::DrawSelection::skyBoxes;
			clientChatMsg( "toggle: sky boxes" );
		}
		break;

	case KeyCode::KEY_LBRACKET:
	{
		if (ChunkManager::instance().cameraSpace().getObject() != NULL)
		{
			EnviroMinder & enviro =
				ChunkManager::instance().cameraSpace()->enviro();

			if (event.isShiftDown() )
			{
				// Move back an hour.
				enviro.timeOfDay()->gameTime(
					enviro.timeOfDay()->gameTime() - 1.0f );
				clientChatMsg( "Move backward one hour" );
			}
			else
			{
				// Move back by 10 minutes.
				enviro.timeOfDay()->gameTime(
					enviro.timeOfDay()->gameTime() - 10.0f/60.0f );
				clientChatMsg( "Move backward 10 minutes" );
			}
		}
		break;
	}


	case KeyCode::KEY_RBRACKET:
	{
		if (ChunkManager::instance().cameraSpace().getObject() != NULL)
		{
			EnviroMinder & enviro =
				ChunkManager::instance().cameraSpace()->enviro();

			if (event.isShiftDown() )
			{
				// Move forward an hour.
				enviro.timeOfDay()->gameTime(
					enviro.timeOfDay()->gameTime() + 1.0f );
				clientChatMsg( "Move forward one hour" );
			}
			else
			{
				// Move forward 10 minutes.
				enviro.timeOfDay()->gameTime(
					enviro.timeOfDay()->gameTime() + 10.0f/60.0f );
				clientChatMsg( "Move forward 10 minutes" );
			}
		}
		break;
	}

	case KeyCode::KEY_F5:
		if (!event.isCtrlDown())
		{
			ConsoleManager::instance().toggle( "Statistics" );
		}
		else
		{
			ConsoleManager::instance().toggle( "Special" );
		}
		break;
	case KeyCode::KEY_F7:
		if (!event.isCtrlDown())
		{
			ConsoleManager::instance().toggle( "Watcher" );
		}
		else
		{
			ParticleSystemManager::instance().active( !ParticleSystemManager::instance().active() );
		}
		break;

	case KeyCode::KEY_JOYB:
		ConsoleManager::instance().toggle( "Statistics" );
		break;
	case KeyCode::KEY_JOYX:
		ConsoleManager::instance().toggle( "Watcher" );
		break;
	case KeyCode::KEY_JOYY:
		PyOutputWriter::flush();
		ConsoleManager::instance().toggle( "Python" );
		break;
	case KeyCode::KEY_JOYLTRIGGER:
		if (InputDevices::isKeyDown( KeyCode::KEY_JOYRTRIGGER ))
		{
			DEBUG_MSG( "Reloading entity script classes...\n" );
			XConsole * pConsole = ConsoleManager::instance().find( "Python" );

			if (EntityType::reload())
			{
				DEBUG_MSG( "Script reload successful!\n" );
				clientChatMsg( "App: Script reload succeeded." );
			}
			else
			{
				DEBUG_MSG( "Script reload failed.\n" );
				clientChatMsg( "App: Script reload failed." );

				if (PyErr_Occurred())
				{
					PyErr_PrintEx(0);
					PyErr_Clear();
				}
			}
		}
		else
		{
			handled = false;
		}
		break;
	case KeyCode::KEY_JOYDUP:
	{
		EnviroMinder & enviro =
			ChunkManager::instance().cameraSpace()->enviro();
		enviro.timeOfDay()->gameTime(
			enviro.timeOfDay()->gameTime() + 0.5f );
		break;
	}
	case KeyCode::KEY_JOYDDOWN:
	{
		EnviroMinder & enviro =
			ChunkManager::instance().cameraSpace()->enviro();
		enviro.timeOfDay()->gameTime(
			enviro.timeOfDay()->gameTime() - 0.5f );
		break;
	}
	case KeyCode::KEY_JOYARPUSH:
	case KeyCode::KEY_F:	// F for flush
	{
		reloadChunks();
		clientChatMsg( "Reloading all chunks" );
		break;
	}

	default:
		handled = false;
	}

	return handled;

#endif // ENABLE_DEBUG_KEY_HANDLER
}

/**
 *	Checks to see if the given key belongs to a debug key combination
 *	and also checks if the other keys in the combination are currently down.
 */
bool App::checkDebugKeysState( KeyCode::Key key )
{
	for( size_t i = 0; i < debugKeys_.size(); i++ )
	{
		KeyCode::KeyArray& keys = debugKeys_[i];

		bool keyInCombo = false;
		bool otherKeysDown = true;
		for( size_t k = 0; k < keys.size(); k++ )
		{
			if ( key == keys[k] )
			{
				keyInCombo = true;
			}
			else if ( !InputDevices::isKeyDown( keys[k] ) )
			{
				otherKeysDown = false;
				break;
			}
		}

		// If this is true and we got here, then all other keys in 
		// the combination are currently pressed.
		if (keyInCombo && otherKeysDown)
		{
			return true;
		}
	}

	return false;
}

/**
 *	Checks to see if the given key belongs to any debug key combo.
 */
bool App::isDebugKey( KeyCode::Key key )
{
	for( size_t i = 0; i < debugKeys_.size(); i++ )
	{
		KeyCode::KeyArray& keys = debugKeys_[i];
		for( size_t k = 0; k < keys.size(); k++ )
		{
			if (keys[k] == key)
			{
				return true;
			}
		}
	}

	return false;
}


/**
 *	This method overrides the InputHandler method to handle mouse events.
 *
 *	@param event	The current mouse event.
 *
 *	@return			True if the event was handled, false otherwise.
 */
bool App::handleMouseEvent( const MouseEvent & event )
{
	BW_GUARD;
	bool handled = false;

#if SCALEFORM_SUPPORT
	POINT mousePos;
	::GetCursorPos( &mousePos );
	::ScreenToClient( App::instance().hWnd(), &mousePos );
	int buttons = Scaleform::mouseButtonsFromEvent(Scaleform::Manager::instance().lastMouseKeyEvent_);
	Scaleform::Manager::instance().onMouse( buttons, 0, mousePos.x, mousePos.y );
#endif

	// First give the personality script a go
	if (!handled)
	{
		// We give the personality script any movements
		PyObject * ret = Script::ask(
			PyObject_GetAttrString( Personality::instance(), "handleMouseEvent" ),
			Py_BuildValue("(N)", Script::getData(event) ),
			"Personality handleMouseEvent: " );

		Script::setAnswer( ret, handled,
			"Personality Script handleMouseEvent retval" );
	}

	// And finally the active cursor gets its turn
	if (!handled && activeCursor_ != NULL)
	{
		handled = activeCursor_->handleMouseEvent( event );
	}

	return true;
}



/**
 *	This method overrides the InputHandler method to joystick axis events.
 *
 *	@return			True if the event was handled, false otherwise.
 */
bool App::handleAxisEvent( const AxisEvent & event )
{
	BW_GUARD;
	bool handled = false;

	// The debug consoles get in first
	if (!handled)
	{
		handled = ConsoleManager::instance().handleAxisEvent( event );
	}

	// Now give the personality script a go, if it ever needs this
	if (!handled)
	{
		PyObject * ret = Script::ask(
			PyObject_GetAttrString( Personality::instance(), "handleAxisEvent" ),
			Py_BuildValue("(N)", Script::getData(event) ),
			"Personality handleAxisEvent: " );
		Script::setAnswer( ret, handled, "Personality handleAxisEvent retval" );
	}

	// And finally the active cursor gets its turn
	if (!handled && activeCursor_ != NULL)
	{
		handled = activeCursor_->handleAxisEvent( event );
	}

	// Physics gets anything that's left
	if (!handled)
	{
		handled = Physics::handleAxisEventAll( event );
	}

	return true;
}


/**
 *	Returns the current active cursor.
 */
InputCursorPtr App::activeCursor()
{
	return this->activeCursor_;
}


/**
 *	Sets the active cursor. The active cursor will get all mouse,
 *	keyboard and axis events forwarded to it. Only one cursor can
 *	be active at a time. The deactivate method will be called on
 *	the current active cursor (the one being deactivated), if any.
 *	The activate method will be called on the new active cursor.
 *	if any.
 *
 *	@param	cursor	the new active cursor, or NULL to simply
 *					deactivate the current one.
 *
 */
void App::activeCursor( InputCursor * cursor )
{
	BW_GUARD;
	if (this->activeCursor_)
	{
		this->activeCursor_->deactivate();
	}

	this->activeCursor_ = cursor;

	if (this->activeCursor_)
	{
		this->activeCursor_->activate();
		if ( InputDevices::instance().hasFocus() )
			activeCursor_->focus( true );
	}
}


bool App::savePreferences()
{
	BW_GUARD;
	return DeviceApp::instance.savePreferences();
}

// -----------------------------------------------------------------------------
// Section: Window Events
// -----------------------------------------------------------------------------


/**
 *	This method is called when the window has resized. It is called in response
 *	to the Windows event.
 */
void App::resizeWindow( void )
{
	BW_GUARD;
	if( Moo::rc().windowed() == true )
	{
		Moo::rc().resetDevice();
	}
}


void App::resizeWindow(int width, int height)
{
	BW_GUARD;
	if (Moo::rc().device() == NULL || Moo::rc().windowed())
	{
		RECT	clientRect;
		GetClientRect( hWnd_, &clientRect );

		RECT	parentRect;
		HWND	parent = GetParent( hWnd_ );

		if (parent)
		{
			GetWindowRect( parent, &parentRect );
		}
		else
		{
			parentRect.left = 0;
			parentRect.top = 0;
		}

		RECT	borderRect;
		GetWindowRect( hWnd_, &borderRect );

		POINT	border;
		border.x = (borderRect.right - borderRect.left) - clientRect.right;
		border.y = (borderRect.bottom - borderRect.top) - clientRect.bottom;

		MoveWindow(
			hWnd_, borderRect.left - parentRect.left, borderRect.top - parentRect.top,
			width + border.x, height + border.y, true );
	}
}

const POINT App::windowSize() const
{
	BW_GUARD;
	POINT result;
	result.x = LONG(Moo::rc().screenWidth());
	result.y = LONG(Moo::rc().screenHeight());
	return result;
}

/**
 *	This method is called when the window has moved. It is called in response to
 *	the Windows event and is important for DirectX.
 */
void App::moveWindow( int16 x, int16 y )
{
	BW_GUARD;
	//Moo::rc().device()->moveFrame( x, y );
}


/**
 *	This method is called when the application gets the focus.
 */
/*static*/ void App::handleSetFocus( bool state )
{
	BW_GUARD;
	DEBUG_MSG("App::handleSetFocus: %d\n", state);
	InputDevices::setFocus( state, pInstance_ );

	if (pInstance_ != NULL && pInstance_->activeCursor_ != NULL)
	{
		pInstance_->activeCursor_->focus( state );
	}

	MouseCursor::staticSetFocus( state );

	//Check that either there is no SimpleGUI yet or automaticCursorManagement is on
	if (SimpleGUI::pInstance() == NULL || SimpleGUI::instance().mouseCursor().automaticCursorManagement())
	{
		Moo::rc().restoreCursor(!state);
	}
	if ( Moo::rc().device() && !Moo::rc().windowed())
		Moo::rc().changeMode( Moo::rc().modeIndex(), true );
}


extern const wchar_t * APP_TITLE;

/**
 *	This function sets (or clears, if the input string is empty)
 *	the title note identified by 'pos'
 */
void App::setWindowTitleNote( int pos, const std::string & note )
{
	BW_GUARD;
	if (note.empty())
	{
		TitleNotes::iterator it = titleNotes_.find( pos );
		if (it != titleNotes_.end()) titleNotes_.erase( it );
	}
	else
	{
		titleNotes_[ pos ] = note;
	}

	std::string newTitle;
	bw_wtoutf8( APP_TITLE, newTitle );
	if (!titleNotes_.empty())
	{
		int n = 0;
		for (TitleNotes::iterator it = titleNotes_.begin();
			it != titleNotes_.end();
			it++, n++)
		{
			newTitle += n ? ", " : " [";
			newTitle += it->second;
		}
		newTitle += "]";
	}

	std::wstring wnewTitle;
	bw_utf8tow( newTitle, wnewTitle );
	SetWindowText( hWnd_, wnewTitle.c_str() );
}


// -----------------------------------------------------------------------------
// Section: Statistics and Debugging
// -----------------------------------------------------------------------------

/**
 *	This method calculates the time between this frame and last frame.
 */
void App::calculateFrameTime()
{
	BW_GUARD;
	uint64 thisTime = frameTimerValue();

	// Need to be careful of the wraparound case here.
	FRAME_TIMER_TYPE delta = FRAME_TIMER_TYPE( thisTime - lastTime_ );

	dTime_ = float( delta / frameTimerFreq() );

	if (DebugApp::instance.slowTime_ > 0.000001f)
		dTime_ /= DebugApp::instance.slowTime_;

	totalTime_ += dTime_;
	lastTime_ = thisTime;
}


/**
 *	This method gets the name of the active console
 */
std::string App::activeConsole() const
{
	XConsole* con = ConsoleManager::instance().pActiveConsole();
	if (!con)
	{
		return "";
	}


	return ConsoleManager::instance().consoleName( con );
}

/**
 *	This method sets the active console by name
 */
void App::activeConsole(std::string v)
{
	if ( !v.empty() )
	{
		ConsoleManager::instance().activate( v.c_str() );		
	}
	else
	{
		ConsoleManager::instance().deactivate();
	}
}


// -----------------------------------------------------------------------------
// Section: Miscellaneous App methods
// -----------------------------------------------------------------------------



/// Make sure the Python object ring hasn't been corrupted
void App::checkPython()
{
	BW_GUARD;	
#ifdef Py_DEBUG
	PyObject* head = PyInt_FromLong(1000000);
	PyObject* p = head;

	INFO_MSG("App::checkPython: checking python...\n");

	while(p && p->_ob_next != head)
	{
		if((p->_ob_prev->_ob_next != p) || (p->_ob_next->_ob_prev != p))
		{
			CRITICAL_MSG("App::checkPython: Python object %0.8X is screwed\n", p);
		}
		p = p->_ob_next;
	}

	Py_DECREF(head);
	INFO_MSG("App::checkPython: done..\n");
#endif
}





/**
 *	Returns whether or not the camera is outside
 */
bool isCameraOutside()
{
	BW_GUARD;
	Chunk * pCC = ChunkManager::instance().cameraChunk();
	return pCC == NULL || pCC->isOutsideChunk();
}

/**
 *	Returns whether or not the player is outside
 */
bool isPlayerOutside()
{
	BW_GUARD;
	Entity * pPlayer;
	if ((pPlayer = Player::entity()) == NULL) return true;

	return pPlayer->pPrimaryEmbodiment() == NULL ||
		pPlayer->pPrimaryEmbodiment()->chunk() == NULL ||
		pPlayer->pPrimaryEmbodiment()->chunk()->isOutsideChunk();
}

/*~ function BigWorld.quit
 *  Ask the application to quit.
 */
/**
 *	Ask the app to quit
 */
static void quit()
{
	BW_GUARD;
	App::instance().quit();
}
PY_AUTO_MODULE_FUNCTION( RETVOID, quit, END, BigWorld )

/*~ function BigWorld.playMovie
 *	@components{ client }
 *
 *	Placeholder for deprecated functionality.
 */
static void playMovie()
{
	//not on PC
}
PY_AUTO_MODULE_FUNCTION( RETVOID, playMovie, END, BigWorld )




/*~ function BigWorld timeOfDay
 *  Gets and sets the time of day in 24 hour time, as used by the environment
 *  system. If the camera is not currently in a space, then this function will
 *	not do anything, and will return an empty string.
 *
 *  This can also be changed manually via the following key combinations:
 *
 *  DEBUG + "[":			Rewind time of day by 10 minutes
 *
 *  DEBUG + Shift + "[":	Rewind time of day by 1 hour
 *
 *  DEBUG + "]":			Advance time of day by 10 minutes
 *
 *  DEBUG + Shift + "]":	Advance time of day by 1 hour
 *
 *  @param time Optional string. If provided, the time of day is set to this.
 *	This can be in the format "hour:minute" (eg "01:00", "1:00", "1:0", "1:"
 *  ), or as a string representation of a float (eg "1", "1.0"). Note that an
 *  incorrectly formatted string will not throw an exception, nor will it
 *  change the time of day.
 *  @return A string representing the time of day at the end of the function
 *  call in the form "hh:mm" (eg "01:00" ). Returns an empty string if the
 *	camera is not currently in a space.
 */
/**
 *	Function to let scripts set the time of day
 */
const std::string & timeOfDay( const std::string & tod )
{
	BW_GUARD;
	ChunkSpacePtr cameraSpace = ChunkManager::instance().cameraSpace();
	if (!cameraSpace.exists())
	{
		static const std::string empty;
		return empty;
	}

	EnviroMinder & enviro = cameraSpace->enviro();
	if (tod.size())
	{
		enviro.timeOfDay()->setTimeOfDayAsString(tod);
	}
	return enviro.timeOfDay()->getTimeOfDayAsString();
}
PY_AUTO_MODULE_FUNCTION(
	RETDATA, timeOfDay, OPTARG( std::string, "", END ), BigWorld )

/*~ function BigWorld.spaceTimeOfDay
 *  Gets and sets the time of day in 24 hour time, as used by the environment
 *  system.
 *	@param spaceID The spaceID of the space to set/get the time.
 *  @param time Optional string. If provided, the time of day is set to this.
 *	This can be in the format "hour:minute" (eg "01:00", "1:00", "1:0", "1:"
 *  ), or as a string representation of a float (eg "1", "1.0"). Note that an
 *  incorrectly formatted string will not throw an exception, nor will it
 *  change the time of day.
 *  @return A string representing the time of day at the end of the function
 *  call in the form "hh:mm" (eg "01:00" ).
 */
/**
 *	Function to let scripts set the time of day for a given space.
 */
const std::string & spaceTimeOfDay( ChunkSpaceID spaceID, const std::string & tod )
{
	BW_GUARD;
	ChunkSpacePtr pSpace = ChunkManager::instance().space( spaceID );

	if ( pSpace )
	{
		EnviroMinder & enviro = pSpace->enviro();
		if (tod.size())
		{
			enviro.timeOfDay()->setTimeOfDayAsString(tod);
		}

		return enviro.timeOfDay()->getTimeOfDayAsString();
	}

	static std::string s_nullSpaceTime = "00:00";
	return s_nullSpaceTime;
}
PY_AUTO_MODULE_FUNCTION(
	RETDATA, spaceTimeOfDay, ARG( SpaceID, OPTARG( std::string, "", END )), BigWorld )



/*~	attribute BigWorld.platform
 *
 *	This is a string that represents which platform the client is running on,
 *	for example, "windows".
 */
/// Add the platform name attribute
PY_MODULE_ATTRIBUTE( BigWorld, platform, Script::getData( "windows" ) )


/*~	attribute BigWorld.isEmbedded
 *
 *	This is a boolean that represents if the client is running embedded
 */
/// Add the isEmbedded name attribute
PY_MODULE_ATTRIBUTE( BigWorld, isEmbedded, Script::getData( App::instance().isEmbedded() ) )

// app.cpp
