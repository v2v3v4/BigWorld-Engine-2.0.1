/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_HORIZON_SHADOW_MAP1_HPP
#define TERRAIN_HORIZON_SHADOW_MAP1_HPP

#include "../horizon_shadow_map.hpp"

namespace Terrain
{
    class TerrainBlock1;
}

namespace Terrain
{
    /**
     *  This class implements the HorizonShadowMap interface for the second
     *  generation of terrain.
     */
    class HorizonShadowMap1 : public HorizonShadowMap
    {
    public:
        explicit HorizonShadowMap1(TerrainBlock1 &block);

        HorizonShadowPixel shadowAt(int32 x, int32 z) const;

        uint32 width() const;
        uint32 height() const;

        bool lock(bool readOnly);

        ImageType &image();
        ImageType const &image() const;

        bool unlock();

        bool save( DataSectionPtr ) const;
        bool load(DataSectionPtr dataSection, std::string *error = NULL);

        BaseTerrainBlock &baseTerrainBlock();
        BaseTerrainBlock const &baseTerrainBlock() const;

    private:
        TerrainBlock1               *block_;
        HorizonShadowImage          image_;
		bool                        readOnly_;
    };

    typedef SmartPointer<HorizonShadowMap1>     HorizonShadowMap1Ptr;
}

#endif // TERRAIN_HORIZON_SHADOW_MAP1_HPP
