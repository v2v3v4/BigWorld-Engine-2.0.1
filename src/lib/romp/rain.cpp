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

#include "rain.hpp"
#include "weather.hpp"
#include "chunk/chunk.hpp"
#include "chunk/chunk_exit_portal.hpp"
#include "chunk/chunk_boundary.hpp"
#include "particle/py_meta_particle_system.hpp"
#include "particle/meta_particle_system.hpp"
#include "particle/actions/source_psa.hpp"
#include "particle/actions/tint_shader_psa.hpp"
#include "particle/actions/vector_generator.hpp"
#include "particle/renderers/particle_system_renderer.hpp"
#include "enviro_minder.hpp"
#include "moo/material.hpp"
#include "moo/texturestage.hpp"
#include "moo/managed_texture.hpp"
#include "moo/render_context.hpp"
#include "moo/texture_manager.hpp"
#include "math/colour.hpp"
#include "resmgr/auto_config.hpp"
#include "romp/custom_mesh.hpp"

#ifndef CODE_INLINE
#include "rain.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "Romp", 0 )

static const float groundRateMultiplier = 4800.0f;
static const float avatarRateMultiplier = 3.0f;
static float s_goingOutsideTransitionSpeed = 0.f;
static float s_goingInsideTransitionSpeed = 0.2f;
static float s_rps = 6.f;
static float s_ris = 10.f;
// The MINIMUM_SPLASH_ALPHA_PERCENT const determines the percent of the maximum
// alpha value in the tints of the raindrop splash particle system, used as the
// minimum alpha value. Valid values are between 0.0f and 1.0f.
static const float MINIMUM_SPLASH_ALPHA_PERCENT = 0.2f;

static AutoConfigString s_rainBmpName( "environment/rainBmpName" );
static AutoConfigString s_rainSplashesName( "environment/rainSplashParticles" );


Rain::Rain()
:numRainBits_( 0 ),
 sphereSize_( 1.f ),
 maxPseudoDepth_( 16 ),
 minPseudoDepth_( 2 ),
 baseIllumination_( 64 ),
 wind_( 0.f, 0.f, 0.f ),
 amount_( 0.f ),
 velocitySensitivity_( 0.5f ),
 rainSplashes_( NULL ),
 //headRainSplashes_( NULL ),
 //leftShoulderRainSplashes_( NULL ),
 //rightShoulderRainSplashes_( NULL ),
 //headRainColour_( NULL ),
 //leftShoulderRainColour_( NULL ),
 //rightShoulderRainColour_( NULL ),
 lastCameraPos_( -32768.f, -32768.f, -32768.f ),
 cameraNode_( NULL ),
 outside_( true ),
 maxFarFog_( 5.f ),
 maxNearFog_( 0.3f ),
 emitterID_( 0 ),
 transitionFactor_( 1.f ),
 transiting_( true )
{
	buildMaterial();

	cameraNode_ = new Moo::Node();

	// Create new particle system.
	createRainSplashes( groundRateMultiplier * amount_ );
	//createRainSplashesForAvatar( avatarRateMultiplier * amount_ );

	emitter_.colour_ = Colour::getUint32( 53, 57, 70, 0 );
	emitter_.maxMultiplier_ = 0.f;
	emitter_.nearMultiplier_ = 0.f;
	emitter_.position_ = Vector3( 0.f, 0.f, 0.f );
	emitter_.innerRadius( 0.f );
	emitter_.outerRadius( 1000.f );
	emitter_.localised_ = false;
}

Rain::~Rain()
{
	Py_XDECREF( rainSplashes_ );
	rainSplashes_ = NULL;

	//if ( headRainSplashes_ != NULL )
	//{
	//	Py_DECREF( headRainSplashes_ );
	//	headRainSplashes_ = NULL;
	//	headRainColour_ = NULL;
	//}

	//if ( leftShoulderRainSplashes_ != NULL )
	//{
	//	Py_DECREF( leftShoulderRainSplashes_ );
	//	leftShoulderRainSplashes_ = NULL;
	//	leftShoulderRainColour_ = NULL;
	//}

	//if ( rightShoulderRainSplashes_ != NULL )
	//{
	//	Py_DECREF( rightShoulderRainSplashes_ );
	//	rightShoulderRainSplashes_ = NULL;
	//	rightShoulderRainColour_ = NULL;
	//}
}


void Rain::activate( const class EnviroMinder&, DataSectionPtr pSpaceSettings )
{
	MF_WATCH( "Client Settings/Rain/density",
		numRainBits_,
		Watcher::WT_READ_ONLY,
		"Number of full-screen rain texture sheets." );
	MF_WATCH( "Client Settings/Rain/area",
		sphereSize_,
		Watcher::WT_READ_WRITE,
		"Area covered by the rain" );
	MF_WATCH( "Client Settings/Rain/min uv",
		minPseudoDepth_,
		Watcher::WT_READ_WRITE,
		"Rain minimum pseudo depth (uv).  Affects tiling of full-screen "
		"rain texturing." );
	MF_WATCH( "Client Settings/Rain/max uv",
		maxPseudoDepth_,
		Watcher::WT_READ_WRITE,
		"Rain maximum pseudo depth (uv).  Affects tiling of full-screen "
		"rain texturing." );
	MF_WATCH( "Client Settings/Rain/max near fog",
		maxNearFog_,
		Watcher::WT_READ_WRITE,
		"Amount by which to adjust the near fog when full rain is occurring." );
	MF_WATCH( "Client Settings/Rain/max far fog",
		maxFarFog_,
		Watcher::WT_READ_WRITE,
		"Amount by which to adjust the far fog when full rain is occurring." );
	MF_WATCH( "Client Settings/Rain/brightness",
		baseIllumination_,
		Watcher::WT_READ_ONLY,
		"Brightness of rain particles" );
	MF_WATCH( "Client Settings/Rain/wind",
		wind_,
		Watcher::WT_READ_ONLY,
		"Amount the rain is affected by the wind" );
	MF_WATCH( "Client Settings/Rain/vel sensitivity",
		velocitySensitivity_,
		Watcher::WT_READ_WRITE,
		"Sensivity of the rain to the camera velocity." );
	MF_WATCH( "Client Settings/Rain/amount",
		*this,
		MF_ACCESSORS( float, Rain, amount ),
		"Current amount of rain, from 0.0 to 1.0" );
	MF_WATCH( "Client Settings/Rain/going inside speed",
		s_goingInsideTransitionSpeed,
		Watcher::WT_READ_WRITE,
		"Speed at which the rain fades out when going inside.");
	MF_WATCH( "Client Settings/Rain/going outside speed",
		s_goingOutsideTransitionSpeed,
		Watcher::WT_READ_WRITE,
		"Speed at which the rain fades in when going outside.");
	MF_WATCH( "Client Settings/Rain/Rain Portal Scale",
		s_rps,
		Watcher::WT_READ_WRITE,
		"How much the rain visible from the inside resists depth perception" );

	MF_WATCH( "Client Settings/Rain/Rain Illumination Scale",
		s_ris,
		Watcher::WT_READ_WRITE,
		"How much the rain visible from the inside fades out with distance.");
}


void Rain::deactivate( const class EnviroMinder& )
{
#if ENABLE_WATCHERS
	Watcher::rootWatcher().removeChild("Client Settings/Rain/density");
	Watcher::rootWatcher().removeChild("Client Settings/Rain/area");
	Watcher::rootWatcher().removeChild("Client Settings/Rain/min uv");
	Watcher::rootWatcher().removeChild("Client Settings/Rain/max uv");
	Watcher::rootWatcher().removeChild("Client Settings/Rain/max near fog");
	Watcher::rootWatcher().removeChild("Client Settings/Rain/max far fog");
	Watcher::rootWatcher().removeChild("Client Settings/Rain/brightness");
	Watcher::rootWatcher().removeChild("Client Settings/Rain/wind");
	Watcher::rootWatcher().removeChild("Client Settings/Rain/vel sensitivity");
	Watcher::rootWatcher().removeChild("Client Settings/Rain/amount");
	Watcher::rootWatcher().removeChild("Client Settings/Rain/going inside speed");
	Watcher::rootWatcher().removeChild("Client Settings/Rain/going outside speed");
	Watcher::rootWatcher().removeChild("Client Settings/Rain/Rain Portal Scale");
	Watcher::rootWatcher().removeChild("Client Settings/Rain/Rain Illumination Scale");
#endif
}


/**
 *	Add attachments to given vector
 */
void Rain::addAttachments( class PlayerAttachments & pa )
{
	// TODO: ParticleSystem
	if (rainSplashes_)
	{
		pa.add( rainSplashes_, "" );
	}
	//pa.add( headSplash_, "head" );
	//pa.add( leftShoulderSplash_, "shoulderL" );
	//pa.add( rightShoulderSplash_, "shoulderR" );
}


void Rain::addFogEmitter()
{
	emitterID_ = FogController::instance().addEmitter( emitter_ );
}


void Rain::remFogEmitter()
{
	FogController::instance().delEmitter( emitterID_ );
}


void Rain::amount( float a )
{
	if ( a > 1.f )
		a = 1.f;
	if ( a < 0.f )
		a = 0.f;

	amount_ = a;

	if ( a < 0.01f )
		numRainBits_ = 0;
	else
		numRainBits_ = 4;

	baseIllumination_ = int( a * 160.f );

	// Update the particle system's rate.
	for (uint32 i=0; i<rainSplashSources_.size(); i++)	
	{
		SourcePSA* src = rainSplashSources_[i];		
		src->rate(maxRates_[i] * amount_ * transitionFactor_);
	}
	// Also update the particle system's alpha, according to the rate
	for (uint32 i=0; i<rainSplashTints_.size(); i++)	
	{
		TintShaderPSA* tint = rainSplashTints_[i];		
		TintShaderPSA::Tints& tintSet = tint->tintSet();
		// multiply the max alpha value by the rain intensity, but keeping a
		// minimum alpha of MINIMUM_SPLASH_ALPHA_PERCENT * max
		float minAlpha = maxTintAlphas_[i] * MINIMUM_SPLASH_ALPHA_PERCENT;
		float alpha = (maxTintAlphas_[i] - minAlpha) * amount_ +  minAlpha;
		alpha *= transitionFactor_;
		for (TintShaderPSA::Tints::iterator j = tintSet.begin();
			j != tintSet.end(); ++j )	
		{
			(*j).second.w = alpha;
		}
	}

	//if ( leftShoulderRainColour_ != NULL )
	//{
	//	leftShoulderRainColour_->setColour( Vector4(
	//		0.5f, 0.5f, 0.5f, amount_ ) );
	//}

	//if ( rightShoulderRainColour_ != NULL )
	//{
	//	rightShoulderRainColour_->setColour( Vector4(
	//		0.5f, 0.5f, 0.5f, amount_ ) );
	//}
}


void Rain::tick( float dt )
{
	if ( rainSplashes_ != NULL )
	{
		for (uint32 i=0; i<rainSplashes_->nSystems(); i++)
		{
			rainSplashes_->pSystem()->systemFromIndex(i)->explicitPosition(
				Moo::rc().invView().applyToOrigin() );
		}

		Moo::rc().push();
		Moo::rc().world( Moo::rc().invView() );
		cameraNode_->traverse();
		Moo::rc().pop();

		// Since rain is now held by the PlayerAvatar model in the Client we need to tell it to draw
		// itself, because it is not attached to anything.
#ifdef EDITOR_ENABLED
		rainSplashes_->tick( dt );
#endif
	}

	if (transiting_)
	{
		if (outside_)
		{
			if (s_goingOutsideTransitionSpeed <= 0.01f)
				s_goingOutsideTransitionSpeed = 0.01f;
			transitionFactor_ += dt / s_goingOutsideTransitionSpeed;
			if (transitionFactor_ > 1.f)
				transiting_ = false;
		}
		else
		{
			if (s_goingInsideTransitionSpeed <= 0.01f)
				s_goingInsideTransitionSpeed = 0.01f;
			transitionFactor_ -= dt / s_goingInsideTransitionSpeed;
			if (transitionFactor_ < 0.f)
				transiting_ = false;
		}
		transitionFactor_ = Math::clamp( 0.f, transitionFactor_, 1.f );
	}
}


void Rain::draw( void )
{
	if ( numRainBits_ == 0 )
		return;

	if ( s_disable )
		return;

	Moo::rc().setPixelShader( NULL );
	Moo::rc().setVertexShader( NULL );
	Moo::rc().setFVF(
		D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 );
	Moo::rc().setRenderState( D3DRS_LIGHTING, FALSE );
	Moo::rc().setRenderState(D3DRS_CULLMODE, D3DCULL_NONE );	
	Moo::rc().setRenderState(D3DRS_CLIPPING, TRUE );	

	//Draw inside rain or outside rain
	float amount = 0.f;
	if ( !outside_ )
	{
		ChunkExitPortal::Vector::iterator it =
			ChunkExitPortal::seenExitPortals().begin();
		ChunkExitPortal::Vector::iterator end =
			ChunkExitPortal::seenExitPortals().end();
		while (it != end)
		{
			ChunkExitPortal* cep = *it++;
			drawInside(&cep->portal(),cep->chunk());			
		}

		if (!transiting_)
			return;
	}

	//set up d3d device render states
//	Moo::rc().setRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, TRUE );
//	Moo::rc().setRenderState(D3DRS_CLIPPING, TRUE );

	//set up transforms for camera-space drawing
	Moo::rc().device()->SetTransform( D3DTS_WORLD, &Matrix::identity );
	Moo::rc().device()->SetTransform( D3DTS_VIEW, &Moo::rc().view() );
	Moo::rc().device()->SetTransform( D3DTS_PROJECTION, &Moo::rc().projection() );
	Moo::rc().world( Matrix::identity );

	Vector3 camRight( Moo::rc().invView().applyToUnitAxisVector( 0 ) );
	Vector3 camUp( Moo::rc().invView().applyToUnitAxisVector( 1 ) );
	Vector3 camForward( Moo::rc().invView().applyToUnitAxisVector( 2 ) );

	//The rot matrix is applied to the rain sheets, and is sensitive to the
	//camera pitch.  When looking forward, the rain sheets are all in front
	//of the camera.  When looking up / down, they surreptitiously slide around
	//the camera to encase it in a cylinder
	Matrix rot;
	float rotationPerSheet = DEG_TO_RAD( ( 360.f / numRainBits_ ) * camForward.y );
	rot.setRotateY( -rotationPerSheet * ( numRainBits_ / 2.f ) );
	rot.applyVector( camRight, camRight );
	rot.applyVector( camUp, camUp );
	rot.applyVector( camForward, camForward );
	rot.setRotateY( rotationPerSheet );

	Vector3 camPos = Moo::rc().invView().applyToOrigin();
	Vector3 movement( 0.f, 0.f, 0.f );
	if ( lastCameraPos_.y != -32768.f )
		 movement = camPos - lastCameraPos_;
	movement *= velocitySensitivity_;
	lastCameraPos_ = camPos;

	material_.set();

	for ( int i = 0; i < numRainBits_; i++ )
	{
		//rotate the rain sheets
		rot.applyVector( camRight, camRight );
		rot.applyVector( camForward, camForward );

		//jitter the texture coords, for a depth effect
		if ( maxPseudoDepth_ <= minPseudoDepth_ )
			maxPseudoDepth_ = minPseudoDepth_ + 1;

		float particleScale = minPseudoDepth_ + (float)( rand() % ( maxPseudoDepth_ - minPseudoDepth_ ) );

		//jitter the brightness
		int currIllumination = (int)( (float)(baseIllumination_) * transitionFactor_ );
		int jitterRange = 64;
		int illum = currIllumination + (rand () % jitterRange);
		illum = (illum << 24) | (illum << 16) | (illum << 8) | (illum);

		//jitter the location of the rain sheet,
		//making sure that the sheet is still in front of the camera
		Vector3 offset( this->randomOffset() );		
		offset *= sphereSize_;
		offset += camForward;
		offset += camPos;

		//create the vertices ( spin-to-face, with a constant y-axis )
		Vector3 cr( camRight );
		cr *= sphereSize_;

		verts_[0].pos_ = Vector3( -cr );
		verts_[0].pos_.y = -sphereSize_;
		verts_[1].pos_ = Vector3( -cr );
		verts_[1].pos_.y = sphereSize_;
		verts_[2].pos_ = Vector3( cr );
		verts_[2].pos_.y = -sphereSize_;
		verts_[3].pos_ = Vector3( cr );
		verts_[3].pos_.y = sphereSize_;

		verts_[0].pos_ += offset;
		verts_[1].pos_ += offset;
		verts_[2].pos_ += offset;
		verts_[3].pos_ += offset;

		//offset the top vertices by the camera movement
		verts_[1].pos_ += movement;
		verts_[3].pos_ += movement;

		//offset the bottom vertices by the wind
		verts_[0].pos_ += wind_;
		verts_[2].pos_ += wind_;

		verts_[0].colour_ = illum;
		verts_[1].colour_ = illum;
		verts_[2].colour_ = illum;
		verts_[3].colour_ = illum;

		verts_[0].uv_.x = 0.f;				verts_[0].uv_.y = particleScale;
		verts_[1].uv_.x = 0.f;				verts_[1].uv_.y = 0.f;
		verts_[2].uv_.x = particleScale;	verts_[2].uv_.y = particleScale;
		verts_[3].uv_.x = particleScale;	verts_[3].uv_.y = 0.f;

		Moo::rc().drawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, verts_, sizeof(verts_[0]) );
	}

	Moo::rc().setRenderState(D3DRS_CULLMODE, D3DCULL_CCW );

	// Since rain is now held by the PlayerAvatar model in the Client we need to tell it to draw
	// itself, because it is not attached to anything.
#ifdef EDITOR_ENABLED
	if ( rainSplashes_ != NULL )
	{
		Matrix cameraPosition;
		cameraPosition.setTranslate( Moo::rc().invView().applyToOrigin() );
		// Lod value here doesn't really matter.
		rainSplashes_->draw( cameraPosition, 100.f );
	}
#endif
}


/**
 *	This method draws rain onto exit portals.
 */
void Rain::drawInside( ChunkBoundary::Portal* portal, Chunk* chunk )
{	
	Moo::rc().push();
	Moo::rc().world( chunk->transform() );

	Vector3 outsideVector;

	//calc world, camera positions for derived calcs
	Vector3 avgWPos(0,0,0);
	Vector3 wPos[4];
	Vector3 cPos[4];
	Vector3 tPos[4];	//projected into a useful texture space
	for (uint32 j=0; j<4; j++)
	{
		portal->objectSpacePoint(j,wPos[j]);
		wPos[j] = Moo::rc().world().applyPoint(wPos[j]);		
		cPos[j] = Moo::rc().view().applyPoint(wPos[j]);
		avgWPos += wPos[j];
	}
	avgWPos /= 4.f;
	avgWPos.y = 0.f;
	Vector3 xzCamPos( Moo::rc().invView().applyToOrigin() );
	xzCamPos.y = 0.f;
	float z = Vector3(xzCamPos - avgWPos).length();
	z = max( 1.f, z ) - 1.f;
	//so now, z is the distance in the XZ plane from the camera to
	//1 metre from the centre of the portal.
	//(usually, portals are aligned in the y-axis.)

	//calculate outside vector,to push plane outside slightly (jitter)
	Vector3 aVector( wPos[0] - wPos[1] );
	aVector.normalise();
	Vector3 bVector( wPos[2] - wPos[1] );
	bVector.normalise();
	outsideVector = aVector.crossProduct( bVector );

	Moo::rc().pop();

	//this code doesn't work for portals if they are facing up/down	
	if (fabsf(outsideVector.y > 0.707f))
	{		
		return;
	}

	//now we know which way the portal is facing in world space, work
	//out a projection for texture coordinates.
	Vector3 wZ(1,0,0);
	float angle = wZ.dotProduct(outsideVector);
	Matrix rotY;
	rotY.setRotateY(angle * (MATH_PI/2.f));
	for ( uint32 j=0; j<4; j++ )
	{
		tPos[j] = rotY.applyPoint(wPos[j]);
	}

	//k we are ready to draw now. set up transforms for world-space drawing	
	Moo::rc().push();
	Moo::rc().world( Matrix::identity );
	Moo::rc().device()->SetTransform( D3DTS_WORLD, &Matrix::identity );
	Moo::rc().device()->SetTransform( D3DTS_VIEW, &Moo::rc().view() );
	Moo::rc().device()->SetTransform( D3DTS_PROJECTION, &Moo::rc().projection() );	

	material_.set();

	for ( int i = 0; i < numRainBits_; i++ )
	{
		//jitter the brightness
		float fillum = (float)(rand() % 64 + baseIllumination_);
		fillum /= (1.f + z/s_ris);
		int illum = (int)fillum;
		illum = (illum << 24) | (illum << 16) | (illum << 8) | (illum);

		CustomMesh<Moo::VertexXYZDUV> mesh(D3DPT_TRIANGLEFAN);
		Moo::VertexXYZDUV vertex;
		vertex.colour_ = illum;

		float particleScale = minPseudoDepth_ + (float)( rand() % ( maxPseudoDepth_ - minPseudoDepth_ ) );		

		float randZ = (float)rand() / (float)RAND_MAX;
		particleScale /= (1.f + z/s_rps);
		Vector3 offset( this->randomOffset() );
		
		for (uint32 j=0; j<4; j++)
		{
			vertex.pos_ = wPos[j];
			mesh.push_back(vertex);
			mesh[j].pos_ += (outsideVector*randZ);
			//calculate uvs by projecting the mesh onto the
			//default mesh, which is a 2x2 metre plane set
			//1 metre in front of the camera.
			mesh[j].uv_.x = ((tPos[j].x-avgWPos.x) * particleScale) / 2.f + offset.x;
			mesh[j].uv_.y = ((tPos[j].y-avgWPos.y) * particleScale) / 2.f + offset.y;
		}				

		mesh.draw();
	}
	Moo::rc().pop();
}


void Rain::buildMaterial( void )
{
	Moo::TextureStage ts, ts1;

	ts.colourOperation( Moo::TextureStage::MODULATE, Moo::TextureStage::TEXTURE, Moo::TextureStage::CURRENT );
	ts.alphaOperation( Moo::TextureStage::SELECTARG1, Moo::TextureStage::TEXTURE, Moo::TextureStage::CURRENT );
	ts.pTexture( Moo::TextureManager::instance()->get( s_rainBmpName, true, true, true, "texture/environment" ) );
	ts.textureWrapMode( Moo::TextureStage::REPEAT );

	material_.addTextureStage( ts );
	material_.addTextureStage( ts1 );
	material_.srcBlend( Moo::Material::SRC_ALPHA );
	material_.destBlend( Moo::Material::ONE );
	material_.alphaBlended( true );
	material_.alphaTestEnable( true );
	material_.alphaReference( 0x00000080 );
	material_.fogged( false );
	material_.zBufferRead( true );
	material_.zBufferWrite( false );
}


void Rain::createRainSplashes( float rate )
{
	// Create the particle system.
	rainSplashes_ = new PyMetaParticleSystem( new MetaParticleSystem );

	if (rainSplashes_->pSystem()->load( s_rainSplashesName, "" ))
	{				
		for (uint32 i=0; i<rainSplashes_->pSystem()->nSystems(); i++)
		{
			ParticleSystemPtr ps = rainSplashes_->pSystem()->systemFromIndex( i );
			// get access to the emitter rate
			ParticleSystemAction* pa = &*ps->pAction(PSA_SOURCE_TYPE_ID);
			SourcePSA* src = static_cast<SourcePSA*>(pa);
			rainSplashSources_.push_back(src);
			maxRates_.push_back(src->rate());
			// get access to tints, to change the alpha according to the rate
			pa = &*ps->pAction(PSA_TINT_SHADER_TYPE_ID);
			TintShaderPSA* tint = static_cast<TintShaderPSA*>(pa);
			if ( tint )
			{
				TintShaderPSA::Tints& tintSet = tint->tintSet();
				if ( !tintSet.empty() )
				{
					rainSplashTints_.push_back(tint);
					float maxAlpha = 0.0f;
					for (TintShaderPSA::Tints::iterator j = tintSet.begin();
						j != tintSet.end(); ++j )	
					{
						if ( maxAlpha < (*j).second.w )
						{
							maxAlpha = (*j).second.w;
						}
					}
					maxTintAlphas_.push_back( maxAlpha );
				}
			}
		}
	}
	else
	{		
		rainSplashes_ = NULL;
		ERROR_MSG( "Rain splash particle system (%s)could not be loaded\n",
			s_rainSplashesName.value().c_str() );
	}

}


/**
 *	Update our internal parameters based on the input weather settings
 */
void Rain::update( const Weather & w, bool outside )
{
	// TODO: Use wind_ properly ... as a velocity!
	wind_ = Vector3( w.wind().x, 0, w.wind().y ) / 50.f;

	if ( this->amount() > 0.1f )
	{
		emitter_.maxMultiplier_ = ( this->amount() - 0.1f ) * maxFarFog_;
		emitter_.nearMultiplier_ = ( this->amount() - 0.1f ) * maxNearFog_;
	}
	else
	{
		emitter_.maxMultiplier_ = 0.f;
		emitter_.nearMultiplier_ = 0.f;
	}

	if ( outside != outside_ )
	{
		transiting_ = true;		
		outside_ = outside;
	}
}


Vector3 Rain::randomOffset()
{
	int x = rand() % 1000;
	int y = rand() % 1000;
	int z = rand() % 1000;

	x -= rand() % 1000;
	y -= rand() % 1000;
	z -= rand() % 1000;

	Vector3 offset;
	offset.x = float(x) / 2000.f;
	offset.y = float(y) / 2000.f;
	offset.z = float(z) / 2000.f;

	return offset;
}



std::ostream& operator<<(std::ostream& o, const Rain& t)
{
	o << "Rain\n";
	return o;
}


void Rain::disable( bool state )
{
	Rain::s_disable = state;
}

bool Rain::s_disable = false;

// rain.cpp
