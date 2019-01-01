/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_COMMON_TERRAIN_BLOCK1_HPP
#define TERRAIN_COMMON_TERRAIN_BLOCK1_HPP

#include "../editor_base_terrain_block.hpp"
#include "resmgr/datasection.hpp"

namespace Terrain
{
	class TerrainHeightMap1;
	class TerrainHoleMap1;
	class DominantTextureMap;
	class TerrainTextureLayer;

	typedef SmartPointer<TerrainHeightMap1>		TerrainHeightMap1Ptr;
	typedef SmartPointer<TerrainHoleMap1>		TerrainHoleMap1Ptr;
	typedef SmartPointer<DominantTextureMap>	DominantTextureMapPtr;
	typedef SmartPointer<TerrainTextureLayer>	TerrainTextureLayerPtr;
}

namespace Terrain
{
#ifdef EDITOR_ENABLED
	typedef EditorBaseTerrainBlock		CommonTerrainBlock1Base;
#else
	typedef BaseTerrainBlock			CommonTerrainBlock1Base;
#endif // EDITOR_ENABLED

    /**
     *  This class implements the BaseTerrainBlock interface for the
	 *	second generation of terrain.
     */
    class CommonTerrainBlock1 : public CommonTerrainBlock1Base
    {
    public:
        CommonTerrainBlock1();
        virtual ~CommonTerrainBlock1();

        bool load(	std::string const &filename, 
					Matrix  const &worldTransform,
					Vector3 const &cameraPosition,
					std::string *error = NULL);

		uint32 blocksWidth() const { return blocksWidth_; };
		uint32 blocksHeight() const { return blocksHeight_; };
		uint32 width() const { return width_; };
		uint32 height() const { return height_; };

		TerrainHeightMap &heightMap();
        TerrainHeightMap const &heightMap() const;

        TerrainHoleMap &holeMap();
        TerrainHoleMap const &holeMap() const;

		DominantTextureMapPtr dominantTextureMap();
        DominantTextureMapPtr const dominantTextureMap() const;

        BoundingBox const &boundingBox() const;

        bool 
        collide
        (
            Vector3                 const &start, 
            Vector3                 const &end,
            TerrainCollisionCallback *callback
        ) const;

        bool 
        collide
        (
            WorldTriangle           const &start, 
            Vector3                 const &end,
            TerrainCollisionCallback *callback
        ) const;

        float heightAt(float x, float z) const;
        Vector3 normalAt(float x, float z) const;

		const std::string dataSectionName() const { return "terrain"; };

		// methods used when parsing the 'terrain' data section
		uint32 numMapElements() const;
		uint32 textureNameOffset( uint32 index ) const;
		uint32 textureNamesOffset() const;
		uint32 heightsOffset() const;
		uint32 blendsOffset() const;
		uint32 shadowsOffset() const;
		uint32 holesOffset() const;
		uint32 textureNameSize() const { return textureNameSize_; }
		float spacing() const { return spacing_; }
		uint32 verticesWidth() const { return verticesWidth_; }
		uint32 verticesHeight() const { return verticesHeight_; }
		uint32 detailWidth() const { return detailWidth_; }
		uint32 detailHeight() const { return detailHeight_; }

	protected:
        typedef std::vector<TerrainTextureLayerPtr> TextureLayers;

		#pragma pack( push, 1 )
		struct TerrainBlockHeader
		{
			uint32		version_;
			uint32		heightMapWidth_;
			uint32		heightMapHeight_;
			float		spacing_;
			uint32		nTextures_;
			uint32		textureNameSize_;
			uint32		detailWidth_;
			uint32		detailHeight_;
			uint32		padding_[64 - 8];
		};
		#pragma pack( pop )

		virtual bool postLoad
        (
            std::string     const &filename, 
            Matrix          const &worldTransform,
            Vector3         const &cameraPosition,
            DataSectionPtr  pTerrain,
            TextureLayers   &textureLayers,
            std::string     *error 
        );

		void dominantTextureMap(DominantTextureMapPtr dominantTextureMap);

    private:
		float spacing_;
		uint32 width_;
		uint32 height_;
		uint32 blocksWidth_;
		uint32 blocksHeight_;
		uint32 verticesWidth_;
		uint32 verticesHeight_;
		uint32 detailWidth_;
		uint32 detailHeight_;
		uint32 numMapElements_;
		uint32 numTextures_;
		uint32 textureNameSize_;

	    TerrainHeightMap1Ptr        pHeightMap_;
        TerrainHoleMap1Ptr          pHoleMap_;
		DominantTextureMapPtr       pDominantTextureMap_;
        mutable BoundingBox         bb_;
    };
}


#endif // TERRAIN_COMMON_TERRAIN_BLOCK1_HPP
