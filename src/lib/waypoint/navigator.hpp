/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef NAVIGATOR_HPP
#define NAVIGATOR_HPP

#include "chunk_waypoint_set.hpp"

#include "cstdmf/smartpointer.hpp"
#include "cstdmf/stdmf.hpp"

#include "math/vector3.hpp"

class Chunk;
class ChunkSpace;
class ChunkWPSetState;
class ChunkWaypointSet;
class ChunkWaypointState;
class NavLoc;
class NavigatorCache;

typedef SmartPointer< ChunkWaypointSet > ChunkWaypointSetPtr;

typedef std::vector< Vector3 > Vector3Path;

/**
 *	This class guides vessels through the treacherous domain of chunk
 *	space navigation. Each instance caches recent data so similar searches
 *	can reuse previous effort.
 */
class Navigator
{
public:
	Navigator();
	~Navigator();

	bool findPath( const NavLoc & src, const NavLoc & dst, float maxDistance,
		bool blockNonPermissive, NavLoc & nextWaypoint,
		bool & passedActivatedPortal );

	bool findFullPath( const NavLoc & src, const NavLoc & dst,
			float maxDistance, bool blockNonPermissive, 
			Vector3Path & waypointPath ) const;

	void getCachedWaypointPath( Vector3Path & vec3Path );

	int getWaySetPathSize() const;

	void clearCachedWayPath();
	void clearCachedWaySetPath();

	bool infiniteLoopProblem() const 
		{ return infiniteLoopProblem_; }

private:
	Navigator( const Navigator & other );
	Navigator & operator = ( const Navigator & other );

	bool searchNextWaypoint( NavigatorCache & cache,
		const NavLoc & src,
		const NavLoc & dst,
		float maxDistance,
		bool blockNonPermissive,
		bool * pPassedActivatedPortal = NULL ) const;

	bool searchNextLocalWaypoint( NavigatorCache & cache,
		const ChunkWaypointState & srcState, 
		const ChunkWaypointState & dstState, float maxDistanceInSet ) const;
	
	bool searchNextWaypointSet( NavigatorCache & cache,
		const ChunkWPSetState & srcState,
		const ChunkWPSetState & dstState,
		float maxDistance ) const;

	NavigatorCache * pCache_;

	mutable bool infiniteLoopProblem_;
};

#endif // NAVIGATOR_HPP
