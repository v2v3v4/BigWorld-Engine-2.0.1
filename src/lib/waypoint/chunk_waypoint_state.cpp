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

#include "chunk_waypoint_state.hpp"

#include <sstream>

/**
 *	Constructor
 */
ChunkWaypointState::ChunkWaypointState() :
	distanceFromParent_( 0.f )
{
}


/**
 *	Constructor.
 *
 *	@param pDstSet A pointer to the destination ChunkWaypointSet.
 *	@param dstPoint The destination point in the coordinates of the destination
 *		ChunkWaypointSet.
 */
ChunkWaypointState::ChunkWaypointState( ChunkWaypointSetPtr pDstSet,
		const Vector3 & dstPoint ):
	distanceFromParent_( 0.f )
{
	navLoc_.pSet_ = pDstSet;
	navLoc_.waypoint_ = -1;

	navLoc_.point_ = dstPoint;

	navLoc_.clip();
}


/**
 *	Constructor
 */
ChunkWaypointState::ChunkWaypointState( const NavLoc & navLoc ) :
	navLoc_( navLoc ),
	distanceFromParent_( 0.f )
{
}


/**
 *	Return a string description of this search state.
 */
std::string ChunkWaypointState::desc() const
{
	if (!navLoc_.pSet())
	{
		return "no set";
	}

	if (!navLoc_.pSet()->chunk())
	{
		return "no chunk";
	}

	const Vector3 & v = navLoc_.point();
	std::stringstream ss;
	ss << navLoc_.waypoint() << 
		" (" << v.x << ' ' << v.y << ' ' << v.z << ')' <<
		" in " << navLoc_.pSet()->chunk()->identifier();
	return ss.str();
}


/**
 *  This method gets the given adjacency, if it can be traversed
 */
bool ChunkWaypointState::getAdjacency( int index, ChunkWaypointState & neigh,
	const ChunkWaypointState & goal ) const
{
	const ChunkWaypoint & cw = navLoc_.pSet()->waypoint( navLoc_.waypoint() );
	const ChunkWaypoint::Edge & cwe = cw.edges_[ index ];

	int waypoint = cwe.neighbouringWaypoint();
	bool waypointAdjToChunk = cwe.adjacentToChunk();
	if (waypoint >= 0)
	{
		neigh.navLoc_.pSet_ = navLoc_.pSet();
		neigh.navLoc_.waypoint_ = waypoint;
	}
	else if (waypointAdjToChunk)
	{
		neigh.navLoc_.pSet_ =
			navLoc_.pSet()->connectionWaypoint( cwe );
		neigh.navLoc_.waypoint_ = -1;

		if (!neigh.navLoc_.pSet() || !neigh.navLoc_.pSet()->chunk())
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	WaypointSpaceVector3 wsrc = MappedVector3( navLoc_.point(),
		navLoc_.pSet()->chunk(), MappedVector3::WORLD_SPACE );
	WaypointSpaceVector3 wdst = MappedVector3( goal.navLoc_.point(),
		goal.navLoc_.pSet()->chunk(), MappedVector3::WORLD_SPACE );
	Vector2 src( wsrc.x, wsrc.z );
	Vector2 dst( wdst.x, wdst.z );
	Vector2 way;
	Vector2 del = dst - src;
	const Vector2 & p1 = navLoc_.pSet()->vertexByIndex( cwe.vertexIndex_ );
	const Vector2 & p2 = navLoc_.pSet()->vertexByIndex( 
		cw.edges_[ (index+1) % cw.edges_.size() ].vertexIndex_ );

	float cp1 = del.crossProduct( p1 - src );
	float cp2 = del.crossProduct( p2 - src );
	// see if our path goes through this edge
	if (cp1 > 0.f && cp2 < 0.f)
	{

		// calculate the intersection of the line (src->dst) and (p1->p2).
		// Brief description of how this works. cp1 and cp2 are the areas of
		// the parallelograms formed by the intervals of the cross product.
		// We want the ratio that the intersection point divides p1->p2.
		// This is the same as the ratio of the perpendicular heights of p1
		// and p2 to del. This is also the same as the ratio between the
		// area of the parallelograms (divide by len(del)).
		// Trust me, this works.
		way = p1 + (cp1/(cp1-cp2)) * (p2-p1);

		/*
		// yep, use the intersection
		LineEq moveLine( src, dst, true );
		LineEq edgeLine( p1, p2, true );
		way = moveLine.param( moveLine.intersect( edgeLine ) );
		*/
	}
	else
	{
		way = (fabs(cp1) < fabs(cp2)) ? p1 : p2;
	}

	if (neigh.navLoc_.waypoint_ == -1)
	{
		del.normalise();
		way += del * 0.2f;
	}

	WaypointSpaceVector3 wway( way.x, cw.maxHeight_, way.y );
	WorldSpaceVector3 wv = MappedVector3( wway, navLoc_.pSet()->chunk() );

	neigh.navLoc_.point_ = wv;

	if (waypointAdjToChunk)
	{
		BoundingBox bb = neigh.navLoc_.pSet()->chunk()->boundingBox();
		static const float inABit = 0.01f;
		neigh.navLoc_.point_.x = Math::clamp( bb.minBounds().x + inABit,
			neigh.navLoc_.point_.x, bb.maxBounds().x - inABit );
		neigh.navLoc_.point_.z = Math::clamp( bb.minBounds().z + inABit,
			neigh.navLoc_.point_.z, bb.maxBounds().z - inABit );
	}

	neigh.navLoc_.clip();
	neigh.distanceFromParent_ =
		(neigh.navLoc_.point() - navLoc_.point()).length();

	//DEBUG_MSG( "AStar: Considering adjacency from %d to %d "
	//	"dest (%f,%f,%f) dist %f\n",
	//	navLoc_.waypoint(), neigh.navLoc_.waypoint(),
	//	way.x, cw.maxHeight_, way.y, neigh.distanceFromParent_ );

	return true;
}



// chunk_waypoint_state.cpp
