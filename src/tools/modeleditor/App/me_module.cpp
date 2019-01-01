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
#include "me_module.hpp"

#include "appmgr/closed_captions.hpp"
#include "appmgr/module_manager.hpp"
#include "appmgr/options.hpp"
#include "ashes/simple_gui.hpp"
#include "chunk/chunk_item_amortise_delete.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"
#include "duplo/material_draw_override.hpp"
#include "duplo/shadow_caster_common.hpp"
#include "gizmo/gizmo_manager.hpp"
#include "gizmo/tool_manager.hpp"
#include "main_frm.h"
#include "math/planeeq.hpp"
#include "me_app.hpp"
#include "me_shell.hpp"
#include "me_consts.hpp"
#include "model_editor.h"
#include "terrain/base_terrain_renderer.hpp"
#include "moo/effect_visual_context.hpp"
#include "terrain/terrain2/terrain_lod_controller.hpp"
#include "moo/texture_manager.hpp"
#include "moo/visual_channels.hpp"
#include "page_materials.hpp"
#include "page_object.hpp"
#include "pyscript/py_callback.hpp"
#include "resmgr/auto_config.hpp"
#include "resmgr/bwresource.hpp"
#include "romp/custom_mesh.hpp"
#include "romp/flora.hpp"
#include "romp/geometrics.hpp"
#include "romp/texture_renderer.hpp"
#include "romp/time_of_day.hpp"
#include "romp_harness.hpp"
#include "speedtree/speedtree_renderer.hpp"
#include "tools_camera.hpp"
#include "ual/ual_manager.hpp"
#include "fmodsound/sound_manager.hpp"

static DogWatch s_detailTick( "detail_tick" );
static DogWatch s_detailDraw( "detail_draw" );

DECLARE_DEBUG_COMPONENT2( "Shell", 0 )

// Here are all our tokens to get the various components to link
extern int ChunkModel_token;
extern int ChunkLight_token;
extern int ChunkTerrain_token;
extern int ChunkFlare_token;
extern int ChunkWater_token;
extern int ChunkTree_token;
extern int ChunkParticles_token;

static int s_chunkTokenSet = ChunkModel_token | ChunkLight_token |
	ChunkTerrain_token | ChunkFlare_token | ChunkWater_token |
	ChunkTree_token | ChunkParticles_token;

extern int genprop_gizmoviews_token;
static int giz = genprop_gizmoviews_token;

extern int Math_token;
extern int PyScript_token;
extern int GUI_token;
extern int ResMgr_token;
static int s_moduleTokens =
	Math_token |
	PyScript_token |
	GUI_token |
	ResMgr_token;

extern int PyShimmerCountProvider_token;
extern int PyGraphicsSetting_token;
static int pyTokenSet = PyShimmerCountProvider_token | PyGraphicsSetting_token;

namespace PostProcessing
{
	extern int tokenSet;
	static int ppTokenSet = tokenSet;
}

//These are used for Script::tick and the BigWorld.callback fn.
static double g_totalTime = 0.0;

void incrementTotalTime( float dTime )
{
	g_totalTime += (double)dTime;
}

double getTotalTime()
{		
	return g_totalTime;
}

typedef ModuleManager ModuleFactory;

IMPLEMENT_CREATOR(MeModule, Module);

static AutoConfigString s_light_only( "system/lightOnlyEffect" ); 
static AutoConfigString s_shadowsXML( "system/shadowsXML" );

static bool s_enableScripting = true;
static const float NUM_FRAMES_AVERAGE = 16.f;

MeModule* MeModule::s_instance_ = NULL;

MeModule::MeModule()
	: selectionStart_( Vector2::zero() )
	, currentSelection_( GridRect::zero() )
	, localToWorld_( GridCoord::zero() )
	, averageFps_(0.0f)
	, scriptDict_(NULL)
	, renderingThumbnail_(false)
	, materialPreviewMode_(false)
	, rt_( NULL )
{
	BW_GUARD;

	ASSERT(s_instance_ == NULL);
	s_instance_ = this;

	lastCursorPosition_.x = 0;
	lastCursorPosition_.y = 0;
}

MeModule::~MeModule()
{
	BW_GUARD;

	ASSERT(s_instance_);
	delete wireframeRenderer_;
	s_instance_ = NULL;
}

bool MeModule::init( DataSectionPtr pSection )
{
	BW_GUARD;

	//Let's initialise the shadow renderer
	pShadowCommon_ = new ShadowCasterCommon;
	DataSectionPtr spSection = BWResource::instance().openSection( s_shadowsXML );
	pShadowCommon_->init( spSection );

	//Initialise the shadow caster
	pCaster_ = new ShadowCaster;
	if (!pCaster_->init( pShadowCommon_, false, 0, NULL ))
	{
		WARNING_MSG( "MeModule::init: Unable to initialise ShadowCaster, no "
			"shadows will be drawn\n" );
		pCaster_ = NULL;
		delete pShadowCommon_;
		pShadowCommon_ = NULL;
	}

	//Register the shadow caster for the flora
	EnviroMinderSettings::instance().shadowCaster( pCaster_.get() );

	//Make a MaterialDraw override for wireframe rendering
	std::string light_only = s_light_only;
	light_only = light_only.substr( 0, light_only.rfind(".") );
	wireframeRenderer_ = new MaterialDrawOverride( light_only, true, true );

	// Set callback for PyScript so it can know total game time
	Script::setTotalGameTimeFn( getTotalTime );
	
	return true;
}

void MeModule::onStart()
{
	BW_GUARD;

	if (s_enableScripting)
		initPyScript();

	cc_ = new ClosedCaptions();
	Commentary::instance().addView( &*cc_ );
	cc_->visible( true );
}


bool MeModule::initPyScript()
{
	BW_GUARD;

	// make a python dictionary here
	PyObject * pScript = PyImport_ImportModule("ModelEditorDirector");

	scriptDict_ = PyModule_GetDict(pScript);

	PyObject * pInit = PyDict_GetItemString( scriptDict_, "init" );
	if (pInit != NULL)
	{
		PyObject * pResult = PyObject_CallFunction( pInit, "" );

		if (pResult != NULL)
		{
			Py_DECREF( pResult );
		}
		else
		{
			PyErr_Print();
			return false;
		}
	}
	else
	{
		PyErr_Print();
		return false;
	}

	return true;
}


bool MeModule::finiPyScript()
{
	BW_GUARD;

	// make a python dictionary here
	PyObject * pScript = PyImport_ImportModule("ModelEditorDirector");

	scriptDict_ = PyModule_GetDict(pScript);

	PyObject * pFini = PyDict_GetItemString( scriptDict_, "fini" );
	if (pFini != NULL)
	{
		PyObject * pResult = PyObject_CallFunction( pFini, "" );

		if (pResult != NULL)
		{
			Py_DECREF( pResult );
		}
		else
		{
			PyErr_Print();
			return false;
		}
	}
	else
	{
		PyErr_Print();
		return false;
	}

	return true;
}



int MeModule::onStop()
{
	BW_GUARD;

	::ShowCursor( 0 );

	if (s_enableScripting)
		finiPyScript();

	//TODO: Work out why I can't call this here...
	//Save all options when exiting
	//Options::save();

	Py_DecRef(cc_.getObject());
	cc_ = NULL;

	pCaster_ = NULL;

	delete pShadowCommon_;
	pShadowCommon_ = NULL;

	rt_ = NULL;

	EnviroMinderSettings::instance().shadowCaster( NULL );
	return 0;
}


bool MeModule::updateState( float dTime )
{
	BW_GUARD;

	s_detailTick.start();

	static float s_lastDTime = 1.f/60.f;
	if (dTime != 0.f)
		dTime = dTime/NUM_FRAMES_AVERAGE + (NUM_FRAMES_AVERAGE-1.f)*s_lastDTime/NUM_FRAMES_AVERAGE;
	s_lastDTime = dTime;

	//Update the commentary
	cc_->update( dTime );
	
	SimpleGUI::instance().update( dTime );

	// update the camera.  this interprets the view direction from the mouse input
	MeApp::instance().camera()->update( dTime, true );

	// tick time and update the other components, such as romp
	MeShell::instance().romp().update( dTime, false );

	// gizmo manager
	POINT cursorPos = CMainFrame::instance().currentCursorPosition();
	Vector3 worldRay = CMainFrame::instance().getWorldRay(cursorPos.x, cursorPos.y);
	ToolPtr spTool = ToolManager::instance().tool();
	if (spTool)
	{
		spTool->calculatePosition( worldRay );
		spTool->update( dTime );
	}
	else if (GizmoManager::instance().update(worldRay))
		GizmoManager::instance().rollOver();

	// set input focus as appropriate
	bool acceptInput = CMainFrame::instance().cursorOverGraphicsWnd();
	CModelEditorApp::instance().mfApp()->handleSetFocus( acceptInput );
		
	ChunkManager::instance().tick( dTime );
	AmortiseChunkItemDelete::instance().tick();
	BgTaskManager::instance().tick();
	ProviderStore::tick( dTime );
	incrementTotalTime( dTime );
	Script::tick( getTotalTime() );

	speedtree::SpeedTreeRenderer::tick( dTime );

#if FMOD_SUPPORT
	//Tick FMod by setting the camera position
	Matrix view = MeApp::instance().camera()->view();
	view.invert();
	Vector3 cameraPosition = view.applyToOrigin();
	Vector3 cameraDirection = view.applyToUnitAxisVector( 2 );
	Vector3 cameraUp = view.applyToUnitAxisVector( 1 );
	SoundManager::instance().setListenerPosition(
		cameraPosition, cameraDirection, cameraUp, dTime );
	
	s_detailTick.stop();
#endif // FMOD_SUPPORT

	return true;
}

void MeModule::beginRender()
{
	BW_GUARD;

	Moo::Camera cam = Moo::rc().camera();
	cam.nearPlane( Options::getOptionFloat( "render/misc/nearPlane", 0.01f ));
	cam.farPlane( Options::getOptionFloat( "render/misc/farPlane", 300.f ));
	Moo::rc().camera( cam );
	
	if (Moo::rc().mixedVertexProcessing())
		Moo::rc().device()->SetSoftwareVertexProcessing( TRUE );

	Moo::rc().nextFrame();
	Moo::rc().reset();
	Moo::rc().updateViewTransforms();
	Moo::rc().updateProjectionMatrix();
}

bool MeModule::renderThumbnail( const std::string& fileName )
{
	BW_GUARD;

	std::string modelName = BWResource::resolveFilename( fileName );
	std::replace( modelName.begin(), modelName.end(), '/', '\\' );
	std::wstring screenShotName = bw_utf8tow( BWResource::removeExtension( modelName ) );
	screenShotName += L".thumbnail.jpg";
		
	if (!rt_)
	{
		rt_ = new Moo::RenderTarget( "Model Thumbnail" );
		if (!rt_ || !rt_->create( 128, 128 ))
		{
			WARNING_MSG( "Could not create render target for model thumbnail.\n" );
			rt_ = NULL;
			return false;
		}
	}
	
	if ( rt_->push() )
	{
		//Use the same aspect ratio for the camera as the render target (1:1)
		Moo::Camera cam = Moo::rc().camera();
		cam.aspectRatio( 1.f );
		Moo::rc().camera( cam );

		updateFrame( 0.f );
		
		Moo::rc().beginScene();
		renderingThumbnail_ = true;

		DX::Viewport viewport;
		viewport.Width = (DWORD)Moo::rc().screenWidth();
		viewport.Height = (DWORD)Moo::rc().screenHeight();
		viewport.MinZ = 0.f;
		viewport.MaxZ = 1.f;
		viewport.X = 0;
		viewport.Y = 0;
		Moo::rc().setViewport( &viewport );

		//Make sure the colour channels are enabled for alpha blended models
		Moo::rc().setRenderState( D3DRS_COLORWRITEENABLE, 
			D3DCOLORWRITEENABLE_RED |
			D3DCOLORWRITEENABLE_GREEN |
			D3DCOLORWRITEENABLE_BLUE );

		Moo::rc().setWriteMask( 1, D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_ALPHA );

		Moo::rc().device()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
			RGB( 255, 255, 255), 1, 0 );

		render( 0.f );

		renderingThumbnail_ = false;
		Moo::rc().endScene();

		HRESULT hr = D3DXSaveTextureToFile( screenShotName.c_str(),
			D3DXIFF_JPG, rt_->pTexture(), NULL );

		rt_->pop();

		if ( FAILED( hr ) )
		{
			WARNING_MSG( "Could not create model thumbnail \"%s\" (D3D error 0x%x).\n", screenShotName.c_str(), hr);
			return false;
		}

		PageObject::currPage()->OnUpdateThumbnail();
		
		UalManager::instance().updateItem( bw_utf8tow( modelName ) );

		return true;
	}
	
	return false;
}

void MeModule::render( float dTime )
{
	BW_GUARD;

	s_detailDraw.start();
	static float s_lastDTime = 1.f/60.f;
	if (dTime != 0.f)
		dTime = dTime/NUM_FRAMES_AVERAGE + (NUM_FRAMES_AVERAGE-1.f)*s_lastDTime/NUM_FRAMES_AVERAGE;
	s_lastDTime = dTime;

	bool groundModel = !!Options::getOptionInt( "settings/groundModel", 0 );
	bool centreModel = !!Options::getOptionInt( "settings/centreModel", 0 );
	bool showLightModels = !!Options::getOptionInt( "settings/showLightModels", 1 );
	bool useCustomLighting = !!Options::getOptionInt( "settings/useCustomLighting", 0 );
	bool checkForSparkles = !!Options::getOptionInt( "settings/checkForSparkles", 0 );
	bool showAxes = !checkForSparkles && !!Options::getOptionInt( "settings/showAxes", 0 );
	bool showBoundingBox = !checkForSparkles && !!Options::getOptionInt( "settings/showBoundingBox", 0 );
	bool useTerrain = !checkForSparkles && !!Options::getOptionInt( "settings/useTerrain", 1 );
	bool useFloor = !checkForSparkles && !useTerrain && !!Options::getOptionInt( "settings/useFloor", 1 );
	bool showBsp = !checkForSparkles && !!Options::getOptionInt( "settings/showBsp", 0 );
	bool special = checkForSparkles || showBsp;
	bool showModel = special || !!Options::getOptionInt( "settings/showModel", 1 );
	bool showWireframe = !special && !!Options::getOptionInt( "settings/showWireframe", 0 );
	bool showSkeleton = !checkForSparkles && !!Options::getOptionInt( "settings/showSkeleton", 0 );
	bool showPortals = !special && !!Options::getOptionInt( "settings/showPortals", 0 );
	bool showShadowing = showModel && !showWireframe && !showBsp &&
			!!Options::getOptionInt( "settings/showShadowing", 1 ) && pCaster_ &&
			!MeApp::instance().mutant()->isSkyBox();
	bool showOriginalAnim = !special && !!Options::getOptionInt( "settings/showOriginalAnim", 0 );
	bool showNormals = !special && showModel && !!Options::getOptionInt( "settings/showNormals", 0 );
	bool showBinormals = !special && showModel && !!Options::getOptionInt( "settings/showBinormals", 0 );
	bool showHardPoints = !special && showModel && !!Options::getOptionInt( "settings/showHardPoints", 0 );
	bool showCustomHull = !special && showModel && !!Options::getOptionInt( "settings/showCustomHull", 0 );
	bool showEditorProxy = !special && showModel && !!Options::getOptionInt( "settings/showEditorProxy", 0 );
	bool showActions = !special && showModel && !!Options::getOptionInt( "settings/showActions", 0 );

	Vector3 bkgColour = Options::getOptionVector3( "settings/bkgColour", Vector3( 255.f, 255.f, 255.f )) / 255.f;

	float modelDist = 0.f;
	int numTris = 0;
	bool posChanged = true;
	
	if ( MeApp::instance().mutant()->modelName() != "" )
	{
		MeApp::instance().mutant()->groundModel( groundModel );
		MeApp::instance().mutant()->centreModel( centreModel );
		MeApp::instance().mutant()->updateActions( dTime );
		MeApp::instance().mutant()->updateFrameBoundingBox();

		if (!materialPreviewMode_)
		{
			static bool s_last_groundModel = !groundModel;
			static bool s_last_centreModel = !centreModel;
			if ((s_last_groundModel != groundModel) || (s_last_centreModel != centreModel))
			{
				//Do this to ensure the model nodes are up to date
				MeApp::instance().mutant()->drawModel();
				MeApp::instance().camera()->boundingBox(
					MeApp::instance().mutant()->zoomBoundingBox() );
				s_last_groundModel = groundModel;
				s_last_centreModel = centreModel;
				MeApp::instance().camera()->update();
				bool posChanged = true;
				Moo::SortedChannel::draw();
			}
		}
	}

	MeApp::instance().camera()->render( dTime );
	
	TextureRenderer::updateDynamics( dTime );
	
	beginRender();

	MeShell::instance().romp().drawPreSceneStuff( checkForSparkles, useTerrain );

	//Do this so that the skeleton is defined if the model is not drawn
	if (((!showModel) || (showBsp)) && showSkeleton)
	{
		modelDist = MeApp::instance().mutant()->drawModel();
	}

	Moo::RenderContext& rc = Moo::rc();
	DX::Device* pDev = rc.device();
	
	rc.device()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
			Moo::Colour( 
			checkForSparkles ? 1.f : bkgColour[0], 
			checkForSparkles ? 1.f : bkgColour[1], 
			checkForSparkles ? 1.f : bkgColour[2],
			0.f), 1, 0 );

	rc.setVertexShader( NULL );
	rc.setPixelShader( NULL );
	rc.device()->SetPixelShader( NULL );

	rc.setRenderState( D3DRS_CLIPPING, TRUE );
	rc.setRenderState( D3DRS_LIGHTING, FALSE );	

	static Vector3 N(0.0, 0.0, 0.0);
	static Vector3 X(1.0, 0.0, 0.0); 
	static Vector3 Y(0.0, 1.0, 0.0); 
	static Vector3 Z(0.0, 0.0, 1.0); 
	static Moo::Colour colourRed( 0.5f, 0.f, 0.f, 1.f );
	static Moo::Colour colourGreen( 0.f, 0.5f, 0.f, 1.f );
	static Moo::Colour colourBlue( 0.f, 0.f, 0.5f, 1.f );

	this->setLights(checkForSparkles, useCustomLighting);
	
	// Use the sun direction as the shadowing direction.
	EnviroMinder & enviro = ChunkManager::instance().cameraSpace()->enviro();
	Vector3 lightTrans = enviro.timeOfDay()->lighting().sunTransform[2];

	if ((useCustomLighting) && (MeApp::instance().lights()->lightContainer()->directionals().size()))
	{
		lightTrans = Vector3(0,0,0) - MeApp::instance().lights()->lightContainer()->directional(0)->direction();
	}

	if (useTerrain)
	{
		Moo::EffectVisualContext::instance().initConstants();
		
		speedtree::SpeedTreeRenderer::beginFrame( &enviro );
		
		this->renderChunks();

		speedtree::SpeedTreeRenderer::endFrame();

		// chunks don't keep the same light container set
		this->setLights(checkForSparkles, useCustomLighting);

		this->renderTerrain( dTime, showShadowing );

		// draw sky etc.
		MeShell::instance().romp().drawDelayedSceneStuff();
	}

	//Make sure we restore valid texture stages before continuing
	rc.setTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
	rc.setTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	rc.setTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	rc.setTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	rc.setTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

	// Draw the axes
	if (showAxes)
	{
		Geometrics::drawLine(N, X, colourGreen);
		Geometrics::drawLine(N, Y, colourBlue);
		Geometrics::drawLine(N, Z, colourRed);
	}

	this->setLights(checkForSparkles, useCustomLighting);

	if (useFloor)
	{
		MeApp::instance().floor()->render();
	}

	if (showBsp)
	{
		rc.lightContainer( MeApp::instance().whiteLight() );
		rc.specularLightContainer( MeApp::instance().blackLight() );
	}

	if ((showWireframe) && (!materialPreviewMode_))
	{
		rc.setRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
		rc.lightContainer( MeApp::instance().blackLight() );
		rc.specularLightContainer( MeApp::instance().blackLight() );
		
		// Use a lightonly renderer in case the material will render nothing (e.g. some alpha shaders)
		Moo::Visual::s_pDrawOverride = wireframeRenderer_; 
	}
	else
	{
		rc.setRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	}

	if (materialPreviewMode_)
	{
		if (PageMaterials::currPage()->previewObject())
		{
			PageMaterials::currPage()->previewObject()->draw();
			if (showBoundingBox)
			{
				Geometrics::wireBox( PageMaterials::currPage()->previewObject()->boundingBox(), 0x000000ff );
			}
		}
	}
	else if (MeApp::instance().mutant())
	{
		numTris = rc.liveProfilingData().nPrimitives_;
		// Render the model
		int renderState = 0;
		renderState |= showModel ? SHOW_MODEL : 0;
		renderState |= showBoundingBox ? SHOW_BB : 0;
		renderState |= showSkeleton ? SHOW_SKELETON : 0;
		renderState |= showPortals ? SHOW_PORTALS : 0;
		renderState |= showBsp ? SHOW_BSP : 0;
		renderState |= showEditorProxy ? SHOW_EDITOR_PROXY : 0;
		renderState |= showNormals ? SHOW_NORMALS : 0;
		renderState |= showBinormals ? SHOW_BINORMALS : 0;
		renderState |= showHardPoints ? SHOW_HARD_POINTS : 0;
		renderState |= showCustomHull ? SHOW_CUSTOM_HULL : 0;
		renderState |= showActions ? SHOW_ACTIONS : 0;
		Moo::rc().updateProjectionMatrix();
		modelDist = MeApp::instance().mutant()->render( dTime, renderState );

		numTris = rc.liveProfilingData().nPrimitives_ - numTris;
		
		if (showShadowing)
		{
			//TODO: Move the following line so that the watcher can work correctly.
			//Set the shadow quailty
			pShadowCommon_->shadowQuality( Options::getOptionInt( "renderer/shadows/quality", 2 )  );

			// Render to the shadow caster
			pCaster_->begin( MeApp::instance().mutant()->shadowVisibililtyBox(), lightTrans );
			MeApp::instance().mutant()->drawModel( false, modelDist );
			pCaster_->end();
		}
	}

	//Make sure we restore these after we are done
	rc.setRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	Moo::Visual::s_pDrawOverride = NULL;

	this->setLights(checkForSparkles, useCustomLighting);
		
	// render the other components, such as romp and flora.
	MeShell::instance().romp().drawPostSceneStuff( useTerrain, useTerrain, showShadowing );

	if ((!materialPreviewMode_) && (showShadowing))
	{
		speedtree::SpeedTreeRenderer::beginFrame( &enviro );
		
		// Start the shadow rendering and render the shadow onto the terrain (if required)
		pCaster_->beginReceive( useTerrain );

		speedtree::SpeedTreeRenderer::endFrame();

		//Do self shadowing
		if (MeApp::instance().mutant())
		{
			MeApp::instance().mutant()->render( 0, showModel ? SHOW_MODEL : 0 );
		}

		//Shadow onto the floor
		if (useFloor)
		{
			MeApp::instance().floor()->render( pShadowCommon_->pReceiverOverride()->pRigidEffect_ );
		}

		pCaster_->endReceive();
	}

	if (!renderingThumbnail_)
	{	
		if ((showOriginalAnim) && (!materialPreviewMode_) && (MeApp::instance().mutant()))
		{
			Moo::LightContainerPtr lc = rc.lightContainer();

			rc.setRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
			Moo::Visual::s_pDrawOverride = wireframeRenderer_; 

			rc.lightContainer( MeApp::instance().blackLight() );
			rc.specularLightContainer( MeApp::instance().blackLight() );

			MeApp::instance().mutant()->drawOriginalModel();

			rc.setRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
			Moo::Visual::s_pDrawOverride = NULL;

			rc.lightContainer( lc );
			rc.specularLightContainer( lc );
		}
		
		if (showLightModels)
		{
			// gizmo me
			ToolPtr spTool = ToolManager::instance().tool();
			if (spTool)
			{
				spTool->render();
			}

			GizmoManager::instance().draw();
		}
		
		SimpleGUI::instance().draw();

		std::wstring fps;
		
		// This is done every frame to have constant fluid frame rate.
		if (dTime != 0.f)
		{
			fps = Localise(L"MODELEDITOR/APP/ME_MODULE/POSITIVE_FPS", Formatter( 1.f/dTime, L"%.1f" ) );
		}
		else
		{
			fps = Localise(L"MODELEDITOR/APP/ME_MODULE/ZERO_FPS");
		}
		CMainFrame::instance().setStatusText( ID_INDICATOR_FRAMERATE, fps.c_str() );

		static int s_lastNumTris = -1;
		if (numTris != s_lastNumTris)
		{
			CMainFrame::instance().setStatusText( ID_INDICATOR_TRIANGLES, Localise(L"MODELEDITOR/APP/ME_MODULE/TRIANGLES", numTris) );
			s_lastNumTris = numTris;
		}
	}

	Chunks_drawCullingHUD();

	// render the camera with the new view direction
	endRender();
	s_detailDraw.stop();
}

void MeModule::setLights( bool checkForSparkles, bool useCustomLighting )
{
	BW_GUARD;

	if (checkForSparkles)
	{
		MeApp::instance().lights()->unsetLights();

		Moo::rc().lightContainer( MeApp::instance().blackLight() );
		Moo::rc().specularLightContainer( MeApp::instance().blackLight() );
	}
	else if (useCustomLighting)
	{
		MeApp::instance().lights()->setLights();
	}
	else // Game Lighting
	{
		MeApp::instance().lights()->unsetLights();

		Moo::LightContainerPtr lc = new Moo::LightContainer;
		lc->addDirectional( ChunkManager::instance().cameraSpace()->sunLight() );
		lc->ambientColour( ChunkManager::instance().cameraSpace()->ambientLight() );
		Moo::rc().lightContainer( lc );
		Moo::rc().specularLightContainer( lc );		
	}
}

void MeModule::renderChunks()
{
	BW_GUARD;

	Moo::rc().setRenderState( D3DRS_FILLMODE,
		Options::getOptionInt( "render/scenery/wireFrame", 0 ) ?
			D3DFILL_WIREFRAME : D3DFILL_SOLID );

	ChunkManager::instance().camera( Moo::rc().invView(), *&(ChunkManager::instance().cameraSpace()) );	
	ChunkManager::instance().draw();

	Moo::rc().setRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
}

void MeModule::renderTerrain( float dTime /* = 0.f */, bool shadowing /* = false */ )
{
	BW_GUARD;

	if (Options::getOptionInt( "render/terrain", 1 ))
	{
		// draw terrain
		bool canSeeTerrain = 
			Terrain::BaseTerrainRenderer::instance()->canSeeTerrain();

		// Set camera position for terrain, so lods can be set up per frame. 
		Terrain::BasicTerrainLodController::instance().setCameraPosition( 
			Moo::rc().invView().applyToOrigin() );

		Moo::rc().setRenderState( D3DRS_FILLMODE,
			Options::getOptionInt( "render/terrain/wireFrame", 0 ) ?
				D3DFILL_WIREFRAME : D3DFILL_SOLID );

		Terrain::BaseTerrainRenderer::instance()->drawAll();

		Moo::rc().setRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	}
	else
	{
		Terrain::BaseTerrainRenderer::instance()->clearBlocks();
	}
}


void MeModule::endRender()
{
	//Moo::rc().endScene();
}


bool MeModule::handleKeyEvent( const KeyEvent & event )
{
	BW_GUARD;

	// usually called through py script
	bool handled = MeApp::instance().camera()->handleKeyEvent(event);

	if (event.key() == KeyCode::KEY_LEFTMOUSE)
	{
		if (event.isKeyDown())
		{
			handled = true;

			GizmoManager::instance().click();
		}
	}

	return handled;
}

bool MeModule::handleMouseEvent( const MouseEvent & event )
{
	BW_GUARD;

	// Its ugly, but for the mean time...it needs to be done in CPP
	if ( InputDevices::isKeyDown( KeyCode::KEY_SPACE ) && ( event.dz() != 0 ) )
	{
		// Scrolling mouse wheel while holding space: change camera speed
		std::string currentSpeed = Options::getOptionString( "camera/speed", "Slow" );
		if ( event.dz() > 0 )
		{
			if ( currentSpeed == "Slow" )
				Options::setOptionString( "camera/speed", "Medium" );
			else if ( currentSpeed == "Medium" )
				Options::setOptionString( "camera/speed", "Fast" );
			else if ( currentSpeed == "Fast" )
				Options::setOptionString( "camera/speed", "SuperFast" );
		}
		if ( event.dz() < 0 )
		{
			if ( currentSpeed == "Medium" )
				Options::setOptionString( "camera/speed", "Slow" );
			else if ( currentSpeed == "Fast" )
				Options::setOptionString( "camera/speed", "Medium" );
			else if ( currentSpeed == "SuperFast" )
				Options::setOptionString( "camera/speed", "Fast" );
		}
		std::string newSpeed = Options::getOptionString( "camera/speed", currentSpeed );
		float speed = newSpeed == "Medium" ? 8.f : ( newSpeed == "Fast" ? 24.f : ( newSpeed == "SuperFast" ? 48.f : 1.f ) );
		MeApp::instance().camera()->speed( Options::getOptionFloat( "camera/speed/"+newSpeed, speed ) );
		MeApp::instance().camera()->turboSpeed( Options::getOptionFloat( "camera/speed/"+newSpeed+"/turbo", 2*speed ) );
		GUI::Manager::instance().update();
		return true;
	}
	else
		return MeApp::instance().camera()->handleMouseEvent(event);
}

Vector2 MeModule::currentGridPos()
{
	BW_GUARD;

	POINT pt = MeShell::instance().currentCursorPosition();
	Vector3 cursorPos = Moo::rc().camera().nearPlanePoint(
			(float(pt.x) / Moo::rc().screenWidth()) * 2.f - 1.f,
			1.f - (float(pt.y) / Moo::rc().screenHeight()) * 2.f );

	Matrix view;
	view.setTranslate( viewPosition_ );

	Vector3 worldRay = view.applyVector( cursorPos );
	worldRay.normalise();

	PlaneEq gridPlane( Vector3(0.f, 0.f, 1.f), .0001f );

	Vector3 gridPos = gridPlane.intersectRay( viewPosition_, worldRay );

	return Vector2( gridPos.x, gridPos.y );
}


Vector3 MeModule::gridPosToWorldPos( Vector2 gridPos )
{
	Vector2 w = (gridPos + Vector2( float(localToWorld_.x), float(localToWorld_.y) )) *
		GRID_RESOLUTION;

	return Vector3( w.x, 0, w.y);
}



// ----------------
// Python Interface
// ----------------

/**
 *	The static python render method
 */
PyObject * MeModule::py_render( PyObject * args )
{
	BW_GUARD;

	float dTime = 0.033f;

	if (!PyArg_ParseTuple( args, "|f", &dTime ))
	{
		PyErr_SetString( PyExc_TypeError, "ModelEditor.render() "
			"takes only an optional float argument for dtime" );
		return NULL;
	}

	s_instance_->render( dTime );

	Py_Return;
}
