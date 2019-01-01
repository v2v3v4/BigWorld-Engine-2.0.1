/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_CHUNK_TEXTURE_TOOL_VIEW_HPP
#define TERRAIN_CHUNK_TEXTURE_TOOL_VIEW_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "gizmo/tool.hpp"


/**
 *	This class implements a toolview that draws a texture
 *	over each relevant chunk,
 *
 *	This view can scale its material bitmap.
 **/
class TerrainChunkTextureToolView : public TextureToolView
{
	Py_Header( TerrainChunkTextureToolView, TextureToolView )

public:
	explicit TerrainChunkTextureToolView(
		const std::string &		resourceID	= "resources/maps/gizmo/square.dds",
		PyTypePlus *			pType		= &s_type_ );

	virtual void render( const Tool & tool );

	void numPerChunk( float num );

	PyObject *		pyGetAttribute( const char * pAttr );
	int				pySetAttribute( const char * pAttr, PyObject * pValue );

	PY_RW_ATTRIBUTE_DECLARE( numPerChunk_, numPerChunk )

	PY_FACTORY_DECLARE()

private:
	VIEW_FACTORY_DECLARE( TerrainChunkTextureToolView() )

	float numPerChunk_;
};


#endif // TERRAIN_CHUNK_TEXTURE_TOOL_VIEW_HPP
