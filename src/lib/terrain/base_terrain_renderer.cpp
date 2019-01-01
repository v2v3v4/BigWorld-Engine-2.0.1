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
#include "base_terrain_renderer.hpp"
#include "manager.hpp"

// TODO: We should get rid of globals and externs!!!
bool g_drawTerrain = true;

namespace Terrain
{

/**
 *	This method is a wrapper for the manager's currentRenderer setter method.
 *
 *	@param newInstance	The new current renderer
 *	@see Terrain::Manager::currentRenderer
 */
/*static*/ void BaseTerrainRenderer::instance( BaseTerrainRenderer* newInstance )
{
	Manager::instance().currentRenderer( newInstance );
}

/**
 *	This method is a wrapper for the manager's currentRenderer getter method.
 *
 *	@param newInstance	The new current renderer
 *	@see Terrain::Manager::currentRenderer
 */
/*static*/ BaseTerrainRenderer* BaseTerrainRenderer::instance()
{
	return Manager::instance().currentRenderer();
}

/**
 *  This method sets a flag that indicates whether or not hole map is used during 
 *  terrain texture rendering.
 * 
 *  @return old value of the flag
 */
bool BaseTerrainRenderer::enableHoleMap( bool value ) 
{ 
	bool ret = holeMapEnabled_;
	holeMapEnabled_ = value; 

	return ret;
}

} // namespace Terrain
