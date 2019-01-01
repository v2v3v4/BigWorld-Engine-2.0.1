/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ELEVATION_UNDO_HPP
#define ELEVATION_UNDO_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "gizmo/undoredo.hpp"
#include "terrain/terrain_height_map.hpp"
#include <list>


/**
 *  Used to store grid positions and height map data.
 */
struct ElevationUndoPos
{
    int16									x_;
    int16									y_;
    Terrain::TerrainHeightMap::ImageType    data_;

    ElevationUndoPos(int16 x, int16 y);
    ElevationUndoPos(ElevationUndoPos const &other);
    ~ElevationUndoPos();
    ElevationUndoPos &operator=(ElevationUndoPos const &other);
};


/**
 *  This can be used to undo/redo elevation changes to blocks.  You create a 
 *  list of ElevationUndoPos where you set the x_ and y_ coordinates and this
 *  will restore the height positions of the given chunks.
 */
class ElevationUndo : public UndoRedo::Operation
{
public:
    typedef std::list<ElevationUndoPos>  ElevationUndoPosList;

    explicit ElevationUndo(ElevationUndoPosList const &positions);

    /*virtual*/ void undo();

    /*virtual*/ bool iseq(UndoRedo::Operation const &other) const;

private:
    ElevationUndoPosList    positions_;
};


#endif // ELEVATION_UNDO_HPP
