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

#include "waypoint_neighbour_iterator.hpp"

#include "navloc.hpp"

/**
 *	Constructor.
 */
WaypointNeighbourIterator::WaypointNeighbourIterator( 
			ChunkWaypointSetPtr pSet, int waypoint ) :
		pSet_( pSet ), 
		waypointIndex_( waypoint ), 
		currentEdgeIndex_( -1 ),
		neighbourWaypoint_( -1 )
{
	this->advance();
}


/**
 *	Advance to the next neighbouring waypoint.
 */
void WaypointNeighbourIterator::advance()
{
	pNeighbourSet_ = NULL;
	neighbourWaypoint_ = -1;

	const ChunkWaypoint & wp = pSet_->waypoint( waypointIndex_ );

	while (!pNeighbourSet_ &&
			currentEdgeIndex_ < int(wp.edges_.size()))
	{
		++currentEdgeIndex_;

		if (currentEdgeIndex_ >= int(wp.edges_.size()))
		{
			break;
		}

		const ChunkWaypoint::Edge & wpe = wp.edges_[ currentEdgeIndex_ ];
		int neighbour = wpe.neighbouringWaypoint();
		bool adjToChunk = wpe.adjacentToChunk();

		if (neighbour != -1)
		{
			pNeighbourSet_ = pSet_;
			neighbourWaypoint_ = neighbour;
		}
		else if (adjToChunk)
		{
			const ChunkWaypoint::Edge & next =
				wp.edges_[ ( currentEdgeIndex_ + 1 ) % wp.edges_.size() ];
			const Vector2 & start = 
				pSet_->vertexByIndex( wpe.vertexIndex_ );
			const Vector2 & end = 
				pSet_->vertexByIndex( next.vertexIndex_ );

			Vector3 position( ( start.x + end.x ) / 2,
				wp.maxHeight_, 
				( start.y + end.y ) / 2 );

			NavLoc other( pSet_->chunk()->space(), position, 
				pSet_->girth() );

			if (other.valid())
			{
				pNeighbourSet_ = other.pSet();
				neighbourWaypoint_ = other.waypoint();
			}
		}
	}
}

// waypoint_neighbour_iterator.cpp
