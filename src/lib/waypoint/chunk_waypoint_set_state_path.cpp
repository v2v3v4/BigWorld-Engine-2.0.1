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

#include "chunk_waypoint_set_state_path.hpp"

#include <vector>


/**
 *	Initialise the chunk waypoint set path from a A* search result.
 */
void ChunkWaypointSetStatePath::init( AStar< ChunkWPSetState > & astar )
{
	this->clear();

	typedef std::vector< const ChunkWPSetState * > StatePtrs;
	StatePtrs forwardPath;
	
	passedShellBoundary_ = false;

	// get out all the pointers
	const ChunkWPSetState * pSearchState = astar.first();
	while (pSearchState != NULL)
	{
		// See if we crossed between an inside chunk and an outside chunk
		// anywhere along this path.
		passedShellBoundary_ = passedShellBoundary_ || 
			pSearchState->passedShellBoundary();

		forwardPath.push_back( pSearchState );
		pSearchState = astar.next();
	}

	// and store the path in reverse order (for pop_back)

	StatePtrs::reverse_iterator iState = forwardPath.rbegin();
	while (iState != forwardPath.rend())
	{
		reversePath_.push_back( **iState );
		++iState;
	}
}


// chunk_waypoint_set_state_path.cpp
