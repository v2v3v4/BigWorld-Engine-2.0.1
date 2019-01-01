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
#include "chunk_obstacle.hpp"
#include "chunk_manager.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/guard.hpp"
#include "physics2/bsp.hpp"

#ifndef CODE_INLINE
#include "chunk_obstacle.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "Chunk", 0 )

// -----------------------------------------------------------------------------
// Section: ChunkObstacle
// -----------------------------------------------------------------------------


/// static Obstacle data initialiser
uint32 ChunkObstacle::s_nextMark_ = 0;

/**
 *	Obstacle constructor
 *  NOTE: The caller must ensure that the life time of bb covers
 *  the life time of the obstacle object, since bb_ is a pointer.
 *	@param	transform	world transform matrix
 *	@param	bb	BoundingBox of the obstacle
 *	@param	pItem	pointer to ChunkItem link to this obstacle
 */
ChunkObstacle::ChunkObstacle( const Matrix & transform,
		const BoundingBox* bb, ChunkItemPtr pItem ) :
	mark_( s_nextMark_ - 16 ),
	bb_( *bb ),
	pItem_( pItem ),
	transform_( transform )
{
	transformInverse_.invert( transform );

	//MF_ASSERT_DEV( this->pChunk() != NULL );
	// can't check this for dynamic obstacles
}

/**
 *	Obstacle destructor
 */
ChunkObstacle::~ChunkObstacle()
{
}

/**
 *	This method sets the mark to the current 'next mark' value.
 *	@return true if the mark was already set
 */
bool ChunkObstacle::mark() const
{
	if (mark_ == s_nextMark_) return true;
	mark_ = s_nextMark_;
	return false;
}


/**
 * Clip a line against this obstacle's bounding box.
 */
bool ChunkObstacle::clipAgainstBB( Vector3 & start, Vector3 & extent, 
								  float bloat ) const
{
	return bb_.clip( start, extent, bloat );
}


// -----------------------------------------------------------------------------
// Section: ChunkBSPObstacle
// -----------------------------------------------------------------------------


/**
 *	This private class is the chunk space collision visitor,
 *	for interacting with the BSP class, until it is moved over
 *	to the CollisionCallback or traversal object inspection style.
 *
 *	@note Currently, the BSP will not get all the triangles that
 *	 are hit - it will skip multiple collisions on the same plane
 *	 (i.e. with different triangles). This is not good.
 *	TODO: Fix the problem described in the note above
 */
class CSCV : public CollisionVisitor
{
public:
	/**
	 *	constructor
	 *	@param	ob	the obstacle to collide with
	 *	@param	cs	the CollisionState object that returns the result of the collision
	 */
	CSCV( const ChunkBSPObstacle & ob, CollisionState & cs ) :
		ob_( ob ), cs_( cs ), stop_( false ) { }

private:
	virtual bool visit( const WorldTriangle & hitTriangle, float rd );

	const ChunkBSPObstacle & ob_;
	CollisionState & cs_;
	bool	stop_;

	friend class ChunkBSPObstacle;
};


/**
 *	This is the visit method of the chunk space collision visitor.
 *	It returns true to tell the BSP to stop looking for more intersections.
 *	@param	hitTriangle	the triangle hit by the ray
 *	@param	rd	the relative distance between hit point and source point
 *	@return	true if stop, false if continue searching
 */
bool CSCV::visit( const WorldTriangle & hitTriangle, float rd )
{
	BW_GUARD;
	// see how far we really travelled (handles scaling, etc.)
	float ndist = cs_.sTravel_ + (cs_.eTravel_-cs_.sTravel_) * rd;

	if (cs_.onlyLess_ && ndist > cs_.dist_) return true;	// stop now
	if (cs_.onlyMore_ && ndist < cs_.dist_) return false;	// keep looking	

	// call the callback function
	int say = cs_.cc_( ob_, hitTriangle, ndist );

	// record the distance, if the collision callback reported
	// an appropriate triangle.
	if( (say & 3) != 3) cs_.dist_ = ndist;

	// see if any other collisions are wanted
	if (!say)
	{
		stop_ = true;
		return true;
	}

	// some are wanted ... see if it's only one side
	cs_.onlyLess_ = !(say & 2);
    cs_.onlyMore_ = !(say & 1);

	// stop looking in this BSP if we only want closer collisions
	return cs_.onlyLess_;
}


/**
 *	Constructor
 *	@param	bspTree	the bsp tree of the obstacle
 *	@param	transform	the world transform of the obstacle
 *	@param	bb	pointer to a bounding box of this obstacle
 *	@param	pItem	pointer to the ChunkItem associates with the obstacle
 */
ChunkBSPObstacle::ChunkBSPObstacle( const BSPTree & bspTree,
		const Matrix & transform, const BoundingBox * bb, ChunkItemPtr pItem ) :
	ChunkObstacle( transform, bb, pItem ),
	bspTree_( bspTree )
{
}

/**
 *	This is the collide function for BSP obstacles, called once intersection
 *	with the bounding box has been determined. It returns true to stop
 *	the chunk space looking for more collisions (nearer or further),
 *	or it can set some values in the state object to indicate what
 *	other collisions it is interested in.
 *	@param	source	the start point of the ray
 *	@param	extent	the extent of the ray
 *	@param	state	the collision state object used to return
 *					the collision result
 *	@result	true to stop, false to continue searching
 */
bool ChunkBSPObstacle::collide( const Vector3 & source,
	const Vector3 & extent, CollisionState & state ) const
{
	BW_GUARD;
	CSCV cscv( *this, state );

	float rd = 1.0f;
#ifdef COLLISION_DEBUG
	DEBUG_MSG( "ChunkBSPObstacle::collide(pt): %s\n",
			bspTree_.name().c_str() );
#endif
	bspTree_.pRoot()->intersects( source, extent, rd, NULL, &cscv );

	return cscv.stop_;
}

/**
 *	This is the collide function for prisms formed from a world triangle.
 *
 *	@see collide
 *	@param	source	The start triangle to collide with
 *	@param	extent	The extent of the triangle
 *	@param	state	The collision state object used to return
 *					the collision result
 *	@result	true to stop, false to continue searching
 */
bool ChunkBSPObstacle::collide( const WorldTriangle & source,
	const Vector3 & extent, CollisionState & state ) const
{
	CSCV cscv( *this, state );

#ifdef COLLISION_DEBUG
	DEBUG_MSG( "ChunkBSPObstacle::collide(tri): %s\n",
			bspTree_.name().c_str() );
#endif
	bspTree_.pRoot()->intersects( source, extent - source.v0(), &cscv );

	return cscv.stop_;
}


// -----------------------------------------------------------------------------
// Section: ChunkObstacleOccluder
// -----------------------------------------------------------------------------

#ifndef MF_SERVER

#include "chunk/chunk_space.hpp"

/**
 *	This class is a chunk obstacle version of the bsp photon visibility tester
 */
class PhotonVisibility2 : public CollisionCallback
{
public:
	/**
	 *	Constructor
	 */
	PhotonVisibility2() : gotone_( false ), blended_( false ) { }

	/**
	 *	The overridden operator()
	 */
	int operator()( const ChunkObstacle & co,
		const WorldTriangle & hitTriangle, float dist )
	{
		// record if this one's blended
		if (hitTriangle.isBlended()) blended_ = true;

		// if it's not transparent, we can stop now
		if (!hitTriangle.isTransparent()) { gotone_ = true; return 0; }

		// otherwise we have to keep on going
		return COLLIDE_BEFORE | COLLIDE_AFTER;
	}

	bool gotone()			{ return gotone_; }
	bool hitBlended()		{ return blended_; }

private:
	bool gotone_;
	bool blended_;
};


/**
 *	This method performs the main function of this class: a collision test
 */
float ChunkObstacleOccluder::collides(
	const Vector3 & photonSourcePosition,
	const Vector3 & cameraPosition,
	const LensEffect& le )
{
	BW_GUARD;
	float result = 1.f;

	Vector3 trSourcePos;
	//csTransformInverse_.applyPoint( trSourcePos, photonSourcePosition );
	trSourcePos = photonSourcePosition;

	Vector3 trCamPos;
	//csTransformInverse_.applyPoint( trCamPos, cameraPosition );
	trCamPos = cameraPosition;

	//Fudge the source position by 50cm.
	//This is a test fix for lens flares that are placed
	//inside light bulbs, or geometric candle flame.
	Vector3 ray(trCamPos - trSourcePos);
	ray.normalise();
	Vector3 fudgedSource = ray * 0.5f + trSourcePos;

	PhotonVisibility2	obVisitor;

	ChunkSpace * pCS = &*ChunkManager::instance().cameraSpace();
	if (pCS != NULL)
		pCS->collide( trCamPos, fudgedSource, obVisitor );

	if (obVisitor.gotone())
	{
		//sun ended up on a solid poly
		result = 0.f;
	}
	else
	{
		if (obVisitor.hitBlended())
		{
			//sun is essentially visible, but maybe through a translucent poly
			result = 0.5f;
		}
		else
		{
			//sun is completely unobstructed
			result = 1.f;
		}
	}

	return result;
}



// -----------------------------------------------------------------------------
// Section: ChunkRompCollider
// -----------------------------------------------------------------------------


/**
 *	This method returns the height of the ground under pos, doing only
 *  one-sided collisions tests.
 *
 *	@param pos				The position from which to drop.
 *	@param dropDistance		The distance over which the drop is checked.
 *  @param oneSided         If true then only consider collisions where 
 *                          the collision triangle is pointing up.
 *
 *	@return The height of the ground found. If no ground was found within the
 *			dropDistance from the position supplied, 
 *          RompCollider::NO_GROUND_COLLISION is returned.
 */
float ChunkRompCollider::ground( const Vector3 &pos, float dropDistance, bool oneSided )
{
	BW_GUARD;
	Vector3 lowestPoint( pos.x, pos.y - dropDistance, pos.z );

	float dist = -1.f;
	ChunkSpace * pCS = &*ChunkManager::instance().cameraSpace();
	if (pCS != NULL)
	{
		ClosestObstacleEx coe
		(
			oneSided, 
			Vector3(0.0f, -1.0f, 0.0f)
		);

		dist = pCS->collide
		( 
			pos, 
			lowestPoint, 
			coe
		);
	}
	if (dist < 0.0f) return RompCollider::NO_GROUND_COLLISION;

	return pos.y - dist;
}


/**
 *	This method returns the height of the ground under pos.
 */
float ChunkRompCollider::ground( const Vector3 & pos )
{
	BW_GUARD;
	Vector3 dropSrc( pos.x, pos.y + 15.f, pos.z );
	Vector3 dropMax( pos.x, pos.y - 15.f, pos.z );

	float dist = -1.f;
	ChunkSpace * pCS = &*ChunkManager::instance().cameraSpace();
	if (pCS != NULL)
		dist = pCS->collide( dropSrc, dropMax, ClosestObstacle::s_default );
	if (dist < 0.f) return RompCollider::NO_GROUND_COLLISION;

	return pos.y + 15.f - dist;
}


/**
 *	This method returns the height of the terrain under pos.
 */
#define GROUND_TEST_INTERVAL 500.f
float ChunkRompTerrainCollider::ground( const Vector3 & pos )
{
	BW_GUARD;
	//RA: increased the collision test interval
	Vector3 dropSrc( pos.x, pos.y + GROUND_TEST_INTERVAL, pos.z );
	Vector3 dropMax( pos.x, pos.y - GROUND_TEST_INTERVAL, pos.z );

	float dist = -1.f;
	ChunkSpace * pCS = &*ChunkManager::instance().cameraSpace();
	ClosestTerrainObstacle terrainCallback;
	if (pCS != NULL)
		dist = pCS->collide( dropSrc, dropMax, terrainCallback );
	if (dist < 0.f) return RompCollider::NO_GROUND_COLLISION;

	return pos.y + GROUND_TEST_INTERVAL - dist;
}


// -----------------------------------------------------------------------------
// Section: CRCClosestTriangle
// -----------------------------------------------------------------------------

/**
 *	This method returns the height of the ground under pos.
 */
#include "physics2/worldtri.hpp"
#ifdef EDITOR_ENABLED
extern char * g_chunkParticlesLabel;
#endif // EDITOR_ENABLED

class CRCClosestTriangle : public CollisionCallback
{
public:
	virtual int operator()( const ChunkObstacle & obstacle,
		const WorldTriangle & triangle, float dist )
	{
		BW_GUARD;	
#ifdef EDITOR_ENABLED
		// don't collide against the particle system selectable representation
		if (obstacle.pItem()->label() == g_chunkParticlesLabel)
			return COLLIDE_ALL;
#endif // EDITOR_ENABLED

		// transform into world space
		s_collider = WorldTriangle(
			obstacle.transform_.applyPoint( triangle.v0() ),
			obstacle.transform_.applyPoint( triangle.v1() ),
			obstacle.transform_.applyPoint( triangle.v2() ) );

		return COLLIDE_BEFORE;
	}

	static CRCClosestTriangle s_default;
	WorldTriangle s_collider;
};
CRCClosestTriangle CRCClosestTriangle::s_default;

float ChunkRompCollider::collide( const Vector3 &start, const Vector3& end, WorldTriangle& result )
{
	BW_GUARD;
	float dist = -1.f;
	ChunkSpace * pCS = &*ChunkManager::instance().cameraSpace();
	if (pCS != NULL)
		dist = pCS->collide( start, end, CRCClosestTriangle::s_default );
	if (dist < 0.f) return RompCollider::NO_GROUND_COLLISION;

	//set the world triangle.
	result = CRCClosestTriangle::s_default.s_collider;

	return dist;
}


// -----------------------------------------------------------------------------
// Section: ClosestTerrainObstacle
// -----------------------------------------------------------------------------

ClosestTerrainObstacle::ClosestTerrainObstacle() :
	dist_( -1 ),
	collided_( false )
{
}


int ClosestTerrainObstacle::operator() ( const ChunkObstacle & obstacle,
		const WorldTriangle & triangle, float dist )
{
    if (triangle.flags() & TRIANGLE_TERRAIN)
    {
    	// Set the distance if it's the first collision, or if the distance is
    	// less than the previous collision.
		if (!collided_ || dist < dist_)
		{
	        dist_ = dist;
			collided_ = true;
		}

        return COLLIDE_BEFORE;
    }
    else
	{
		return COLLIDE_ALL;
	}
}


void ClosestTerrainObstacle::reset()
{
	dist_ = -1;
	collided_ = false;
}


float ClosestTerrainObstacle::dist() const
{
	return dist_;
}


bool ClosestTerrainObstacle::collided() const
{
	return collided_;
}


#endif // MF_SERVER


// -----------------------------------------------------------------------------
// Section: Static initialisers
// -----------------------------------------------------------------------------


/// static initialiser for the default 'any-obstacle' callback object
CollisionCallback CollisionCallback::s_default;
/// static initialiser for the 'closest-obstacle' callback object
ClosestObstacle ClosestObstacle::s_default;

/// static initialiser for the reference to the default callback object
CollisionCallback & CollisionCallback_s_default = CollisionCallback::s_default;
// (it's a reference so that chunkspace.hpp can use it without including us)

// chunk_obstacle.cpp
