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

#include "chunk_navigator.hpp"

#include "chunk_waypoint_set.hpp"
#include "girth_grid_list.hpp"
#include "navigator_find_result.hpp"

#include "math/vector3.hpp"

#include <cfloat>

// static initialisers
ChunkNavigator::Instance< ChunkNavigator > ChunkNavigator::instance;

bool ChunkNavigator::s_useGirthGrids_ = true;


/**
 *  This is the ChunkNavigator constructor.
 *
 *	@param chunk		The chunk to cache the ChunkWaypointSets.
 */
ChunkNavigator::ChunkNavigator( Chunk & chunk ) :
	chunk_( chunk ),
	wpSets_(),
	girthGrids_(),
	girthGridOrigin_(),
	girthGridResolution_( 0.f )
{
  if (s_useGirthGrids_)
  {
	// set up the girth grid parameters
	BoundingBox bb = chunk_.localBB();
	Matrix m = chunk_.transform();

	m.postMultiply( chunk_.mapping()->invMapper() );
	bb.transformBy( m );

	float maxDim = std::max( bb.maxBounds().x - bb.minBounds().x,
		bb.maxBounds().z - bb.minBounds().z );

	// extra grid square off each edge
	float oneSqProp = 1.f / (GIRTH_GRID_SIZE - 2);

	girthGridOrigin_ = Vector2( bb.minBounds().x - maxDim*oneSqProp,
		bb.minBounds().z - maxDim*oneSqProp );

	girthGridResolution_ = 1.f / (maxDim * oneSqProp);
  }
}


/**
 *  This is the ChunkNavigator destructor.
 */
ChunkNavigator::~ChunkNavigator()
{
  if (s_useGirthGrids_)
  {
	for (GirthGrids::iterator git = girthGrids_.begin();
		git != girthGrids_.end();
		++git)
	{
		delete [] git->grid;
	}
	girthGrids_.clear();
  }
}


/**
 *	This is the bind method that tells all our waypoint sets about it.
 *
 *	@param isUnbind		Not used.
 */
void ChunkNavigator::bind( bool /*isUnbind*/ )
{
	// TODO: Should this be being called in the isUnbind == true case?
	ChunkWaypointSets::iterator it;
	for (it = wpSets_.begin(); it != wpSets_.end(); ++it)
	{
		(*it)->bind();
	}
}


/**
 *  Try a grid to see if it is a better choice
 */
void ChunkNavigator::tryGrid( GirthGridList* gridList, const WaypointSpaceVector3& point,
	float& bestDistanceSquared, int xi, int zi, NavigatorFindResult& res ) const
{
	int x = (xi);
	int z = (zi);

	if (uint(x) < GIRTH_GRID_SIZE && uint(z) < GIRTH_GRID_SIZE)
	{
		gridList[x + z * GIRTH_GRID_SIZE].findNonVisited( &chunk_, point,
			bestDistanceSquared, res );
	}
}

/**
 *  Find the waypoint and its set closest to the given point of matching
 *	girth.
 *
 *	@param point			The point to test.
 *	@param girth			The girth to search with.
 *	@param res				The result of the find.
 *	@param ignoreHeight		If true then the search is done only in the x-z
 *							directions.
 *	@return					True if the search was successful, false otherwise.
 */
bool ChunkNavigator::find( const WorldSpaceVector3 & wpoint, float girth,
		NavigatorFindResult & res, bool ignoreHeight ) const
{
	GirthGrids::const_iterator git;
	for (git = girthGrids_.begin(); git != girthGrids_.end(); ++git)
	{
		if (git->girth == girth)
		{
			break;
		}
	}

	GeoSpaceVector3 point = MappedVector3( wpoint, &chunk_ );

	if (git != girthGrids_.end())
	{
		// we have an appropriate girth grid
		int xg = int((point.x - girthGridOrigin_.x) * girthGridResolution_);
		int zg = int((point.z - girthGridOrigin_.y) * girthGridResolution_);
		// no need to round above conversions as always +ve
		if (uint(xg) >= GIRTH_GRID_SIZE || uint(zg) >= GIRTH_GRID_SIZE)
		{
			return false;	// hmmm
		}
		GirthGridList * gridList = git->grid;

		// try to find closest one then
		float bestDistanceSquared = FLT_MAX;

		if (gridList[xg + zg * GIRTH_GRID_SIZE].findMatchOrLower( point, res ))
		{
			ChunkWaypoint & wp = res.pSet()->waypoint( res.waypoint() );
			WaypointSpaceVector3 wv = MappedVector3( point, &chunk_ );

			if (wp.minHeight_ <= wv.y && wv.y <= wp.maxHeight_)
			{
				return true;
			}
			else
			{
				bestDistanceSquared = wp.maxHeight_ - point.y;
				bestDistanceSquared *= bestDistanceSquared;
			}
		}

		bool result = false;

		// first try the original grid square
		tryGrid( gridList, point, bestDistanceSquared, xg, zg, res );

		// now try ever-increasing rings around it
		for (int r = 1; r < (int)GIRTH_GRID_SIZE; r++)
		{
			bool hadCandidate = (res.pSet() != NULL);

			// r is the radius of our ever-increasing square
			int xgCorner = xg - r;
			int zgCorner = zg - r;

			for (int n = 0; n < r+r; n++)
			{
				tryGrid( gridList, point, bestDistanceSquared,
					xgCorner + n  , zg - r, res );
				tryGrid( gridList, point, bestDistanceSquared,
					xgCorner + n + 1, zg + r, res );
				tryGrid( gridList, point, bestDistanceSquared,
					xg - r, zgCorner + n + 1, res );
				tryGrid( gridList, point, bestDistanceSquared,
					xg + r, zgCorner + n, res );
			}

			// if we found one in the _previous_ ring then we can stop now
			if (hadCandidate)
			{
				result = true;
				break;
			}
			// NOTE: Actually, this is not entirely true, due to the
			// way that large triangular waypoints are added to the
			// grids.  This should do until we go binary w/bsps however.
		}

		std::vector<ChunkWaypoint*>::iterator iter;

		for (iter = ChunkWaypoint::s_visitedWaypoints_.begin();
			iter != ChunkWaypoint::s_visitedWaypoints_.end(); ++iter)
		{
			(*iter)->visited_ = 0;
		}

		ChunkWaypoint::s_visitedWaypoints_.clear();

		return result;
#undef TRY_GRID
	}

	ChunkWaypointSets::const_iterator it;
	for (it = wpSets_.begin(); it != wpSets_.end(); ++it)
	{
		if ((*it)->girth() != girth)
		{
			continue;
		}

		int found = (*it)->find( point, ignoreHeight );
		if (found >= 0)
		{
			res.pSet( *it );
			res.waypoint( found );
			res.exactMatch( true );
			return true;
		}
	}

	// no exact match, so use the closest one then
	float bestDistanceSquared = FLT_MAX;
	for (it = wpSets_.begin(); it != wpSets_.end(); ++it)
	{
		if ((*it)->girth() != girth)
		{
			continue;
		}

		int found = (*it)->find( point, bestDistanceSquared );

		if (found >= 0)
		{
			res.pSet( *it );
			res.waypoint( found );
			res.exactMatch( false );
		}
	}

	return (res.pSet() != NULL);
}



/**
 *	Add the given waypoint set to our cache.
 *
 *	@param pSet			The set to add to the cache.
 */
void ChunkNavigator::add( ChunkWaypointSet * pSet )
{
	wpSets_.push_back( pSet );

	// get out now if girth grids are disabled
	if (!s_useGirthGrids_)
	{
		return;
	}

	// only use grids on outside chunks
	if (!chunk_.isOutsideChunk())
	{
		return;
	}

	GirthGridList * gridList = NULL;

	// ensure a grid exists for this girth
	GirthGrids::iterator git;
	for (git = girthGrids_.begin(); git != girthGrids_.end(); ++git)
	{
		if (git->girth == pSet->girth())
		{
			gridList = git->grid;
			break;
		}
	}

	if (git == girthGrids_.end())
	{
		girthGrids_.push_back( GirthGrid() );
		GirthGrid & girthGrid = girthGrids_.back();
		girthGrid.girth = pSet->girth();
		gridList = girthGrid.grid = new GirthGridList[GIRTH_GRID_SIZE * GIRTH_GRID_SIZE];
	}


	// add every waypoint to it
	for (int i = 0; i < pSet->waypointCount(); ++i)
	{
		const ChunkWaypoint & wp = pSet->waypoint( i );

		float minX = FLT_MAX;
		float minZ = FLT_MAX;
		float maxX = -FLT_MAX;
		float maxZ = -FLT_MAX;

		for (ChunkWaypoint::Edges::const_iterator eit = wp.edges_.begin();
				eit != wp.edges_.end();
				++eit)
		{
			const Vector2 & start = pSet->vertexByIndex( eit->vertexIndex_ );
			Vector2 gf = (start - girthGridOrigin_) * girthGridResolution_;

			minX = std::min( minX, gf.x );
			minZ = std::min( minZ, gf.y );
			maxX = std::max( maxX, gf.x );
			maxZ = std::max( maxZ, gf.y );

		}

		for (int xg = int(minX); xg <= int(maxX); ++xg)
		{
			for (int zg = int(minZ); zg <= int(maxZ); ++zg)
			{
				if (uint(xg) < GIRTH_GRID_SIZE && uint(zg) < GIRTH_GRID_SIZE)
				{
					gridList[xg + zg * GIRTH_GRID_SIZE].add( 
						GirthGridElement( pSet, i ) );
				}
			}
		}
	}
}


/**
 *	This returns true if our chunk doesn't have any waypoint sets.
 *
 *	@return			True if the chunk has no waypoint sets, false otherwise.
 */
bool ChunkNavigator::isEmpty() const
{
	return wpSets_.empty();
}


/**
 *	This returns true if our chunk has any sets of the given girth.
 *
 *	@return			True if the chunk was any waypoint sets of the given
 *					girth, false otherwise.
 */
bool ChunkNavigator::hasNavPolySet( float girth ) const
{
	bool hasGirth = false;

	ChunkWaypointSets::const_iterator it;
	for (it = wpSets_.begin(); it != wpSets_.end(); ++it)
	{
		if ((*it)->girth() != girth)
		{
			continue;
		}
		hasGirth = true;
		break;
	}
	return hasGirth;
}


/**
 *	Delete the given waypoint set from our cache.
 *
 *	@param pSet		The set to delete from the cache.
 */
void ChunkNavigator::del( ChunkWaypointSet * pSet )
{
	ChunkWaypointSets::iterator found = std::find(
		wpSets_.begin(), wpSets_.end(), pSet );
	MF_ASSERT( found != wpSets_.end() )
	wpSets_.erase( found );

	// get out now if girth grids are disabled
	if (!s_useGirthGrids_)
	{
		return;
	}

	// only use grids on outside chunks
	if (!chunk_.isOutsideChunk())
	{
		return;
	}

	// find grid
	GirthGrids::iterator git;
	for (git = girthGrids_.begin(); git != girthGrids_.end(); ++git)
	{
		if (git->girth == pSet->girth())
		{
			break;
		}
	}

	if (git != girthGrids_.end())
	{
		// remove all traces of this set from it
		GirthGridList * gridLists = git->grid;
		for (uint i = 0; i < GIRTH_GRID_SIZE * GIRTH_GRID_SIZE; ++i)
		{
			GirthGridList & gridList = gridLists[i];

			for (uint j = 0; j < gridList.size(); ++j)
			{
				if (gridList.index( j ).pSet() == pSet)
				{
					gridList.eraseIndex( j );
					--j;
				}
			}
		}
	}
}



// chunk_waypoint_set.cpp
