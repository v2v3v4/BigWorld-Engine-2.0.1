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
#include "flash_bang_effect.hpp"
#include "custom_mesh.hpp"
#include "geometrics.hpp"
#include "moo/render_target.hpp"

DECLARE_DEBUG_COMPONENT2( "Romp", 0 )

// -----------------------------------------------------------------------------
// Section: FlashBangEffect
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
FlashBangEffect::FlashBangEffect()
: fadeValues_( 0, 0, 0, 0 ),
  haveLastFrame_( false ),
  pRT_( NULL )
{
	// set up the material and texture stages.
	blendMaterial_.zBufferWrite( false );
	blendMaterial_.zBufferRead( false );
	blendMaterial_.fogged( false );
	blendMaterial_.destBlend( Moo::Material::ONE );
	blendMaterial_.srcBlend( Moo::Material::SRC_ALPHA );
	blendMaterial_.alphaBlended( true );
	blendMaterial_.textureFactor( 0 );

	Moo::TextureStage ts;
	ts.colourOperation( Moo::TextureStage::ADD, Moo::TextureStage::TEXTURE, Moo::TextureStage::TEXTURE_FACTOR );
	ts.alphaOperation( Moo::TextureStage::SELECTARG2, Moo::TextureStage::TEXTURE, Moo::TextureStage::TEXTURE_FACTOR );
	ts.minFilter( Moo::TextureStage::LINEAR );
	ts.magFilter( Moo::TextureStage::LINEAR );
	ts.useMipMapping( false );
	ts.textureWrapMode( Moo::TextureStage::CLAMP );

	blendMaterial_.addTextureStage( ts );
	blendMaterial_.addTextureStage( Moo::TextureStage::TextureStage() );

	pRT_ = new Moo::RenderTarget("FlashBang");
	pRT_->create(512,512);

	MF_WATCH( "Client Settings/fx/FlashBang/fadeValues",
		fadeValues_,
		Watcher::WT_READ_ONLY,
		"Displays the current colour and alpha fade value for the effect." );
}


/**
 *	Destructor.
 */
FlashBangEffect::~FlashBangEffect()
{
	delete pRT_;
}


void FlashBangEffect::draw()
{
	// if the effect is completely faded out, don't draw.
	if (fadeValues_.w <= 0.f)
	{
		haveLastFrame_ = false;
		return;
	}

	// get the device
	DX::Device* pDev = Moo::rc().device();
	DX::Texture* pTexture = NULL;


	// blend last frame's back buffer onto the back buffer
	if (haveLastFrame_)
	{
		pTexture = static_cast<IDirect3DTexture9*>(pRT_->pTexture());
	}

	// Set the blend factor for blending the last frame on top of this one.
	// backbuffer = backbuffer + ((frontbuffer + fadevalues.rgb) * fadevalues.a)
	blendMaterial_.textureFactor( (Moo::Colour&) fadeValues_ );

	float sHeight = Moo::rc().screenHeight() + 4.f;
	float sWidth = Moo::rc().screenWidth() + 4.f;

	CustomMesh<Moo::VertexTLUV> mesh( D3DPT_TRIANGLESTRIP );
	Geometrics::createRectMesh( Vector2(-2,-2),
		Vector2(sWidth,sHeight),
		Moo::Colour(0xffffffff),
		false,
		mesh );

	// set up the rendering states
	blendMaterial_.set();
	Moo::rc().setTexture( 0, pTexture );
	pDev->SetVertexShader( NULL );

	mesh.draw();

	// clean up after ourselves.
	Moo::rc().setTexture( 0, NULL );	

	// save the current back buffer to our render target
	ComObjectWrap<DX::Surface> pSrc = NULL;
	ComObjectWrap<DX::Surface> pDest = NULL;
	pSrc = Moo::rc().getRenderTarget( 0 );
	((IDirect3DTexture9*)pRT_->pTexture())->GetSurfaceLevel(0,&pDest);
	if (pSrc.hasComObject() && pDest.hasComObject())
	{
		pDev->StretchRect( pSrc.pComObject(), NULL, pDest.pComObject(), NULL, D3DTEXF_LINEAR );
		pSrc = NULL;
		pDest = NULL;
		haveLastFrame_ = true;
	}
}

// flash_bang_effect.cpp
