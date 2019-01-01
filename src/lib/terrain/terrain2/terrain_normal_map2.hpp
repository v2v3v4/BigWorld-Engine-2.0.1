/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_NORMAL_MAP2_HPP
#define TERRAIN_NORMAL_MAP2_HPP

#include "../terrain_data.hpp"
#include "cstdmf/bgtask_manager.hpp"
#include "terrain_blends.hpp"

namespace Terrain
{

typedef SmartPointer<class TerrainHeightMap2> TerrainHeightMap2Ptr;

/**
 *	This class implements the normal map resource for the terrain block 2
 *	It handles creation of normal maps and also streaming of hight quality
 *	maps when needed.
 */
class TerrainNormalMap2 : public SafeReferenceCount
{
public:

	TerrainNormalMap2();
	~TerrainNormalMap2();

	bool init(const std::string& terrainResource);

#ifdef EDITOR_ENABLED

	bool save( DataSectionPtr pTerrainSection );
	
	bool generate( const TerrainHeightMap2Ptr	heightMap,
		NormalMapQuality quality, uint32 size );
#endif

	inline void evaluate( uint8 renderTextureMask );

	bool isLoading() const
	{
		return pTask_ != NULL;
	}

	void stream();

	/**
	 * Get the highest detail available normal map
	 */
	ComObjectWrap<DX::Texture> pMap() const 
	{ 
		return pNormalMap_.hasComObject() ? pNormalMap_ : pLodNormalMap_;
	}

	/**
	 *  Get the lod normal map
	 */
	ComObjectWrap<DX::Texture> pLodMap() const 
	{
		return pLodNormalMap_;
	}

	
	/**
	 * Get the size of the highest detail available normal map
	 */
	uint32 size() const 
	{ 
		return pNormalMap_.hasComObject() ? size_ : lodSize_; 
	}

	/*
	 *	Get the lod normal map size
	 */
	uint32 lodSize() const 
	{ 
		return lodSize_; 
	}
private:
	/**
	 *	This class is the background taks that streams the normal map
	 */
	class NormalMapTask : public BackgroundTask
	{
	public:
		NormalMapTask( SmartPointer<TerrainNormalMap2> pMap );
		~NormalMapTask();
		void doBackgroundTask( BgTaskManager & mgr );
		void doMainThreadTask( BgTaskManager & mgr );
	private:
		SmartPointer<TerrainNormalMap2> pMap_;
		ComObjectWrap<DX::Texture>  pNormalMap_;
		uint32						size_;
	};

	static ComObjectWrap<DX::Texture> loadMap( DataSectionPtr pMapSection, 
		uint32& size );

#ifdef EDITOR_ENABLED
	static ComObjectWrap<DX::Texture> generateMap( 
		const TerrainHeightMap2Ptr	heightMap, 
		NormalMapQuality quality, uint32 size );
	static bool saveMap( DataSectionPtr pSection, 
		ComObjectWrap<DX::Texture> pMap );
#endif

	ComObjectWrap<DX::Texture>  pNormalMap_;
	ComObjectWrap<DX::Texture>  pLodNormalMap_;
	uint32					    size_;
	uint32						lodSize_;

	bool						wantQualityMap_;

	std::string					terrainResource_;
	NormalMapTask*				pTask_;
};

void TerrainNormalMap2::evaluate( uint8 renderTextureMask )
{
	wantQualityMap_ = false;
	if (renderTextureMask & TerrainRenderer2::RTM_PreloadNormals)
		wantQualityMap_ = true;
}

typedef SmartPointer<TerrainNormalMap2> TerrainNormalMap2Ptr;

}


#endif // TERRAIN_NORMAL_MAP2_HPP
