/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_EDITOR_TERRAIN_BLOCK1_HPP
#define TERRAIN_EDITOR_TERRAIN_BLOCK1_HPP

#ifdef EDITOR_ENABLED

#include "terrain_block1.hpp"

#define TERRAINBLOCK1 EditorTerrainBlock1

namespace Terrain
{
    class EditorTerrainBlock1 : public TerrainBlock1
    {
    public:
        EditorTerrainBlock1();

        bool load(	std::string const &filename, 
					Matrix  const &worldTransform,
					Vector3 const &cameraPosition,
					std::string *error = NULL );

        bool save(DataSectionPtr dataSection) const;
        bool saveToCData(DataSectionPtr pCDataSection) const;
        bool save(std::string const &filename) const;

        size_t numberTextureLayers() const;
        size_t maxNumberTextureLayers() const;

        size_t insertTextureLayer(
			uint32 blendWidth = 0, uint32 blendHeight = 0 );
        bool removeTextureLayer(size_t idx);
		TerrainTextureLayer &textureLayer(size_t idx);
        TerrainTextureLayer const &textureLayer(size_t idx) const;

        void draw(TerrainTextureSetter *tts);
		bool draw( Moo::EffectMaterialPtr pMaterial );
        void drawIgnoringHoles(bool setTextures = true);

		void setHeightMapDirty();

		void rebuildNormalMap( NormalMapQuality normalMapQuality );
		bool rebuildLodTexture( const Matrix& transform  ) { return true;}
		void setLodTextureDirty( bool state ) {}

	protected:
		bool postLoad(std::string const &filename, 
				Matrix          const &worldTransform,
				Vector3         const &cameraPosition,
				DataSectionPtr  pTerrain,
				TextureLayers   &textureLayers,
				std::string     *error = NULL );

	private:
		size_t realTextureLayerIndex( size_t idx ) const;
		bool textureLayerEmpty( size_t realIdx ) const;
		void recalcLayerStates();
		uint32 size() const;

		std::vector<bool> isLayerEmpty_;
    };
}

#else

#ifdef MF_SERVER
#define TERRAINBLOCK1 CommonTerrainBlock1
#else
#define TERRAINBLOCK1 TerrainBlock1
#endif

#endif // EDITOR_ENABLED

#endif // TERRAIN_EDITOR_TERRAIN_BLOCK1_HPP
