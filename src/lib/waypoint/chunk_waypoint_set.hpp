/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_WAYPOINT_SET_HPP
#define CHUNK_WAYPOINT_SET_HPP

#include "chunk_waypoint.hpp"
#include "chunk_waypoint_set_data.hpp"
#include "chunk_waypoint_vertex_provider.hpp"
#include "dependent_array.hpp"

#include "chunk/chunk_item.hpp"
#include "chunk/chunk_boundary.hpp"
#include "chunk/chunk.hpp"

#include <map>
#include <vector>


typedef uint32 WaypointEdgeIndex;


class	ChunkWaypointSet;
typedef SmartPointer< ChunkWaypointSet > ChunkWaypointSetPtr;

typedef std::vector< ChunkWaypointSetPtr > ChunkWaypointSets;

typedef std::map< ChunkWaypointSetPtr, ChunkBoundary::Portal * >
	ChunkWaypointConns;

typedef std::map< WaypointEdgeIndex, ChunkWaypointSetPtr >
	ChunkWaypointEdgeLabels;


/**
 *	This class is a set of connected waypoints in a chunk.
 *	It may have connections to other waypoint sets when its chunk is bound.
 */
class ChunkWaypointSet : public ChunkItem, public ChunkWaypointVertexProvider
{
	DECLARE_CHUNK_ITEM( ChunkWaypointSet )

public:
	ChunkWaypointSet( ChunkWaypointSetDataPtr pSetData );
	~ChunkWaypointSet();

	virtual void toss( Chunk * pChunk );

	void bind();

	/**
	 *	This finds the waypoint that contains the given point.
	 *
	 *	@param lpoint		The point that is used to find the waypoint.
	 *	@param ignoreHeight	Flag indicates vertical range should be considered
	 *						in finding waypoint.  If not, the best waypoint
	 *						that is closest to give point is selected (of
	 *						course the waypoint should contain projection of
	 *						the given point regardless).
	 *	@return				The index of the waypoint that contains the point,
	 *						-1 if no waypoint contains the point.
	 */
	int find( const WaypointSpaceVector3 & point, bool ignoreHeight = false )
	{
		return pSetData_->find( point, ignoreHeight );
	}

	/**
	 *	This finds the waypoint that contains the given point.
	 *
	 *	@param lpoint		The point that is used to find the waypoint.
	 *	@param bestDistanceSquared	The point must be closer than this to the
	 *						waypoint.  It is updated to the new best distance
	 *						if a better waypoint is found.
	 *	@return				The index of the waypoint that contains the point,
	 *						-1 if no waypoint contains the point.
	 */
	int find( const WaypointSpaceVector3 & point, float & bestDistanceSquared )
	{ 
		return pSetData_->find( chunk(), point, bestDistanceSquared ); 
	}

	/**
	 *	This gets the girth of the waypoints.
	 *
	 *	@returns			The girth of the waypoints.
	 */
	float girth() const
	{
		return pSetData_->girth();
	}

	/**
	 *	This gets the number of waypoints in the set.
	 *
	 *	@returns			The number of waypoints.
	 */
	int waypointCount() const
	{
		return pSetData_->waypoints().size();
	}

	/**
	 *	This gets the index'th waypoint.
	 *
	 *	@param index		The index of the waypoint to get.
	 *	@return				A reference to the index'th waypoint.
	 */
	ChunkWaypoint & waypoint( int index )
	{
		return pSetData_->waypoints()[index];
	}

	/**
	 *	This gets the index'th waypoint.
	 *
	 *	@param index		The index of the waypoint to get.
	 *	@return				A reference to the index'th waypoint.
	 */
	const ChunkWaypoint & waypoint( int index ) const
	{
		return pSetData_->waypoints()[index];
	}

	/**
	 *	This returns an iterator to the first connection.
	 *
	 *	@return				An iterator to the first connection.
	 */
	ChunkWaypointConns::iterator connectionsBegin()
	{
		return connections_.begin();
	}

	/**
	 *	This returns a const iterator to the first connection.
	 *
	 *	@return				A const iterator to the first connection.
	 */
	ChunkWaypointConns::const_iterator connectionsBegin() const
	{
		return connections_.begin();
	}

	/**
	 *	This returns an iterator that points one past the last connection.
	 *
	 *	@return				An iterator to one past the last connection.
	 */
	ChunkWaypointConns::iterator connectionsEnd()
	{
		return connections_.end();
	}

	/**
	 *	This returns a const iterator that points one past the last connection.
	 *
	 *	@return				A const iterator to one past the last connection.
	 */
	ChunkWaypointConns::const_iterator connectionsEnd() const
	{
		return connections_.end();
	}

	/**
	 *	This gets the portal for the given waypoint set.
	 *
	 *	@param pWaypointSet	The ChunkWaypointSet to get the portal for.
	 *	@return				The portal for the ChunkWaypointSet.
	 */
	ChunkBoundary::Portal * connectionPortal( ChunkWaypointSetPtr pWaypointSet )
	{
		return connections_[pWaypointSet];
	}

	/**
	 *	This method gets the corresponding ChunkWaypointSet for an edge.
	 *
	 *	@param edge			The ChunkWaypoint::Edge to get the ChunkWaypointSet
	 *						for.
	 *	@return				The ChunkWaypointSet along the edge.
	 */
	ChunkWaypointSetPtr connectionWaypoint( const ChunkWaypoint::Edge & edge )
	{
		return edgeLabels_[pSetData_->getAbsoluteEdgeIndex( edge )];
	}

	void addBacklink( ChunkWaypointSetPtr pWaypointSet );

	void removeBacklink( ChunkWaypointSetPtr pWaypointSet );

	void print() const;

	// From ChunkWaypointVertexProvider
	virtual const Vector2 & vertexByIndex( EdgeIndex index ) const
		{ return pSetData_->vertexByIndex( index ); }

private:
	bool readyToBind() const;
	void deleteConnection( ChunkWaypointSetPtr pSet );

	void removeOurConnections();
	void removeOthersConnections();

	void connect(
		ChunkWaypointSetPtr pWaypointSet,
		ChunkBoundary::Portal * pPortal,
		const ChunkWaypoint::Edge & edge );

protected:
	ChunkWaypointSetDataPtr		pSetData_;
	ChunkWaypointConns			connections_;
	ChunkWaypointEdgeLabels		edgeLabels_;
	ChunkWaypointSets			backlinks_;
};



#endif // CHUNK_WAYPOINT_SET_HPP
