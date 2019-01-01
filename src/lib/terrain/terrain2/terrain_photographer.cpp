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
#include "terrain_photographer.hpp"

#include "../base_terrain_renderer.hpp"
#include "../base_terrain_block.hpp"
#include "moo/texture_compressor.hpp"
#include "moo/render_target.hpp"

using namespace Terrain;

DECLARE_DEBUG_COMPONENT2( "Terrain", 0 )

class SurfaceHelper
{
public:
	typedef ComObjectWrap<DX::Surface> Surface;

	bool initOffscreenPlain( uint32 width, uint32 height, D3DFORMAT format, 
		D3DPOOL pool )
	{
		BW_GUARD;
		pSurface_ = NULL;
		HRESULT hr = Moo::rc().device()->CreateOffscreenPlainSurface( width, height,
			format, pool, &pSurface_, NULL );
		return SUCCEEDED( hr );
	}

	bool initRenderTarget( uint32 width, uint32 height, D3DFORMAT format )
	{
		BW_GUARD;
		pSurface_ = NULL;

		HRESULT hr = Moo::rc().device()->CreateRenderTarget( width, height, format,
			D3DMULTISAMPLE_NONE, 0, false, &pSurface_, NULL );

		return SUCCEEDED( hr );
	}

	bool initFromMooRenderTarget( Moo::RenderTargetPtr pTarget )
	{
		BW_GUARD;
		bool res = false;
		pSurface_ = NULL;
		DX::Texture* pTexture = (DX::Texture*)pTarget->pTexture();
		if (pTexture)
		{
			HRESULT hr = pTexture->GetSurfaceLevel( 0, &pSurface_ );
			if (SUCCEEDED(hr))
			{
				res = true;
			}
		}
		return res;
	}

	operator DX::Surface*()
	{
		return pSurface_.pComObject();
	}
	
	DX::Surface* operator ->()
	{
		return pSurface_.pComObject();
	}

private:
	ComObjectWrap<DX::Surface> pSurface_;
};

// -----------------------------------------------------------------------------
// Section: TerrainPhotographer
// -----------------------------------------------------------------------------

TerrainPhotographer::TerrainPhotographer()
{

}

TerrainPhotographer::~TerrainPhotographer()
{
	// dereference smart pointer
	pBasePhoto_ = NULL;
}


bool TerrainPhotographer::init( uint32 basePhotoSize )
{
	BW_GUARD;
	bool ret = true;
	
	// re-create base photo if its the wrong size or doesn't exist.
	if ( !pBasePhoto_ || ( pBasePhoto_->width() != basePhotoSize ) )
	{
		// pBasePhoto_ is a smart pointer, just re-assign.
		pBasePhoto_ = new Moo::RenderTarget("terrain photographer");
		ret = pBasePhoto_->create( basePhotoSize, basePhotoSize );

		if (!ret)
		{
			pBasePhoto_ = NULL;
		}
	}
	
	return ret;
}

bool TerrainPhotographer::photographBlock( BaseTerrainBlock*	pBlock,
										   const Matrix&		transform )
{
	BW_GUARD;
	bool res = false;
	if (pBasePhoto_)
	{
		Moo::LightContainerPtr pLightContainer = new Moo::LightContainer;
		pLightContainer->ambientColour( Moo::Colour(1,1,1,1) );

		Moo::LightContainerPtr prclc = Moo::rc().lightContainer();
		Moo::LightContainerPtr prcslc = Moo::rc().specularLightContainer();
		
		Moo::rc().lightContainer( pLightContainer );
		Moo::rc().specularLightContainer( pLightContainer );

		if (pBasePhoto_->push())
		{

			Moo::rc().device()->Clear( 0, NULL, 
				D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1, 0 );

			Moo::rc().pushRenderState( D3DRS_COLORWRITEENABLE );

			Moo::rc().setRenderState( D3DRS_COLORWRITEENABLE, 
				D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | 
				D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA );

			// Save old texture filter setting
 			Moo::GraphicsSetting::GraphicsSettingPtr filterSettingPtr = 
 				Moo::GraphicsSetting::getFromLabel( "TEXTURE_FILTERING" );
			MF_ASSERT( filterSettingPtr && "Missing texture filter option" );
 			int oldFilterOption = filterSettingPtr->activeOption();

			// set new to highest supported option
			filterSettingPtr->selectOption( 0 );

			float imageSize = 100.f;

			Matrix projection;
			projection.orthogonalProjection( imageSize, imageSize, -5000, 5000 );
			Matrix view;
			view.lookAt( Vector3( 50.375f, 0, 49.625f ) + transform.applyToOrigin(), 
				Vector3( 0, -1, 0), Vector3( 0, 0, 1 ) );

			Moo::rc().projection( projection );	
			Moo::rc().view( view );
			Moo::rc().updateViewTransforms();

			Moo::rc().push();

			bool fogState = Moo::rc().fogEnabled();
			Moo::rc().fogEnabled( false );

			// Render
			BaseTerrainRenderer::instance()->drawSingle( pBlock, transform );

			// Reset old texture filter option
 			filterSettingPtr->selectOption( oldFilterOption );

			Moo::rc().popRenderState();

			Moo::rc().fogEnabled( fogState );

			Moo::rc().pop();
			pBasePhoto_->pop();

			res = true;
		}

		Moo::rc().lightContainer( prclc );
		Moo::rc().specularLightContainer( prcslc );
	}

	return res;
}

/**
 *	This method copies the photograph to a texture. If supplied texture is empty, then
 *	new one is created with given format.
 *
 *	@param pDestTexture		An empty texture or an existing texture to reuse.
 *	@param destImageFormat	Format of new texture (if needed).
 *
 */
bool TerrainPhotographer::output(	ComObjectWrap<DX::Texture>&	pDestTexture,
									D3DFORMAT					destImageFormat)
{
	bool				ok = false;

	if (pBasePhoto_->pTexture())
	{
		TextureCompressor	tc( (DX::Texture*)pBasePhoto_->pTexture(), 
								destImageFormat );
		ok = tc.convertTo( pDestTexture );
	}

	return ok;	
}
