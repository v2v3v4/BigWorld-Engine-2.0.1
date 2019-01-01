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
#include "device_app.hpp"
#include "app.hpp"
#include "app_config.hpp"
#include "bw_winmain.hpp"
#include "canvas_app.hpp"
#include "client_camera.hpp"
#include "connection_control.hpp"
#include "critical_handler.hpp"
#include "entity_manager.hpp"
#include "message_time_prefix.hpp"
#include "player.hpp"
#include "shadow_manager.hpp"
#include "camera/annal.hpp"
#include "camera/direction_cursor.hpp"
#include "chunk/chunk_manager.hpp"
#include "connection/server_connection.hpp"
#include "cstdmf/memory_trace.hpp"
#include "cstdmf/processor_affinity.hpp"
#include "math/colour.hpp"
#include "moo/visual_channels.hpp"
#include "resmgr/datasection.hpp"
#include "resmgr/xml_section.hpp"
#include "romp/font_manager.hpp"
#include "romp/progress.hpp"
#include "romp/texture_feeds.hpp"
#include "romp/texture_renderer.hpp"
#include "romp/water.hpp"
#include "post_processing/manager.hpp"
#include "scaleform/config.hpp"
#if SCALEFORM_SUPPORT
	#include "scaleform/manager.hpp"
#if SCALEFORM_IME
	#include "scaleform/ime.hpp"
#endif
#endif


DeviceApp DeviceApp::instance;

int DeviceApp_token = 1;

PROFILER_DECLARE( AppDraw_Device, "AppDraw Device" );

static DogWatch	g_watchInput("Input");

namespace {

#if SCALEFORM_IME
const AutoConfigString s_scaleformIMEMovie( 
	"scaleform/imeMovie", 
	"IME.swf" );
#endif

}


DeviceApp::DeviceApp() :
		dTime_( 0.f ),
		soundEnabled_( true )
{
	BW_GUARD;
	MainLoopTasks::root().add( this, "Device/App", NULL );
}


DeviceApp::~DeviceApp()
{
	BW_GUARD;
	/*MainLoopTasks::root().del( this, "Device/App" );*/
}


bool DeviceApp::init()
{
	BW_GUARD;	
#if ENABLE_WATCHERS
	DEBUG_MSG( "DeviceApp::init: Initially using %d(~%d)KB\n",
		memUsed(), memoryAccountedFor() );
#endif

	MEM_TRACE_BEGIN( "DeviceApp::init" )

	// Open the preferences
	DataSectionPtr configSection = AppConfig::instance().pRoot();

	// preferences
	preferencesFilename_.init( 
				configSection->openSection( "preferences" ),
				"preferences.xml", 
				PathedFilename::BASE_EXE_PATH );
	
	// Set up the processor affinity
	DataSectionPtr preferences;
	DataResource dataRes;
	if (dataRes.load( preferencesFilename_.resolveName() ) == DataHandle::DHE_NoError)
	{
		preferences = dataRes.getRootSection();
	}

	if (preferences)
	{
		DataSectionPtr appPreferences = 
			preferences->openSection("appPreferences");
		if (appPreferences.exists())
		{
			uint32 affinity = appPreferences->readInt( "mainThreadCpuAffinity", 
				ProcessorAffinity::get() );
			ProcessorAffinity::set( affinity );
		}
	}

	// Init the timestamp, this can take up to 1 second
	double sps = stampsPerSecondD();

	// 1. Input

	MEM_TRACE_BEGIN( "Input" )
	// init input devices
	if (!InputDevices::instance().init( s_hInstance_, s_hWnd_ ))
	{
		ERROR_MSG( "App::init: Init inputDevices FAILED\n" );
		MEM_TRACE_END()
		MEM_TRACE_END()
		return false;
	}

	MEM_TRACE_END()


	// 2. Network
	MEM_TRACE_BEGIN( "Network" )

	initNetwork();

	ConnectionControl::serverConnection()->pTime( &App::instance().totalTime_ );
	ConnectionControl::serverConnection()->initDebugInfo();

	MEM_TRACE_END()

	// 3. Graphics
	MEM_TRACE_BEGIN( "Graphics" )

	// Search suitable video mode
	const Moo::DeviceInfo& di = Moo::rc().deviceInfo( 0 );

	int maxWindowWidth = 0;
	int maxWindowHeight = 0;
	int maxNumberPixels = 0;
	for ( std::vector< D3DDISPLAYMODE >::const_iterator it = di.displayModes_.begin() ;
			it != di.displayModes_.end() ; ++it )
	{
		const D3DDISPLAYMODE & dm = *it;

		int numberPixels = dm.Width * dm.Height;
		if ( numberPixels > maxNumberPixels )
		{
			maxNumberPixels = numberPixels;
			maxWindowWidth = dm.Width;
			maxWindowHeight = dm.Height;
		}
	}

	bool windowed     = true;
	bool waitVSync    = true;
	bool tripleBuffering = true;
	float aspectRatio = 4.0f/3.0f;
	int windowWidth   = 1024;
	int windowHeight  = 768;
	int fullscreenWidth = 1024;
	int fullscreenHeight = 768;
	bool useWrapper	= false;
	useWrapper		= configSection->readBool("renderer/useJobSystem", useWrapper );

	if ( DebugMsgHelper::automatedTest() )
	{
		char log[80];
		bw_snprintf( log, sizeof( log ), "JobSystem: %s", useWrapper ? "On" : "Off" );
		DebugMsgHelper::logToFile( log );
	}

	// load graphics settings
	if (preferences)
	{
		DataSectionPtr graphicsPreferences =
			preferences->openSection("graphicsPreferences");

		Moo::GraphicsSetting::init(graphicsPreferences);

		DataSectionPtr devicePreferences =
			preferences->openSection("devicePreferences");

		if (devicePreferences.exists())
		{
			windowed     = devicePreferences->readBool("windowed", windowed);
			waitVSync    = devicePreferences->readBool("waitVSync", waitVSync);
			tripleBuffering = devicePreferences->readBool("tripleBuffering", tripleBuffering);
			aspectRatio  = devicePreferences->readFloat("aspectRatio", aspectRatio);
			windowWidth  = devicePreferences->readInt("windowedWidth", windowWidth);
			windowHeight = devicePreferences->readInt("windowedHeight", windowHeight);
			fullscreenWidth = devicePreferences->readInt("fullscreenWidth", fullscreenWidth);
			fullscreenHeight = devicePreferences->readInt("fullscreenHeight", fullscreenHeight);

			// limit width and height
			windowWidth  = Math::clamp(512, windowWidth, maxWindowWidth);
			windowHeight = Math::clamp(384, windowHeight, maxWindowHeight);
		}

		// console history
		DataSectionPtr consoleSect = 
			preferences->openSection("consoleHistory");
			
		if (consoleSect.exists())
		{
			CanvasApp::StringVector history;
			consoleSect->readWideStrings("line", history);
			
			// unfortunately, the python console hasn't been 
			// created yet at this point. Delegate to the canvas 
			// app to restore the python console history
			CanvasApp::instance.setPythonConsoleHistory(history);
		}

		s_scriptsPreferences =
			preferences->openSection("scriptsPreferences", true);
	}
	else
	{
		// set blank xml data section
		s_scriptsPreferences = new XMLSection("root");
	}

	bgColour_ = Vector3( 160, 180, 250 ) * 0.9f;
	uint32 deviceIndex = 0;

	// search for a suitable video mode.
	int modeIndex = 0;
	const int numDisplay = di.displayModes_.size();

	int i = 0;
	for ( ; i < numDisplay; i++ )
	{
		if ( ( di.displayModes_[i].Width == fullscreenWidth ) &&
			 ( di.displayModes_[i].Height == fullscreenHeight ) && 
			 ( ( di.displayModes_[i].Format == D3DFMT_X8R8G8B8 ) ||
			   ( di.displayModes_[i].Format == D3DFMT_A8B8G8R8 ) ) )
		{
			modeIndex = i;
			break;
		}

	}
	if ( i >= numDisplay )
	{
		// This will be used as a fallback.
		const int numTraits = 6;
		const int32 modeTraits[numTraits][4] = {
			{ 1024, 768, D3DFMT_X8R8G8B8, D3DFMT_A8B8G8R8 },
			{ 800,  600, D3DFMT_X8R8G8B8, D3DFMT_A8B8G8R8 },
			{ 640,  480, D3DFMT_X8R8G8B8, D3DFMT_A8B8G8R8 },
			{ 1024, 768, D3DFMT_R5G6B5,   D3DFMT_X1R5G5B5 },
			{ 800,  600, D3DFMT_R5G6B5,   D3DFMT_X1R5G5B5 },
			{ 640,  480, D3DFMT_R5G6B5,   D3DFMT_X1R5G5B5 },
		};

		int searchIndex = 0;
		while(searchIndex < numDisplay * numTraits)
		{
			const int mIndex = searchIndex % numDisplay;
			const int tIndex = (searchIndex / numDisplay) % numTraits;
			if( di.displayModes_[mIndex].Width  == modeTraits[tIndex][0]  &&
				di.displayModes_[mIndex].Height == modeTraits[tIndex][1] && (
				di.displayModes_[mIndex].Format == modeTraits[tIndex][2] ||
				di.displayModes_[mIndex].Format == modeTraits[tIndex][3]))
			{
				modeIndex = mIndex;
				break;
			}
			++searchIndex;
		}
	}

	// Enable NVIDIA PerfKit hooks for all builds except consumer builds. This
	// means we can debug an issue without needing a rebuild of the application.
#if ENABLE_NVIDIA_PERFHUD
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
#endif

	if (GetParent( s_hWnd_ ))// we are a child, ignore any sizing
	{
		RECT rect;

		GetClientRect( s_hWnd_, &rect );
		windowWidth = rect.right;
		windowHeight = rect.bottom;
	}

	App::instance().resizeWindow(windowWidth, windowHeight);
	Moo::rc().fullScreenAspectRatio(aspectRatio);
	Moo::rc().waitForVBL(waitVSync);
	Moo::rc().tripleBuffering(tripleBuffering);

	int maxModeIndex = Moo::rc().deviceInfo(deviceIndex).displayModes_.size() - 1;
	modeIndex = std::min(modeIndex, maxModeIndex);
	Vector2 size = Vector2(float(windowWidth), float(windowHeight));
	if (!Moo::rc().createDevice( s_hWnd_, deviceIndex, modeIndex, windowed, true, 
							size, true, false, useWrapper ))
	{
		ERROR_MSG( "DeviceApp::init()  Could not create Direct3D device\n" );
		return false;
	}

#if SCALEFORM_SUPPORT
	Scaleform::Manager::init();
	D3DPRESENT_PARAMETERS presentParams = Moo::rc().presentParameters();
	Scaleform::Manager::instance().createRenderer( Moo::rc().device(), &presentParams, true, DeviceApp::s_hWnd_ );
#if SCALEFORM_IME
	Scaleform::IME::init( s_scaleformIMEMovie.value() );
#endif
#endif

	messageTimePrefix_ = new MessageTimePrefix;
	DebugFilter::instance().addMessageCallback( messageTimePrefix_  );

	Moo::VisualChannel::initChannels();

	bool ret = true;

	DataSectionPtr ptr = BWResource::instance().openSection("shaders/formats");
	if (ptr)
	{
		DataSectionIterator it = ptr->begin();
		DataSectionIterator end = ptr->end();
		while (it != end)
		{
			std::string format = (*it)->sectionName();
			uint32 off = format.find_first_of( "." );
			format = format.substr( 0, off );
			Moo::VertexDeclaration::get( format );
			it++;
		}
	}

	// wait for windows to
	// send us a paint message
	uint64	tnow = timestamp();
	while ( (timestamp() - tnow < stampsPerSecond()/2) && ret)
	{
		ret = BWProcessOutstandingMessages();
	}

	MEM_TRACE_END()

	// init the texture feed instance, this
	// registers material section processors.
	setupTextureFeedPropertyProcessors();

	// 4. Sound
#if FMOD_SUPPORT
	MEM_TRACE_BEGIN( "Sound" )
	soundEnabled_ = configSection->readBool( "soundMgr/enabled", soundEnabled_ );
	if (soundEnabled_)
	{
		DataSectionPtr dsp = configSection->openSection( "soundMgr" );

		if (dsp)
		{
			if (!SoundManager::instance().initialise( dsp ))
			{
				ERROR_MSG( "DeviceApp::init: Failed to initialise sound\n" );
			}
		}
		else
		{
			ERROR_MSG( "DeviceApp::init: "
				"No <soundMgr> config section found, sound support is "
				"disabled\n" );
		}

#if 0
		soundMgr().init( s_pProgress_ );
		DataSectionPtr dsp = configSection->openSection("soundMgr");
		if (dsp)
		{
			soundMgr().init(hWnd_,
				dsp->readInt("performanceChannels", 0),
				dsp->readFloat("rolloffFactor", 0),
				dsp->readFloat("dopplerFactor", 0),
				dsp->readInt("maxFxPaths", 0),
				dsp->readInt("maxAmbientPaths", 0),
				dsp->readInt("maxStatic3DPaths", 0),
				dsp->readInt("maxSimplePaths", 0)
			);
		}
		else
		{
			::soundMgr().init(hWnd_);
		}
#endif	// 0
	}
	else
	{
		SoundManager::instance().errorLevel( SoundManager::ERROR_LEVEL_SILENT );
	}
	MEM_TRACE_END()
#endif // FMOD_SUPPORT

	TextureFeeds::init();
	
	PostProcessing::Manager::init();

	DataSectionPtr spSection = BWResource::instance().openSection( s_shadowsXML );
	ShadowManager::instance().init( spSection );

	MEM_TRACE_END()

	FontManager::init();

	return ret;
}


void DeviceApp::fini()
{
	BW_GUARD;
	FontManager::fini();
#if FMOD_SUPPORT
	SoundManager::instance().fini();
#endif // FMOD_SUPPORT
	s_scriptsPreferences = NULL;
	ShadowManager::instance().fini();
	PostProcessing::Manager::fini();
	TextureFeeds::fini();
	Moo::VertexDeclaration::fini();
	delete InputDevices::pInstance();
	delete DeviceApp::s_pStartupProgTask_;
	DeviceApp::s_pStartupProgTask_ = NULL;
	delete DeviceApp::s_pProgress_;
	DeviceApp::s_pProgress_ = NULL;
#if SCALEFORM_SUPPORT
	InitSingleton<Scaleform::Manager>::fini();
#endif
	// release the render context
	// has to be done here and not in device, as this may free up
	// some pythonised stuff
	Moo::rc().releaseDevice();
	DebugFilter::instance().deleteMessageCallback( messageTimePrefix_  );
	delete messageTimePrefix_;
	messageTimePrefix_ = NULL;
	updateModels_.clear();
}


void DeviceApp::tick( float dTime )
{
	BW_GUARD;

	if (s_pProgress_ != NULL)
	{
		delete s_pStartupProgTask_;
		delete s_pProgress_;
		s_pStartupProgTask_ = NULL;
		s_pProgress_ = NULL;
	}

	dTime_ = dTime;
	g_watchInput.start();
	InputDevices::processEvents( App::instance() );
	//  make sure we are still connected.
	ConnectionControl::instance().tick();
	EntityManager::instance().gatherInput();
	g_watchInput.stop();
	// get the direction cursor to process its input immediately here too
	DirectionCursor::instance().tick( dTime );
	Moo::EffectManager::instance().finishEffectInits();
}


void DeviceApp::inactiveTick(float dTime)
{
	BW_GUARD;
	dTime_ = dTime;
	//  make sure we are still connected.
	ConnectionControl::instance().tick();
	EntityManager::instance().gatherInput();
}


void DeviceApp::draw()
{
	BW_GUARD_PROFILER( AppDraw_Device );

	Moo::rc().beginScene();

	// begin rendering
	if (Moo::rc().mixedVertexProcessing())
		Moo::rc().device()->SetSoftwareVertexProcessing( TRUE );

	DX::Viewport viewport;
	viewport.Width = (DWORD)Moo::rc().screenWidth();
	viewport.Height = (DWORD)Moo::rc().screenHeight();
	viewport.MinZ = 0.f;
	viewport.MaxZ = 1.f;
	viewport.X = 0;
	viewport.Y = 0;
	Moo::rc().setViewport( &viewport );

	uint32 clearFlags = D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER;
	if ( Moo::rc().stencilAvailable() )
		clearFlags |= D3DCLEAR_STENCIL;
	Moo::rc().device()->Clear( 0, NULL, clearFlags,
		Colour::getUint32( bgColour_ ), 1, 0 );

	Moo::rc().setWriteMask( 1, D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_ALPHA );

	Moo::rc().nextFrame();

	//HACK_MSG( "Heap size %d\n", heapSize() );
	// update any dynamic textures
	Moo::rc().setRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA |
		D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE );
	TextureRenderer::updateDynamics( dTime_ );
	Moo::rc().setRenderState( D3DRS_COLORWRITEENABLE,
		D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE );

	//TODO: under water effect..
	Waters::instance().checkVolumes();

#if FMOD_SUPPORT
    // ### FMOD
    Vector3 cameraPosition = ClientCamera::instance().camera()->invView().applyToOrigin();
    Vector3 cameraDirection = ClientCamera::instance().camera()->invView().applyToUnitAxisVector( 2 );
    Vector3 cameraUp = ClientCamera::instance().camera()->invView().applyToUnitAxisVector( 1 );
    SoundManager::instance().setListenerPosition( cameraPosition, cameraDirection, cameraUp, dTime_ );
#endif // FMOD_SUPPORT

	// 'draw' sounds, i.e. commit our position
	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (pSpace)
	{
		EnviroMinder & enviro = pSpace->enviro();

//		g_watchSound.start();
		if( Player::entity() )
		{
			PyModel* pModel = Player::entity()->pPrimaryModel();

			Vector3 bbDiag;
			float minDim;
			bool inShell = false;//pModel != NULL && !pModel->isOutside();

/*			if (inShell) {
				if (pModel->chunk() != NULL)
				{
					bbDiag = pModel->chunk()->boundingBox().maxBounds() -
							pModel->chunk()->boundingBox().minBounds();
				}
				minDim = min(fabsf(bbDiag.x), fabsf(bbDiag.z));
			} else*/
				minDim = 0;
#if 0
			soundMgr().setListenerPos(
				Moo::rc().invView(),
				Vector3(Player::entity()->position()),
				inShell,
				minDim,
				ChunkManager::instance().cameraSpace()->enviro().
					timeOfDay()->gameTime(),
				float()//this->totalTime_)
			);
			soundMgr().doWork();
#endif
			// TODO: Re-enable this sound with the new FMOD API
//			BWSound::playRain( enviro.rain()->amount(), dTime_ );
		}

//		g_watchSound.stop();
	} // end of 'if (pSpace)'

	ShadowManager::instance().captureShadows();

	// This is some slightly dodgy code to get around problems caused by PyModelNode
	// only being updated when an object is rendered.
	std::vector< PyModelPtr >::iterator it = DeviceApp::updateModels_.begin();
	while (it != DeviceApp::updateModels_.end())
	{
		PyModelPtr pModel = *it;
		if (pModel)
		{
			BoundingBox bb;
			pModel->localVisibilityBox(bb,true);
			Matrix cullTransform = Moo::rc().viewProjection();
			cullTransform.preMultiply( pModel->worldTransform() );
			bb.calculateOutcode( cullTransform );
			if (bb.combinedOutcode())
			{
				pModel->draw( pModel->worldTransform(), 0 );
			}
		}
		++it;
	}

}


void DeviceApp::deleteGUI()
{
	BW_GUARD;
	delete DeviceApp::s_pProgress_;
	DeviceApp::s_pProgress_ = NULL;
}


bool DeviceApp::savePreferences()
{
	BW_GUARD;
	bool result = false;
	std::string preferencesFilename = preferencesFilename_.resolveName();
	if (!preferencesFilename.empty())
	{
		BWResource::ensureAbsolutePathExists( preferencesFilename );

		DataResource dataRes(preferencesFilename, RESOURCE_TYPE_XML, true);
		DataSectionPtr root = dataRes.getRootSection();

		// graphics preferences
		DataSectionPtr grPref = root->openSection("graphicsPreferences", true);
		Moo::GraphicsSetting::write(grPref);

		// device preferences
		DataSectionPtr devPref = root->openSection("devicePreferences", true);
		devPref->delChildren();
		devPref->writeBool( "windowed",    Moo::rc().windowed());
		devPref->writeBool( "waitVSync",   Moo::rc().waitForVBL());
		devPref->writeBool( "tripleBuffering",   Moo::rc().tripleBuffering());
		devPref->writeFloat("aspectRatio", Moo::rc().fullScreenAspectRatio());

		Vector2 windowSize = Moo::rc().windowedModeSize();
		devPref->writeInt("windowedWidth",  int(windowSize.x));
		devPref->writeInt("windowedHeight", int(windowSize.y));
		
		const Moo::DeviceInfo& di = Moo::rc().deviceInfo( 0 );

		if (di.displayModes_.size() > Moo::rc().modeIndex())
		{
			const D3DDISPLAYMODE& mode = di.displayModes_[Moo::rc().modeIndex()];
			devPref->writeInt( "fullscreenWidth", mode.Width );
			devPref->writeInt( "fullscreenHeight", mode.Height );
		}

		// script preferences
		DataSectionPtr scrptPref = root->openSection("scriptsPreferences", true);
		scrptPref->delChildren();
		scrptPref->copy(s_scriptsPreferences);

		CanvasApp::StringVector history = CanvasApp::instance.pythonConsoleHistory();
		DataSectionPtr consoleSect = root->openSection("consoleHistory", true);
		consoleSect->delChildren();
		consoleSect->writeWideStrings("line", history);

		// save it
		if (dataRes.save() == DataHandle::DHE_NoError)
		{
			result = true;
		}
		else
		{
			ERROR_MSG("Could not save preferences file.\n");
		}
	}
	return result;
}


// device_app.cpp
