/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_WAYPOINT_SET_STATE_PATH_HPP
#define CHUNK_WAYPOINT_SET_STATE_PATH_HPP

#include "chunk_waypoint_set_state.hpp"
#include "search_path_base.hpp"

/**
 *	This class stores a path between waypoint sets.
 */
class ChunkWaypointSetStatePath : public SearchPathBase< ChunkWPSetState >
{
public:
	/**
	 *	Constructor.
	 */
	ChunkWaypointSetStatePath() :
		passedShellBoundary_( false )
	{}

	virtual ~ChunkWaypointSetStatePath()
	{}


	virtual void init( AStar< ChunkWPSetState > & astar );

	/**
	 *	Return true if two ChunkWPSetState objects are equivalent. For our
	 *	purposes, we check that they refer to the same waypoint set.
	 */
	virtual bool statesAreEquivalent( const ChunkWPSetState & s1, 
			const ChunkWPSetState & s2 ) const
		{ return s1.pSet() == s2.pSet(); }

	/**
	 *	Return the size of the path.
	 */
	size_t size() const 
		{ return reversePath_.size(); }

	/**
	 *	Return true if one of the states in the path indicates that it passed
	 *	through a shell boundary.
	 */
	bool passedShellBoundary() const
		{ return passedShellBoundary_; }

private:
	bool passedShellBoundary_;
};

#endif // CHUNK_WAYPOINT_SET_STATE_PATH_HPP
