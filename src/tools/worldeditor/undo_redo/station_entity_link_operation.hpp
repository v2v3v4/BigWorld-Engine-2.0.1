/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef STATION_ENTITY_LINK_OPERATION_HPP
#define STATION_ENTITY_LINK_OPERATION_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/world/items/editor_chunk_station.hpp"
#include "worldeditor/world/items/editor_chunk_entity.hpp"
#include "gizmo/undoredo.hpp"


/**
 *  This class saves the station-node link information for an entity with this
 *  property and undoes changes to this link information.
 */
class StationEntityLinkOperation : public UndoRedo::Operation
{
public:
    explicit StationEntityLinkOperation
    (
        EditorChunkEntityPtr        entity
    );

	/*virtual*/ void undo();

    /*virtual*/ bool iseq(UndoRedo::Operation const &other) const;

private:
    StationEntityLinkOperation(StationEntityLinkOperation const &);
    StationEntityLinkOperation &operator=(StationEntityLinkOperation const &);

protected:
    std::string                 entityNodeID_;
    std::string                 entityGraphID_;
    EditorChunkEntityPtr        entity_;
};


#endif // STATION_ENTITY_LINK_OPERATION_HPP
