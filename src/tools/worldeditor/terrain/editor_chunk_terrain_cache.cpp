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
#include "worldeditor/terrain/editor_chunk_terrain_cache.hpp"
#include "worldeditor/terrain/editor_chunk_terrain.hpp"
#include "chunk/base_chunk_space.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_terrain.hpp"
#include "math/boundbox.hpp"


DECLARE_DEBUG_COMPONENT2( "WorldEditor", 2 )


EditorChunkTerrainCache& EditorChunkTerrainCache::instance()
{
	static EditorChunkTerrainCache s_cache;
	return s_cache;
}


EditorChunkTerrainPtr 
EditorChunkTerrainCache::findChunkFromPoint(const Vector3 &pos) const
{
	BW_GUARD;

	ChunkSpacePtr chunkSpace = ChunkManager::instance().cameraSpace();
	if (chunkSpace == NULL)
		return NULL;

	Chunk *chunk = chunkSpace->findChunkFromPoint(pos);
	if (chunk == NULL)
		return NULL;

	EditorChunkTerrain *ect = 
		static_cast<EditorChunkTerrain *>
		( 
			ChunkTerrainCache::instance(*chunk).pTerrain() 
		);
	return ect;
}


size_t 
EditorChunkTerrainCache::findChunksWithinBox
(
	BoundingBox             const &bb,
	EditorChunkTerrainPtrs  &outVector 
) const
{
	BW_GUARD;

	int32 startX = (int32) floorf(bb.minBounds().x/GRID_RESOLUTION);
	int32 endX   = (int32) floorf(bb.maxBounds().x/GRID_RESOLUTION);

	int32 startZ = (int32) floorf(bb.minBounds().z/GRID_RESOLUTION);
	int32 endZ   = (int32) floorf(bb.maxBounds().z/GRID_RESOLUTION);

	for ( int32 x = startX; x <= endX; ++x )
	{
		for ( int32 z = startZ; z <= endZ; ++z )
		{
			if ( ChunkManager::instance().cameraSpace() )
			{
				ChunkSpace::Column* pColumn =
					ChunkManager::instance().cameraSpace()->column
                    ( 
                        Vector3
                        ( 
                            x * GRID_RESOLUTION + 0.5f*GRID_RESOLUTION, 
                            bb.minBounds().y, 
                            z * GRID_RESOLUTION + 0.5f*GRID_RESOLUTION 
                        ), 
                        false 
                    );

				if ( pColumn )
				{
					if ( pColumn->pOutsideChunk() )
					{
                        EditorChunkTerrain *ect = 
                            static_cast<EditorChunkTerrain *>
                            ( 
                                ChunkTerrainCache::instance
                                ( 
                                    *pColumn->pOutsideChunk() 
                                ).pTerrain() 
                             );
						outVector.push_back( ect );
					}
				}
			}
		}
	}
	return outVector.size();
}


EditorChunkTerrainCache::EditorChunkTerrainCache()
{
}
