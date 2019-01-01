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
#include "dapple.hpp"
#include "geometrics.hpp"
#include "cstdmf/debug.hpp"
#include "chunk/base_chunk_space.hpp"	//for GRID_RESOLUTION

DECLARE_DEBUG_COMPONENT2( "romp", 0 )

static int s_dappleEnable = 1;
static float s_tile = 20.f;

Dapple::Dapple():
	pDappleTex_( NULL ),
	dappleMaterial_( NULL ),
	spaceMin_( -10.f, -10.f ),
	spaceMax_( -10.f, 10.f )
{
	MF_WATCH( "Client Settings/Clouds/dapple enabled",
		s_dappleEnable,
		Watcher::WT_READ_WRITE,
		"Enable dapple mapping (not supported)" );

	MF_WATCH( "Client Settings/Clouds/dapple map tile",
		s_tile,
		Watcher::WT_READ_WRITE,
		"Tiling factor of dapple map (not supported)" );
}


bool Dapple::dappleEnabled()
{
	return (s_dappleEnable != 0) && (pDappleTex_.getObject() != NULL);
}


/**
 *	This method is called by the sky when it is
 *	activated, i.e. when the camera enters this
 *	space.
 */
void Dapple::activate( const EnviroMinder& enviro,
					  DataSectionPtr pSpaceSettings,
					  SkyLightMap* skyLightMap )
{
	spaceMin_.x = (float)pSpaceSettings->readInt( "bounds/minX", (int)spaceMin_.x );
	spaceMin_.y = (float)pSpaceSettings->readInt( "bounds/minY", (int)spaceMin_.y );
	spaceMax_.x = (float)pSpaceSettings->readInt( "bounds/maxX", (int)spaceMax_.x );
	spaceMax_.y = (float)pSpaceSettings->readInt( "bounds/maxY", (int)spaceMax_.y );

	std::string dappleBmpName = pSpaceSettings->readString( "dappleMask", "" );
	if (dappleBmpName.empty())
	{
		pDappleTex_ = NULL;
	}
	else
	{
		pDappleTex_ = Moo::TextureManager::instance()->get( dappleBmpName );
	}
	
	if (pDappleTex_)
	{
		bool loadedOK = false;

		std::string mfmName = pSpaceSettings->readString( "dappleMaterial" );
		if ( mfmName.empty() )
		{
			ERROR_MSG( "Missing dapple mfm section in EffectLightMap data\n" );
			pDappleTex_ = NULL;
			return;
		}

		dappleMaterial_ = new Moo::EffectMaterial();
		DataSectionPtr pSect = BWResource::openSection( mfmName );
		if ( pSect )
		{
			loadedOK = dappleMaterial_->load( pSect );							
		}		

		if (loadedOK)
		{
			dappleMaterial_->pEffect()->pEffect()->GetFloat( "tile", &s_tile );

			// add ourselves as a sky light map contributor
			skyLightMap->addContributor( *this );
		}
		else
		{
			delete dappleMaterial_;
			dappleMaterial_ = NULL;
			pDappleTex_ = NULL;
			ERROR_MSG( "Could not load light map dapple effect %s", mfmName.c_str() );
		}
	}
}


/**
 *	This method is called by the sky when it is
 *	deactivated, i.e. when the camera leaves this
 *	space.
 */
void Dapple::deactivate( const EnviroMinder& enviro,
						SkyLightMap* skyLightMap )
{
	pDappleTex_ = NULL;
	if (dappleMaterial_)
	{
		delete dappleMaterial_;
		dappleMaterial_ = NULL;
	}

	// remove ourselves as a sky light map contributor
	skyLightMap->delContributor( *this );
}


bool Dapple::needsUpdate()
{
	return this->dappleEnabled();
}


/** 
 *	This private method applies a dappled light map to the world. 
 *
 *	The dapple map is written into the colour channel of the sky
 *	light map.	The effect adds the colour to the underlying
 *	diffuse lighting, allowing over-brightening of areas receiving
 *	dappled light.
 *
 *	The xOffset variable is passed in from the calling function.  It
 *	represents how far across the texture map is shifted to account
 *	for the sun going down.  When the sun is low, we fade out the sky
 *	light map.
 */
void Dapple::render( SkyLightMap* lightMap, Moo::EffectMaterial* material, float sunAngle )
{		
	if (!s_dappleEnable)
		return;

	float fadeAmount = Math::clamp( 0.f, fabsf(sunAngle) - 0.5f, 1.f );
	
	Vector2 camPos( Moo::rc().invView().applyToOrigin().x, Moo::rc().invView().applyToOrigin().z );

	Moo::rc().push();
	Moo::rc().world( Matrix::identity );
	Moo::rc().view( Matrix::identity );
	Moo::rc().projection( Matrix::identity );	

	Moo::Colour colour(1.0,1.0,1.0,1.0);
	CustomMesh<Moo::VertexXYZUV> mesh( D3DPT_TRIANGLESTRIP );	
	Geometrics::createRectMesh(
		Vector2(-1.f,1.f),
		Vector2(1.f,-1.f), 
		colour,		
		mesh );	
	
	float minX = spaceMin_.x * GRID_RESOLUTION;
	float maxX = spaceMax_.x * GRID_RESOLUTION;
	float minY = spaceMin_.y * GRID_RESOLUTION;
	float maxY = spaceMax_.y * GRID_RESOLUTION;
	float spaceRangeX = (maxX - minX + GRID_RESOLUTION);
	float spaceRangeY = (maxY - minY + GRID_RESOLUTION);

	//Vector2 centre( minX + spaceRangeX / 2.f, minY + spaceRangeY / 2.f );
	static bool s_done = false;
	static Vector2 s_centre(
		(0.f - minX) / (spaceRangeX + 1.f),
		-(0.f - minY) / (spaceRangeY + 1.f));
	if (!s_done)
	{
		s_done = true;
		MF_WATCH( "center",
			s_centre,
			Watcher::WT_READ_WRITE,
			"Alleged centre of dapple map (not supported)" );
	}
	//Vector2 centre( 0.f,0.f );
	Vector2 centre( s_centre );
	
	//uv is world position of edges of space, relative to camera
	float farPlane = Moo::rc().camera().farPlane();
	float farPlaneRange = 2.f * farPlane;

	//the map fits 2*far plane in it, so scale accordingly
	float scaleX = farPlaneRange / spaceRangeX;
	float scaleZ = farPlaneRange / spaceRangeY;

	//this is how much of the space's map we can fit on our mesh
	Vector2 uvSize(farPlane / spaceRangeX, farPlane / spaceRangeY);

	//camera position
	Vector2 camUV( camPos );
	camUV /= farPlaneRange;
	camUV.x *= scaleX;
	camUV.y *= scaleZ;
	camUV.x += centre.x;
	camUV.y = centre.y - camUV.y;

	//calculate corners as offsets to camera position
	mesh[0].uv_ = camUV + Vector2( -uvSize.x, -uvSize.y );
	mesh[1].uv_ = camUV + Vector2( uvSize.x, -uvSize.y );
	mesh[2].uv_ = camUV + Vector2( -uvSize.x, uvSize.y );
	mesh[3].uv_ = camUV + Vector2( uvSize.x, uvSize.y );		

	if ( dappleMaterial_->begin() )
	{
		for ( uint32 i=0; i<dappleMaterial_->nPasses(); i++ )
		{
			dappleMaterial_->beginPass(i);
			D3DXHANDLE param = dappleMaterial_->pEffect()->pEffect()->GetParameterByName( NULL, "tile" );
			dappleMaterial_->pEffect()->pEffect()->SetFloat( param, s_tile );
			param = dappleMaterial_->pEffect()->pEffect()->GetParameterByName( NULL, "fade" );
			dappleMaterial_->pEffect()->pEffect()->SetFloat( param, fadeAmount );
			param = dappleMaterial_->pEffect()->pEffect()->GetParameterByName( NULL, "dappleMask" );
			dappleMaterial_->pEffect()->pEffect()->SetTexture( param, pDappleTex_->pTexture() );
			dappleMaterial_->pEffect()->pEffect()->CommitChanges();
			mesh.drawEffect();
			dappleMaterial_->endPass();
		}
		dappleMaterial_->end();
	}	

	Moo::rc().pop();
}