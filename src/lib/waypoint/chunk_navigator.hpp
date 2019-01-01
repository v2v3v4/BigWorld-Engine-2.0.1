/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_NAVIGATOR_HPP
#define CHUNK_NAVIGATOR_HPP

#include "chunk_waypoint_set.hpp"
#include "girth_grids.hpp"

#include "chunk/chunk.hpp"

#include "cstdmf/smartpointer.hpp"

#include <vector>

class NavigatorFindResult;
class Vector3;

/**
 *  This class is a cache of all the waypoint sets in a chunk.
 */
class ChunkNavigator : public ChunkCache
{
public:
	ChunkNavigator( Chunk & chunk );
	~ChunkNavigator();

	virtual void bind( bool isUnbind );

	bool find( const WorldSpaceVector3& point, float girth,
				NavigatorFindResult & res, bool ignoreHeight = false ) const;

	bool isEmpty() const;
	bool hasNavPolySet( float girth ) const;

	void add( ChunkWaypointSet * pSet );
	void del( ChunkWaypointSet * pSet );

	static bool shouldUseGirthGrids()
		{ return s_useGirthGrids_; }

	static void shouldUseGirthGrids( bool value )
		{ s_useGirthGrids_ = value; }

	static Instance< ChunkNavigator > instance;

private:
	Chunk & chunk_;

	ChunkWaypointSets	wpSets_;

	GirthGrids	girthGrids_;
	Vector2		girthGridOrigin_;
	float		girthGridResolution_;

	static bool s_useGirthGrids_;

	static const uint GIRTH_GRID_SIZE = 12;

	void tryGrid( GirthGridList* gridList, const WaypointSpaceVector3& point,
		float& bestDistanceSquared, int xi, int zi, NavigatorFindResult& res ) const;
};


#endif // CHUNK_NAVIGATOR_HPP
