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
#include "enviro_minder.hpp"

#ifndef CODE_INLINE
#include "enviro_minder.ipp"
#endif

#include "cstdmf/debug.hpp"
DECLARE_DEBUG_COMPONENT2( "Romp", 0 )

#include "resmgr/datasection.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/auto_config.hpp"

#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_obstacle.hpp"
#include "duplo/foot_print_renderer.hpp"
#include "duplo/decal.hpp"

#include "terrain/terrain_settings.hpp"

#include "time_of_day.hpp"
#include "weather.hpp"
#include "sky_gradient_dome.hpp"
#include "sun_and_moon.hpp"
#include "sea.hpp"
#include "rain.hpp"
#include "flora.hpp"
#include "water.hpp"
#include "environment_cube_map.hpp"
#include "sky_light_map.hpp"
#include "sky_dome_occluder.hpp"
#include "sky_dome_shadows.hpp"
#include "z_buffer_occluder.hpp"
#include "lens_effect_manager.hpp"
#include "moo/visual_manager.hpp"
#include "moo/cube_render_target.hpp"
#include "moo/material.hpp"
#include "moo/visual_channels.hpp"
#include "particle/particle_system_manager.hpp"
#include "particle/particle_system.hpp"

#include <limits>

//TODO : unsupported feature at the moment.  to be finished.
//#include "dapple.hpp"

#define SKY_LIGHT_MAP_ENABLE 1

#ifdef EDITOR_ENABLED
	extern bool g_disableSkyLightMap;
#endif

#include "math/colour.hpp"

static AutoConfigString s_floraXML( "environment/floraXML" );


namespace // anonymous
{
    /**
    *	A little helper function that should go in cstdmf
    */
    template <class C> inline void del_safe( C * & object )
    {
	    if (object != NULL)
	    {
		    delete object;
		    object = NULL;
	    }
    }
	
	// Multiplier for near and far planes while rendering decals and footprints.
	float decalClipPlaneBias= 1.01f;

	class WatcherInitialiser
	{
	public:
		WatcherInitialiser()
		{
			MF_WATCH( "Render/DecalClipPlaneBias", decalClipPlaneBias, 
				Watcher::WT_READ_WRITE, 
				"Multiplier for near and far planes while rendering decals"
				" and footprints." );
		}
	};

	WatcherInitialiser s_watcherInitializer;
} // namespace anonymous 


//---------------------------------------------------------------------------
// Section: SkyBoxScopedViewport
//---------------------------------------------------------------------------
SkyBoxScopedSetup::SkyBoxScopedSetup():
	cps_( 1.f, 2000.f )
{
	// Get old viewport
	Moo::rc().getViewport( &oldVp_ );

	// Draw to the far plane
	D3DVIEWPORT9 vp = oldVp_;
	vp.MinZ	= 1.0f;
	vp.MaxZ	= 2.0f;

	Moo::rc().setViewport( &vp );
	FogController::instance().commitFogToDevice();
}


SkyBoxScopedSetup::~SkyBoxScopedSetup()
{
	Moo::rc().setViewport( &oldVp_ );
	//call dtor explicitly so near/far planes restored
	//before we reset the fog values.
	cps_.~CameraPlanesSetter();
	FogController::instance().commitFogToDevice();
}


// -----------------------------------------------------------------------------
// Section: PlayerAttachments
// -----------------------------------------------------------------------------


/**
 *	This method adds a wannabe attachment to our list
 */
void PlayerAttachments::add( PyMetaParticleSystem * pSys, const std::string & node )
{
	PlayerAttachment pa;
	pa.pSystem = pSys;
	pa.onNode = node;
	this->push_back( pa );
}


// -----------------------------------------------------------------------------
// Section: SkyBoxController
// -----------------------------------------------------------------------------
/**
 *	This class exposes a Vector4 the effect file engine, which
 *	provides python control over sky box rendering.
 */
class SkyBoxController : public Moo::EffectConstantValue, public Aligned
{
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{		
		pEffect->SetVector(constantHandle, (const Vector4*)&value_);
		return true;
	}

public:
	void value( const Vector4& v )
	{		
		value_ = v;
	}	

private:
	Vector4 value_;
};


// -----------------------------------------------------------------------------
// Section: WindAnimation
// -----------------------------------------------------------------------------
/**
 *	This class exposes a Vector4 to the effect file engine, which
 *	provides a wind animation value that can be used to blow texture coords
 *	around (in x,y) and also provides the current wind average speed (in z,w)
 */
class WindAnimation : public Moo::EffectConstantValue
{
public:
	WindAnimation():
		value_(0.f,0.f,0.f,0.f)
	{
	}

	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{		
		pEffect->SetVector(constantHandle, (const Vector4*)&value_);
		return true;
	}

	void tick( float dTime, const Vector2& windAverage )
	{
		value_.x += dTime * windAverage.x;
		value_.y += dTime * windAverage.y;
		value_.z = windAverage.x;
		value_.w = windAverage.y;		
	}

private:
	Vector4 value_;
};


// -----------------------------------------------------------------------------
// Section: EnviroMinder
// -----------------------------------------------------------------------------

EnviroMinder* EnviroMinder::s_activatedEM_ = NULL;
static SkyBoxController* s_skyBoxController = NULL;
static WindAnimation* s_windAnimation = NULL;

/**
 *	Constructor.
 */
EnviroMinder::EnviroMinder(ChunkSpaceID id) :
	timeOfDay_( new TimeOfDay( 0.f ) ),
	weather_( new Weather() ),
	skyGradientDome_( new SkyGradientDome() ),
	sunAndMoon_( new SunAndMoon() ),
	seas_( new Seas() ),
	rain_( new Rain() ),
	flora_( new Flora() ),
	decal_( new Decal ),
	environmentCubeMap_( new EnvironmentCubeMap() ),
#ifndef EDITOR_ENABLED
	footPrintRenderer_( new FootPrintRenderer(id) ),
#endif // EDITOR_ENABLED
	//TODO : unsupported feature at the moment.  to be finished.
	//dapple_( new Dapple() ),
	thunder_( 0.f, 0.f, 0.f, 0.f ),
	playerDead_( false ),
	data_( NULL ),
	skyDomeOccluder_( NULL ),
	skyDomesPartition_( 0 ),
	zBufferOccluder_( NULL ),
	chunkObstacleOccluder_( NULL ),
	farPlaneBaseLine_(500.0),
	farPlane_(500.0),
	weatherControl_(NULL),
	sunlightControl_(NULL),
	ambientControl_(NULL),
	fogControl_(NULL),
	allowUpdate_( true )	
{
	// Create Sky Light Map
#ifdef SKY_LIGHT_MAP_ENABLE
	skyLightMap_ = new SkyLightMap;
	skyDomeShadows_ = new SkyDomeShadows(*this);
	skyLightMap_->addContributor(*skyDomeShadows_);
#endif

	// Create Sky Box Controller
	if (!s_skyBoxController)
	{
		s_skyBoxController = new SkyBoxController;
		*Moo::EffectConstantValue::get( "SkyBoxController" ) = s_skyBoxController;
	}

	if (!s_windAnimation)
	{
		s_windAnimation = new WindAnimation;
		*Moo::EffectConstantValue::get( "WindAnimation" ) = s_windAnimation;
	}
}


/**
 *	Destructor.
 */
EnviroMinder::~EnviroMinder()
{
	del_safe( skyDomeShadows_ );
	del_safe( skyLightMap_ );
	del_safe( rain_ );
	del_safe( seas_ );
	del_safe( sunAndMoon_ );
	del_safe( skyGradientDome_ );
	Py_XDECREF( weather_ );	weather_ = NULL;
	del_safe( timeOfDay_ );
	del_safe( flora_ );
	del_safe( decal_ );
	del_safe( environmentCubeMap_ );
#ifndef EDITOR_ENABLED
	del_safe( footPrintRenderer_ );
#endif // EDITOR_ENABLED
	del_safe( skyDomeOccluder_ );
	del_safe( zBufferOccluder_ );
	del_safe( chunkObstacleOccluder_ );
	//TODO : unsupported feature at the moment.  to be finished.
	//del_safe( dapple_ );
}


/**
 *	This method initialises all related static resources.
 */
/*static*/ void EnviroMinder::init()
{
	MF_VERIFY( ParticleSystemManager::init() );
	ShaderManager::init();
	ZBufferOccluder::init();
}


/**
 *	This method finalises all related static resources.
 */
/*static*/ void EnviroMinder::fini()
{
	ZBufferOccluder::fini();
	ShaderManager::fini();
	SkyLightMapSettings::fini();
	ParticleSystemManager::fini();
}


/**
 *	Load method. Any errors are handled internally, even if loading fails.
 *	This method must be called before the environment classes can be used.
 *
 *	@param	pDS					the space.settings data section for this space
 *	@param	loadFromExternal	true if cofing data should be loaded from external source
 *
 *	@return						always returns true.
 */
bool EnviroMinder::load( DataSectionPtr pDS, bool loadFromExternal /*= true*/ )
{
	BW_GUARD;
	data_ = pDS;
	this->farPlaneBaseLine_ = 500.f;

	// initialise ourselves from the space.settings or project file
	if (pDS)
	{
        loadTimeOfDay(pDS, loadFromExternal);
        loadSkyGradientDome(pDS, loadFromExternal, this->farPlaneBaseLine_);

		// Load Seas
		seas_->clear();
		DataSectionPtr spSeas = pDS->openSection( "seas" );
		if (spSeas)
		{
			for (DataSectionIterator it = spSeas->begin();
				it != spSeas->end();
				it++)
			{
				Sea * ns = new Sea();
				ns->load( *it );
				seas_->push_back( ns );
			}
		}

		// Load Sky Domes
		skyDomes_.clear();		
		std::vector<DataSectionPtr>	pSections;
		pDS->openSections( "skyDome", pSections );
		if (pSections.size())
		{
			for (std::vector<DataSectionPtr>::iterator it = pSections.begin();
				it != pSections.end();
				it++)
			{
				Moo::VisualPtr spSkyDome = Moo::VisualManager::instance()->get( (*it)->asString() );
				if (spSkyDome.hasObject())
				{
					skyDomes_.push_back( spSkyDome );					
				}
			}
		}

		skyDomesPartition_ = pDS->readInt( "skyDomePartition", 0 );

		//Load flora, with terrain version as specified by space settings.
		std::string floraXML = pDS->readString( "flora", s_floraXML );
		DataSectionPtr spFlora = BWResource::instance().openSection( floraXML );
			
		flora_->init( spFlora, pDS->readInt( "terrain/version" )  );

		// note: farPlane may have been set in the Sky xml file. ( more general )
		// that setting can be overridden in the space file.     ( less general )
		this->farPlaneBaseLine_ = pDS->readFloat( "farPlane", this->farPlaneBaseLine_ );	
	}
	
	sunAndMoon_->create();
	sunAndMoon_->timeOfDay( timeOfDay_ );	

	rain_->addAttachments( playerAttachments_ );

	return true;
}


#ifdef EDITOR_ENABLED
/**
 *	Save method. Any errors are handled internally, even if saving fails.
 *	This can only be called after the load routine is called.
 *
 *	@param	pDS					the space.settings data section for this space
 *	@param	saveToExternal		true if cofing data should be saved to external source
 *
 *	@return						always returns true.
 */
bool EnviroMinder::save( DataSectionPtr pDS, bool saveToExternal /*= true*/ ) const
{
	// initialise ourselves from the space.settings or project file
	if (pDS)
	{
		// Save TimeOfDay
		if ( !todFile_.empty() )
	        pDS->writeString( "timeOfDay", todFile_ );
        DataSectionPtr pTODSect = NULL;
        if (saveToExternal)
		{
			if ( !todFile_.empty() )
		        pTODSect = BWResource::openSection( todFile_ );
			else
				ERROR_MSG( "EnviroMinder::save: Could not save Time Of Day because its file path is empty.\n" );
		}
        else
        {
            pTODSect = pDS;
        }
		if ( pTODSect )
		{
    		timeOfDay_->save( pTODSect );
			if (saveToExternal)
				pTODSect->save();
		}

		// Save SkyGradientDome
		if ( !sgdFile_.empty() )
		    pDS->writeString( "skyGradientDome", sgdFile_ );
        DataSectionPtr pSkyDomeSect = NULL;
        if (saveToExternal)
		{
			if ( !sgdFile_.empty() )
			    pSkyDomeSect = BWResource::openSection( sgdFile_ );
			else
				ERROR_MSG( "EnviroMinder::save: Could not save Sky Dome because its file path is empty.\n" );
		}
        else
        {
            pSkyDomeSect = pDS;
        }
		if ( pSkyDomeSect )
		{
			skyGradientDome_->save( pSkyDomeSect );
			pSkyDomeSect->writeFloat
			( 
				"farPlane", 
				this->farPlaneBaseLine()
			);
			if (saveToExternal)
				pSkyDomeSect->save();
		}

		// Save Seas
        if (seas_ != NULL && !seas_->empty())
        {
            pDS->deleteSections("seas");
		    DataSectionPtr spSeas = pDS->openSection( "seas", true );
            int idx = 0;
            for 
            (
                Seas::const_iterator it = seas_->begin();
                it != seas_->end();
                ++it, ++idx
            )
			{
                char buffer[64];
                bw_snprintf(buffer, sizeof(buffer), "sea_%d", idx);
                DataSectionPtr pSeaSect = BWResource::openSection( buffer );
				(*it)->save( pSeaSect );
            }
        }
        
		// Save Sky Domes:
        pDS->deleteSections("skyDome");
        for
        (
            std::vector<Moo::VisualPtr>::const_iterator it = skyDomes_.begin();
            it != skyDomes_.end();
            ++it
        )
        {
            DataSectionPtr skyDomeSection = pDS->newSection( "skyDome" );
            skyDomeSection->setString( (*it)->resourceID() );
		}

		pDS->writeInt( "skyDomePartition", skyDomesPartition_ );
	}

	//note:farPlane may have been set in the Sky xml file.  ( more general )
	//that setting can be overridden in the space file.		( less general )
    pDS->writeFloat
    ( 
        "farPlane", 
        this->farPlaneBaseLine()
    );
	
	return true;
}
#endif


/**
 *	This method ticks all the environment stuff.
 *
 *	@param	dTime				time elapsed since last call.
 *	@param	outside				true if player is outdoors, false if indoors.
 *	@param	pWeatherOverride	override weather settings with this.
 */
void EnviroMinder::tick( float dTime, bool outside,
	const WeatherSettings * pWeatherOverride )
{
	s_skyBoxController->value( Vector4( 0,0,0,1 ) );
	s_windAnimation->tick( dTime, weather_->wind() );

	ParticleSystemManager::instance().windVelocity( Vector3(
		weather_->wind().x,
		0,
		weather_->wind().y ) );

	weather_->tick( dTime );

	// tell the clouds, rain and snow about the weather
	Vector3 sunDir =
		timeOfDay_->lighting().sunTransform.applyToUnitAxisVector(2);
	sunDir.x = -sunDir.x;
	sunDir.z = -sunDir.z;
	sunDir.normalise();
	uint32 sunCol = Colour::getUint32( timeOfDay_->lighting().sunColour );

	float sunAngle = 2.f * MATH_PI - DEG_TO_RAD( ( timeOfDay_->gameTime() / 24.f ) * 360 );	
	rain_->update( *weather_, outside );

	// update the sun and moon positions and light/ambient/fog/etc colours
	timeOfDay_->tick( dTime );
	skyGradientDome_->update( timeOfDay_->gameTime() );

	this->decideLightingAndFog();

	// get the sky to decide what if any lightning it wants
	//TODO : ask weather system

	// and tick the weather
	static DogWatch weatherTick("Weather");
	weatherTick.start();
	rain_->tick( dTime );
	this->tickSkyDomes( dTime );
	weatherTick.stop();

	//Update the flora
	flora_->update( dTime, *this );
}


/**
 *	This method is called when the environment is about to be used.
 *	This occurs when the camera enters a new space, or the engine
 *	is about to draw this space through some kind of portal.
 */
void EnviroMinder::activate()
{
	//Can only activate one enviro minder at a time.  If this assert
	//is hit, there is a coding error.
	MF_ASSERT( !s_activatedEM_ );
	s_activatedEM_ = this;

	if (weather_ != NULL)
	{
		weather_->activate();
	}

	if (flora_ != NULL)
	{	
		flora_->activate();
	}

	if (rain_ != NULL)
	{
		rain_->activate(*this,data_);
	}
	
	//Set the far plane for this space
	if (data_)
	{
		if (skyGradientDome_)
		{
			skyGradientDome_->activate(*this,data_);
		}

		//Set the sky light map and dapple map for this space
		if (skyLightMap_)
		{
			skyLightMap_->activate(*this,data_);
		}

		//TODO : unsupported feature at the moment.  to be finished.
		//dapple_->activate(*this,data_,skyLightMap_);

		//Hitting this assert means activate has been called twice in a row.bad
		MF_ASSERT( !skyDomeOccluder_ );
		MF_ASSERT( !zBufferOccluder_ );
		MF_ASSERT( !chunkObstacleOccluder_ );
		if (SkyDomeOccluder::isAvailable())
		{
			skyDomeOccluder_ = new SkyDomeOccluder( *this );
			LensEffectManager::instance().addPhotonOccluder( skyDomeOccluder_ );
		}
		else
		{
			INFO_MSG( "Sky domes will not provide lens flare occlusion, "
				"because scissor rects are unsuppported on this card\n" );
		}

		if (ZBufferOccluder::isAvailable())
		{
			zBufferOccluder_ = new ZBufferOccluder();
			LensEffectManager::instance().addPhotonOccluder( zBufferOccluder_ );
		}
		else
		{
			chunkObstacleOccluder_ = new ChunkObstacleOccluder();
			LensEffectManager::instance().addPhotonOccluder( chunkObstacleOccluder_ );
			INFO_MSG( "The ZBuffer will not provide lens flare occlusion, "
				"because this feature is unsuppported on this card\n" );
		}

		// Sets the terrain settings from the space settings file, and also
		// initialises the appropriate terrain renderer, etc.
		DataSectionPtr terrainSettings = NULL;
		if ( data_ )
			terrainSettings = data_->openSection( "terrain" );
	}		

    EnviroMinderSettings::instance().activate(this);
}


/**
 *	This method is called when the environment is to be replaced
 *	by another active environment.
 */
void EnviroMinder::deactivate()
{
	//Must deactivate the current enviro minder before activating
	//the new one.  If this assert is hit, fix the code.
	MF_ASSERT( s_activatedEM_ == this );
	s_activatedEM_ = NULL;

	if (skyGradientDome_)
	{
		skyGradientDome_->deactivate(*this);
	}

	if (skyLightMap_)
	{
		skyLightMap_->deactivate(*this);
	}

	if (flora_)
	{
		flora_->deactivate();
	}

	if (rain_ != NULL)
	{
		rain_->deactivate(*this);
	}

	if (skyDomeOccluder_)
	{
		LensEffectManager::instance().delPhotonOccluder( skyDomeOccluder_ );
		del_safe( skyDomeOccluder_ );		
	}
	if (zBufferOccluder_)
	{
		LensEffectManager::instance().delPhotonOccluder( zBufferOccluder_ );
		del_safe( zBufferOccluder_ );		
	}
	if (chunkObstacleOccluder_)
	{
		LensEffectManager::instance().delPhotonOccluder( chunkObstacleOccluder_ );
		del_safe( chunkObstacleOccluder_ );		
	}

	if (weather_ != NULL)
	{
		weather_->deactivate();
	}

    EnviroMinderSettings::instance().activate(NULL);
}

/**
 *	Returns the current far plane in use (generally 
 *	farPlaneBaseLine * graphics settings multiplier).
 *
 *	@return		current far plane value.
 */
float EnviroMinder::farPlane() const
{
	return this->farPlane_;
}

/**
 *	Sets the current camera far plane.
 *
 *	@param	farPlane	the new farPlane value.
 */
void EnviroMinder::setFarPlane(float farPlane)
{
	ChunkManager::instance().autoSetPathConstraints(farPlane);
	Moo::rc().camera().farPlane(farPlane);
	this->farPlane_ = farPlane;
}

/**
 *	Returns the far plane base line. This is normally read from the
 *	space.settings file. It will be adjusted by the graphics settings 
 *	multiplier before being used as the far plane.
 *
 *	@return		far plane base line value.
 */
float EnviroMinder::farPlaneBaseLine() const
{
	return this->farPlaneBaseLine_;
}

/**
 *	Sets the far plane base line.
 *
 *	@param	farPlaneBaseLine	the new farPlane base line value.
 */
void EnviroMinder::setFarPlaneBaseLine(float farPlaneBaseLine)
{
	this->farPlaneBaseLine_ = farPlaneBaseLine;
	EnviroMinderSettings::instance().refresh();
}

/**
 *	This method draws the selected hind parts of the environment stuff
 *
 *	@param	dTime		time elapsed since last call.
 *	@param	drawWhat	bitset specifying what bits of the sky to draw.
 *	@param	showWeather true if fog emmiter should be shown.
 */
void EnviroMinder::drawHind( float dTime, DrawSelection drawWhat, bool showWeather /* = true */ )
{
	//add all known fog emitters
	skyGradientDome_->addFogEmitter();
	if (showWeather)
	{
		rain_->addFogEmitter();
	}

	//update and commit fog.
	FogController::instance().tick();
	FogController::instance().commitFogToDevice();

#ifdef EDITOR_ENABLED
	//link the clouds shadows to cloud drawing
	//TODO : doesn't not drawing the skyboxes automatically mean
	//there are no shadows from the sky boxes?
	bool drawWeather = ((drawWhat & DrawSelection::skyBoxes) != 0);
	g_disableSkyLightMap = !drawWeather;
#endif

	//Update light maps that will be used when we draw the rest of the scene.
	if (skyLightMap_ && allowUpdate_)
	{
		//TODO : change how this works to make it generic.
		float sunAngle = 2.f * MATH_PI - DEG_TO_RAD( ( timeOfDay_->gameTime() / 24.f ) * 360 );
		skyLightMap_->update( sunAngle, Vector2::zero() );
	}

	//This seems dumb, but the sky light map uses a Geometrics fn, which
	//ends up un-setting the fog.  So we need to reset it.
	FogController::instance().commitFogToDevice();

	// draw the environment cube map
	environmentCubeMap_->update( dTime, true, 1, drawWhat.value );

	//On old video cards we draw the clouds, sky etc. at the back
	//of the scene, and do not use the z-buffer.
	if (EnviroMinder::primitiveVideoCard())
		drawSkySunCloudsMoon( dTime, drawWhat );
}


/** 
 *	This method draws the delayed background of our environment.
 *	We do this here in order to save fillrate, as we can
 *	test the delayed background against the opaque scene that
 *
 *	@param	dTime		time elapsed since last call.
 *	@param	drawWhat	bitset specifying what bits of the sky to draw.
 */
void EnviroMinder::drawHindDelayed( float dTime, DrawSelection drawWhat )
{
	//On newer video cards we draw the clouds, sky etc. after the
	//rest of the scene to save fillrate.	see EnviroMinder::drawHind
	if (!EnviroMinder::primitiveVideoCard())
		drawSkySunCloudsMoon( dTime, drawWhat );
}

/**
 *	This method draws the fore parts of the environment stuff
 *
 *	@param	dTime				time elapsed since last call.
 *	@param	showWeather			true if fog emmiter should be shown.
 *	@param	showFlora			true if flora should be drawn
 *	@param	showFloraShadowing	true if flora shadows should be drawn
 *	@param	drawOverLays		true if the rain and snow are to be drawn
 *	@param	drawObjects			true if scene based (solid) objects are drawn
 */
void EnviroMinder::drawFore( float dTime,	bool showWeather /* = true */,
											bool showFlora /* = true */, 
											bool showFloraShadowing /* = false */,
											bool drawOverLays /* = true */,
											bool drawObjects /* = true */ )
{
	// Draw the rain
	// Note: there is a brake to slow down the change in rain
	// amount so it can't suddenly start or stop
	//TODO : ask sky and clouds for rain
	float wantRain = 0.f;
	if (weatherControl_ && allowUpdate_)
	{
		Vector4 value;
		weatherControl_->output(value);
		wantRain += value[0];
	}	
	float haveRain = rain_->amount();
	if (drawObjects)
	{
		if (allowUpdate_)
		{
			if (weatherControl_)
			{
				rain_->amount( wantRain );
			}
			else
			{
				//If rain is not being controlled by python, then apply some
				//antihysteresis to the rain amount. (put the brakes on).
				rain_->amount( haveRain + Math::clamp(dTime*0.03f, wantRain - haveRain) );
			}
		}

		// Draw the seas
		seas_->draw( dTime, timeOfDay_->gameTime() );

		// Draw layered objects with near and far planes pushed out, to prevent
		// z-fighting.
		beginClipPlaneBiasDraw( decalClipPlaneBias );
			decal_->draw();
#ifndef EDITOR_ENABLED
			footPrintRenderer_->draw();
#endif // EDITOR_ENABLED
		endClipPlaneBiasDraw();

		if (showFlora)
		{
			//Render the flora
			flora_->draw( dTime, *this );
			
			//Render shadows onto the flora
			if (showFloraShadowing && (EnviroMinderSettings::instance().shadowCaster()))
			{
				flora_->drawShadows( EnviroMinderSettings::instance().shadowCaster() );
			}
		}

	}

	if (drawOverLays)
	{
		if (showWeather)
		{
			rain_->draw();
		}

		//remove all known fog emitters
		skyGradientDome_->remFogEmitter();
		rain_->remFogEmitter();
	}
}


#ifdef EDITOR_ENABLED

std::string EnviroMinder::timeOfDayFile() const
{
    return todFile_;
}

void EnviroMinder::timeOfDayFile(std::string const &filename)
{
    todFile_ = filename;
    if (data_ != NULL)
    {
        data_->writeString( "timeOfDay", filename );
        loadTimeOfDay(data_, true);
    }
}


std::string EnviroMinder::skyGradientDomeFile() const
{
    return sgdFile_;
}


void EnviroMinder::skyGradientDomeFile(std::string const &filename)
{
    sgdFile_ = filename;
    if (data_ != NULL)
    {
        data_->writeString( "skyGradientDome", filename );
        float fp = std::numeric_limits<float>::max();
        loadSkyGradientDome(data_, true, fp);
        if (fp != std::numeric_limits<float>::max())
        {
            setFarPlaneBaseLine(fp);
    	}
    }
}


#endif // EDITOR_ENABLED


/**
 *	This method returns true if the video card supports a shader version.
 *	If no shaders are supported, we may draw the environment slightly
 *	differently, in particular the sky + clouds are drawn before the scene
 *	instead of afterwards.
 */
bool EnviroMinder::primitiveVideoCard()
{
	return (Moo::rc().psVersion() + Moo::rc().vsVersion() == 0);
}


// some relevant dog watches
DogWatch g_skyStuffWatch( "Sky Stuff" );

/**
 *	Sets up sun lighting and fog.
 */
void EnviroMinder::decideLightingAndFog()
{
	Vector4 control(0.f,0.f,1.f,1.f);
	Vector4 sunlightControl(1.f,1.f,1.f,1.f);
	Vector4 ambientControl(1.f,1.f,1.f,1.f);
	Vector4 fogControl(1.f,1.f,1.f,1.f);

	if (weatherControl_ != NULL)
	{				
		weatherControl_->output( control );
	}
	if (sunlightControl_ != NULL)
	{				
		sunlightControl_->output( sunlightControl );
	}
	if (ambientControl_ != NULL)
	{				
		ambientControl_->output( ambientControl );
	}
	if (fogControl_ != NULL)
	{				
		fogControl_->output( fogControl );
	}

	// tone down the scene lighting	
	// TODO : ask sky and clouds for light modulation
	float dimBy = 1.f;
	Vector4 lightDimmer( dimBy, dimBy, dimBy, 1.f );	
	//Vector4 lightDimmer( sky_->avgColourDim(), 1.f );	//not implemented
	//Vector4 lightDimmer( clouds_->avgColourDim(), 1.f );
	OutsideLighting & outLight = timeOfDay_->lighting();
	outLight.sunColour = outLight.sunColour * lightDimmer * sunlightControl;
	outLight.ambientColour = outLight.ambientColour * lightDimmer * ambientControl;

	// calculate fog colours	
	float avd = 0.f;
	Vector3 modcol( 1.0f, 1.0f, 1.f );

	// calculate final fog density
	float fogDensity = fogControl.w;
	float extraFog = Math::clamp(0.f, fogDensity - 1.f, 1.f);
	Vector3 controlFog = Vector3((float*)(&fogControl.x));
	modcol = (extraFog * controlFog) + ( (1.f-extraFog) * Vector3(1.f,1.f,1.f) );	
	modcol = modcol * Vector3(255.f,255.f,255.f);
	
	skyGradientDome_->fogModulation( modcol, fogDensity );
	skyGradientDome_->farMultiplier( fogDensity );
}


/** 
 *	This method draws the sky, sun, clouds and moon.  It may be called either
 *	before the scene is rendered (non-shader cards), or after the scene is
 *	rendered (shader cards).
 *
 *	@param	dTime		time elapsed since last call.
 *	@param	drawWhat	bitset specifying what bits of the sky to draw.
 */
void EnviroMinder::drawSkySunCloudsMoon( float dTime, DrawSelection drawWhat )
{
	if (!drawWhat) return;

	g_skyStuffWatch.start();	

	{
		// Setup sky rendering things in this scope (viewport).
		SkyBoxScopedSetup scopedSkySetup;

		// Set up the clipping render state as some older graphics cards
		// (i.e. radeon 7200) has problems rendering the sky without it
		Moo::rc().pushRenderState( D3DRS_CLIPPING );
		Moo::rc().setRenderState( D3DRS_CLIPPING, TRUE );

		// sky dome is the explicit, static hand-painted dome.
		// we also draw a dynamic gradient dome, and sun, moon and clouds
		if (drawWhat & DrawSelection::skyGradient)
			skyGradientDome_->draw( timeOfDay_ );

		//draw the sun and moon for real
		if (drawWhat & DrawSelection::sunAndMoon)
			sunAndMoon_->draw();

		//we must set these, because sky boxes + clouds are not meant to set their
		//own COLORWRITEENABLE flags, so if somebody does, then we must restore the
		//state in code.
		Moo::rc().setRenderState( D3DRS_COLORWRITEENABLE,
			D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE );
		
		if (drawWhat & DrawSelection::skyBoxes)
		{
			//we must set these, because sky boxes + clouds are not meant to set their
			//own COLORWRITEENABLE flags, so if somebody does, then we must restore the
			//state in code.
			Moo::rc().setRenderState( D3DRS_COLORWRITEENABLE,
				D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE );

			this->drawSkyDomes();		
		}

		Moo::rc().popRenderState();

		// Restore old viewport at the end of the scope
	}

	g_skyStuffWatch.stop();
}


/**
 *	This method ticks the EnviroMinder's sky boxes.
 */
void EnviroMinder::tickSkyDomes( float dTime )
{
	for (size_t i=0; i<pySkyDomes_.size(); i++)
	{
		PyAttachmentPtr& sd = pySkyDomes_[i];
		sd->tick( dTime );
		sd->move( dTime );
	}
}


/**
 *	This method draws the EnviroMinder's sky boxes.
 */
void EnviroMinder::drawSkyDomes( bool isOcclusionPass )
{
	//fogDensity, windMultiplier, occludes, fade in/out)
	Vector4 staticSkyBoxControl( 1.f, 1.f, 1.f, 1.f );
	staticSkyBoxControl.x = FogController::instance().multiplier() - 1.f;	

	skyDomesPartition_ = Math::clamp( (size_t)0, skyDomesPartition_, skyDomes_.size() );

	//Any skydomes that need triangle sorting must be sorted in a different
	//bucket to the rest of the scene's sorted objects.
	Moo::SortedChannel::push();

	//Draw the far static sky domes
	s_skyBoxController->value(staticSkyBoxControl);
	for (size_t i=0; i<skyDomesPartition_; i++)		
	{
		Moo::VisualPtr& sd = skyDomes_[i];
		sd->draw(true);
		Moo::SortedChannel::draw();
	}	

	//Draw the weather/custom sky boxes
	for (size_t i=0; i<pySkyDomes_.size(); i++)
	{
		PyAttachmentPtr& sd = pySkyDomes_[i];
		Vector4ProviderPtr& pv = skyDomeControllers_[i];
		Vector4 value;
		pv->output(value);

		if (!isOcclusionPass || value[2] > 0.001f)
		{
			if (value[3] > 0.001f)
			{
				s_skyBoxController->value(value);
				sd->draw( sd->worldTransform() , 0.f);
			}
		}

		Moo::SortedChannel::draw();
	}

	//Draw the near static sky domes
	s_skyBoxController->value(staticSkyBoxControl);
	for (size_t i=skyDomesPartition_; i<skyDomes_.size(); i++)		
	{
		Moo::VisualPtr& sd = skyDomes_[i];
		sd->draw(true);
		Moo::SortedChannel::draw();
	}

	Moo::SortedChannel::pop();
}


/**
 *	This method adds a PySkyDome, which is a combination of
 *	PyAttachment and Vector4Provider.
 *	This method is the converse of delPySkyDome
 *
 *	@param	attachment	PyAttachment to use as a sky dome.
 *	@param	provider	Vector4Provider that provides control over the skybox.
 */
void EnviroMinder::addPySkyDome( PyAttachmentPtr attachment, Vector4ProviderPtr provider )
{
	pySkyDomes_.push_back(attachment);
	skyDomeControllers_.push_back(provider);
}


/*~ function BigWorld.addSkyBox
 *	@components{ client, tools }
 *
 *	This function registers a PyAttachment and Vector4Provider pair.
 *	The output of the provider sets various values on the sky box, in
 *	particular the alpha value controls the blend in/out amount.
 *
 *	The Vector4Provider is set onto the sky box's effect file using the
 *	SkyBoxController semantic.
 *
 *	@param a the PyAttchment to use as a python sky box
 *	@param p the Vector4Provider to set on the sky box
 */
PyObject* py_addSkyBox( PyObject* args )
{
	PyObject * attachment = NULL;
	PyObject * provider = NULL;
	
	if (!PyArg_ParseTuple( args, "OO", &attachment, &provider ) || 
			!PyAttachment::Check(attachment) || !Vector4Provider::Check( provider ) )
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.addSkyBox: "
			"Argument parsing error. Expected a PyAttachment and a Vector4Provider" );
		return NULL;
	}

	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (pSpace)
	{
		pSpace->enviro().addPySkyDome( (PyAttachment*)attachment, (Vector4Provider*)provider );		
	}

	Py_Return;
}

PY_MODULE_FUNCTION( addSkyBox, BigWorld )


/**
 *	This method deletes a PySkyDome, which is a combination of
 *	PyAttachment and Vector4Provider.  If it can't find the combination,
 *	it fails silently.
 *	This method is the converse of addPySkyDome
 *
 *	@param	attachment	PyAttachment to use as a sky dome.
 *	@param	provider	Vector4Provider that provides control over the skybox.
 */
void EnviroMinder::delPySkyDome( PyAttachmentPtr attachment, Vector4ProviderPtr provider )
{
	for (uint32 i=0; i<pySkyDomes_.size(); i++)
	{
		if ((pySkyDomes_[i] == attachment) && (skyDomeControllers_[i] == provider))
		{
			pySkyDomes_.erase( pySkyDomes_.begin() + i );
			skyDomeControllers_.erase( skyDomeControllers_.begin() + i );
		}
	}
}

/*~ function BigWorld.delSkyBox
 *	@components{ client, tools }
 *
 *	This function registers a PyAttachment and Vector4Provider pair.
 *	The output of the provider sets various values on the sky box, in
 *	particular the alpha value controls the blend in/out amount.
 *
 *	The Vector4Provider is set onto the sky box's effect file using the
 *	SkyBoxController semantic.
 *
 *	@param m the PyAttachment to use as a python sky box
 *	@param p the Vector4Provider to set on the sky box
 */
PyObject* py_delSkyBox( PyObject* args )
{
	PyObject * attachment = NULL;
	PyObject * provider = NULL;

	if (!PyArg_ParseTuple( args, "OO:BigWorld.delSkyBox", &attachment, &provider ) ||
			!PyAttachment::Check(attachment) || !Vector4Provider::Check( provider ) )
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.delSkyBox: "
			"Argument parsing error. Expected a PyAttachment and a Vector4Provider" );
		return NULL;
	}

	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (pSpace)
	{
		pSpace->enviro().delPySkyDome( (PyAttachment*)attachment, (Vector4Provider*)provider );
	}

	Py_Return;
}

PY_MODULE_FUNCTION( delSkyBox, BigWorld )


/**
 *	This method removes the static sky boxes, i.e. the ones added via WorldEditor.
 */
void EnviroMinder::delStaticSkyBoxes()
{
	skyDomes_.clear();
}


/*~ function BigWorld.delStaticSkyBoxes
 *	@components{ client, tools }
 *
 *	This function removes the static sky boxes added via Worldeditor.
 *	This is usually used when you want script control and dynamic sky boxes,
 *	in which case you'd use this method as well as addSkyBox / delSkyBox.
 */
PyObject* py_delStaticSkyBoxes( PyObject* args )
{
	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (pSpace)
	{
		pSpace->enviro().delStaticSkyBoxes();
	}	

	Py_Return;
}

PY_MODULE_FUNCTION( delStaticSkyBoxes, BigWorld )


/*~ function BigWorld.weatherController
 *	@components{ client, tools }
 *
 *	This function registers a vector4 provider to provide further control
 *	over the weather.  It is interpreted as (extraRain, reserved, reserved, reserved)
 *	The output of the provider sets various values on the weather. 
 *
 *	@param p the Vector4Provider to set
 */
PyObject* py_weatherController( PyObject* args )
{
	PyObject * p = NULL;

	if (!PyArg_ParseTuple( args, "O:BigWorld.weatherController", &p ) ||
			!Vector4Provider::Check( p ))
	{
		PyErr_SetString( PyExc_TypeError,
			"BigWorld.weatherController: Expected a Vector4Provider" );
		return NULL;
	}

	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (pSpace)
	{
		pSpace->enviro().weatherControl_ = (Vector4Provider*)p;
	}
	
	Py_Return;
}

PY_MODULE_FUNCTION( weatherController, BigWorld )


/*~ function BigWorld.sunlightController
 *	@components{ client, tools }
 *
 *	This function registers a vector4 provider to provide further control
 *	over the sunlight.  It is interpreted as an (r,g,b,reserved) multiplier
 *	on the existing time-of-day sunlight colour.
 *
 *	@param p the Vector4Provider to set
 */
PyObject* py_sunlightController( PyObject* args )
{
	PyObject * p = NULL;

	if (!PyArg_ParseTuple( args, "O", &p ) || !Vector4Provider::Check( p ) )
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.sunlightController: "
				"Expected a Vector4Provider" );
		return NULL;
	}

	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (pSpace)
	{
		pSpace->enviro().sunlightControl_ = (Vector4Provider*)p;
	}
	
	Py_Return;
}

PY_MODULE_FUNCTION( sunlightController, BigWorld )


/*~ function BigWorld.ambientController
 *	@components{ client, tools }
 *
 *	This function registers a vector4 provider to provide further control
 *	over the ambient lighting. It is interpreted as an (r,g,b,reserved)
 *	multiplier on the existing time-of-day ambient colour.
 *
 *	@param p the Vector4Provider to set
 */
PyObject* py_ambientController( PyObject* args )
{
	PyObject * p = NULL;

	if (!PyArg_ParseTuple( args, "O", &p ) || !Vector4Provider::Check( p ) )
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.ambientController: "
			"Expected a Vector4Provider" );
		return NULL;
	}

	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (pSpace)
	{
		pSpace->enviro().ambientControl_ = (Vector4Provider*)p;
	}
	
	Py_Return;
}

PY_MODULE_FUNCTION( ambientController, BigWorld )


/*~ function BigWorld.fogController
 *	@components{ client, tools }
 *
 *	This function registers a vector4 provider to provide further control
 *	over the fogging. (x,y,z) is a multiplier on the existing fog colour.
 *	(w) is a multiplier on the fog density.
 *
 *	@param p the Vector4Provider to set
 */
PyObject* py_fogController( PyObject* args )
{
	PyObject * p = NULL;

	if (!PyArg_ParseTuple( args, "O:BigWorld.fogController", &p ) ||
			!Vector4Provider::Check( p ))
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.fogController: "
			"Expected a Vector4Provider" );
		return NULL;
	}

	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (pSpace)
	{
		pSpace->enviro().fogControl_ = (Vector4Provider*)p;
	}
	
	Py_Return;
}

PY_MODULE_FUNCTION( fogController, BigWorld )


/**
 *	Loads time-of-day from the provided data section or the 
 *	external file defined in the data section.
 *
 *	@param	pDS					data section from where to load the time-of-day.
 *	@param	loadFromExternal	true if cofing data should be loaded from external source.
 */
void EnviroMinder::loadTimeOfDay(DataSectionPtr data, bool loadFromExternal)
{
    // Load TimeOfDay
    std::string todFile = data->readString( "timeOfDay" );
#ifdef EDITOR_ENABLED
    if (!todFile.empty())
        todFile_ = todFile;
#endif
	if (!todFile.empty() && loadFromExternal)
	{
		DataSectionPtr pSubSect = BWResource::openSection( todFile );
		if (pSubSect)
		{
			timeOfDay_->load( pSubSect );
		}
		else
		{
			ERROR_MSG( "EnviroMinder::load: "
				"Cannot open timeOfDay resource '%s'\n",
				todFile.c_str() );
		}
	}
	else
	{
		timeOfDay_->load( data, !loadFromExternal );
	}
}

/**
 *	Loads sky gradient dome from the provided data section or the 
 *	external file defined in the data section.
 *
 *	@param	pDS					data section from where to load the sky gradient dome.
 *	@param	loadFromExternal	true if cofing data should be loaded from external source.
 *	@param	farPlane			(out) will store the farPlane defined in the data section.
 */
void EnviroMinder::loadSkyGradientDome
(
    DataSectionPtr  data, 
    bool            loadFromExternal,
    float           &farPlane
)
{
	// Load SkyGradientDome
	std::string sgdFile = data->readString( "skyGradientDome" );
#ifdef EDITOR_ENABLED
    if (!sgdFile.empty())
        sgdFile_ = sgdFile;
#endif
	if (!sgdFile.empty() && loadFromExternal)
	{
		DataSectionPtr pSubSect = BWResource::openSection( sgdFile );
		if (pSubSect)
		{
			skyGradientDome_->load( pSubSect );
			farPlane = pSubSect->readFloat( "farPlane", farPlane );
		}
		else
		{
			ERROR_MSG( "EnviroMinder::load: "
				"Cannot open skyGradientDome resource '%s'\n",
				sgdFile.c_str() );
		}
	}
	else
	{
		skyGradientDome_->load( data );
	}
}

/*
 * Push the near/far clip planes out to modify z-bias for decals, footprints
 * or any other effect were we need to draw layered textures with non-shared
 * vertices.
 */
void EnviroMinder::beginClipPlaneBiasDraw( float bias )
{
	Moo::RenderContext& rc = Moo::rc();

	savedNearPlane_	= rc.camera().nearPlane();
	savedFarPlane_	= rc.camera().farPlane();

	rc.camera().nearPlane( savedNearPlane_ * bias );
	rc.camera().farPlane(  savedFarPlane_ *	bias );

	rc.updateProjectionMatrix();
	rc.updateViewTransforms();
}

/*
 * Restore old planes.
 */
void EnviroMinder::endClipPlaneBiasDraw()
{
	Moo::RenderContext& rc = Moo::rc();

	rc.camera().nearPlane( savedNearPlane_ );
	rc.camera().farPlane( savedFarPlane_ );

	rc.updateProjectionMatrix();
	rc.updateViewTransforms();
}

// -----------------------------------------------------------------------------
// Section: EnviroMinderSettings
// -----------------------------------------------------------------------------

/**
 *	Register the graphics settings controlled by 
 *	the envirominder. Namely, the FAR_PLANE setting.
 */
void EnviroMinderSettings::init(DataSectionPtr resXML)
{
#ifndef EDITOR_ENABLED
	// far plane settings
	this->farPlaneSettings_ = 
		Moo::makeCallbackGraphicsSetting(
			"FAR_PLANE", "Far Plane", *this, 
			&EnviroMinderSettings::setFarPlaneOption,
			-1, false, false);
				
	if (resXML.exists())
	{
		DataSectionIterator sectIt  = resXML->begin();
		DataSectionIterator sectEnd = resXML->end();
		while (sectIt != sectEnd)
		{
			static const float undefined = -1.0f;
			float farPlane = (*sectIt)->readFloat("value", undefined);
			std::string label = (*sectIt)->readString("label");
			if (!label.empty() && farPlane != undefined)
			{
				this->farPlaneSettings_->addOption(label, label, true);
				this->farPlaneOptions_.push_back(farPlane);
			}
			++sectIt;
		}
	}
	else
	{
		this->farPlaneSettings_->addOption("HIGHT", "Height", true);
		this->farPlaneOptions_.push_back(1.0f);
	}
	Moo::GraphicsSetting::add(this->farPlaneSettings_);
#endif // EDITOR_ENABLED
}


/**
 *	Sets the far plane. This value will be adjusted by the current
 *	FAR_PLANE setting before it gets set into the camera.
 */
void EnviroMinderSettings::activate(EnviroMinder * activeMinder)
{
	// EnviroMinderSettings needs 
	// to be initialised first
	this->activeMinder_ = activeMinder;
	this->refresh();	
}

/**
 *	Refreshes the current far plane.
 */
void EnviroMinderSettings::refresh()
{
	// Initialise the far plane settings
#ifndef  EDITOR_ENABLED
	if (this->isInitialised())
	{
		this->setFarPlaneOption(this->farPlaneSettings_->activeOption());
	}
	else
#endif // EDITOR_ENABLED
	if ( this->activeMinder_ )
	{
		float farPlaneBaseLine = this->activeMinder_->farPlaneBaseLine();
		this->activeMinder_->setFarPlane(farPlaneBaseLine);
	}
}


/**
 *	Returns true if settings have been initialised.
 */
bool EnviroMinderSettings::isInitialised() const
{
#ifndef EDITOR_ENABLED
	return this->farPlaneSettings_.exists();
#else
	return true;
#endif // EDITOR_ENABLED
}


/**
 * Registers the shadow caster to use when casting shadows for the flora.
 */
void EnviroMinderSettings::shadowCaster( ShadowCaster* shadowCaster )
{
	shadowCaster_ = shadowCaster;
}


/**
 * Returns the registered the shadow caster to use when casting shadows for the flora.
 */
ShadowCaster* EnviroMinderSettings::shadowCaster() const
{
	return shadowCaster_;
}


/**
 *	Returns singleton EnviroMinderSettings instance.
 */
EnviroMinderSettings & EnviroMinderSettings::instance()
{
	static EnviroMinderSettings instance;
	return instance;
}


#ifndef EDITOR_ENABLED
/**
 *	Sets the viewing distance. Implicitly called 
 *	whenever the user changes the FAR_PLANE setting.
 *
 *	The far plane setting's options can be set in the resource.xml file.
 *	Bellow is an example of how to setup the options:
 *
 *	<resources.xml>
 *		<system>	
 *			<farPlaneOptions>
 *				<option>
 *					<farPlane>1.0</farPlane>
 *					<label>FAR</label>
 *				</option>
 *				<option>
 *					<farPlane>0.5</farPlane>
 *					<label>NEAR</label>
 *				</option>
 *			</farPlaneOptions>
 *		</system>
 *	</resources.xml>
 *
 *	For each <option> entry in <farPlaneOptions>, <label> will be used
 *	to identify the option and farPlane will be multiplied to the absolute
 *	far plane value defined in space.settings or sky.xml files. See 
 *	EnviroMinderSettings::farPlaneBaseLine().
 *	
 */
void EnviroMinderSettings::setFarPlaneOption(int optionIndex)
{
	MF_ASSERT(this->isInitialised());
	if (this->activeMinder_ != NULL)
	{
		float ratio = this->farPlaneOptions_[optionIndex];
		float farPlaneBaseLine = this->activeMinder_->farPlaneBaseLine();
		this->activeMinder_->setFarPlane(farPlaneBaseLine * ratio);
	}
}
#endif // EDITOR_ENABLED
