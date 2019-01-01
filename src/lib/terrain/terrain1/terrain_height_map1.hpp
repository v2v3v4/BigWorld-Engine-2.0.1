/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TERRAIN_HEIGHT_MAP1_HPP
#define TERRAIN_TERRAIN_HEIGHT_MAP1_HPP

#include "../terrain_height_map.hpp"
#include "../base_terrain_block.hpp"

namespace Terrain
{
    class CommonTerrainBlock1;
    class TerrainCollisionCallback;
}

namespace Terrain
{
    /**
     *  This class provides access to terrain height map data for the 
     *  second generation of terrain.
     */
    class TerrainHeightMap1 : public TerrainHeightMap
    {
    public:
        explicit TerrainHeightMap1(CommonTerrainBlock1 &terrainBlock1);

        ~TerrainHeightMap1();      

        uint32 width() const;
        uint32 height() const;

#ifdef EDITOR_ENABLED
		// Editor specific functionality
		bool lock(bool readOnly);
        ImageType &image();        
		bool unlock();
		bool save( DataSectionPtr pSection ) const;
#endif	// EDITOR_ENABLED

		ImageType const &image() const;

		bool load(DataSectionPtr dataSection, std::string *error = NULL);
        BaseTerrainBlock &baseTerrainBlock();
        BaseTerrainBlock const &baseTerrainBlock() const;

        float spacingX() const;
        float spacingZ() const;

        uint32 blocksWidth() const;
        uint32 blocksHeight() const;

        uint32 verticesWidth() const;
        uint32 verticesHeight() const;

        uint32 polesWidth() const;
        uint32 polesHeight() const;

        uint32 xVisibleOffset() const;
        uint32 zVisibleOffset() const;

        float minHeight() const;
        float maxHeight() const;

        Vector3 normalAt(int x, int z) const;
        Vector3 normalAt(float x, float z) const;

        float heightAt(int x, int z) const;
		float heightAt(float x, float z) const;

	    bool collide
        ( 
            Vector3             const &source, 
            Vector3             const &extent,
		    TerrainCollisionCallback *pCallback 
        ) const;

	    bool collide
        ( 
            WorldTriangle       const &source, 
            Vector3             const &extent,
		    TerrainCollisionCallback *pCallback 
        ) const;

	    bool hmCollide
        (
            Vector3             const &originalSource,
            Vector3             const &source, 
            Vector3             const &extent,
		    TerrainCollisionCallback *pCallback
        ) const;

	    bool hmCollide
        (
            WorldTriangle		const &source, 
            Vector3             const &extent,
			float				xStart,
			float				zStart,
			float				xEnd,
			float				zEnd,
		    TerrainCollisionCallback *pCallback
        ) const;		

    private:

		bool 
		checkGrid
		( 
			int  				gridX, 
			int  				gridZ, 
			Vector3				const &source, 
			Vector3				const &extent, 
			TerrainCollisionCallback* pCallback 
		) const;

		bool 
		checkGrid
		( 
			int  				gridX, 
			int  				gridZ, 
			WorldTriangle		const &source, 
			Vector3				const &extent, 
			TerrainCollisionCallback* pCallback 
		) const;

#ifdef EDITOR_ENABLED
		// editor specific functionality
		size_t				lockCount_;
		bool				lockReadOnly_;
#endif

		void recalcMinMax();

		Moo::Image<float>	heights_;
		float				minHeight_;
		float				maxHeight_;

		// This member is used to cache an expensive calculation in order to
		// optimise "normalAt" (avoiding expensive sqrtf call).
		float				diagonalDistanceX4_;

		// pointer to parent block
		CommonTerrainBlock1*		terrainBlock1_;
    };

	typedef TerrainMapIter<TerrainHeightMap1>   TerrainHeightMap1Iter;
	typedef TerrainMapHolder<TerrainHeightMap1> TerrainHeightMap1Holder;
}

#endif // TERRAIN_TERRAIN_HEIGHT_MAP1_HPP
