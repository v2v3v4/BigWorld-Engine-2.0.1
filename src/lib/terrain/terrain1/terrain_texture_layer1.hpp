/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TERRAIN_TEXTURE_LAYER1_HPP
#define TERRAIN_TERRAIN_TEXTURE_LAYER1_HPP

#include "../terrain_texture_layer.hpp"


namespace Terrain
{
	class CommonTerrainBlock1;

	/**
     *  This implements the TerrainTextureLayer interface for the second
     *  generation terrain.
     */
    class TerrainTextureLayer1 : public TerrainTextureLayer
    {
    public:
        TerrainTextureLayer1(CommonTerrainBlock1 &terrainBlock, uint32 index);
		virtual ~TerrainTextureLayer1();

        std::string textureName() const;
        bool textureName(std::string const &filename);

#ifndef MF_SERVER
		Moo::BaseTexturePtr texture() const;
		void texture(Moo::BaseTexturePtr texture, std::string const &textureName);  
#endif

        bool hasUVProjections() const;
        Vector4 const &uProjection() const; 
        void uProjection(Vector4 const &u);
        Vector4 const &vProjection() const;
        void vProjection(Vector4 const &v);

        uint32 width() const;
        uint32 height() const;

#ifdef EDITOR_ENABLED
        bool lock(bool readOnly);
		ImageType &image();
		bool unlock();
		bool save(DataSectionPtr) const;
#endif
        ImageType const &image() const;
        bool load(DataSectionPtr dataSection, std::string *error = NULL);

	    BaseTerrainBlock &baseTerrainBlock();
        BaseTerrainBlock const &baseTerrainBlock() const;	

    private:
        CommonTerrainBlock1*			terrainBlock_;
	    std::string				        textureName_;
        TerrainTextureLayer1::ImageType blends_;
		uint32							width_;
		uint32							height_;
	    Vector4					        uProjection_;
	    Vector4					        vProjection_;
		uint32                          index_;

#ifndef MF_SERVER
		Moo::BaseTexturePtr			    pTexture_;
#endif
    };

    typedef SmartPointer<TerrainTextureLayer1>  TerrainTextureLayer1Ptr;
}


#endif // TERRAIN_TERRAIN_TEXTURE_LAYER1_HPP
