/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef COLLISION_TERRAIN_ONLY_HPP
#define COLLISION_TERRAIN_ONLY_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "chunk/chunk_obstacle.hpp"


/**
 *  This callback is used to filter out collisions with all objects expect the
 *	terrain.
 */
class CollisionTerrainOnly : public CollisionCallback
{   
public:
    virtual int operator()(
		const ChunkObstacle& /*obstacle*/, const WorldTriangle& triangle,
		float /*dist*/ ); 

    static CollisionTerrainOnly s_default;
};


#endif // COLLISION_TERRAIN_ONLY_HPP
