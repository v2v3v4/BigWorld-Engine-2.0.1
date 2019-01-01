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
#include "facade_app.hpp"

#include "app.hpp"
#include "canvas_app.hpp"
#include "connection_control.hpp"
#include "device_app.hpp"
#include "entity_picker.hpp"
#include "player.hpp"
#include "shadow_manager.hpp"
#include "time_globals.hpp"
#include "world_app.hpp"

#include "chunk/chunk_manager.hpp"

#include "connection/server_connection.hpp"

#include "duplo/foot_print_renderer.hpp"
#include "romp/lens_effect_manager.hpp"

#include "moo/visual_channels.hpp"

#include "pyscript/personality.hpp"
#include "pyscript/py_data_section.hpp"

#include "romp/distortion.hpp"
#include "romp/geometrics.hpp"
#include "romp/progress.hpp"
#include "romp/rain.hpp"
#include "romp/water.hpp"


static DogWatch g_sortedWatch( "DrawSorted" );


FacadeApp FacadeApp::instance;

int FacadeApp_token = 1;

PROFILER_DECLARE( AppDraw_Facade, "AppDraw Facade" );


/**
 *	This class sets the time of day on the server when it changes
 *	on the client.
 */
class ServerTODUpdater : public TimeOfDay::UpdateNotifier
{
public:
	ServerTODUpdater( ServerConnection * pSC ) : pSC_( pSC ) { }

	virtual void updated( const TimeOfDay & tod )
	{
		PyObject * pPers = Personality::instance();
		if (pPers == NULL) return;

		Script::call(
			PyObject_GetAttrString( pPers, "onTimeOfDayLocalChange" ),
			Py_BuildValue( "(ff)", tod.gameTime(), tod.secondsPerGameHour() ),
			"ServerTODUpdater notification: ",
			true );
	}

private:
	ServerConnection	* pSC_;
};



FacadeApp::FacadeApp() : dTime_( 0.f )
{
	BW_GUARD;
	MainLoopTasks::root().add( this, "Facade/App", NULL );
}


FacadeApp::~FacadeApp()
{
	BW_GUARD;
	/*MainLoopTasks::root().del( this, "Facade/App" );*/
}


bool FacadeApp::init()
{
	BW_GUARD;
	EntityPicker::instance().selectionFoV( DEG_TO_RAD( 10.f ) );
	EntityPicker::instance().selectionDistance( 80.f );
	EntityPicker::instance().deselectionFoV( DEG_TO_RAD( 80.f ) );

	todUpdateNotifier_ =
		new ServerTODUpdater( ConnectionControl::serverConnection() );

	Waters::instance().init();
	FootPrintRenderer::init();
	LensEffectManager::init();

	return DeviceApp::s_pStartupProgTask_->step(APP_PROGRESS_STEP);
}


void FacadeApp::fini()
{
    BW_GUARD;
	Waters::instance().fini();
	FootPrintRenderer::fini();

	if ( lastCameraSpace_.exists() )
	{
		EnviroMinder & enviro = lastCameraSpace_->enviro();
		enviro.deactivate();
	}

	// throw away this reference
	lastCameraSpace_ = NULL;

	LensEffectManager::fini();
}


/*~ callback Personality.onCameraSpaceChange
 *
 *	This callback method is called on the personality script when
 *	the camera has moved to a new space, and the space's environment
 *	has been set up.  The space ID and the space.settings file is
 *	passed in as arguments.
 */
void FacadeApp::tick( float dTime )
{
	BW_GUARD;
	dTime_ = dTime;

	// update the entity picker
	//  (must be here, after the camera position has been set)
	EntityPicker::instance().update( dTime );

	// update the weather
	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (!pSpace) return;

	EnviroMinder & enviro = pSpace->enviro();

	// if this space is new to us than transfer our Facade apparatus to it
	if (lastCameraSpace_ != pSpace)
	{
		// removing it from the old one first if necessary
		if (lastCameraSpace_)
		{
			EnviroMinder & oldEnviro = lastCameraSpace_->enviro();
			oldEnviro.deactivate();
			oldEnviro.timeOfDay()->delUpdateNotifier( todUpdateNotifier_ );
		}

		lastCameraSpace_ = pSpace;
		enviro.activate();
		enviro.timeOfDay()->addUpdateNotifier( todUpdateNotifier_ );

		// Probably not the ideal spot for this now.
		TimeGlobals::instance().setupWatchersFirstTimeOnly();

		// Inform the personality script that the space has changed.
		PyObject * pPers = Personality::instance();
		if (pPers != NULL)
		{
			PyObject* enviroData;
			if (enviro.pData())
			{
				enviroData = new PyDataSection(enviro.pData());
			}
			else
			{
				enviroData = Py_None;
			}

			Script::call(
				PyObject_GetAttrString( pPers, "onCameraSpaceChange" ),
				Py_BuildValue( "iO", pSpace->id(), enviroData ),
				"Personality Camera Space Change notification: ",
				true );
		}
	}

	enviro.tick( dTime, isCameraOutside() );

	// play thunder if there is any
	const Vector4 & thunder = enviro.thunder();
	if (thunder.w < 1.f)
	{
		// TODO: Re-enable this sound with the new FMOD API
		// BWSound::playThunder( Vector3( (const float*)thunder ), thunder.w );
	}
}


void FacadeApp::draw()
{
	BW_GUARD_PROFILER( AppDraw_Facade );

	// Draw the main batch of sorted triangles
	Moo::rc().setRenderState( D3DRS_FILLMODE,
		(WorldApp::instance.wireFrameStatus_ & 2) ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	if (WorldApp::instance.debugSortedTriangles_ % 4)
	{
		switch ( WorldApp::instance.debugSortedTriangles_ % 4 )
		{
		case 1:
			Moo::rc().device()->Clear( 0, NULL, D3DCLEAR_ZBUFFER, 0x00, 1, 0 );

			Moo::Material::setVertexColour();
			Moo::rc().setRenderState( D3DRS_ZENABLE, FALSE );
			Moo::rc().setRenderState( D3DRS_ZFUNC, D3DCMP_ALWAYS );
			Moo::rc().setTexture(0, Moo::TextureManager::instance()->get(s_blackTexture)->pTexture());
			Geometrics::texturedRect( Vector2(0.f,0.f),
					Vector2(Moo::rc().screenWidth(),Moo::rc().screenHeight()),
					Moo::Colour( 1.f,1.f,1.f, 0.75f ), true );
			break;
		case 2:
			Moo::rc().device()->Clear( 0, NULL, D3DCLEAR_ZBUFFER, 0x00, 1, 0 );

			Moo::Material::setVertexColour();
			Moo::rc().setRenderState( D3DRS_BLENDOP, D3DBLENDOP_SUBTRACT );
			Moo::rc().setRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
			Moo::rc().setRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
			Moo::rc().setRenderState( D3DRS_ZENABLE, FALSE );
			Moo::rc().setRenderState( D3DRS_ZFUNC, D3DCMP_ALWAYS );
			Moo::rc().setTexture(0, Moo::TextureManager::instance()->get(s_blackTexture)->pTexture());
			Geometrics::texturedRect( Vector2(0,0),
					Vector2(Moo::rc().screenWidth(),Moo::rc().screenHeight()),
					Moo::Colour( 0.f, 1.f, 1.f, 1.f ), true );
			Moo::rc().setRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
			break;
		case 3:
			Moo::rc().device()->Clear( 0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, 0x00000000, 1, 0 );

			Moo::Material::setVertexColour();
			Moo::rc().setRenderState( D3DRS_ZENABLE, FALSE );
			Moo::rc().setRenderState( D3DRS_ZFUNC, D3DCMP_ALWAYS );
			Moo::rc().setTexture(0, Moo::TextureManager::instance()->get(s_blackTexture)->pTexture());
			Geometrics::texturedRect( Vector2(0.f,0.f),
					Vector2(Moo::rc().screenWidth(),Moo::rc().screenHeight()),
					Moo::Colour( 1.f,1.f,1.f, 1.f ), true );
			break;
		}
	}
	Moo::rc().setRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (pSpace)
	{
		// Draw waters
		Waters& phloem = Waters::instance();
		phloem.rainAmount( pSpace->enviro().rain()->amount() );

		if (Player::instance().entity())
		{
			phloem.playerPos( Player::instance().entity()->position() );
			WaterSceneRenderer::setPlayerModel( Player::instance().entity()->pPrimaryModel() );
		}

		// Update the water simulations
		phloem.updateSimulations( dTime_ );

		bool canDrawFlora = WorldApp::instance.canSeeTerrain();

		// Draw the forward part of our environment including flora
		pSpace->enviro().drawFore( dTime_, true, canDrawFlora, false, false, true );

		// Render the shadowing onto the flora
		if(canDrawFlora)
			ShadowManager::instance().renderFloraShadows( pSpace->enviro().flora() );

		// Draw the distortion buffer (including water)
		CanvasApp::instance.updateDistortionBuffer();

		pSpace->enviro().drawFore( dTime_, true, WorldApp::instance.canSeeTerrain(), false, true, false );
	}

	// Draw the sorted triangles
	g_sortedWatch.start();
	Moo::SortedChannel::draw();
	g_sortedWatch.stop();
}



// facade_app.cpp
