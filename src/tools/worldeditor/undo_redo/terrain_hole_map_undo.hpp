/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_HOLE_MAP_UNDO_HPP
#define TERRAIN_HOLE_MAP_UNDO_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "gizmo/undoredo.hpp"
#include "terrain/editor_base_terrain_block.hpp"
#include "terrain/terrain_hole_map.hpp"
#include "chunk/chunk.hpp"


/**
 *  This class can be used to save and restore the hole map of a terrain 
 *  block.
 */
class TerrainHoleMapUndo : public UndoRedo::Operation
{
public:
    TerrainHoleMapUndo(Terrain::EditorBaseTerrainBlockPtr block, ChunkPtr chunk);

    virtual void undo();

    virtual bool iseq( const UndoRedo::Operation & oth ) const;

private:
    Terrain::EditorBaseTerrainBlockPtr  block_;
    ChunkPtr				            chunk_;
    Terrain::TerrainHoleMap::ImageType  holes_;
};


#endif // TERRAIN_HOLE_MAP_UNDO_HPP
