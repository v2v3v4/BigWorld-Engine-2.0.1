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
#include "worldeditor/collisions/closest_obstacle_no_edit_stations.hpp"


ClosestObstacleNoEditStations ClosestObstacleNoEditStations::s_default;


/*virtual*/ int ClosestObstacleNoEditStations::operator()
( 
    ChunkObstacle   const &obstacle,
    WorldTriangle   const &/*triangle*/, 
    float           /*dist*/ 
)
{
	BW_GUARD;

    if 
    (
        obstacle.pItem()->isEditorChunkStationNode()
        ||
        obstacle.pItem()->isEditorChunkLink()
		||
		obstacle.pItem()->isEditorUserDataObject()
		||
		obstacle.pItem()->isEditorEntity()
    )
    {
        return COLLIDE_ALL;
    }
    else
    {
        return COLLIDE_BEFORE;
    }
}   
