/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TERRAIN_TEXTURE_LAYER2_HPP
#define TERRAIN_TERRAIN_TEXTURE_LAYER2_HPP

#include "../terrain_texture_layer.hpp"
#include "moo/image.hpp"

namespace Terrain
{
    class CommonTerrainBlock2;
}

namespace Terrain
{
    /**
     *  This implements the TerrainTextureLayer interface for the second
     *  generation terrain.
     */
    class TerrainTextureLayer2 : public TerrainTextureLayer
    {
    public:
        explicit TerrainTextureLayer2(
			CommonTerrainBlock2 &terrainBlock, uint32 blendWidth = 0, uint32 blendHeight = 0 );
		virtual ~TerrainTextureLayer2();

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

		void onLoaded();
		
	    BaseTerrainBlock &baseTerrainBlock();
        BaseTerrainBlock const &baseTerrainBlock() const;	

	protected:
#ifdef EDITOR_ENABLED
		enum State
		{
			LOADING			= 1,		// Still loading
			COMPRESSED		= 2,		// Loaded, compressedBlend_ has blends
			BLENDS			= 4			// Loaded, blends_ has blends
		};

		void compressBlend();
		void decompressBlend();
		State state();

		friend class TTL2Cache;
#endif

    private:
        CommonTerrainBlock2             *terrainBlock_;
	    std::string				        textureName_;
        TerrainTextureLayer2::ImageType blends_;
		uint32							width_;
		uint32							height_;
	    Vector4					        uProjection_;
	    Vector4					        vProjection_;
        size_t                          lockCount_;

#ifndef MF_SERVER
		Moo::BaseTexturePtr				pTexture_;
#endif

#ifdef EDITOR_ENABLED
		State							state_;
		BinaryPtr						compressedBlend_;
#endif
    };

    typedef SmartPointer<TerrainTextureLayer2>  TerrainTextureLayer2Ptr;
}


#endif // TERRAIN_TERRAIN_TEXTURE_LAYER2_HPP
