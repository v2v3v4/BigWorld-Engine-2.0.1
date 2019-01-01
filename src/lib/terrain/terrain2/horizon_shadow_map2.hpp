/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_HORIZON_SHADOW_MAP2_HPP
#define TERRAIN_HORIZON_SHADOW_MAP2_HPP

#include "../horizon_shadow_map.hpp"

namespace Terrain
{
    class TerrainBlock2;
}

namespace Terrain
{
    /**
     *  This class implements the HorizonShadowMap interface for the second
     *  generation of terrain.
     */
    class HorizonShadowMap2 : public HorizonShadowMap
    {
    public:
        explicit HorizonShadowMap2(TerrainBlock2 &block);

#ifdef EDITOR_ENABLED
		bool create( uint32 size, std::string *error = NULL );
#endif

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

        DX::Texture *texture() const;

    private:
        TerrainBlock2               *block_;
        ComObjectWrap<DX::Texture>  texture_;
        uint32                      width_;
        uint32                      height_;
        HorizonShadowImage          image_;
        size_t                      lockCount_;
    };

    typedef SmartPointer<HorizonShadowMap2>     HorizonShadowMap2Ptr;
}

#endif // TERRAIN_HORIZON_SHADOW_MAP2_HPP
