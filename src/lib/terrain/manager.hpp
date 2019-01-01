/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_HPP
#define TERRAIN_HPP


#include "cstdmf/init_singleton.hpp"
#include "cstdmf/smartpointer.hpp"
#include "resmgr/datasection.hpp"

#ifndef MF_SERVER

#include "terrain1/terrain_renderer1.hpp"
#include "terrain2/terrain_renderer2.hpp"
#include "terrain2/terrain_lod_controller.hpp"
#include "terrain_graphics_options.hpp"

#ifdef EDITOR_ENABLED
#include "terrain2/ttl2cache.hpp"
#endif // EDITOR_ENABLED

#endif // MF_SERVER


namespace Terrain
{


/**
 *	This class is the main entry point for the Terrain Library, holding library
 *	objects such as the terrain renderers. It also keeps track of the current
 *	active renderer, and a reference to the terrain2 defaults DataSection.
 *
 *	The init method must be called before using any terrain code, and fini 
 *	must be called at the end of the app to release resources.
 *
 *  NOTE: We shouldn't be adding new global objects here, we should instead try
 *  to keep objects independent of each other and avoid singletons altogether.
 */
class Manager : public InitSingleton<Manager>
{
public:
	Manager();

	const DataSectionPtr pTerrain2Defaults() const { return pTerrain2Defaults_; }

#ifndef MF_SERVER

	void currentRenderer( BaseTerrainRenderer* newInstance );
	BaseTerrainRenderer* currentRenderer();

	const BasicTerrainLodController& lodController() const { return lodController_; }
	BasicTerrainLodController& lodController() { return lodController_; }

	const TerrainGraphicsOptions& graphicsOptions() const { return graphicsOptions_; }
	TerrainGraphicsOptions& graphicsOptions() { return graphicsOptions_; }

	void wireFrame( bool wireFrame ) { wireFrame_ = wireFrame; }
	bool wireFrame() const { return wireFrame_; }

#ifdef EDITOR_ENABLED
	TTL2Cache& ttl2Cache() { return ttl2Cache_; }
#endif // EDITOR_ENABLED

#endif // MF_SERVER

protected:
	bool doInit();
	bool doFini();

	DataSectionPtr				pTerrain2Defaults_;

#ifndef MF_SERVER

	BaseTerrainRenderer*		currentRenderer_;
	BasicTerrainLodController	lodController_;
	TerrainGraphicsOptions		graphicsOptions_;
	bool						wireFrame_;
#ifdef EDITOR_ENABLED
	TTL2Cache					ttl2Cache_;
#endif // EDITOR_ENABLED

#endif // MF_SERVER
};


} // namespace Terrain


#endif // TERRAIN_HPP
