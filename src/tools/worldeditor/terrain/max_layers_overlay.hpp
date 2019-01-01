/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MAX_WARNING_OVERLAY_HPP
#define MAX_WARNING_OVERLAY_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/misc/editor_renderable.hpp"


class MaxLayersOverlay : public EditorRenderable
{
public:
	explicit MaxLayersOverlay( EditorChunkTerrain * pTerrain );

	void render();

private:
	Moo::BaseTexturePtr pTexture_;
	EditorChunkTerrain* pTerrain_;
};


typedef SmartPointer<MaxLayersOverlay> MaxLayersOverlayPtr;


#endif // MAX_WARNING_OVERLAY_HPP
