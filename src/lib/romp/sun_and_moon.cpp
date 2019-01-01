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

#include "sun_and_moon.hpp"
#include "enviro_minder.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/datasection.hpp"
#include "resmgr/auto_config.hpp"
#include "moo/render_context.hpp"
#include "moo/texture_manager.hpp"
#include "lens_effect_manager.hpp"
#include "fog_controller.hpp"

#include "time_of_day.hpp"

DECLARE_DEBUG_COMPONENT2( "Romp", 0 );

#ifndef CODE_INLINE
#include "sun_and_moon.ipp"
#endif


static AutoConfigString s_sunMaterial( "environment/sunMaterial" );
static AutoConfigString s_moonMaterial( "environment/moonMaterial" );
static AutoConfigString s_sunFlareXML( "environment/sunFlareXML" );
static AutoConfigString s_moonFlareXML( "environment/moonFlareXML" );


///Constructor
SunAndMoon::SunAndMoon()
:moon_	( NULL ),
 sun_( NULL ),
 timeOfDay_( NULL ),
 sunMat_( NULL ),
 moonMat_( NULL ) 
{
}


///Destructor
SunAndMoon::~SunAndMoon()
{
	this->destroy();	
}


/**
 *	This method draws a square mesh, using a given material.
 *	This method is used for the moon, moon mask, sun and flare.
 *
 *	@param cam		the world matrix
 *	@param pMesh	a pointer to the square mesh to draw
 *	@param pMat		a pointer to the material to draw with
 */
void SunAndMoon::drawSquareMesh( const Matrix & world, CustomMesh<Moo::VertexXYZNDUV> * pMesh, Moo::EffectMaterialPtr pMat )
{	
	Moo::rc().push();	
	Moo::rc().world( world );

	if (pMat->begin())
	{
		for (uint32 i=0; i<pMat->nPasses(); i++)
		{
			pMat->beginPass(i);
			D3DXHANDLE handle = pMat->pEffect()->pEffect()->GetParameterByName( NULL, "world" );
			pMat->pEffect()->pEffect()->SetMatrix( handle, &world );
			pMat->pEffect()->pEffect()->CommitChanges();
			pMesh->drawEffect();
			pMat->endPass();
		}
		pMat->end();
	}

	Moo::rc().pop();
}


/**
 *	This method creates the sun and moon meshes / materials
 */
void SunAndMoon::create( void )
{
	this->destroy();

	createMoon( );
	createSun( );

	//Sun flare
	DataSectionPtr pLensEffectsSection = BWResource::openSection( s_sunFlareXML );
	if ( pLensEffectsSection )
	{
		LensEffect l;
		l.load( pLensEffectsSection );
		sunLensEffect_ = l;
		sunLensEffect_.clampToFarPlane( true );
	}
	else
	{
		WARNING_MSG( "Could not find lens effects definitions for the Sun\n" );
	}	
}


/**
 *	This method cleans up memory allocated for the sun and moon
 */
void SunAndMoon::destroy( void )
{
	if ( moon_ )
	{
		delete moon_;
		moon_ = NULL;
	}

	moonMat_ = NULL;

	if ( sun_ )
	{
		delete sun_;
		sun_ = NULL;
	}

	sunMat_ = NULL;
}


/**
 *	This method creates the mesh and material for the sun + flares.
 */
void SunAndMoon::createSun( void )
{
	sun_ = new CustomMesh<Moo::VertexXYZNDUV>( D3DPT_TRIANGLESTRIP );

	//this value scales its apparent size
	float z = -6.78f;

	Moo::VertexXYZNDUV vert;
	vert.colour_ = 0x80777777;
	vert.normal_.set( 0,0,1 );

	vert.pos_.set(-1, 1, z );
	vert.uv_.set( 0, 0 );
	sun_->push_back( vert );

	vert.pos_.set( 1, 1, z );
	vert.uv_.set( 1, 0 );
	sun_->push_back( vert );

	vert.pos_.set( -1, -1, z );
	vert.uv_.set( 0, 1 );
	sun_->push_back( vert );

	vert.pos_.set( 1, -1, z );
	vert.uv_.set( 1,1 );
	sun_->push_back( vert );

	sunMat_ = new Moo::EffectMaterial();
	sunMat_->load( BWResource::openSection( s_sunMaterial ) );
}


/**
 *	This method creates the mesh + materials for the moon and mask
 */
void SunAndMoon::createMoon( void )
{	
	moon_ = new CustomMesh<Moo::VertexXYZNDUV>( D3DPT_TRIANGLESTRIP );

	//this value scales its apparent size
	float z = -6.78f;

	Moo::VertexXYZNDUV vert;
	vert.colour_ = 0x80777777;
	vert.normal_.set( 0,0,1 );

	vert.pos_.set( -0.6f, 0.6f, z );
	vert.uv_.set( 0, 0 );	
	moon_->push_back( vert );

	vert.pos_.set( 0.6f, 0.6f, z );
	vert.uv_.set( 1, 0 );	
	moon_->push_back( vert );

	vert.pos_.set( -0.6f, -0.6f, z );
	vert.uv_.set( 0, 1 );	
	moon_->push_back( vert );

	vert.pos_.set( 0.6f, -0.6f, z );
	vert.uv_.set( 1,1 );	
	moon_->push_back( vert );	

	//Create the moon material
	moonMat_ = new Moo::EffectMaterial();	
	moonMat_->load( BWResource::openSection( s_moonMaterial ) );
}


/**
 *	This method draws the sun and moon.
 *	The sun and moon use the timeOfDay object to position themselves
 *	in the sky.
 */
void SunAndMoon::draw()
{	
	if ( !timeOfDay_ )
		return;

	//apply the texture factor
	//TODO : put this into the effect
	//uint32 additiveTextureFactor = FogController::instance().additiveFarObjectTFactor();
	//uint32 textureFactor = FogController::instance().farObjectTFactor();
	//sunMat_->textureFactor( additiveTextureFactor );
	//moonMat_->textureFactor( textureFactor );

	drawSquareMesh( timeOfDay_->lighting().sunTransform, sun_, sunMat_ );
	drawSquareMesh( timeOfDay_->lighting().moonTransform, moon_, moonMat_ );

	//post request for sun flare
	if (!Moo::rc().reflectionScene())
	{
		float SUN_DISTANCE = Moo::rc().camera().farPlane() * 0.9999f;
		Vector3 sunPosition(
			timeOfDay_->lighting().sunTransform.applyPoint( Vector3(0,0,-1.f) ) );	
		sunPosition.normalise();
		float dotp = sunPosition.dotProduct( Moo::rc().invView().applyToUnitAxisVector(2) );	
		Vector3 actualSunPosition( Moo::rc().invView().applyToOrigin() + sunPosition * SUN_DISTANCE );

		LensEffectManager::instance().add( 0, actualSunPosition, sunLensEffect_ );			
	}

	Moo::rc().setVertexShader( NULL );
	Moo::rc().setPixelShader( NULL );
}



std::ostream& operator<<(std::ostream& o, const SunAndMoon& t)
{
	o << "SunAndMoon\n";
	return o;
}

// sun_and_moon.cpp