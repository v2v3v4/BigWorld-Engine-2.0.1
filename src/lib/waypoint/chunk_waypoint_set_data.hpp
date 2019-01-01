/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_WAYPOINT_SET_DATA_HPP
#define CHUNK_WAYPOINT_SET_DATA_HPP

#include "chunk_waypoint.hpp"
#include "chunk_waypoint_vertex_provider.hpp"

#include "cstdmf/smartpointer.hpp"

#include <string>

class ChunkWaypointSetData;
typedef SmartPointer< ChunkWaypointSetData > ChunkWaypointSetDataPtr;

class DataSection;
typedef SmartPointer< DataSection > DataSectionPtr;

class Chunk;
class ChunkWaypointSet;
class Matrix;
class Vector3;

/**
 *	This class contains the data read in from a waypoint set.
 *	It may be shared between chunks. (In which case it is in local co-ords.)
 */
class ChunkWaypointSetData : public SafeReferenceCount,
		public ChunkWaypointVertexProvider
{
public:
	ChunkWaypointSetData();

	virtual ~ChunkWaypointSetData();

	int find( const WaypointSpaceVector3 & point, bool ignoreHeight = false );

	int find( const Chunk* chunk, const WaypointSpaceVector3 & point, 
		float & bestDistanceSquared );

	int getAbsoluteEdgeIndex( const ChunkWaypoint::Edge & edge ) const;

	// From ChunkWaypointVertexProvider
	virtual const Vector2 & vertexByIndex( EdgeIndex index ) const
	{
		MF_ASSERT( index < vertices_.size() );
		return vertices_[index];
	}

	float girth() const
		{ return girth_; }
	
	void girth( float girth )
		{ girth_ = girth; }

	ChunkWaypoints & waypoints()
		{ return waypoints_; }

	const ChunkWaypoints & waypoints() const
		{ return waypoints_; }

	const char * readWaypointSet( const char * pNavPolyData, int numWaypoints,
		int numEdges );

	static ChunkItemFactory::Result navmeshFactory( Chunk * pChunk,
		DataSectionPtr pSection );

private:
	float						girth_;
	ChunkWaypoints				waypoints_;
	std::string					source_;
	ChunkWaypoint::Edge * 		edges_;
	uint						numEdges_;

	typedef std::vector< Vector2 > WaypointVertices;
	WaypointVertices 			vertices_;
};


#endif // CHUNK_WAYPOINT_SET_DATA_HPP
