/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TERRAIN_BLOCK2_HPP
#define TERRAIN_TERRAIN_BLOCK2_HPP

#include "common_terrain_block2.hpp"
#include "terrain_blends.hpp"
#ifdef EDITOR_ENABLED
#include "../editor_base_terrain_block.hpp"
#include "editor_vertex_lod_manager.hpp"
#else
#include "../base_terrain_block.hpp"
#include "vertex_lod_manager.hpp"
#endif // EDITOR_ENABLED

namespace Moo
{
	class LightContainer;
	typedef SmartPointer<LightContainer>		LightContainerPtr;
}

namespace Terrain
{
#ifdef EDITOR_ENABLED
	typedef EditorVertexLodManager		VerticesResource;
#else
	typedef VertexLodManager			VerticesResource;
#endif // EDITOR_ENABLED

    class HorizonShadowMap2;
	class TerrainNormalMap2;
	class TerrainLodMap2;
	class HeightMapResource;

    typedef SmartPointer<HorizonShadowMap2>     HorizonShadowMap2Ptr;
	typedef SmartPointer<TerrainNormalMap2>		TerrainNormalMap2Ptr;
	typedef SmartPointer<TerrainLodMap2>		TerrainLodMap2Ptr;
	typedef	SmartPointer<VerticesResource>		VerticesResourcePtr;
	typedef SmartPointer<HeightMapResource>		HeightMapResourcePtr;
}

namespace Terrain
{
#ifndef MF_SERVER
	typedef SmartPointer<TerrainBlends>			TerrainBlendsPtr;

	class TerrainBlock2 : public CommonTerrainBlock2
	{
	public:
        TerrainBlock2(TerrainSettingsPtr pSettings);
        virtual ~TerrainBlock2();

        bool load(	std::string const &filename, 
					Matrix  const &worldTransform,
					Vector3 const &cameraPosition,
					std::string *error = NULL);

        HorizonShadowMap &shadowMap();
        HorizonShadowMap const &shadowMap() const;

		bool preDraw( bool useCachedLighting, bool doSubstitution );

		virtual bool readyToDraw() const;

		virtual bool draw( Moo::EffectMaterialPtr pMaterial);
		virtual void createUMBRAMesh( UMBRAMesh& umbraMesh ) const;

		virtual void cacheCurrentLighting( bool addSpecular = true );

		virtual void evaluate( const Vector3& relativeCameraPos );
		virtual void stream();

		virtual bool canDrawLodTexture() const;

		virtual bool doingBackgroundTask() const;

		const char* getFileName() const { return fileName_.c_str(); }
		
		const TerrainHeightMap2Ptr getHighestLodHeightMap() const;

		virtual BoundingBox const &boundingBox() const;
		
		// override collision methods
		virtual bool collide
			(
			Vector3                 const &start, 
			Vector3                 const &end,
			TerrainCollisionCallback *callback
			) const;

		virtual bool collide
			(
			WorldTriangle           const &start, 
			Vector3                 const &end,
			TerrainCollisionCallback *callback
			) const;

		virtual float heightAt(float x, float z) const;
		virtual Vector3 normalAt(float x, float z) const;

		/*
		 * This structure holds distance information about this block, this
		 * information is used to create lod rendering information.
		 */
		struct DistanceInfo
		{
			inline DistanceInfo();

			Vector3			relativeCameraPos_;
			float			minDistanceToCamera_;
			float			maxDistanceToCamera_;
			uint32			currentVertexLod_;
			uint32			nextVertexLod_;
		};

		/*
		 * This structure holds lod rendering information about this block.
		 */

		struct LodRenderInfo
		{
			inline LodRenderInfo();

			MorphRanges		morphRanges_;
			NeighbourMasks	neighbourMasks_;
			uint8			subBlockMask_;
			uint8			renderTextureMask_;
		};

	protected:	
		bool initVerticesResource(
				std::string*	error = NULL );

		void normalMap2( TerrainNormalMap2Ptr normalMap2 );
		TerrainNormalMap2Ptr normalMap2() const;

		void horizonMap2( HorizonShadowMap2Ptr horizonMap2 );
		HorizonShadowMap2Ptr horizonMap2() const;

	    uint32 normalMapSize() const;
	    DX::Texture* pNormalMap() const;

	    uint32 horizonMapSize() const;
	    DX::Texture* pHorizonMap() const;

	    DX::Texture* pHolesMap() const;

		uint32 depthPassMark() const				{ return depthPassMark_; }
		void depthPassMark( uint32 depthPassMark )	{ depthPassMark_ = depthPassMark; }

	    uint32 nLayers() const;
	    const CombinedLayer* layer( uint32 index ) const;

        TextureLayers *		 textureLayers();
        TextureLayers const* textureLayers() const;

		TerrainLodMap2Ptr lodMap() const;
		
		// data
    protected:

		// This structure holds copy of information for drawing a single block.
		// Anything that is streamed asynchronously should be stored in here,
		// or it could be come unavailable whilst a draw operation is in place.
		struct DrawState
		{
			VertexLodEntryPtr				currentVertexLodPtr_;
			VertexLodEntryPtr				nextVertexLodPtr_;
			TerrainBlendsPtr				blendsPtr_;
			bool							blendsRendered_;
			bool							lodsRendered_;
		};

		VerticesResourcePtr			pVerticesResource_;
		TerrainBlendsResourcePtr	pBlendsResource_;
		HeightMapResourcePtr		pDetailHeightMapResource_;
		
		TerrainNormalMap2Ptr		pNormalMap_;
		HorizonShadowMap2Ptr        pHorizonMap_;
		TerrainLodMap2Ptr			pLodMap_;
		
		Moo::LightContainerPtr		pDiffLights_;
		Moo::LightContainerPtr		pSpecLights_;
	
		DistanceInfo				distanceInfo_;
		LodRenderInfo				lodRenderInfo_;

		uint32						depthPassMark_;
		uint32						preDrawMark_;

		// store full file path for this block, so streaming resources can
		// use to load.
		std::string					fileName_;

		// Information for current drawing operation - this is valid during a
		// single render target, and will not change due to streaming.
		DrawState			currentDrawState_;

		friend class  TerrainRenderer2;
	};

	inline TerrainBlock2::DistanceInfo::DistanceInfo()
		:	maxDistanceToCamera_( 0.0f ),
			minDistanceToCamera_( 0.0f ),
			currentVertexLod_( 0 ),
			nextVertexLod_( 0 )
	{
		relativeCameraPos_.setZero();
	}

	inline TerrainBlock2::LodRenderInfo::LodRenderInfo()
		:	subBlockMask_( 0 ),
			renderTextureMask_( 0 )
	{
	}

#endif
}

#endif // TERRAIN_TERRAIN_BLOCK2_HPP
