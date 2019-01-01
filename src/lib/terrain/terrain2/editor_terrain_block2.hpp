/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_EDITOR_TERRAIN_BLOCK2_HPP
#define TERRAIN_EDITOR_TERRAIN_BLOCK2_HPP



#ifdef EDITOR_ENABLED

#include "terrain_block2.hpp"
#include "terrain_height_map2.hpp"

#define TERRAINBLOCK2 EditorTerrainBlock2

namespace Terrain
{
	class ETB2UnlockCallback;

    class EditorTerrainBlock2 : public TerrainBlock2
    {
    public:
        EditorTerrainBlock2(TerrainSettingsPtr pSettings);
		virtual ~EditorTerrainBlock2();

		bool create( DataSectionPtr settings, std::string* error = NULL );

        bool load(	std::string const &filename, 
					Matrix  const &worldTransform,
					Vector3 const &cameraPosition,
					std::string *error = NULL);

		bool saveLodTexture( DataSectionPtr dataSection ) const;
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

        virtual bool draw( Moo::EffectMaterialPtr pMaterial );

		void drawIgnoringHoles(bool setTextures = true) {};
		
		void setHeightMapDirty();

		void rebuildCombinedLayers(
			bool compressTextures = true, 
			bool generateDominantTextureMap = true );

		void rebuildNormalMap( NormalMapQuality normalMapQuality );
		bool rebuildLodTexture( const Matrix& transform  );
		bool isLodTextureDirty() const { return lodMapDirty_; };
		void setLodTextureDirty( bool state );

		virtual void stream();

		bool canDrawLodTexture() const;

		static uint32 blendBuildInterval();
		static void blendBuildInterval( uint32 interval );

		static void nextBlendBuildMark();

	private:
		void streamBlends();
		void ensureBlendsLoaded() const;
		bool saveHeightmap( DataSectionPtr dataSection ) const;

		bool				lodMapDirty_;
		ETB2UnlockCallback* unlockCallback_;

		static uint32 s_nextBlendBuildMark_;

		static uint32 s_blendBuildInterval_;
    };

	/**
	* This is a callback class that allows the EditorTerrainBlock2 to be notified
	* when the height map changes.
	*/
	class ETB2UnlockCallback : public TerrainHeightMap2::UnlockCallback
	{
	public:
		explicit ETB2UnlockCallback( EditorTerrainBlock2& owner )
			: owner_( &owner )
		{}

		virtual bool notify()
		{
			owner_->setHeightMapDirty();
			return true;
		}

	private:
		EditorTerrainBlock2* owner_;
	};

}

#else

#ifdef MF_SERVER
#define TERRAINBLOCK2 CommonTerrainBlock2
#else
#define TERRAINBLOCK2 TerrainBlock2
#endif

#endif // EDITOR_ENABLED

#endif // TERRAIN_EDITOR_TERRAIN_BLOCK2_HPP
