/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TEXTURE_TOOL_VIEW_HPP
#define TERRAIN_TEXTURE_TOOL_VIEW_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "gizmo/tool.hpp"


/**
 *	This class implements a toolview that draws into a render
 *	target, and projects the render target onto the terrain.
 **/
class TerrainTextureToolView : public TextureToolView
{
	Py_Header( TerrainTextureToolView, TextureToolView )
public:
	TerrainTextureToolView(
		const std::string& resourceID = "resources/maps/gizmo/disc.dds",
		PyTypePlus * pType = &s_type_ );

	virtual void render( const Tool& tool );

	PyObject *		pyGetAttribute( const char * attr );
	int				pySetAttribute( const char * attr, PyObject * value );

	PY_RW_ATTRIBUTE_DECLARE( rotation_, rotation )
	PY_RW_ATTRIBUTE_DECLARE( showHoles_, showHoles )

	PY_FACTORY_DECLARE()
private:
	VIEW_FACTORY_DECLARE( TerrainTextureToolView() )

	float rotation_;
	bool  showHoles_;
};


#endif // TERRAIN_TEXTURE_TOOL_VIEW_HPP
