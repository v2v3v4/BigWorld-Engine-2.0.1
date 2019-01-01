/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_HORIZON_SHADOW_MAP
#define TERRAIN_HORIZON_SHADOW_MAP

#include "terrain_map.hpp"
#include "terrain_map_holder.hpp"

namespace Terrain
{
#pragma pack(push, 1)
    /**
     *  This represents a pixel in the horizon shadow map used by the terrain.
     */
    struct HorizonShadowPixel
    {
        uint16          east;
        uint16          west;
    };
#pragma pack(pop)

    /**
     *  This represents a HorizonShadowMap for the terrain.
     */
    class HorizonShadowMap : public TerrainMap<HorizonShadowPixel>
    {
    public:
        /**
         *  The underlying image type for HorizonShadowMaps.
         */
		typedef Moo::Image<HorizonShadowPixel>   HorizonShadowImage;

        /**
         *  This function determines the shadow at the given point.
         *
         *  @param x            The x coordinate to get the shadow at.
         *  @param z            The z coordinate to get the shadow at.
         *  @returns            The shadow at x, z.
         */ 
        virtual HorizonShadowPixel shadowAt(int32 x, int32 z) const = 0;
    };

    typedef TerrainMapIter<HorizonShadowMap>    HorizonShadowMapIter;
    typedef TerrainMapHolder<HorizonShadowMap>  HorizonShadowMapHolder;
}

#endif // TERRAIN_HORIZON_SHADOW_MAP
