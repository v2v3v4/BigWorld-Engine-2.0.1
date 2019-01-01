/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TEX_PROJ_UNDO_HPP
#define TERRAIN_TEX_PROJ_UNDO_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "gizmo/undoredo.hpp"
#include "worldeditor/terrain/editor_chunk_terrain.hpp"


/**
 *	This class is used to save/restore the projection of terrain texture 
 *	layer.
 */
class TerrainTexProjUndo : public UndoRedo::Operation
{
public:
	TerrainTexProjUndo
	(
		EditorChunkTerrainPtr			terrain,
		size_t							layerIdx
	);

    virtual void undo();

    virtual bool iseq(const UndoRedo::Operation &oth) const;

private:
	EditorChunkTerrainPtr				terrain_;
	size_t								layerIdx_;
	Vector4								uProj_;
	Vector4								vProj_;
};


#endif // TERRAIN_TEX_PROJ_UNDO_HPP
