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
#include "collision_advance.hpp"

#include "physics2/worldtri.hpp"
#include "physics2/worldpoly.hpp"

class WorldPolygon;

static uint32 g_triangleColour	= 0x00ff0000;
static uint32 g_polyColour		= 0x000000ff;

extern bool g_debugDrawCollisions;
extern void s_dtriadd( const WorldTriangle & wt, uint32 col );
extern void s_dpolyadd( const WorldPolygon & wt, uint32 col );


/**
 *	Constructor.
 */
CollisionAdvance::CollisionAdvance( const Vector3 & origin,
			const Vector3 & axis1, const Vector3 & axis2,
			const Vector3 & direction,
			float maxAdvance ) :
		dir_( direction ),
		advance_( maxAdvance ),
		ignoreFlags_( 0 )
{
	BW_GUARD;
	bool isRightHanded = (direction.dotProduct( axis1.crossProduct( axis2 ) ) ) > 0.f;

	Vector3 v1;
	Vector3 v2;

	if (isRightHanded)
	{
		v1 = axis2;
		v2 = axis1;
	}
	else
	{
		v1 = axis1;
		v2 = axis2;
	}

	planeEq0_.init( origin,			direction.crossProduct( v2 ) );
	planeEq1_.init( origin + v2,	direction.crossProduct( v1 ) );
	planeEq2_.init( origin + v1,	v2.crossProduct( direction ) );
	planeEq3_.init( origin,			v1.crossProduct( direction ) );

	Vector3 normal( v2.crossProduct( v1 ) );
	normal.normalise();
	adjust_ = 1.f/dir_.dotProduct( normal );

	planeEqBase_.init( origin,	normal );
}


/**
 *	Destructor.
 */
CollisionAdvance::~CollisionAdvance()
{
}


/**
 *	This is the function that is called by the collision function for each
 *	triangle that we hit.
 */
int CollisionAdvance::operator()( const ChunkObstacle & obstacle,
	const WorldTriangle & triangle, float dist )
{
	BW_GUARD;
	if (triangle.flags() & ignoreFlags_)
	{
		return COLLIDE_ALL;
	}

	static WorldPolygon poly(3);
	poly.resize( 3 );

	poly[0] = obstacle.transform_.applyPoint( triangle.v0() );
	poly[1] = obstacle.transform_.applyPoint( triangle.v1() );
	poly[2] = obstacle.transform_.applyPoint( triangle.v2() );

	// DEBUG: Draw this triangle red.
	WorldTriangle wt( poly[0], poly[1], poly[2],
		triangle.flags() );
	
	if (g_debugDrawCollisions)
	{
		s_dtriadd( wt, g_triangleColour );
	}

	poly.chop( planeEq0_ );
	poly.chop( planeEq1_ );
	poly.chop( planeEq2_ );
	poly.chop( planeEq3_ );
	poly.chop( planeEqBase_ );

	WorldPolygon::const_iterator iter = poly.begin();

	while (iter != poly.end())
	{
		const float dist = planeEqBase_.distanceTo( *iter ) * adjust_;

		if (dist < advance_)
		{
			advance_ = dist;

			hitTriangle_ = wt;
			hitItem_ = obstacle.pItem();
		}

		iter++;
	}

#if 0
	if (poly.empty())
	{
		s_dtriadd( wt, 0x00ff9999 );
	}
	else
	{
		s_dpolyadd( poly, g_polyColour );
	}
#endif

	return COLLIDE_ALL;
}



// collision_advance.cpp