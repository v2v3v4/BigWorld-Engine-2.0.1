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
#include "cstdmf/guard.hpp"
#include "terrain/base_render_terrain_block.hpp"

#if UMBRA_ENABLE
#include "umbra_proxies.hpp"
#endif

#ifndef MF_SERVER
#include "fmodsound/fmod_config.hpp"
#include "fmodsound/sound_occluder.hpp"
#endif

namespace Terrain
{
	class   BaseTerrainBlock;
	typedef SmartPointer<BaseTerrainBlock> BaseTerrainBlockPtr;
}


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
	virtual void	draw();

	virtual uint32 typeFlags() const;

	Terrain::BaseTerrainBlockPtr block()				{ return block_; }
    const Terrain::BaseTerrainBlockPtr block() const	{ return block_; }
	const BoundingBox & bb() const						{ return bb_; }

    void calculateBB();

    static bool outsideChunkIDToGrid( const std::string& chunkID, 
											int32& x, int32& z );

#if EDITOR_ENABLED
	static Terrain::BaseTerrainBlockPtr loadTerrainBlockFromChunk(
			 Chunk * pChunk );
#endif

	bool doingBackgroundTask() const;

#if UMBRA_ENABLE
	void disableOccluder();
	void enableOccluder();
#endif
	virtual void syncInit();

	virtual bool reflectionVisible() { return true; }

protected:
	friend class ChunkTerrainCache;

	bool load( DataSectionPtr pSection, Chunk * pChunk, std::string* errorString = NULL );
	virtual bool addYBounds( BoundingBox& bb ) const;
	
	ChunkTerrain( const ChunkTerrain& );
	ChunkTerrain& operator=( const ChunkTerrain& );

	Terrain::BaseTerrainBlockPtr    block_;
	BoundingBox						bb_;		// in local coords


#if UMBRA_ENABLE
	Terrain::BaseRenderTerrainBlock::UMBRAMesh	umbraMesh_;
	bool										umbraHasHoles_;

	UmbraModelProxyPtr	pUmbraWriteModel_;
#endif

#if FMOD_SUPPORT
	SoundOccluder soundOccluder_;
#endif
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
