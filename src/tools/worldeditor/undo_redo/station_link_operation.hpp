/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef STATION_LINK_OPERATION_HPP
#define STATION_LINK_OPERATION_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/world/items/editor_chunk_station.hpp"
#include "gizmo/undoredo.hpp"


class StationLinkOperation : public UndoRedo::Operation
{
public:
	StationLinkOperation
    ( 
        EditorChunkStationNodePtr           a,
        EditorChunkStationNodePtr           b,
        ChunkLink::Direction                dir 
    );

	/*virtual*/ void undo();

	/*virtual*/ bool iseq( const UndoRedo::Operation & oth ) const;

protected:
    EditorChunkStationNodePtr getNodeA() const;

    EditorChunkStationNodePtr getNodeB() const;

private:
    UniqueID                            idA;
    UniqueID                            idGraphA;
    UniqueID                            idB;
    UniqueID                            idGraphB;
	ChunkLink::Direction                dir_;
};


#endif // STATION_LINK_OPERATION_HPP
