/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_WAYPOINT_HPP
#define CHUNK_WAYPOINT_HPP

#include "dependent_array.hpp"

#include "cstdmf/stdmf.hpp"
#include "cstdmf/debug.hpp"

#include "math/vector2.hpp"

#include "mapped_vector3.hpp"


class Chunk;
class ChunkWaypointVertexProvider;
class Vector3;


typedef GeoSpaceVector3 WaypointSpaceVector3;
typedef uint16 EdgeIndex;

/**
 *	This class is a waypoint as it exists in a chunk, when fully loaded.
 */
class ChunkWaypoint
{
public:
	ChunkWaypoint();
	ChunkWaypoint( const ChunkWaypoint & other );
	~ChunkWaypoint();


	bool contains( const ChunkWaypointVertexProvider & provider, 
		const WaypointSpaceVector3 & point ) const;

	bool containsProjection( const ChunkWaypointVertexProvider & provider, 
		const WaypointSpaceVector3 & point ) const;

	float distanceSquared( const ChunkWaypointVertexProvider & provider,
		const Chunk * chunk,
		const WorldSpaceVector3 & point ) const;

	void clip( const ChunkWaypointVertexProvider & provider,
		const Chunk * chunk, WorldSpaceVector3 & point ) const;

	void makeMaxHeight( const Chunk * chunk, WorldSpaceVector3 & point ) const;

	void print( const ChunkWaypointVertexProvider & provider ) const;

	void calcCentre( const ChunkWaypointVertexProvider & provider );

// Member data

	/**
	 *	This is the minimum height of the waypoint.
	 */
	float minHeight_;

	/**
	 *	This is the maximum height of the waypoint.
	 */
	float maxHeight_;

	/**
	 *	This is a point inside the waypoint. it might
	 *  not be the exact centre but could be used as
	 *	an internal point in some cases.
	 */
	Vector2 centre_;

	/**
	 *	This class is an edge in a waypoint/navpoly.
	 */
	struct Edge
	{
		/**
		 *	The index of the start coordinate of the edge.
		 */
		EdgeIndex	vertexIndex_;

		/**
		 *	This contains adjacency information.  If this value ranges
		 *	between 0 and 32768, then that is the ID of the waypoint adjacent
		 *	to this edge.  If this value is between 32768 and 65535, then it
		 *	is adjacent to	the chunk boundary.
		 *	Otherwise, it may contain some vista flags indicating cover, for
		 *	example, if its value's top bit is 1.
		 */
		uint32		neighbour_;

		/**
		 *	This returns the neighbouring waypoint's index.
		 *
		 *	@return			The index of the neighbouring waypoint if there
		 *					is one, or -1 if there	is none.
		 */
		int neighbouringWaypoint() const		// index into waypoints_
		{
			return neighbour_ < 32768 ? int(neighbour_) : -1;
		}

		/**
		 *	This returns whether this edge is adjacent to a chunk boundary.
		 *
		 *	@return			True if the edge is adjacent to a chunk boundary,
		 *					false otherwise.
		 */
		bool adjacentToChunk() const
		{
			return neighbour_ >= 32768 && neighbour_ <= 65535;
		}

		/**
		 *	This gets the vista bit flags.
		 *
		 *	@returns		The vista bit flags.
		 */
		uint32 neighbouringVista() const		// vista bit flags
		{
			return int(neighbour_) < 0 ? ~neighbour_ : 0;
		}
	};


	/**
	 *	This is the list of edges of the ChunkWaypoint.
	 */
	typedef DependentArray< Edge > Edges;
	Edges	edges_;

	/**
	 *	This is the number of edges of the ChunkWaypoint.
	 *
	 *	DO NOT move this away!  DependentArray depends on edgeCount_ to exist
	 *	on its construction.
	 */
	uint16	edgeCount_;

	mutable uint16	visited_;

	static std::vector<ChunkWaypoint*> s_visitedWaypoints_;

};

typedef std::vector< ChunkWaypoint > ChunkWaypoints;

#endif // CHUNK_WAYPOINT_HPP
