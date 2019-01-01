/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_ALIASED_HEIGHT_MAP_HPP
#define TERRAIN_ALIASED_HEIGHT_MAP_HPP

namespace Terrain
{
    class TerrainCollisionCallback;
    class TerrainHeightMap2;
    typedef SmartPointer<TerrainHeightMap2> TerrainHeightMap2Ptr;
}

namespace Terrain
{
    /**
     *	This class implements an aliased heightmap that allows sampling of a 
     *  lower resolution of a parent TerrainHeightMap2.
     */
    class AliasedHeightMap 
    {
    public:
        AliasedHeightMap(uint32 level, TerrainHeightMap2Ptr pParent);

        float height( uint32 x, uint32 z ) const;

    private:
        uint32                  level_;
	    TerrainHeightMap2Ptr	pParent_;        
    };

    typedef SmartPointer<AliasedHeightMap> AliasedHeightMapPtr;
}

#endif // TERRAIN_ALIASED_HEIGHT_MAP_HPP
