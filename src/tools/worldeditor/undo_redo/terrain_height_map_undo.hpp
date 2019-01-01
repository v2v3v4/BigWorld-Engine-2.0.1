/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_HEIGHT_MAP_UNDO_HPP
#define TERRAIN_HEIGHT_MAP_UNDO_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "gizmo/undoredo.hpp"
#include "terrain/editor_base_terrain_block.hpp"
#include "terrain/terrain_height_map.hpp"
#include "chunk/chunk.hpp"


/**
 *  This class can be used to save and restore the height map of a terrain 
 *  block.
 */
class TerrainHeightMapUndo : public UndoRedo::Operation
{
public:
    TerrainHeightMapUndo(Terrain::EditorBaseTerrainBlockPtr block, ChunkPtr chunk);

    virtual void undo();

    virtual bool iseq( const UndoRedo::Operation & oth ) const;

private:
    Terrain::EditorBaseTerrainBlockPtr  block_;
    ChunkPtr				            chunk_;
    BinaryPtr						    heightsCompressed_;
};


#endif // TERRAIN_HEIGHT_MAP_UNDO_HPP
