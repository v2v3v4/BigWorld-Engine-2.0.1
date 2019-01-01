/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_WAYPOINT_VERTEX_PROVIDER
#define CHUNK_WAYPOINT_VERTEX_PROVIDER

#include "chunk_waypoint.hpp"

#include "math/vector2.hpp"


class ChunkWaypointVertexProvider
{
public:
	virtual const Vector2 & vertexByIndex( EdgeIndex index ) const = 0;
};

#endif // CHUNK_WAYPOINT_VERTEX_PROVIDER
