/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TERRAIN_QUAD_TREE_CELL_HPP
#define TERRAIN_TERRAIN_QUAD_TREE_CELL_HPP

#include "math/boundbox.hpp"

#include <vector>

namespace Terrain
{
    class TerrainCollisionCallback;
    class TerrainHeightMap2;
}

class WorldTriangle;

namespace Terrain
{
    /**
     *  This class is used in TerrainHeightMap2 collisions calculations.
     */
    class TerrainQuadTreeCell
    {
    public:
        explicit TerrainQuadTreeCell( const std::string& allocator = "unknown terrain quad tree cell" );
		TerrainQuadTreeCell( const TerrainQuadTreeCell& cell );

        ~TerrainQuadTreeCell();

		void init( const TerrainHeightMap2* map,
			uint32 xOffset, uint32 zOffset, uint32 xSize, uint32 zSize,
			float xMin, float zMin, float xMax, float zMax, float minCellSize );

		bool collide( const Vector3& source, const Vector3& extent,
			const TerrainHeightMap2* pOwner,
            TerrainCollisionCallback* pCallback ) const;

		bool collide( const WorldTriangle& source, const Vector3& extent,
			const TerrainHeightMap2* pOwner, TerrainCollisionCallback* pCallback
			) const;

    private:
		BoundingBox                         boundingBox_;
		std::vector<TerrainQuadTreeCell>    children_;

#if ENABLE_RESOURCE_COUNTERS
		std::string		allocator_;
#endif
	};
}

#endif // TERRAIN_TERRAIN_QUAD_TREE_CELL_HPP
