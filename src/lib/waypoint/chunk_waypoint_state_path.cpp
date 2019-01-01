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

#include "chunk_waypoint_state_path.hpp"

#include <vector>


/**
 *	Initialise from a A* search through waypoints in one waypoint set.
 */
void ChunkWaypointStatePath::init( 
		AStar< ChunkWaypointState > & astar )
{
	reversePath_.clear();

	typedef std::vector< const ChunkWaypointState * > StatePtrs;
	StatePtrs forwardPath;

	// get out all the pointers
	const ChunkWaypointState * pSearchState = astar.first();
	const ChunkWaypointState * pLast = pSearchState;

	bool first = true;
	while (pSearchState != NULL)
	{
		if (first || !almostZero( pSearchState->distanceFromParent() ))
		{
			forwardPath.push_back( pSearchState );
			first = false;
		}

		pSearchState = astar.next();

		if (pSearchState)
		{
			pLast = pSearchState;
		}
	}

	if (forwardPath.size() < 2)
	{
		// make sure that forwardPath has at least 2 nodes
		forwardPath.push_back( pLast );
	}

	// and store the path in reverse order (for pop_back)

	StatePtrs::reverse_iterator iState = forwardPath.rbegin();
	while (iState != forwardPath.rend())
	{
		reversePath_.push_back( **iState );
		++iState;
	}
}


// chunk_waypoint_state_path.cpp
