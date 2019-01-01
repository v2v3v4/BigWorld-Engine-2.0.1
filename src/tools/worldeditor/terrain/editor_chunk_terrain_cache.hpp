/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_TERRAIN_CACHE_HPP
#define EDITOR_CHUNK_TERRAIN_CACHE_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"


/**
 *  This class maintains a cache of EditorChunkTerrains.
 */
class EditorChunkTerrainCache
{
public:
	static EditorChunkTerrainCache& instance();

	EditorChunkTerrainPtr findChunkFromPoint(const Vector3& pos) const;

	size_t 
    findChunksWithinBox
    (
		BoundingBox             const &bb,
		EditorChunkTerrainPtrs  &outVector 
    ) const;

private:
	EditorChunkTerrainCache();
};


#endif // EDITOR_CHUNK_TERRAIN_CACHE_HPP
