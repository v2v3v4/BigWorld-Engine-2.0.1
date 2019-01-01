/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WAYPOINT_NEIGHBOUR_ITERATOR
#define WAYPOINT_NEIGHBOUR_ITERATOR

#include "chunk_waypoint_set.hpp"

/**
 *	This class is used to iterate through the immediate neighbourhood of a
 *	certain waypoint.  
 *	It will also return the neighbour waypoints in other chunks.
 */
class WaypointNeighbourIterator
{
public:
	WaypointNeighbourIterator( ChunkWaypointSetPtr pSet, int waypoint );

	bool ended() const
	{
		const ChunkWaypoint & wp = pSet_->waypoint( waypointIndex_ );
		return currentEdgeIndex_ >= int(wp.edges_.size());
	}

	ChunkWaypointSetPtr pNeighbourSet() const
		{ return pNeighbourSet_; }

	int neighbourWaypointIndex() const
		{ return neighbourWaypoint_; }


	ChunkWaypoint & neighbourWaypoint() const
		{ return pNeighbourSet_->waypoint( neighbourWaypoint_ ); }

	void advance();

private:
	ChunkWaypointSetPtr pSet_;
	int waypointIndex_;

	int currentEdgeIndex_;

	ChunkWaypointSetPtr pNeighbourSet_;
	int neighbourWaypoint_;

};

#endif // WAYPOINT_NEIGHBOUR_ITERATOR
