/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TERRAIN_RENDERER2_HPP
#define TERRAIN_TERRAIN_RENDERER2_HPP

#include "../base_terrain_renderer.hpp"

namespace Moo
{
	class VertexDeclaration;
	class EffectConstantValue;
	typedef SmartPointer<EffectConstantValue> EffectConstantValuePtr;
}

namespace Terrain
{

class BaseTexture;
class EffectMaterial;
class VertexDeclaration;
class TerrainPhotographer;
class TerrainBlock2;
class TerrainSettings;
struct CombinedLayer;
struct TerrainBlends;

typedef SmartPointer<BaseTexture> BaseTexturePtr;
typedef SmartPointer<TerrainBlock2>	TerrainBlock2Ptr;
typedef SmartPointer<TerrainBlends>	TerrainBlendsPtr;
typedef SmartPointer<TerrainSettings>	TerrainSettingsPtr;

/**
 *	This class is the terrain renderer class, it contains the
 *	methods for rendering the new terrain blocks
 */
class TerrainRenderer2 : public BaseTerrainRenderer
{
private:
	class BaseMaterial
	{
	public:
		void					getHandles();

		void					SetParam( D3DXHANDLE param, bool b );
		void					SetParam( D3DXHANDLE param, float f );
		void					SetParam( D3DXHANDLE param, const Vector4* v );
		void					SetParam( D3DXHANDLE param, const Matrix* m );
		void					SetParam( D3DXHANDLE param, DX::BaseTexture* t );

		Moo::EffectMaterialPtr	pEffect_;

		D3DXHANDLE				viewProj_;
		D3DXHANDLE				world_;
		D3DXHANDLE				cameraPosition_;

		D3DXHANDLE				normalMap_;
		D3DXHANDLE				normalMapSize_;

		D3DXHANDLE				horizonMap_;
		D3DXHANDLE				horizonMapSize_;

		D3DXHANDLE				holesMap_;
		D3DXHANDLE				holesMapSize_;
		D3DXHANDLE				holesSize_;

		D3DXHANDLE				layer_[4];
		D3DXHANDLE				layerUProjection_[4];
		D3DXHANDLE				layerVProjection_[4];

		D3DXHANDLE				blendMap_;
		D3DXHANDLE				blendMapSize_;
		D3DXHANDLE				layerMask_;

		D3DXHANDLE				lodTextureStart_;
		D3DXHANDLE				lodTextureDistance_;

		D3DXHANDLE				useMultipassBlending_;
		D3DXHANDLE				hasHoles_;
	};

	class TexturedMaterial : public BaseMaterial
	{
	public:
		void					getHandles();

		D3DXHANDLE				specularPower_;
		D3DXHANDLE				specularMultiplier_;
		D3DXHANDLE				specularFresnelExp_;
		D3DXHANDLE				specularFresnelConstant_;
	};

	class LodTextureMaterial : public BaseMaterial
	{
	public:
		//void					getHandles();
	};

	class ZPassMaterial : public BaseMaterial
	{
	public:
		//void					getHandles();
	};

public:
	~TerrainRenderer2();

	static TerrainRenderer2* instance();

	static bool init();
	
	// BaseTerrainRenderer interface implementation
	uint32 version() { return 200; };

	void addBlock(	BaseRenderTerrainBlock*	pBlock,	
					const Matrix&			transform );
	void drawAll( Moo::EffectMaterialPtr pOverride = NULL, bool clearList = true );
	void drawSingle(BaseRenderTerrainBlock*	pBlock,	
					const Matrix&			transform, 
					Moo::EffectMaterialPtr	altMat = NULL,
					bool					useCachedLighting = false );
	void clearBlocks();

	bool canSeeTerrain() const { return isVisible_; }

	bool supportSmallBlend() const { return supportSmallBlends_; }

	bool zBufferIsClear() const { return zBufferIsClear_; }
	void zBufferIsClear( bool zBufferIsClear ) { zBufferIsClear_ = zBufferIsClear; }

	inline static uint8 getLoadFlag(float minDistance,
										float textureBlendsPreloadDistance,
										float normalPreloadDistance);

	static uint8  getDrawFlag(	uint8 partialResult,
								float minDistance, float maxDistance,
								const TerrainSettingsPtr pSettings );

	
#ifdef EDITOR_ENABLED
	/**
	 * Return a terrain photographer.
	 */
	TerrainPhotographer& photographer() { return *pPhotographer_; }
#endif

	/** 
	 * This enum describes which textures should be rendered
	 * and what resources are needed for a block.
	 */
	enum RenderTextureMask
	{
		RTM_None			= 0,
		RTM_DrawLOD			= 1,
		RTM_DrawBlend		= 1 << 1,
		RTM_PreLoadBlend	= 1 << 2,
		RTM_DrawLODNormals	= 1 << 3,
		RTM_PreloadNormals	= 1 << 4
	};
	
private:

	enum DrawMethod
	{ 
		DM_Z_ONLY,			/** Draw to Z buffer only. */
		DM_SINGLE_LOD,		/** Draw single pass, using lod texture. */
		DM_SINGLE_OVERRIDE,	/** Draw single pass, using override material. */
		DM_BLEND,			/** Draw all blend materials. */
		DM_LOD_NORMALS		/** Draw with low resolution normal map and lod texture */
	};

	TerrainRenderer2();

	bool initInternal( DataSectionPtr pSettingsSection );

	void setTransformConstants( BaseMaterial* pMaterial );
	void setNormalMapConstants( const TerrainBlock2* pBlock, 
								BaseMaterial* pMaterial,
								bool lodNormals );
	void setHorizonMapConstants( const TerrainBlock2* pBlock, 
								BaseMaterial* pMaterial );
	void setHolesMapConstants( const TerrainBlock2* pBlock, 
								BaseMaterial* pMaterial );
	void setTextureLayerConstants( TerrainBlendsPtr pBlend, 
								BaseMaterial* pMaterial, 
								uint32 layer, bool blended = false );
	void setSingleLayerConstants( BaseMaterial* pMaterial, uint32 layerNo, 
								CombinedLayer const &combinedLayer );
	void setLodTextureConstants( const TerrainBlock2*	pBlock, 
								BaseMaterial*			pMaterial,
								bool					doBlend,
								bool					lodNormals);

	void updateConstants(	const TerrainBlock2*	pBlock,
							TerrainBlendsPtr		pBlends,
							BaseMaterial*			pMaterial,
							DrawMethod				drawMethod  );

	void drawSingleInternal(BaseRenderTerrainBlock*	pBlock,	
							const Matrix&			transform, 
							bool					depthOnly = false,
							BaseMaterial*			altMat = NULL,
							bool					useCachedLighting = false );

	bool drawTerrainMaterial(	TerrainBlock2*			pBlock,
								BaseMaterial*			pMaterial, 
								DrawMethod				drawMethod,
								uint32&					passCount );

	void drawPlaceHolderBlock(	TerrainBlock2*			pBlock,
								uint32					colour,
								bool					lowDetail = false );

	// Watch accessors
	float specularPower() const { return specularInfo_.power_; };
	void specularPower( float power );
	float specularMultiplier() const { return specularInfo_.multiplier_; };
	void specularMultiplier( float mult );
	float specularFresnelExp() const { return specularInfo_.fresnelExp_; }
	void specularFresnelExp( float exp );
	float specularFresnelConstant() const { return specularInfo_.fresnelConstant_; };
	void specularFresnelConstant( float c );
	
	// types
	struct Block
	{
		Matrix				transform;
		TerrainBlock2Ptr	block;
		bool				depthOnly;
	};
	typedef AVectorNoDestructor<Block> BlockVector;
	struct Terrain2SpecularInfo
	{
		float power_;
		float multiplier_;
		float fresnelExp_;
		float fresnelConstant_;
	};

	BlockVector				blocks_;
	Terrain2SpecularInfo	specularInfo_;
	Moo::VertexDeclaration*	pDecl_;
	TexturedMaterial		texturedMaterial_;
	LodTextureMaterial		lodTextureMaterial_;
	ZPassMaterial			zPassMaterial_;
	BaseMaterial			overrideMaterial_;
	uint32					frameTimestamp_;
	uint32					nBlocksRendered_;
	TerrainPhotographer*	pPhotographer_;
	bool					supportSmallBlends_;
	bool					zBufferIsClear_;
	bool					useStencilMasking_;

	Moo::EffectConstantValuePtr	sunAngleSetter_;
	Moo::EffectConstantValuePtr	penumbraSizeSetter_;

	static TerrainRenderer2*	s_instance_;
};

inline uint8 TerrainRenderer2::getLoadFlag(float minDistance,
										float textureBlendsPreloadDistance,
										float normalPreloadDistance)
{	
	// If we're in the preload distance, flag a preload
	if ( minDistance <= textureBlendsPreloadDistance )
		return RTM_PreLoadBlend | RTM_PreloadNormals;
	
	if ( minDistance <= normalPreloadDistance )
		return RTM_PreloadNormals;

	// We only need to draw a lod texture
	return RTM_DrawLODNormals;
}


};

#endif // TERRAIN_TERRAIN_RENDERER2_HPP
