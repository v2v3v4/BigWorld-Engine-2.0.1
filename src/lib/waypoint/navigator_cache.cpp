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

#include "chunk_navigator.hpp"
#include "navigator_cache.hpp"
#include "navigator_find_result.hpp"

#include "astar.hpp"

#include "chunk/chunk_space.hpp"


/**
 *	Constructor.
 */
NavigatorCache::NavigatorCache() : 
	ReferenceCount(),
	wayPath_(),
	waySetPath_()
{}


/**
 *  This method saves a waypoint path.
 *
 *  Both the source and destination (goal) are extracted from the
 *  search result in the given AStar object. (i.e. must be unspoilt)
 */
void NavigatorCache::saveWayPath(
		AStar< ChunkWaypointState > & astar )
{
	wayPath_.init( astar );
	MF_ASSERT_DEBUG( wayPath_.isValid() );
}


/**
 *  This method indicates whether the cached path matches up with the next
 *  iteration's source point and the presumed destination. This is typically
 *  called when the entity has moved towards the path.
 *
 *  @param src 		The source waypoint search state.
 *  @param dst 		The destination waypoint search state.
 *
 *  @return 	The next waypoint search state stored in the cache, or NULL if
 * 				the destination changed or we have run out of intermediate
 * 				nodes.
 */
bool NavigatorCache::findWayPath( const ChunkWaypointState & src,
		const ChunkWaypointState & dst )
{
	return wayPath_.matches( src, dst );
}


/**
 *  This method saves a waypoint set path.
 *
 *  Both the source and destination (goal) are extracted from the
 *  search result in the given AStar object. (i.e. must be unspoilt)
 */
void NavigatorCache::saveWaySetPath(
	AStar< ChunkWPSetState > & astar )
{
	waySetPath_.init( astar );

	MF_ASSERT_DEBUG( waySetPath_.isValid() );
}


/**
 *  This method finds a waypoint set path that is stored in the cache.
 *
 *  @param src 	The source waypoint set search state.
 *  @param dst 	The destination waypoint set search state.
 *
 *  @return The intermediate waypoint set search state.
 */
bool NavigatorCache::findWaySetPath(
		const ChunkWPSetState & src, const ChunkWPSetState & dst )
{
	return waySetPath_.matches( src, dst );
}


/**
 *  This method finds a waypoint path
 */
int NavigatorCache::getWaySetPathSize() const
{
	return waySetPath_.size();
}


// navigator_cache.cpp
