/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_COMMON_TERRAIN_BLOCK2_HPP
#define TERRAIN_COMMON_TERRAIN_BLOCK2_HPP

#include "../editor_base_terrain_block.hpp"
#include "resmgr/datasection.hpp"

namespace Terrain
{
	class TerrainHeightMap2;
	class TerrainHoleMap2;
	class DominantTextureMap2;
	class TerrainTextureLayer;

	typedef SmartPointer<TerrainHeightMap2>		TerrainHeightMap2Ptr;
	typedef SmartPointer<TerrainHoleMap2>		TerrainHoleMap2Ptr;
	typedef SmartPointer<DominantTextureMap2>	DominantTextureMap2Ptr;
	typedef SmartPointer<TerrainTextureLayer>	TerrainTextureLayerPtr;
}

namespace Terrain
{
#ifdef EDITOR_ENABLED
	typedef EditorBaseTerrainBlock		CommonTerrainBlock2Base;
#else
	typedef BaseTerrainBlock			CommonTerrainBlock2Base;
#endif // EDITOR_ENABLED

    /**
     *  This class implements the BaseTerrainBlock interface for the
	 *	second generation of terrain.
     */
    class CommonTerrainBlock2 : public CommonTerrainBlock2Base
    {
    public:
        CommonTerrainBlock2(TerrainSettingsPtr pSettings);
        virtual ~CommonTerrainBlock2();

        bool load(	std::string const &filename, 
					Matrix  const &worldTransform,
					Vector3 const &cameraPosition,
					std::string *error = NULL );

        TerrainHeightMap &heightMap();
        TerrainHeightMap const &heightMap() const;

        TerrainHoleMap &holeMap();
        TerrainHoleMap const &holeMap() const;
		uint32 holesMapSize() const;
		uint32 holesSize() const;

		DominantTextureMapPtr dominantTextureMap();
        DominantTextureMapPtr const dominantTextureMap() const;

        virtual BoundingBox const &boundingBox() const;

        virtual bool collide
        (
            Vector3                 const &start, 
            Vector3                 const &end,
            TerrainCollisionCallback *callback
        ) const;

        virtual bool collide
        (
            WorldTriangle           const &start, 
            Vector3                 const &end,
            TerrainCollisionCallback *callback
        ) const;
        
		virtual float heightAt(float x, float z) const;
        virtual Vector3 normalAt(float x, float z) const;

		const std::string dataSectionName() const { return "terrain2"; };

		TerrainSettingsPtr	settings() const { return settings_; }

		DominantTextureMap2Ptr dominantTextureMap2() const;
		void dominantTextureMap2(DominantTextureMap2Ptr dominantTextureMap);

		virtual TerrainHeightMap2Ptr heightMap2() const;

	protected:
		TerrainSettingsPtr	settings_;
   
        bool internalLoad(
			std::string const &filename, 
			DataSectionPtr terrainDS,
			Matrix  const &worldTransform,
			Vector3 const &cameraPosition,
			uint32 heightMapLod,
			std::string *error = NULL);

		void heightMap2( TerrainHeightMap2Ptr heightMap2 );
		void holeMap2( TerrainHoleMap2Ptr holeMap2 );
		TerrainHoleMap2Ptr holeMap2() const;
		
		// data
		mutable BoundingBox         bb_;
    private:
	    TerrainHeightMap2Ptr        pHeightMap_;
        TerrainHoleMap2Ptr          pHoleMap_;
		DominantTextureMap2Ptr		pDominantTextureMap_;
    };
}


#endif // TERRAIN_COMMON_TERRAIN_BLOCK2_HPP
