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
#include "chunk_terrain_obstacle.hpp"

#include "physics2/worldtri.hpp"

#include "terrain/base_terrain_block.hpp"
#include "terrain/terrain_collision_callback.hpp"
#include "terrain/terrain_hole_map.hpp"
#include "terrain/dominant_texture_map.hpp"

#include "cstdmf/guard.hpp"



// TODO: this is being accessed by declaring it as 'extern' in WE, and should be
// modified in another way, such as through a static member variable.
// This flag is set externally to allow colliding with holes
#ifdef EDITOR_ENABLED
bool s_collideTerrainHoles = false;
#endif


ChunkTerrainObstacle::ChunkTerrainObstacle( const Terrain::BaseTerrainBlock & tb,
		const Matrix & transform, const BoundingBox* bb,
		ChunkItemPtr pItem ) :
	ChunkObstacle( transform, bb, pItem ),
	tb_( tb )
{
}

class TerrainObstacleRayCallback : public Terrain::TerrainCollisionCallback
{
public:
	TerrainObstacleRayCallback( CollisionState& cs, const ChunkTerrainObstacle* pObstacle,
		const Vector3& start, const Vector3& end) : 
		cs_( cs ),
		pObstacle_( pObstacle ),
		start_( start ),
		end_( end ),
		finishedColliding_( false )
	{
		dir_ = end - start;
		dist_ = dir_.length();

		dir_ *= 1.f / dist_;
	}
	~TerrainObstacleRayCallback()
	{
	}

	bool collide( const WorldTriangle& triangle, float dist)
	{
		BW_GUARD;
		// see how far we really traveled (handles scaling, etc.)
		float ndist = cs_.sTravel_ + (cs_.eTravel_ - cs_.sTravel_) * (dist / dist_);

		if (cs_.onlyLess_ && ndist > cs_.dist_) return false;
		if (cs_.onlyMore_ && ndist < cs_.dist_) return false;

		// Calculate the impact point
		Vector3 impact = start_ + dir_ * dist;

		// check the hole map
#ifdef EDITOR_ENABLED
		if ( !s_collideTerrainHoles )
		{
#endif // EDITOR_ENABLED
			if (pObstacle_->block().holeMap().holeAt( impact.x, impact.z ))
				return false;
#ifdef EDITOR_ENABLED
		}
#endif // EDITOR_ENABLED

		// get the material kind and insert it in our triangle
		uint32 materialKind = 0;
		if ( pObstacle_->block().dominantTextureMap() != NULL )
			materialKind = pObstacle_->block().dominantTextureMap()->materialKind( impact.x, impact.z );

		WorldTriangle wt = triangle;

		wt.flags( WorldTriangle::packFlags( wt.collisionFlags(), materialKind ) );

		cs_.dist_ = ndist;

		// call the callback function
		int say = cs_.cc_( *pObstacle_, wt, cs_.dist_ );

		// see if any other collisions are wanted
		if (!say) 
		{
			// if no further collisions are needed
			// set the finished colliding flag
			finishedColliding_ = true;
			return true;
		}

		// some are wanted ... see if it's only one side
		cs_.onlyLess_ = !(say & 2);
		cs_.onlyMore_ = !(say & 1);

		// Return true if only nearer collisions are wanted,
		// this allows us to do an early out, as the collision
		// order for terrain blocks are nearest to furthest. 
		// As the finishedColliding_ flag is not set we can 
		// still get collisions from other objects, just not 
		// this terrain block.
		return cs_.onlyLess_;
	}

	bool finishedColliding() const { return finishedColliding_; }
private:

	CollisionState& cs_;
	const ChunkTerrainObstacle* pObstacle_;

	Vector3 start_;
	Vector3 end_;
	Vector3 dir_;
	float	dist_;

	bool finishedColliding_;
};

class TerrainObstaclePrismCallback : public Terrain::TerrainCollisionCallback
{
public:
	TerrainObstaclePrismCallback( CollisionState& cs, const ChunkTerrainObstacle* pObstacle,
		const WorldTriangle& start, const Vector3& end) : 
		cs_( cs ),
		pObstacle_( pObstacle ),
		start_( start ),
		end_( end )
	{
		dir_ = end - start.v0();
		dist_ = dir_.length();

		dir_ *= 1.f / dist_;
	}
	~TerrainObstaclePrismCallback()
	{
	}

	bool collide( const WorldTriangle& triangle, float dist)
	{
		BW_GUARD;
		// see how far we really traveled (handles scaling, etc.)
		float ndist = cs_.sTravel_ + (cs_.eTravel_ - cs_.sTravel_) * (dist / dist_);

		if (cs_.onlyLess_ && ndist > cs_.dist_) return false;
		if (cs_.onlyMore_ && ndist < cs_.dist_) return false;

		// Calculate the impact point we use the centre of the triangle for this
		Vector3 impact = 
			(triangle.v0() + triangle.v1() + triangle.v2()) * (1.f /3.f);

		// check the hole map
		if (pObstacle_->block().holeMap().holeAt( impact.x, impact.z ))
			return false;

		// get the material kind and insert it in our triangle
		uint32 materialKind = 0;
		if ( pObstacle_->block().dominantTextureMap() != NULL )
			materialKind = pObstacle_->block().dominantTextureMap()->materialKind( impact.x, impact.z );

		WorldTriangle wt = triangle;

		wt.flags( WorldTriangle::packFlags( wt.collisionFlags(), materialKind ) );

		cs_.dist_ = ndist;

		// call the callback function
		int say = cs_.cc_( *pObstacle_, wt, cs_.dist_ );

		// see if any other collisions are wanted
		if (!say) return true;	// nope, we're outta here!

		// some are wanted ... see if it's only one side
		cs_.onlyLess_ = !(say & 2);
		cs_.onlyMore_ = !(say & 1);

		// We want more collisions
		return false;
	}

private:

	CollisionState& cs_;
	const ChunkTerrainObstacle* pObstacle_;

	WorldTriangle	start_;
	Vector3			end_;
	Vector3			dir_;
	float			dist_;
};



/**
 *	This method collides the input ray with the terrain block.
 *	start and end should be inside (or on the edges of)
 *	our bounding box.
 *
 *	@param start	The start of the ray.
 *	@param end		The end of the ray.
 *	@param cs		Collision state holder.
 */

bool ChunkTerrainObstacle::collide( const Vector3 & start,
	const Vector3 & end, CollisionState & cs ) const
{
	BW_GUARD;
	TerrainObstacleRayCallback toc( cs, this, start, end );

#ifdef BW_WORLDTRIANGLE_DEBUG
	WorldTriangle::BeginDraw( transform_, 100, 500 );
#endif

	tb_.collide( start, end, &toc ); 

#ifdef BW_WORLDTRIANGLE_DEBUG	
	WorldTriangle::EndDraw();
#endif

	return toc.finishedColliding();
}


/**
 *	This method collides the input prism with the terrain block.
 * 
 *	@param start	The start triangle of prism.
 *	@param end		The position of the first vertex on end triangle.
 *	@param cs		Collision state holder.
 *
 *	@see collide
 */
bool ChunkTerrainObstacle::collide( const WorldTriangle & start,
	const Vector3 & end, CollisionState & cs ) const
{
    BW_GUARD;
	TerrainObstaclePrismCallback toc( cs, this, start, end );

    return tb_.collide( start, end, &toc );
}

/**
 * When terrain clips, it should use the bounding box of the currently loaded
 * height map, or there will be misses.
 */
bool ChunkTerrainObstacle::clipAgainstBB( Vector3 & start, Vector3 & extent, 
											float bloat ) const
{
	return tb_.boundingBox().clip( start, extent, bloat );
}

// chunk_terrain_obstacle.cpp
