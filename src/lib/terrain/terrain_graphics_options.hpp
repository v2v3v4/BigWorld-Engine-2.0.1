/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_GRAPHICS_OPTIONS_HPP
#define TERRAIN_GRAPHICS_OPTIONS_HPP

#include "resmgr/datasection.hpp"
#include "cstdmf/concurrency.hpp"

namespace Terrain
{

class TerrainSettings;

/**
 *  This class implements the terrain graphics options.
 *	The graphics options apply to the terrain settings.
 *	Currently the terrain graphics options let you modify
 *	the lod distances and also the top lod level.
 */
class TerrainGraphicsOptions
{
public:
	TerrainGraphicsOptions();

	static TerrainGraphicsOptions* instance();
	
	void init( DataSectionPtr pTerrain2Defaults );

	void addSettings( TerrainSettings* pSettings );
	void delSettings( TerrainSettings* pSettings );

	float lodModifier() const { return lodModifier_; }

private:

	float lodModifier_;

	void initOptions( DataSectionPtr pOptions );
	
	void setLodDistanceOption(int optionIdx);
	void setTopLodOption(int optionIdx);

	std::vector<float> lodDistanceOptions_;

	SmartPointer<Moo::GraphicsSetting> pLodDistanceSettings_;
	SmartPointer<Moo::GraphicsSetting> pTopLodSettings_;

	std::vector< TerrainSettings* > settings_;

	SimpleMutex	mutex_;
};

} // namespace Terrain

#endif //  TERRAIN_GRAPHICS_OPTIONS_HPP
