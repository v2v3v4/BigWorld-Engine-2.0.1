/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef NAVIGATOR_CACHE_HPP
#define NAVIGATOR_CACHE_HPP

#include "astar.hpp"
#include "chunk_waypoint_set_state.hpp"
#include "chunk_waypoint_set_state_path.hpp"
#include "chunk_waypoint_state.hpp"
#include "chunk_waypoint_state_path.hpp"

#include "cstdmf/smartpointer.hpp"


/**
 *  This class caches data from recent searches. It is purposefully not
 *  defined in the header file so that our users need not know its contents.
 */
class NavigatorCache : public ReferenceCount
{
public:
	NavigatorCache();

	virtual ~NavigatorCache() 
	{}

	void saveWayPath( 
		AStar< ChunkWaypointState > & astar );

	/**
	 * Return this cache's waypoint path.
	 */
	const ChunkWaypointStatePath & wayPath() const 
		{ return wayPath_; }

	bool findWayPath( const ChunkWaypointState & src,
			const ChunkWaypointState & dest );

	/**
	 *	Get the location of the next waypoint in the stored waypoint path.
	 */
	const ChunkWaypointState * pNextWaypointState() const 
		{ return wayPath_.pNext(); }

	/**
	 *	Clear the waypoint path.
	 */
	void clearWayPath()
		{ wayPath_.clear(); }


	void saveWaySetPath( AStar< ChunkWPSetState > & astar );
	
	bool findWaySetPath(
		const ChunkWPSetState & src, const ChunkWPSetState & dst );

	/**
	 *	Return the next waypoint set state in the path.
	 */
	const ChunkWPSetState * pNextWaypointSetState() const
	{ 
		return waySetPath_.pNext();
	}

	/**
	 *	Clears the waypoint set path.
	 *	
	 *	This may be necessary because different results will be obtained
	 *	depending on whether ChunkWPSetState::blockNonPermissive is true or
	 *	false.
	 */
	void clearWaySetPath()
		{ waySetPath_.clear(); }
	
	int getWaySetPathSize() const;

	/**
	 *	Return true if the waypoint set path traverses a shell boundary.
	 */
	bool waySetPathPassedShellBoundary() const
		{ return waySetPath_.passedShellBoundary(); }

private:
	ChunkWaypointStatePath 		wayPath_;
	ChunkWaypointSetStatePath	waySetPath_;

};

#endif // NAVIGATOR_CACHE_HPP
