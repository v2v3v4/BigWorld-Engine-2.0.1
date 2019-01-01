/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef STATION_NODE_LINK_LOCATOR_HPP
#define STATION_NODE_LINK_LOCATOR_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "gizmo/tool_locator.hpp"


class StationNodeLinkLocator : public ToolLocator
{
public:
    enum Type
    {
        LOCATE_NODES    = 1,
        LOCATE_ENTITIES = 2,
        LOCATE_BOTH     = LOCATE_NODES | LOCATE_ENTITIES
    };

    explicit StationNodeLinkLocator(Type type = LOCATE_BOTH);

    /*virtual*/ ~StationNodeLinkLocator();

    /*virtual*/ void calculatePosition(Vector3 const &worldRay, Tool &tool);

    ChunkItemPtr chunkItem();

private:
    StationNodeLinkLocator(StationNodeLinkLocator const &);
    StationNodeLinkLocator &operator=(StationNodeLinkLocator const &);

private:
    ChunkItemPtr        chunkItem_;
    bool                initialPos_;
    Vector3             lastLocatorPos_;
    Vector3             totalLocatorOffset_;
    ToolLocatorPtr      subLocator_;
    Type                type_;
};


#endif // STATION_NODE_LINK_LOCATOR_HPP
