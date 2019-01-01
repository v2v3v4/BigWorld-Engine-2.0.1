/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TERRAIN_COLLISION_CALLBACK_HPP
#define TERRAIN_TERRAIN_COLLISION_CALLBACK_HPP

class WorldTriangle;

namespace Terrain
{
    /**
     *  This interface provides a callback for collisions with terrain.
     */
    class TerrainCollisionCallback
    {
    public:
        /**
         *  This is the TerrainCollisionCallback destructor.
         */
        virtual ~TerrainCollisionCallback();

        /**
         *  This is the callback for terrain collisions.
         *
         *  @param triangle     The impacted triangle.
         *  @param tValue       The t-value of collision (e.g. t-value along
         *                      a line-segment or prism.
         *  @returns            True if the collision was accepted, false if
         *                      more collisions are required.
         */
        virtual bool 
        collide
        (
            WorldTriangle       const &triangle,
            float               tValue
        ) = 0;
    };
}

#endif // TERRAIN_TERRAIN_COLLISION_CALLBACK_HPP
