/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_OBSTACLE_HPP
#define CHUNK_OBSTACLE_HPP

#include "cstdmf/aligned.hpp"
#include "cstdmf/smartpointer.hpp"
#include "math/matrix.hpp"

#ifndef MF_SERVER
#include "romp/photon_occluder.hpp"
#include "romp/romp_collider.hpp"
#endif // MF_SERVER

#include "chunk_item.hpp"

#include "physics2/worldtri.hpp"

class ChunkObstacle;
class CollisionState;
class Chunk;
class ChunkSpace;
class BSPTree;
class BoundingBox;

const int COLLIDE_STOP = 0;
const int COLLIDE_BEFORE = 1;
const int COLLIDE_AFTER = 2;
const int COLLIDE_ALL = COLLIDE_BEFORE | COLLIDE_AFTER;

/**
 *	This class contains the function that is called when a collision with an
 *	obstacle occurs
 */
class CollisionCallback
{
public:
	/**
	 *	Destructor
	 */
	virtual ~CollisionCallback() {};

	/**
	 *	User-defined method to check a collision.
	 *
	 *	@param	obstacle	the obstacle contains the triangle that collides the ray
	 *	@param	triangle	the world triangle that collides the ray
	 *	@param	dist	the distance between the collision point and the source of the ray
	 *	@return - COLLIDE_BEFORE for interested in collisions before this,
	 *			- COLLIDE_AFTER for interested in collisions after it, or
	 *			- COLLIDE_ALL for interested in collisions before and after
	 *			- COLLIDE_STOP for done
	 *
	 *	@note The dist returned from the collide function is whatever
	 *	was passed into the last callback call, or -1 if there were none.
	 *
	 *	@note The WorldTriangle is in the co-ordinate system of the
	 *	ChunkObstacle, and only exists for the duration of the call.
	 */
	virtual int operator()( const ChunkObstacle & obstacle,
		const WorldTriangle & triangle, float dist )
	{
		return COLLIDE_STOP;
	}

	static CollisionCallback s_default;
};


/**
 *	This class finds the first collision with an obstacle
 */
class ClosestObstacle : public CollisionCallback
{
public:
	/**
	 *	the overridden operator()
	 */
	virtual int operator()( const ChunkObstacle & /*obstacle*/,
		const WorldTriangle & /*triangle*/, float /*dist*/ )
	{
		return COLLIDE_BEFORE;
	}

	static ClosestObstacle s_default;
};


/**
 *  This class finds the first collision with an obstacle, optionally only
 *  finding collisions in the same direction as the search ray. 
 */
class ClosestObstacleEx : public CollisionCallback
{
public:
	/**
	 *	Constructor
	 *	@param	oneSided	if it is true, we'll not collide with back faces
	 *	@param	rayDir	the direction of ray
	 */
    ClosestObstacleEx(bool oneSided, const Vector3 & rayDir)
    :
    oneSided_(oneSided),
    rayDir_(rayDir)
    {
        rayDir_.normalise();
    }

	/**
	 *	The overridden operator()
	 */
	virtual int operator()( const ChunkObstacle & /*obstacle*/,
		const WorldTriangle &triangle, float /*dist*/ )
	{
        // Two-sided collisions case:
        if (!oneSided_)
        {
            return COLLIDE_BEFORE; 
        }
        // One sided collisions case:
        else
        {
            // If the normal of the triangle and the ray's direction are the
            // same then this this is not a collision.
            Vector3 normal = triangle.normal();
            normal.normalise();
            if (normal.dotProduct(rayDir_) > 0)
                return COLLIDE_ALL; 
            else
		        return COLLIDE_BEFORE;
        }
	}

private:
    bool        oneSided_;
    Vector3     rayDir_;
};


/**
 *	This class defines an obstacle that can live in a column in a chunk space.
 */
class ChunkObstacle : public ReferenceCount, public Aligned
{
public:
	ChunkObstacle( const Matrix & transform, const BoundingBox* bb,
		ChunkItemPtr pItem );
	virtual ~ChunkObstacle();

	mutable uint32 mark_;

	const BoundingBox & bb_;

private:
	ChunkItemPtr pItem_;

public:
	Matrix	transform_;
	Matrix	transformInverse_;

	bool mark() const;
	static void nextMark() { s_nextMark_++; }
	static uint32 s_nextMark_;

	// return true to stop the search now
	virtual bool collide( const Vector3 & source, const Vector3 & extent,
		CollisionState & state ) const = 0;
	virtual bool collide( const WorldTriangle & source, const Vector3 & extent,
		CollisionState & state ) const = 0;

	virtual bool clipAgainstBB( Vector3 & start, Vector3 & extent, 
		float bloat = 0.f ) const;

	ChunkItemPtr pItem() const;
	Chunk * pChunk() const;
};

typedef SmartPointer<ChunkObstacle> ChunkObstaclePtr;


/**
 *	This class is an obstacle represented by a BSP tree
 */
class ChunkBSPObstacle : public ChunkObstacle
{
public:
	ChunkBSPObstacle( const BSPTree & bsp, const Matrix & transform,
		const BoundingBox * bb, ChunkItemPtr pItem );

	virtual bool collide( const Vector3 & source, const Vector3 & extent,
		CollisionState & state ) const;
	virtual bool collide( const WorldTriangle & source, const Vector3 & extent,
		CollisionState & state ) const;
private:
	const BSPTree & bspTree_;
};




/**
 *	This class contains the state of a chunk space collision, or at least
 *	references to it, that is passed to a ChunkSpaceObstacle's
 *	collide method. See CSCV::visit for for examples of their use.
 */
class CollisionState
{
public:
	/**
	 *	Constructor
	 *	@param	cc	user-supplied collision callback object
	 */
	CollisionState( CollisionCallback & cc ) :
		cc_( cc ),
		onlyLess_( false ),
		onlyMore_( false ),
		sTravel_( 0.f ),
		eTravel_( 0.f ),
		dist_( -1.0f )
	{ }

public:
	CollisionCallback	& cc_;	// (RW) user-supplied collision callback object
	bool onlyLess_;				// (RW) only interested in nearer than dist_
	bool onlyMore_;				// (RW) only interested in further than dist_
	float sTravel_;				// (RO) dist along line at source
	float eTravel_;				// (RO) dist along line at extent
	float dist_;				// (RW) last value passed to cc_
};



#ifndef MF_SERVER
/**
 *	This class checks line-of-sight through the chunk space for the
 *	LensEffectManager.
 */
class ChunkObstacleOccluder : public PhotonOccluder
{
public:
	/**
	 *	Constructor
	 */
	ChunkObstacleOccluder() { }

	virtual float collides(
		const Vector3 & photonSourcePosition,
		const Vector3 & cameraPosition,
		const LensEffect& le );
};


/**
 *	This class implements the standard ground specifier for the
 *	grounded source action of a particle system.
 */
class ChunkRompCollider : public RompCollider
{
public:
	/**
	 *	Constructor
	 */
	ChunkRompCollider() { }

    virtual float ground( const Vector3 &pos, float dropDistance, bool onesided );
	virtual float ground( const Vector3 & pos );
	virtual float collide( const Vector3 &start, const Vector3& end, WorldTriangle& result );
};


/**
 *	This class implements the standard ground specifier for the
 *	grounded source action of a particle system.
 *	It uses terrain as ground
 */
class ChunkRompTerrainCollider : public ChunkRompCollider
{
public:
	/**
	 *	Constructor
	 */
	ChunkRompTerrainCollider() { }

	virtual float ground( const Vector3 & pos );
};


/**
*	This utility class finds the first collision with a terrain block
*/
class ClosestTerrainObstacle : public CollisionCallback
{
public:
	ClosestTerrainObstacle();

	virtual int operator() (const ChunkObstacle & obstacle,
		const WorldTriangle & triangle, float dist );

	void reset();

	float dist() const;

	bool collided() const;

private:
	float dist_;
	bool collided_;
};
#endif // MF_SERVER


#ifdef CODE_INLINE
#include "chunk_obstacle.ipp"
#endif

#endif // CHUNK_OBSTACLE_HPP
