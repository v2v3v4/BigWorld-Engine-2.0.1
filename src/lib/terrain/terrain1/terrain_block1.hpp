/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TERRAIN_BLOCK1_HPP
#define TERRAIN_TERRAIN_BLOCK1_HPP

#include "common_terrain_block1.hpp"
#ifndef MF_SERVER
#include "moo/index_buffer.hpp"
#include "moo/vertex_buffer.hpp"
#ifdef EDITOR_ENABLED
#include "../editor_base_terrain_block.hpp"
#else
#include "../base_terrain_block.hpp"
#endif // EDITOR_ENABLED
#include "terrain_texture_setter.hpp"

namespace Moo
{
    class LightContainer;	
    typedef SmartPointer<LightContainer>		LightContainerPtr;
}

namespace Terrain
{
	class TerrainTextureSetter;
	class HorizonShadowMap1;
	typedef SmartPointer<HorizonShadowMap1>     HorizonShadowMap1Ptr;

	class TerrainBlock1 : public CommonTerrainBlock1, public Moo::DeviceCallback
	{
	public:

        TerrainBlock1();
        virtual ~TerrainBlock1();

        HorizonShadowMap &shadowMap();
        HorizonShadowMap const &shadowMap() const;

		bool set(	const Vector3 & blockPosition, 
					const Vector3 & cameraPosition );
		virtual bool draw( Moo::EffectMaterialPtr );
		virtual void draw( TerrainTextureSetter* tts = NULL );

		virtual void rebuildCombinedLayers(
			bool compressTextures = true, bool generateDominantTextureMap = true );

		// this method has to be public to be accessed from the terrain renderer
        void cacheCurrentLighting( bool addSpecular = true );

		virtual void createUMBRAMesh( UMBRAMesh& umbraMesh ) const;

	protected:
		virtual bool postLoad( std::string const &filename, 
            Matrix          const &worldTransform,
            Vector3         const &cameraPosition,
            DataSectionPtr  pTerrain,
            TextureLayers   &textureLayers,
            std::string     *error );

        TextureLayers &textureLayers();
        TextureLayers const &textureLayers() const;

		uint32 packedBlends( uint32 x, uint32 y );

		bool managedObjectsCreated() const;
		void createManagedObjects();
		void deleteManagedObjects();

	private:
	    TextureLayers			    textureLayers_;
        HorizonShadowMap1Ptr        pHorizonMap_;
		Moo::LightContainerPtr		diffLights_;
		Moo::LightContainerPtr		specLights_;
		
		Moo::VertexBuffer			vertexBuffer_;
		Moo::IndexBuffer			indexBuffer_;

		uint32						nVertices_;
		uint32						nIndices_;
		uint32						nPrimitives_;

	};
}
#endif // MF_SERVER

#endif // TERRAIN_TERRAIN_BLOCK1_HPP
