/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WAYPOINT_STATS_HPP
#define WAYPOINT_STATS_HPP

#include "cstdmf/concurrency.hpp"
#include "cstdmf/singleton.hpp"

/**
 *	Class to count how many waypoint objects are currently stored in memory.
 */
class WaypointStats : public Singleton< WaypointStats >
{
public:
	WaypointStats();
	~WaypointStats();

	void addWaypoint();
	void removeWaypoint();
	void addEdges( uint numEdges );
	void addVertices( uint numVertices );
	void removeEdgesAndVertices( uint numEdges, uint vertices );
#if ENABLE_WATCHERS
	void addWatchers();
#endif

private:
	uint numWaypoints_;
	uint numEdges_;
	uint numVertices_;

	SimpleMutex mutex_;
};

#endif // WAYPOINT_STATS_HPP
