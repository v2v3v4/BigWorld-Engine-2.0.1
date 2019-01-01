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

#include "mask_overlay.hpp"

#include "worldeditor/terrain/editor_chunk_terrain.hpp"
#include "worldeditor/terrain/editor_chunk_terrain_projector.hpp"

#include "chunk/base_chunk_space.hpp"


/**
 *	This is the MaskOverlay constructor.
 *
 *  @param terrain			The terrain that the MaskOverlay should draw over.
 *  @param mask				The mask image itself.
 */
MaskOverlay::MaskOverlay(
		EditorChunkTerrainPtr		pTerrain, 
		const Moo::Image<uint8> &	mask ):
	pTerrain_( pTerrain ),
	pTexture_( NULL )
{
	BW_GUARD;

	pTexture_ = 
		new Moo::ImageTextureARGB(
			mask.width(), 
			mask.height(),
			Moo::ImageTextureARGB::recommendedFormat(),
			1 );	// only need the top-level MIP

	pTexture_->lock();
	Moo::ImageTextureARGB::ImageType & image = pTexture_->image();

	for (uint32 y = 0; y < mask.height(); ++y)
	{
		uint8 * pSrc    = mask.getRow( y );
		uint8 * pSrcEnd = pSrc + mask.width();
		Moo::PackedColour * pDst = image.getRow( y );
		for (;pSrc != pSrcEnd; ++pSrc, ++pDst)
		{
			*pDst = D3DCOLOR_ARGB( 0, *pSrc/4, 0, 0 ); // lower red a little
		}
	}

	pTexture_->unlock();
}


/**
 *	This is called to render the mask overlay.
 */
/*virtual*/ void MaskOverlay::render()
{
	BW_GUARD;

	if (pTerrain_ == NULL || pTexture_ == NULL)
	{
		return;
	}

	// Check to see if the chunk has been unloaded
	Chunk * pChunk = pTerrain_->chunk();
	if (pChunk == NULL || !pChunk->loaded())
	{
		return;
	}

	EditorChunkTerrainPtrs spChunks;
	spChunks.push_back( pTerrain_ );

	EditorChunkTerrainProjector::instance().projectTexture(
		pTexture_,
		GRID_RESOLUTION,	// scale
		0.0f,				// no rotation
		pTerrain_->chunk()->transform().applyToOrigin() +
			Vector3( GRID_RESOLUTION/2.0f, 0.f, GRID_RESOLUTION/2.0f ),
		D3DTADDRESS_CLAMP,	// wrap mode
		spChunks,			// affected chunks
		false );			// don't show holes
}
