/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MASK_OVERLAY_HPP
#define MASK_OVERLAY_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"

#include "worldeditor/misc/editor_renderable.hpp"

#include "moo/image_texture.hpp"


/**
 *	This class is used to draw the terrain mask.
 */
class MaskOverlay : public EditorRenderable
{
public:
	MaskOverlay( EditorChunkTerrainPtr pTerrain, 
		const Moo::Image<uint8> & mask );

	/*virtual*/ void render();

private:
	EditorChunkTerrainPtr		pTerrain_;
	Moo::ImageTextureARGBPtr	pTexture_;
};


#endif // MASK_OVERLAY_HPP
