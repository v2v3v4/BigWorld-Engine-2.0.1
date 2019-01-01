/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_TERRAIN_OBSTACLE_HPP
#define CHUNK_TERRAIN_OBSTACLE_HPP

#include "chunk_obstacle.hpp"

class BoundingBox;
// class WorldTriangle;

namespace Moo
{
	class BaseTerrainBlock;
}

namespace Terrain
{
	class BaseTerrainBlock;
}

/**
 *	This class treats a terrain block as an obstacle
 */
class ChunkTerrainObstacle : public ChunkObstacle
{
public:
	ChunkTerrainObstacle( const Terrain::BaseTerrainBlock & tb,
			const Matrix & transform, const BoundingBox* bb,
			ChunkItemPtr pItem );

	virtual bool collide( const Vector3 & start, const Vector3 & end,
		CollisionState & state ) const;
	virtual bool collide( const WorldTriangle & start, const Vector3 & end,
		CollisionState & state ) const;

	virtual bool clipAgainstBB( Vector3 & start, Vector3 & extent, 
		float bloat = 0.f ) const;

	const Terrain::BaseTerrainBlock& block() const { return tb_; }

private:

	const Terrain::BaseTerrainBlock& tb_;
};

#endif // CHUNK_TERRAIN_OBSTACLE_HPP

// chunk_terrain_obstacle.hpp
