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

#include "navigator.hpp"

#include "astar.hpp"
#include "chunk_navigator.hpp"
#include "chunk_waypoint_set.hpp"
#include "chunk_waypoint_set_state.hpp"
#include "chunk_waypoint_state_path.hpp"
#include "navigator_cache.hpp"
#include "navloc.hpp"

#include "chunk/base_chunk_space.hpp"

#include "cstdmf/debug.hpp"

DECLARE_DEBUG_COMPONENT2( "Waypoint", 0 )

namespace // (anonymous)
{

/**
 *	Convert the ChunkWaypointStatePath to a path of corresponding Vector3
 *	positions.
 *
 *	@param path 			The A* search results for ChunkWaypointStates.
 *	@param waypointPath 	The output path of Vector3 positions. 
 */
void addWaypointPathPoints( const ChunkWaypointStatePath & path,
		Vector3Path & waypointPath )
{
	ChunkWaypointStatePath pathCopy( path );

	// No need to add the start point.

	while (pathCopy.isValid())
	{
		// Add the intermediate points and the destination
		waypointPath.push_back( pathCopy.pNext()->navLoc().point() );
		pathCopy.pop();
	}
}

} // end namespace (anonymous)


// -----------------------------------------------------------------------------
// Section: Navigator
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
Navigator::Navigator() :
	pCache_( new NavigatorCache() ),
	infiniteLoopProblem_( false )
{
}


/**
 *	Destructor.
 */
Navigator::~Navigator()
{
	delete pCache_;
}


/**
 *	Find the path between the given NavLocs. They must be valid and distinct.
 *
 *	@note The NavLoc returned in 'nextWaypoint' should be handled with care, as
 *		  it may only be semi-valid. Do not pass it into any methods without
 *		  first verifying it using the 'guess' NavInfo constructor.  For
 *		  example, do not pass it back into findPath without doing this.  
 *
 *	@param src 					The source position.
 *	@param dst 					The destination position.
 *	@param maxSearchDistance 	The maximum search distance.
 *	@param blockNonPermissive 	Whether to take into account non-permissive
 *								portals.
 *	@param waypointPath 		The Vector3Path which will be filled with the
 *								positions for the full path.
 *
 *	@return 					true if a way could be found, false otherwise.
 */
bool Navigator::findPath( const NavLoc & src, const NavLoc & dst, 
		float maxSearchDistance, bool blockNonPermissive, NavLoc & nextWaypoint,
		bool & passedActivatedPortal )
{
	if (this->searchNextWaypoint( *pCache_, src, dst, maxSearchDistance, 
			blockNonPermissive, &passedActivatedPortal ))
	{
		nextWaypoint = pCache_->pNextWaypointState()->navLoc();
		return true;
	}
	return false;
}


/**
 *	This method finds the complete path between the source position and the
 *	destination position, and fills the waypoint path. It does not clobber the
 *	cache for this Navigator. 
 *
 *	@param src 					The source position.
 *	@param dst 					The destination position.
 *	@param maxSearchDistance 	The maximum search distance.
 *	@param blockNonPermissive 	Whether to take into account non-permissive
 *								portals.
 *	@param waypointPath 		The Vector3Path which will be filled with the
 *								positions for the full path.
 */
bool Navigator::findFullPath( const NavLoc & src, const NavLoc & dst, 
		float maxSearchDistance, bool blockNonPermissive,
		Vector3Path & waypointPath ) const
{
	waypointPath.clear();
	infiniteLoopProblem_ = false;
	
	// We don't clobber the entity's current navigator cache, so we use our
	// internal one.
	NavigatorCache cache; 

	ChunkWPSetState dstSetState( dst );

	NavLoc current( src );
	bool done = false;
	bool ok = false;

	while (!done)
	{
		if (this->searchNextWaypoint( cache, current, dst, maxSearchDistance,
				blockNonPermissive ))
		{
			// We found a way through the current chunk, add the points in the
			// cache.
			const ChunkWaypointStatePath & path = cache.wayPath();
			::addWaypointPathPoints( path, waypointPath );

			// Need to resolve the waypoint path destination's waypoint index.
			NavLoc pathDestLoc = path.pDest()->navLoc();
			current = NavLoc( pathDestLoc, pathDestLoc.point() );
			
			if (current.waypointsEqual( dst ))
			{
				waypointPath.push_back( dst.point() );
				done = true;
				ok = true;
			}
		}
		else
		{
			waypointPath.clear();
			done = true;
		}
	}

	return ok;
}


/**
 *	This returns the cached waypoint path. If the destination point was in a
 *	different chunk, this will be only to the edge of the source point's
 *	waypoint set. If there was no cached path, vec3Path will be empty.
 */
void Navigator::getCachedWaypointPath( Vector3Path & vec3Path )
{
	vec3Path.clear();

	// Add the intermediate points and the destination.
	::addWaypointPathPoints( pCache_->wayPath(), vec3Path );
}

/**
 *  This method returns the number of elements in the waypoint set path
 *  currently cached.
 */
int Navigator::getWaySetPathSize() const
{
	return pCache_->getWaySetPathSize();
}


/**
 *	Clears the cached waypoint path.
 */
void Navigator::clearCachedWayPath()
{
	pCache_->clearWayPath();
}


/**
 *	Clears the cached waypoint set path.
 */
void Navigator::clearCachedWaySetPath()
{
	pCache_->clearWaySetPath();
}


/**
 *	This method indicates whether there is a waypoint path between the two
 *	given search states, that may be in different waypoint sets.
 *
 *	If there is no corresponding path in the cache, it performs a search and,
 *	if successful, stores it in the cache.
 *
 *	@param cache				The navigation cache to use.
 *	@param src					The position to navigate from.
 *	@param dst					The position to navigate to.
 *	@param maxSearchDistance 	The maximum search distance.
 *	@param blockNonPermissive 	If true, then non-permissive portals are taken
 *								into account, otherise permissivity.
 *	@param pPassedActivatedPortal
 *								An optional pointer to a boolean that is filled
 *								with a value indicated whether an activated
 *								portal was passed when traversing between
 *								waypoint sets.
 */
bool Navigator::searchNextWaypoint( NavigatorCache & cache,
		const NavLoc & src, const NavLoc & dst,
		float maxSearchDistance,
		bool blockNonPermissive,
		bool * pPassedActivatedPortal /* = NULL */ ) const
{
	MF_ASSERT_DEBUG( src.valid() && dst.valid() );

	if (pPassedActivatedPortal)
	{
		*pPassedActivatedPortal = false;
	}

	infiniteLoopProblem_ = false;

	ChunkWaypointState srcState( src );
	ChunkWaypointState dstState( dst );
	
	const ChunkWPSetState * pNextSetState = NULL;

	if (src.pSet() != dst.pSet())
	{
		// The source and destination are in different waypoint sets.
		
		// We need to find a path amongst the waypoint sets, it may be cached
		// from an earlier navigation query.
		ChunkWPSetState srcSetState( src );
		ChunkWPSetState dstSetState( dst );

		srcSetState.blockNonPermissive( blockNonPermissive );

		if (!this->searchNextWaypointSet( cache, srcSetState, dstSetState, 
					maxSearchDistance ))
		{
			return false;
		}

		pNextSetState = cache.pNextWaypointSetState();
		MF_ASSERT( pNextSetState != NULL );

		// Search to the edge of the current waypoint set.
		dstState = ChunkWaypointState( pNextSetState->pSet(), dst.point() );
	}

	// It makes no sense to check for maxSearchDistance inside navpoly set
	// if it is greater than GRID_RESOLUTION.
	
	const float maxSearchDistanceInSet = 
		(maxSearchDistance > GRID_RESOLUTION) ?
			-1 : maxSearchDistance;

	if (!this->searchNextLocalWaypoint( cache, srcState, dstState, 
			maxSearchDistanceInSet ))
	{
		return false;
	}

	// If we traversed a set, see if we went through an activated portal.
	if (pPassedActivatedPortal != NULL && 
			pNextSetState != NULL &&
			cache.pNextWaypointState()->navLoc().pSet() != src.pSet() && 
			pNextSetState->passedActivatedPortal())
	{
		*pPassedActivatedPortal = true;
	}

	return true;
}


/**
 *	This method indicates whether there is a waypoint path between the
 *	two given search states, assumed to be in the same waypoint set. If there
 *	is no corresponding path in the cache, it performs a search and, if
 *	successful, stores it in the cache.
 *
 *	@param cache 				The navigation cache to use.
 *	@param srcState 			The source search state.
 *	@param dstState 			The destination search state.
 *	@param maxSearchDistance	The maximum search distance.
 *
 *	@return 	true if a path exists, either from the cache or it was
 *				searched, otherwise false if no path exists. If a search
 *				occurred, the cache is updated with the result. 
 */
bool Navigator::searchNextLocalWaypoint( NavigatorCache & cache,
		const ChunkWaypointState & srcState, 
		const ChunkWaypointState & dstState, 
		float maxSearchDistance ) const
{
	if (!cache.findWayPath( srcState, dstState ))
	{
		// Search, and reset the cache.
		AStar< ChunkWaypointState > astar;

		if (astar.search( srcState, dstState, maxSearchDistance ))
		{
			cache.saveWayPath( astar );
		}
		else
		{
			cache.clearWayPath(); // for sanity

			if (astar.infiniteLoopProblem)
			{
				ERROR_MSG( "Navigator::searchNextLocalWaypoint: "
						"Infinite Loop problem in %s from %d to %d\n", 
					srcState.navLoc().pSet()->chunk()->identifier().c_str(),
					srcState.navLoc().waypoint(), 
					dstState.navLoc().waypoint() );
				infiniteLoopProblem_ = true;
			}
			return false;
		}
	}
	return true;
}


/**
 *	This method indicates whether there is a waypoint set path between the two
 *	given search states. If there is no corresponding path in the cache, it
 *	performs a search and, if successful, stores it in the cache.
 *
 *	@param cache 		The navigation cache to use.
 *	@param srcState 	
 */
bool Navigator::searchNextWaypointSet( NavigatorCache & cache,
		const ChunkWPSetState & srcState,
		const ChunkWPSetState & dstState,
		float maxSearchDistance ) const
{
	bool bypassCache = false;
	
	// Don't use the cache if the cache contains a waypoint set path that
	// passed through a portal belonging to an indoor chunk.
	bypassCache = bypassCache || cache.waySetPathPassedShellBoundary();

	if (bypassCache ||
			!cache.findWaySetPath( srcState, dstState ))
	{
		// Recalculate the waypoint set path via A* search.
		cache.clearWayPath();
		cache.clearWaySetPath();

		AStar< ChunkWPSetState > astarSet;

		if (astarSet.search( srcState, dstState, maxSearchDistance ))
		{
			cache.saveWaySetPath( astarSet );
		}
		else
		{
			cache.clearWaySetPath(); // for sanity

			if (astarSet.infiniteLoopProblem)
			{
				Chunk * pSrcChunk = srcState.pSet()->chunk();
				const Vector3 & srcPoint = srcState.position();
				Chunk * pDstChunk = dstState.pSet()->chunk();
				const Vector3 & dstPoint = dstState.position();

				ERROR_MSG( "Navigator::searchNextWaypointSet: "
						"Infinite Loop problem from %s "
						"(%.2f, %.2f, %.2f) to %s (%.2f, %.2f, %.2f)\n", 
					pSrcChunk->identifier().c_str(),
					srcPoint.x, srcPoint.y, srcPoint.z,
					pDstChunk->identifier().c_str(),
					dstPoint.x, dstPoint.y, dstPoint.z );

				infiniteLoopProblem_ = true;
			}
			return false;
		}
	}

	return true;
}


// navigator.cpp
