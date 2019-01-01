/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TERRAIN_HOLE_MAP1_HPP
#define TERRAIN_TERRAIN_HOLE_MAP1_HPP

#include "../terrain_hole_map.hpp"

namespace Terrain
{
    class CommonTerrainBlock1;
}

namespace Terrain
{
    /**
     *  This is an implementation for TerrainHoleMap for the second generation
     *  of terrain.
     */
    class TerrainHoleMap1 : public TerrainHoleMap
    {
    public:
        explicit TerrainHoleMap1( CommonTerrainBlock1 &block );

        ~TerrainHoleMap1();

        bool noHoles() const;
        bool allHoles() const;

        uint32 width() const;
        uint32 height() const;

        bool lock( bool readOnly );

        ImageType &image();    
        ImageType const &image() const;

        bool unlock();

        bool save(DataSectionPtr dataSection) const;
        bool load(DataSectionPtr dataSection, std::string *error = NULL);

        BaseTerrainBlock &baseTerrainBlock();
        BaseTerrainBlock const &baseTerrainBlock() const;

        uint32 holesSize() const;

		bool holeAt( float x, float z ) const;

		bool holeAt( float xs, float zs, float xe, float ze ) const;
    protected:
        void recalcHoles();

    private:
        CommonTerrainBlock1         *block_;
		Moo::Image<bool>            image_;
        bool                        readOnly_;
        bool                        allHoles_;
        bool                        noHoles_;
        uint32                      holesSize_;
    };

    typedef TerrainMapIter<TerrainHoleMap1>     TerrainHoleMap1Iter;
    typedef TerrainMapHolder<TerrainHoleMap1>   TerrainHoleMap1Holder;
}

#endif // TERRAIN_TERRAIN_HOLE_MAP1_HPP
