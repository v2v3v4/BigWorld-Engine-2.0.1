/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_WAYPOINT_STATE_HPP
#define CHUNK_WAYPOINT_STATE_HPP

#include "chunk_waypoint_set.hpp"
#include "navloc.hpp"

#include "math/vector3.hpp"

#include <vector>

/**
 *	This class is a state in an A-Star search of some waypoint graph.
 */
class ChunkWaypointState
{
public:
	typedef int adjacency_iterator;

	ChunkWaypointState();

	ChunkWaypointState( ChunkWaypointSetPtr dstSet, 
		const Vector3 & dstPoint );

	explicit ChunkWaypointState( const NavLoc & navLoc );

	int	compare( const ChunkWaypointState & other ) const
	{ 
		return (navLoc_.pSet() == other.navLoc_.pSet()) ?
			navLoc_.waypoint() - other.navLoc_.waypoint() :
			intptr( navLoc_.pSet().get() ) - 
				intptr( other.navLoc_.pSet().get() ); 
	}

	std::string desc() const;

	unsigned int hash() const
	{
		return uintptr( navLoc_.pSet().get() ) + navLoc_.waypoint();
	}

	bool isGoal( const ChunkWaypointState & goal ) const
	{ 
		return navLoc_.waypointsEqual( goal.navLoc_ );
	}

	adjacency_iterator adjacenciesBegin() const
		{ return 0; }

	adjacency_iterator adjacenciesEnd() const
	{ 
		return navLoc_.waypoint() >= 0 ?
			navLoc_.pSet()->waypoint( navLoc_.waypoint() ).edges_.size() :
			0; 
	}

	bool getAdjacency( adjacency_iterator iter, ChunkWaypointState & neigh,
		const ChunkWaypointState & goal ) const;

	float distanceFromParent() const
		{ return distanceFromParent_; }

	float distanceToGoal( const ChunkWaypointState & goal ) const
		{ return (navLoc_.point() - goal.navLoc_.point()).length(); }

	const NavLoc & navLoc() const
		{ return navLoc_; }

private:
	NavLoc	navLoc_;
	float	distanceFromParent_;
};


#endif // CHUNK_WAYPOINT_STATE_HPP
