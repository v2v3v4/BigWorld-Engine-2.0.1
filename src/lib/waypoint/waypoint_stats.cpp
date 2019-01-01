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

#include "waypoint_stats.hpp"

#include "cstdmf/watcher.hpp"

BW_SINGLETON_STORAGE( WaypointStats )

/**
 *	Constructor.
 */
WaypointStats::WaypointStats():
		numWaypoints_( 0 ),
		numEdges_( 0 ),
		numVertices_( 0 ),
		mutex_()
{}


/**
 *	Destructor.
 */
WaypointStats::~WaypointStats()
{}


/**
 *	Count a single waypoint.
 */
void WaypointStats::addWaypoint()
{
	SimpleMutexHolder smh( mutex_ );
	++numWaypoints_;
}


/**
 *	Remove a single waypoint from the count.
 */
void WaypointStats::removeWaypoint()
{
	SimpleMutexHolder smh( mutex_ );
	--numWaypoints_;
}


/**
 *	Add the given number of edges to the count.	
 *
 *	@param numEdges	The number of edges to add.
 */
void WaypointStats::addEdges( uint numEdges )
{
	SimpleMutexHolder smh( mutex_ );
	numEdges_ += numEdges;
}


/**
 *	Add the given number of distinct vertices to the count.
 *
 *	@param numVertices	The number of distinct vertices to add.
 */
void WaypointStats::addVertices( uint numVertices )
{
	SimpleMutexHolder smh( mutex_ );
	numVertices_ += numVertices;
}


/**
 *	Remove the given number of edges and distinct vertices from the count.
 *
 *	@param numEdges			The number of edges to remove from the count.
 *	@param numVertices		The number of vertices to remove from the count.
 */
void WaypointStats::removeEdgesAndVertices( uint numEdges, uint numVertices )
{
	SimpleMutexHolder smh( mutex_ );
	numEdges_ -= numEdges;
	numVertices_ -= numVertices;
}


#if ENABLE_WATCHERS
/**
 *	Add watchers for the counts.
 */
void WaypointStats::addWatchers()
{
	MF_WATCH( "stats/waypoint/numWaypoints", numWaypoints_ );
	MF_WATCH( "stats/waypoint/numEdges", numEdges_ );
	MF_WATCH( "stats/waypoint/numVertices", numVertices_ );
}

#endif 

// waypoint_stats.cpp
