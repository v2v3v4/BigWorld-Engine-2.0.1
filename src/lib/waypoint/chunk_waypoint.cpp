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

#include "chunk_waypoint.hpp"
#include "chunk_waypoint_vertex_provider.hpp"
#include "waypoint_stats.hpp"

#include "chunk/chunk.hpp"
#include "math/vector3.hpp"

#include <cfloat>

namespace
{

/**
 *	Get the projected point on line segment delimited by start and
 *	end. Return false if the projected point is not on this segment
 */
bool projectPointToLine( const Vector2 & start, const Vector2 & end,
		Vector2 & point )
{
	Vector2 dir = end - start;
	Vector2 project = point - start;
	float length = dir.length();

	dir.normalise();

	float dot = project.dotProduct( dir );

	if (0.f <= dot && dot <= length)
	{
		point = start + dir * dot;
		return true;
	}

	return false;
}

} // end (anonymous) namespace


std::vector<ChunkWaypoint*> ChunkWaypoint::s_visitedWaypoints_;


/**
 *	Constructor.
 */
ChunkWaypoint::ChunkWaypoint():
		minHeight_( 0.f ),
		maxHeight_( 0.f ),
		centre_( 0.f, 0.f ),
		edges_(),
		edgeCount_( 0 ),
		visited_( 0 )
{
	WaypointStats::instance().addWaypoint();
}


/**
 *	Copy constructor.
 */
ChunkWaypoint::ChunkWaypoint( const ChunkWaypoint & other ):
		minHeight_( other.minHeight_ ),
		maxHeight_( other.maxHeight_ ),
		centre_( other.centre_ ),
		edges_( other.edges_ ),
		edgeCount_( other.edgeCount_ ),
		visited_( other.visited_ )
{
	WaypointStats::instance().addWaypoint();
}


/**
 *	Destructor.
 */
ChunkWaypoint::~ChunkWaypoint()
{
	WaypointStats::instance().removeWaypoint();
}


/**
 *	This method indicates whether or not the waypoint contains the input
 *	point.
 *
 *	@param point		The point to test.
 *	@return				True if the point is contained inside the waypoint,
 *						false otherwise.
 */
bool ChunkWaypoint::contains( const ChunkWaypointVertexProvider & provider,
		const WaypointSpaceVector3 & point ) const
{
	if (point.y < minHeight_ - 0.1f)
	{
		return false;
	}

	if (point.y > maxHeight_ + 0.1f)
	{
		return false;
	}

	return this->containsProjection( provider, point );
}


/**
 *	This method indicates whether the input point lies within the waypoint
 *	polygon in x-z plane.
 *
 *	@param point		The point to test.
 *	@return				True if the point lies inside the waypoint polygon in
 *						the x-z plane, false otherwise.
 */
bool ChunkWaypoint::containsProjection( 
		const ChunkWaypointVertexProvider & provider, 
		const WaypointSpaceVector3 & point ) const
{
	float u, v, xd, zd, c;
	bool inside = true;
	Edges::const_iterator iEdge = edges_.begin();
	Edges::const_iterator end = edges_.end();

	const Vector2 * pLastPoint = &(provider.vertexByIndex( 
		edges_.back().vertexIndex_ ));

	int i = 0;
	while (iEdge != end)
	{
		i = iEdge - edges_.begin();
		const Vector2 & thisPoint = provider.vertexByIndex( 
			iEdge->vertexIndex_ );

		u = thisPoint.x - pLastPoint->x;
		v = thisPoint.y - pLastPoint->y;

		xd = point.x - pLastPoint->x;
		zd = point.z - pLastPoint->y;

		c = xd * v - zd * u;

		// since lpoint now clips to the edge add a
		// fudge factor to accommodate floating point error.
		inside &= (c > -0.01f);

		if (!inside)
		{
			return false;
		}

		pLastPoint = &thisPoint;
		++iEdge;
	}

	return inside;
}


/**
 *	This method returns the squared distance from waypoint to input point.
 *
 *	@param pChunk		The chunk to search in.
 *	@param point		The point to test.
 *	@return				The squared distance from lpoint to the waypoint
 *						polygon.
 */
float ChunkWaypoint::distanceSquared( 
		const ChunkWaypointVertexProvider & provider, const Chunk * pChunk,
		const WorldSpaceVector3 & point ) const
{
	// TODO: This is a fairly inefficient.  We may want to look at
	// optimising it.
	WorldSpaceVector3 clipPoint( point );
	this->clip( provider, pChunk, clipPoint );

	return (point - clipPoint).lengthSquared();
}


/**
 *	This method clips the lpoint to the edge of the waypoint.
 *
 *	@param pChunk		The chunk the waypoint resides.
 *	@param wpoint		Initially this is the point to clip.
 *						The point is modified as follows:
 *						For each edge that the point is outside of,
 *						project the point onto the edge.  There are 3 cases:
 *						1) If the point is before the start of the edge,
 *							then the clip point is the start since it wasn't
 *							a projection point on the previous edge.  First
 *							edge is special cased.
 *						2) If the point is between the start and end of the
 *							edge then this is the clip point.
 *						3) If the projection point is after the end of the
 *							edge, then the clip point may be the end point
 *							or on the next edge.  Leave it for the next edge
 *							to decide.
 */
void ChunkWaypoint::clip( const ChunkWaypointVertexProvider & provider,
		const Chunk * pChunk, WorldSpaceVector3 & wpoint ) const
{
	WaypointSpaceVector3 point = MappedVector3( wpoint, pChunk );

	Edges::const_iterator eit = edges_.begin();
	Edges::const_iterator end = edges_.end();

	const Vector2 * pPrevPoint = &(provider.vertexByIndex( 
		edges_.back().vertexIndex_ ));

	while (eit != end)
	{
		const Vector2 & thisPoint = provider.vertexByIndex(
			eit->vertexIndex_ );

		Vector2 edgeVec( thisPoint - *pPrevPoint );
		Vector2 pointVec( point.x - pPrevPoint->x,
			point.z - pPrevPoint->y );

		bool isOutsidePoly = edgeVec.crossProduct( pointVec ) > 0.f;

		if (isOutsidePoly)
			break;

		pPrevPoint = &thisPoint;
		++eit;
	}

	if (eit != end)
	{
		Vector2 p2d( point.x, point.z );
		float bestDistSquared = FLT_MAX;
		Edges::const_iterator bestIt;

		for (eit = edges_.begin(); eit != end; ++eit)
		{
			const Edge & edge = *eit;

			const Vector2 & start = provider.vertexByIndex( edge.vertexIndex_ );
			float distSquared = ( start - p2d ).lengthSquared();

			if (distSquared < bestDistSquared)
			{
				bestDistSquared = distSquared;
				bestIt = eit;
			}
		}

		const Vector2 & prev = provider.vertexByIndex(
			( bestIt == edges_.begin() ) ?
				edges_.back().vertexIndex_ : ( bestIt - 1 )->vertexIndex_ );

		const Vector2 & bestStart = provider.vertexByIndex(
			bestIt->vertexIndex_ );

		if (!::projectPointToLine( bestStart, prev, p2d ))
		{
			const Vector2 & next = provider.vertexByIndex(
				( bestIt + 1 == end ) ?
					edges_.front().vertexIndex_ : 
					( bestIt + 1 )->vertexIndex_ );

			if (!::projectPointToLine( bestStart, next, p2d ))
			{
				p2d = bestStart;
			}
		}

		point.x = p2d.x;
		point.z = p2d.y;
	}

	const BoundingBox & bb = pChunk->boundingBox();
	float avgHeight = ( minHeight_ + maxHeight_ ) / 2.f;
	wpoint = MappedVector3( point, pChunk );

	wpoint.y = bb.centre().y;

	if (!bb.intersects( wpoint ))
	{
		for (eit = edges_.begin(); eit != end; ++eit)
		{
			const Edge & edge = *eit;

			if (!edge.adjacentToChunk())
			{
				const Edge & next = ( eit + 1 == end ) ?
					edges_.front() : *( eit + 1 );

				const Vector2 & thisStart = provider.vertexByIndex(
					edge.vertexIndex_ );
				const Vector2 & nextStart = provider.vertexByIndex(
					next.vertexIndex_ );

				WaypointSpaceVector3 start( thisStart.x, avgHeight, 
					thisStart.y );
				WaypointSpaceVector3 end( nextStart.x, avgHeight, 
					nextStart.y );

				WorldSpaceVector3 wvStart = MappedVector3( start, pChunk );
				WorldSpaceVector3 wvEnd = MappedVector3( end, pChunk );

				bb.clip( wvStart, wvEnd );

				WorldSpaceVector3 middle( ( wvStart + wvEnd ) / 2 );

				bb.clip( middle, wpoint );
				// move in a bit
				wpoint = WorldSpaceVector3( middle + 
					( wpoint - middle ) * 0.99f );

				break;
			}
		}
	}

	this->makeMaxHeight( pChunk, wpoint );
}


/**
 *	This method makes the world space point the same height as maxHeight_
 *	of this waypoint in waypoint space
 *
 *	@param chunk		The chunk the waypoint resides. 
 *	@param wpoint		This point will be transformed into waypoint space
 *						to do assignment and then transformed back to world
 *						space.
 */
void ChunkWaypoint::makeMaxHeight( const Chunk* chunk, 
		WorldSpaceVector3 & wpoint ) const
{
	WaypointSpaceVector3 point = MappedVector3( wpoint, chunk );
	point.y = maxHeight_;
	wpoint = MappedVector3( point, chunk );
}


/**
 *	This method prints some debugging information about this waypoint.
 *
 *	@param provider The provider to resolve the waypoint vertices.
 */
void ChunkWaypoint::print( const ChunkWaypointVertexProvider & provider ) const
{
	DEBUG_MSG( "MinHeight: %g\tMaxHeight: %g\tEdgeNum:%"PRIzu"\n",
		minHeight_, maxHeight_, edges_.size() );

	for (uint16 i = 0; i < edges_.size(); ++i)
	{
		const Vector2 & start = provider.vertexByIndex(
			edges_[i].vertexIndex_ );
		DEBUG_MSG( "\t%d (%g, %g) %d - %s\n", i,
			start.x, start.y,
			edges_[ i ].neighbouringWaypoint(),
			edges_[ i ].adjacentToChunk() ? "chunk" : "no chunk" );
	}
}


/**
 *	This function calculates the centre of the navmesh
 */
void ChunkWaypoint::calcCentre( const ChunkWaypointVertexProvider & provider )
{
	float totalLength = 0.f;

	centre_ = Vector2( 0, 0 );

	for (uint16 i = 0; i < edgeCount_; ++i)
	{
		const Vector2 & start = provider.vertexByIndex( 
			edges_[ i ].vertexIndex_ );
		const Vector2 & end = provider.vertexByIndex(
			edges_[ ( i + 1 ) % edgeCount_ ].vertexIndex_ );

		float length = ( end - start ).length();

		centre_ += ( end + start ) / 2 * length;
		totalLength += length;
	}

	centre_ /= totalLength;
}


// chunk_waypoint.cpp
