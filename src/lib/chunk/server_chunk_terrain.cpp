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

#include "chunk_terrain_common.hpp"
#include "geometry_mapping.hpp"
#include "server_chunk_terrain.hpp"

#include "cstdmf/smartpointer.hpp"

#include "terrain/base_terrain_block.hpp"
#include "terrain/terrain_settings.hpp"

// TODO: This shouldn't know which type of TerrainBlock it is loading.
#include "terrain/terrain2/terrain_block2.hpp"

#include <map>


namespace Terrain
{
	class BaseTerrainBlock;
	typedef SmartPointer< BaseTerrainBlock > BaseTerrainBlockPtr;
}

#include "chunk.hpp"
#include "chunk_space.hpp"
#include "chunk_obstacle.hpp"

#include "grid_traversal.hpp"

#include "physics2/hulltree.hpp"
#include "resmgr/datasection.hpp"

#ifndef CODE_INLINE
#include "chunk_terrain.ipp"
#endif

int ChunkTerrain_token;


DECLARE_DEBUG_COMPONENT2( "Chunk", 0 )

// -----------------------------------------------------------------------------
// Section: ChunkTerrain
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
ChunkTerrain::ChunkTerrain() :
	ChunkItem( WANTS_DRAW ),
	bb_( Vector3::zero(), Vector3::zero() )
{
}


/**
 *	Destructor.
 */
ChunkTerrain::~ChunkTerrain()
{
}

/**
 *	This method loads this terrain block.
 */
bool ChunkTerrain::load( DataSectionPtr pSection, Chunk * pChunk )
{
	std::string res = pSection->readString( "resource" );

	// Allocate the terrainblock.
	pCacheEntry_ = Terrain::TerrainBlockCache::instance().findOrLoad(
			pChunk->mapping()->path() + res, pChunk->space()->terrainSettings() );
	if (!pCacheEntry_)
	{
		ERROR_MSG( "Could not load terrain block '%s'\n", res.c_str() );
		return false;
	}

	this->calculateBB();

	return true;
}


/**
 *	This method calculates the block's bounding box, and sets into bb_.
 */
void ChunkTerrain::calculateBB()
{
	// TODO: Share with the implementation in chunk_terrain.cpp
	MF_ASSERT( pCacheEntry_ && pCacheEntry_->pBlock() );

    bb_ = pCacheEntry_->pBlock()->boundingBox();

	// regenerate the collision scene since our bb is different now
	if (pChunk_ != NULL)
	{
		BoundingBox localBB = pChunk_->localBB();
		if( addYBounds( localBB ) )
		{
			pChunk_->localBB( localBB );
			localBB.transformBy( pChunk_->transform() );
			pChunk_->boundingBox( localBB );
		}
		//this->toss( pChunk_ );
		// too slow for now
	}
}


/**
 *	Called when we are put in or taken out of a chunk
 */
void ChunkTerrain::toss( Chunk * pChunk )
{
	if (pChunk_ != NULL)
	{
		ChunkTerrainCache::instance( *pChunk_ ).pTerrain( NULL );
	}

	this->ChunkItem::toss( pChunk );

	if (pChunk_ != NULL)
	{
		ChunkTerrainCache::instance( *pChunk ).pTerrain( this );
	}
}

#undef IMPLEMENT_CHUNK_ITEM_ARGS
#define IMPLEMENT_CHUNK_ITEM_ARGS (pSection, pChunk)
IMPLEMENT_CHUNK_ITEM( ChunkTerrain, terrain, 0 )

#include "chunk_terrain_obstacle.hpp"


/**
 *	Constructor
 */
ChunkTerrainCache::ChunkTerrainCache( Chunk & chunk ) :
	pChunk_( &chunk ),
	pTerrain_( NULL ),
	pObstacle_( NULL )
{
}

/**
 *	Destructor
 */
ChunkTerrainCache::~ChunkTerrainCache()
{
}


/**
 *	This method is called when our chunk is focussed. We add ourselves to the
 *	chunk space's obstacles at that point.
 */
int ChunkTerrainCache::focus()
{
	if (!pTerrain_ || !pObstacle_) return 0;

	const Vector3 & mb = pObstacle_->bb_.minBounds();
	const Vector3 & Mb = pObstacle_->bb_.maxBounds();

/*
	// figure out the border
	HullBorder	border;
	for (int i = 0; i < 6; i++)
	{
		// calculate the normal and d of this plane of the bb
		int ax = i >> 1;

		Vector3 normal( 0, 0, 0 );
		normal[ ax ] = (i&1) ? -1.f : 1.f;;

		float d = (i&1) ? -Mb[ ax ] : mb[ ax ];

		// now apply the transform to the plane
		Vector3 ndtr = pObstacle_->transform_.applyPoint( normal * d );
		Vector3 ntr = pObstacle_->transform_.applyVector( normal );
		border.push_back( PlaneEq( ntr, ntr.dotProduct( ndtr ) ) );
	}
*/

	// we assume that we'll be in only one column
	Vector3 midPt = pObstacle_->transform_.applyPoint( (mb + Mb) / 2.f );

	ChunkSpace::Column * pCol = pChunk_->space()->column( midPt );
	MF_ASSERT( pCol );

	// ok, just add the obstacle then
	pCol->addObstacle( *pObstacle_ );

	//dprintf( "ChunkTerrainCache::focus: "
	//	"Adding hull of terrain (%f,%f,%f)-(%f,%f,%f)\n",
	//	mb[0],mb[1],mb[2], Mb[0],Mb[1],Mb[2] );

	// which counts for just one
	return 1;
}


/**
 *	This method sets the terrain pointer
 */
void ChunkTerrainCache::pTerrain( ChunkTerrain * pT )
{
	if (pT != pTerrain_)
	{
		if (pObstacle_)
		{
			// flag column as stale first
			const Vector3 & mb = pObstacle_->bb_.minBounds();
			const Vector3 & Mb = pObstacle_->bb_.maxBounds();
			Vector3 midPt = pObstacle_->transform_.applyPoint( (mb + Mb) / 2.f );
			ChunkSpace::Column * pCol = pChunk_->space()->column( midPt, false );
			if (pCol != NULL) pCol->stale();

			pObstacle_ = NULL;
		}

		pTerrain_ = pT;

		if (pTerrain_ != NULL)
		{
			pObstacle_ = new ChunkTerrainObstacle( *pTerrain_->block(),
				pChunk_->transform(), &pTerrain_->bb_, pT );

			if (pChunk_->focussed()) this->focus();
		}
	}
}


// -----------------------------------------------------------------------------
// Section: Static initialisers
// -----------------------------------------------------------------------------

/// Static cache instance accessor initialiser
ChunkCache::Instance<ChunkTerrainCache> ChunkTerrainCache::instance;



// -----------------------------------------------------------------------------
// Section: TerrainFinder
// -----------------------------------------------------------------------------

#ifndef MF_SERVER
#include "chunk_manager.hpp"
#else
ChunkSpace * g_pMainSpace = NULL;
#endif

/**
 *	This class implements the TerrainFinder interface. Its purpose is to be an
 *	object that Moo can use to access the terrain. It is implemented like this
 *	so that other libraries do not need to know about the Chunk library.
 */
class TerrainFinderInstance : public Terrain::TerrainFinder
{
public:
	TerrainFinderInstance()
	{
		Terrain::BaseTerrainBlock::setTerrainFinder( *this );
	}

	virtual Terrain::TerrainFinder::Details findOutsideBlock( const Vector3 & pos )
	{
		Terrain::TerrainFinder::Details details;

#ifdef MF_SERVER
		ChunkSpace * pSpace = g_pMainSpace;
#else
		// TODO: At the moment, assuming space 0.
		ChunkSpace * pSpace = ChunkManager::instance().pMainSpace();
#endif

		//find the chunk
		if (pSpace != NULL)
		{
			ChunkSpace::Column* pColumn = pSpace->column( pos, false );

			if ( pColumn != NULL )
			{
				Chunk * pChunk = pColumn->pOutsideChunk();

				if (pChunk != NULL)
				{
					//find the terrain block
					ChunkTerrain * pChunkTerrain =
						ChunkTerrainCache::instance( *pChunk ).pTerrain();

					if (pChunkTerrain != NULL)
					{
						details.pBlock_ = pChunkTerrain->block().getObject();
						details.pInvMatrix_ = &pChunk->transformInverse();
						details.pMatrix_ = &pChunk->transform();
					}
				}
			}
		}

		return details;
	}
};

static TerrainFinderInstance s_terrainFinder;

// server_chunk_terrain.cpp
