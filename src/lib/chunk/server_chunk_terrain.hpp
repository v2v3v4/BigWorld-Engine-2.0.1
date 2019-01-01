/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_TERRAIN_HPP
#define CHUNK_TERRAIN_HPP

#include "chunk_item.hpp"
#include "chunk.hpp"
#include "cstdmf/smartpointer.hpp"
#include "terrain/terrain_block_cache.hpp"


/**
 *	This class is the chunk item for a terrain block
 */
class ChunkTerrain : public ChunkItem
{
	DECLARE_CHUNK_ITEM( ChunkTerrain )

public:
	ChunkTerrain();
	~ChunkTerrain();

	virtual void	toss( Chunk * pChunk );

	Terrain::BaseTerrainBlockPtr block()	{ return pCacheEntry_->pBlock(); }
	const BoundingBox & bb()		{ return bb_; }

protected:
	ChunkTerrain( const ChunkTerrain& );
	ChunkTerrain& operator=( const ChunkTerrain& );

	Terrain::TerrainBlockCacheEntryPtr pCacheEntry_;
	BoundingBox					bb_;		// in local coords

	bool load( DataSectionPtr pSection, Chunk * pChunk );

	void			calculateBB();

	friend class ChunkTerrainCache;
};



class ChunkTerrainObstacle;

/**
 *	This class is a one-per-chunk cache of the chunk terrain item
 *	for that chunk (if it has one). It allows easy access to the
 *	terrain block given the chunk, and adds the terrain obstacle.
 */
class ChunkTerrainCache : public ChunkCache
{
public:
	ChunkTerrainCache( Chunk & chunk );
	~ChunkTerrainCache();

	virtual int focus();

	void pTerrain( ChunkTerrain * pT );

	ChunkTerrain * pTerrain()				{ return pTerrain_; }
	const ChunkTerrain * pTerrain() const	{ return pTerrain_; }

	static Instance<ChunkTerrainCache>	instance;

private:
	Chunk * pChunk_;
	ChunkTerrain * pTerrain_;
	SmartPointer<ChunkTerrainObstacle>	pObstacle_;
};





#ifdef CODE_INLINE
#include "chunk_terrain.ipp"
#endif

#endif // CHUNK_TERRAIN_HPP
