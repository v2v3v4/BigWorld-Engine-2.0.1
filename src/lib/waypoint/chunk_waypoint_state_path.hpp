/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_WAYPOINT_STATE_PATH_HPP
#define CHUNK_WAYPOINT_STATE_PATH_HPP

#include "astar.hpp"
#include "chunk_waypoint_state.hpp"
#include "search_path_base.hpp"

/**
 *	This class is responsible for holding the path from an A* search through a
 *	single waypoint set.
 */
class ChunkWaypointStatePath : public SearchPathBase< ChunkWaypointState >
{
public:

	/**
	 *	Constructor.
	 */
	ChunkWaypointStatePath():
			SearchPathBase< ChunkWaypointState >()
	{}

	
	/**
	 *	Destructor.
	 */
	virtual ~ChunkWaypointStatePath()
	{}


	virtual void init( AStar< ChunkWaypointState > & astar );
	
	/**
	 *	Return whether two ChunkWaypointStates are equivalent. For our
	 *	purposes, we see if they refer to the same waypoint in the same
	 *	waypoint set.
	 */
	virtual bool statesAreEquivalent( const ChunkWaypointState & s1, 
			const ChunkWaypointState & s2 ) const
	{
		return s1.navLoc().waypointsEqual( s2.navLoc() );
	}

};

#endif // CHUNK_WAYPOINT_STATE_PATH_HPP
