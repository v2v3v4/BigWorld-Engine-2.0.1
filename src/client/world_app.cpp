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
#include "world_app.hpp"

#include "app.hpp"
#include "app_config.hpp"
#include "canvas_app.hpp"
#include "chunk/chunk_manager.hpp"
#include "cstdmf/main_loop_task.hpp"
#include "cstdmf/memory_trace.hpp"
#include "cstdmf/diary.hpp"
#include "device_app.hpp"
#include "duplo/foot_trigger.hpp"
#include "duplo/py_attachment.hpp"
#include "particle/actions/particle_system_action.hpp"
#include "entity_manager.hpp"
#include "terrain/manager.hpp"
#include "terrain/base_terrain_renderer.hpp"
#include "terrain/terrain2/terrain_lod_controller.hpp"
#include "moo/visual_compound.hpp"
#include "player.hpp"
#include "romp/py_chunk_light.hpp"
#include "romp/py_chunk_spot_light.hpp"
#include "resmgr/datasection.hpp"
#include "romp/progress.hpp"
#include "romp/texture_renderer.hpp"
#include "romp/water.hpp"
#include "shadow_manager.hpp"

#if UMBRA_ENABLE
#include "chunk/chunk_umbra.hpp"
#include "math/colour.hpp"
#endif



WorldApp WorldApp::instance;

int WorldApp_token = 1;

PROFILER_DECLARE( AppDraw_World, "AppDraw World" );
PROFILER_DECLARE( WorldApp_Chunk, "WorldApp Chunk" );
PROFILER_DECLARE( WorldApp_Shadow, "WorldApp Shadow" );
PROFILER_DECLARE( WorldApp_Enviro, "WorldApp Enviro" );

WorldApp::WorldApp() :
	canSeeTerrain_( false ),
	wireFrameStatus_( 0 ),
	debugSortedTriangles_( 0 )
{
	BW_GUARD;
	MainLoopTasks::root().add( this, "World/App", NULL );
}


WorldApp::~WorldApp()
{
	BW_GUARD;
	/*MainLoopTasks::root().del( this, "World/App" );*/
}


bool WorldApp::init()
{
	BW_GUARD;
#if ENABLE_WATCHERS
	DEBUG_MSG( "WorldApp::init: Initially using %d(~%d)KB\n",
		memUsed(), memoryAccountedFor() );
#endif

	MEM_TRACE_BEGIN( "WorldApp::init" )

	DEBUG_MSG( "Terrain: Initialising Terrain Manager...\n" );
	MF_VERIFY( Terrain::Manager::init() );

	DataSectionPtr configSection = AppConfig::instance().pRoot();

	DEBUG_MSG( "Task: Initialising BgTaskManager...\n" );
	BgTaskManager::instance().startThreads( 1, DX::addSecondaryThread );

	DEBUG_MSG( "Chunk: Initialising ChunkManager...\n" );
	ChunkManager::instance().init( configSection );

	MF_WATCH( "Render/Wireframe Mode",
		WorldApp::instance.wireFrameStatus_,
		Watcher::WT_READ_WRITE,
		"Toggle wire frame mode between none, scenery, terrain and all." );

	MF_WATCH( "Render/Debug Sorted Tris",
		WorldApp::instance.debugSortedTriangles_,
		Watcher::WT_READ_WRITE,
		"Toggle sorted triangle debug mode.  Values of 1 or 2 will display "
		"sorted triangles so you can see how many are being drawn and where." );

//	MF_WATCH( "Draw Wireframe", g_drawWireframe );

	MF_WATCH( "Render/Draw Portals",
		ChunkBoundary::Portal::drawPortals_,
		Watcher::WT_READ_WRITE,
		"Draw portals on-screen as red polygons." );

	MF_WATCH( "Render/Draw Skeletons",
		PyModel::drawSkeletons_,
		Watcher::WT_READ_WRITE,
		"Draw the skeletons of PyModels." );

	MF_WATCH( "Render/Draw Node Labels",
		PyModel::drawNodeLabels_,
		Watcher::WT_READ_WRITE,
		"Draw model's node labels?" );

	MF_WATCH( "Render/Plot Foottriggers",
		FootTrigger::plotEnabled_,
		Watcher::WT_READ_WRITE,
		"Plot foot triggers height?" );

	MF_WATCH( "Render/Terrain/draw",
		g_drawTerrain,
		Watcher::WT_READ_WRITE,
		"Enable drawing of the terrain." );

	bool ret = DeviceApp::s_pStartupProgTask_->step(APP_PROGRESS_STEP) &&
					DeviceApp::s_pProgress_->draw( true );	// for if new loading screen

	MEM_TRACE_END()

	return ret;
}


void WorldApp::fini()
{
	BW_GUARD;
	BgTaskManager::instance().stopAll();
	GlobalModels::fini();
    EntityManager::instance().fini();
	ChunkManager::instance().fini();
	Terrain::Manager::fini();
}


void WorldApp::tick( float dTime )
{
	BW_GUARD;
	dTime_ = dTime;
	PyChunkLight::revAll();
	PyChunkSpotLight::revAll();	

	static DogWatch sceneTick("Scene");
	sceneTick.start();

	// do the background task manager tick
	BgTaskManager::instance().tick();

	// do the chunk tick now
	ChunkManager::instance().tick( dTime );

	// tick the shadows
	ShadowManager::instance().tick( dTime );

	// Update terrain lod controller with new info.
	DiaryEntryPtr del = Diary::instance().add( "LodController" );	
	Terrain::BasicTerrainLodController::instance().setCameraPosition( 
		Moo::rc().invView().applyToOrigin() );
	del->stop();

	// update global models as late as possible, we don't know
	// what kind of matrix providers they'll be using.
	GlobalModels::tick( dTime );

	sceneTick.stop();
}

void WorldApp::inactiveTick( float dTime )
{
	BW_GUARD;
	this->tick( dTime );
}

static DogWatch	g_watchCanvasAppDraw("CanvasApp");

void WorldApp::draw()
{
	BW_GUARD_PROFILER( AppDraw_World );

	static DogWatch sceneWatch( "Scene" );

	// draw the design-time scene
	sceneWatch.start();
	Moo::rc().setRenderState( D3DRS_FILLMODE,
		(wireFrameStatus_ & 2) ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	EnviroMinder * envMinder = 0;
	if (ChunkManager::instance().cameraSpace().getObject() != 0)
	{
		envMinder = &ChunkManager::instance().cameraSpace()->enviro();
	}

#if UMBRA_ENABLE
	if (UmbraHelper::instance().umbraEnabled())
	{
		Moo::rc().setRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
		ChunkManager::instance().umbraDraw();

		if (wireFrameStatus_ & (1 | 2))
		{
			Moo::rc().device()->EndScene();

			Vector3 bgColour = Vector3( 0, 0, 0 );
			uint32 clearFlags = D3DCLEAR_ZBUFFER;
			if ( Moo::rc().stencilAvailable() )
				clearFlags |= D3DCLEAR_STENCIL;
			Moo::rc().device()->Clear( 0, NULL,
					clearFlags,
					Colour::getUint32( bgColour ), 1, 0 );

			Moo::rc().device()->BeginScene();

			Moo::rc().setRenderState( D3DRS_FILLMODE,
				(wireFrameStatus_ & 2) ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

			if ( Terrain::BaseTerrainRenderer::instance()->version() == 200 )
				Terrain::TerrainRenderer2::instance()->zBufferIsClear( true );

			UmbraHelper::instance().wireFrameTerrain( wireFrameStatus_ & 1 );
			ChunkManager::instance().umbraRepeat();
			UmbraHelper::instance().wireFrameTerrain( 0 );

			if ( Terrain::BaseTerrainRenderer::instance()->version() == 200 )
				Terrain::TerrainRenderer2::instance()->zBufferIsClear( false );
		}
	}
	else
	{
		ChunkManager::instance().draw();
	}
#else
	ChunkManager::instance().draw();
#endif

	sceneWatch.stop();

	static DogWatch terrainWatch( "Terrain" );
	terrainWatch.start();
	DiaryEntryPtr de = Diary::instance().add( "Terrain" );	
	Moo::rc().setRenderState( D3DRS_FILLMODE,
		(wireFrameStatus_ & 1) ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (pSpace) Moo::rc().lightContainer( pSpace->lights() );

	canSeeTerrain_ = Terrain::BaseTerrainRenderer::instance()->canSeeTerrain();

	DiaryEntryPtr del = Diary::instance().add( "DrawTerrain" );
	Terrain::BaseTerrainRenderer::instance()->drawAll();
	del->stop();

	Moo::rc().setRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	terrainWatch.stop();
	de->stop();

	{
		ScopedDogWatch sdw( sceneWatch );
		PROFILER_SCOPED( WorldApp_Chunk );

		Moo::rc().setRenderState( D3DRS_FILLMODE,
			(wireFrameStatus_ & 2) ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

		if (ChunkManager::instance().cameraSpace())
		{
			Moo::LightContainerPtr pRCLC = Moo::rc().lightContainer();
			Moo::LightContainerPtr pRCSLC = Moo::rc().specularLightContainer();
			static Moo::LightContainerPtr pLC = new Moo::LightContainer;
			static Moo::LightContainerPtr pSLC = new Moo::LightContainer;

			pLC->directionals().clear();
			pLC->ambientColour( ChunkManager::instance().cameraSpace()->ambientLight() );
			if (ChunkManager::instance().cameraSpace()->sunLight())
				pLC->addDirectional( ChunkManager::instance().cameraSpace()->sunLight() );
			pSLC->directionals().clear();
			pSLC->ambientColour( ChunkManager::instance().cameraSpace()->ambientLight() );
			if (ChunkManager::instance().cameraSpace()->sunLight())
				pSLC->addDirectional( ChunkManager::instance().cameraSpace()->sunLight() );
			Moo::rc().lightContainer( pLC );
			Moo::rc().specularLightContainer( pSLC );

			Moo::VisualCompound::drawAll();

			static DogWatch s_dwVisualBatches( "Visual Batches" );
			s_dwVisualBatches.start();
			Moo::Visual::drawBatches();
			s_dwVisualBatches.stop();

			Moo::rc().lightContainer( pRCLC );
			Moo::rc().specularLightContainer( pRCSLC );
		}

		Moo::rc().setRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	}

	{
		static DogWatch shadowWatch( "Shadow" );
		ScopedDogWatch sdw( shadowWatch );
		PROFILER_SCOPED( WorldApp_Shadow );
		ShadowManager::instance().renderShadows( canSeeTerrain_ );
	}

	{
		static DogWatch enviroWatch( "Enviro" );
		ScopedDogWatch sdw( enviroWatch );
		PROFILER_SCOPED( WorldApp_Enviro );
		// Draw the delayed background of our environment.
		// We do this here in order to save fillrate, as we can
		// test the delayed background against the opaque scene that
		// is in the z-buffer.
		if (pSpace)
		{
			EnviroMinder & enviro = pSpace->enviro();
			enviro.drawHindDelayed( dTime_, CanvasApp::instance.drawSkyCtrl_ );
		}
	}

	Entity * pPlayer = Player::entity();
	bool bWasVisible;
	if (pPlayer && pPlayer->pPrimaryModel())
	{
		bWasVisible = pPlayer->pPrimaryModel()->visible( );
		pPlayer->pPrimaryModel()->visible( true );
	}

	Waters::instance().tick( dTime_ );

	static DogWatch dTexWatch( "Dynamic Textures" );
	dTexWatch.start();
	TextureRenderer::updateCachableDynamics( dTime_ );
	dTexWatch.stop();

	if (pPlayer && pPlayer->pPrimaryModel())
		pPlayer->pPrimaryModel()->visible( bWasVisible );
}


// world_app.cpp
