/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "terrain_renderer2.hpp"

#include "../manager.hpp"
#include "../terrain_settings.hpp"
#include "moo/effect_visual_context.hpp"
#include "terrain_block2.hpp"
#include "terrain_texture_layer2.hpp"
#include "terrain_lod_map2.hpp"
#include "terrain_photographer.hpp"
#include "vertex_lod_entry.hpp"
#include "cstdmf/profiler.hpp"
#include "cstdmf/watcher.hpp"
#include "terrain_height_map2.hpp"
#include "romp/line_helper.hpp"
#include "terrain_normal_map2.hpp"

#ifdef EDITOR_ENABLED
#include "ttl2cache.hpp"
#include "editor_terrain_block2.hpp"
#endif // EDITOR_ENABLED

using namespace Terrain;
DECLARE_DEBUG_COMPONENT2( "Terrain", 0 )

PROFILER_DECLARE( TerrainRenderer2_updateConstants, "TerrainRenderer2 UpdateConstants" );
PROFILER_DECLARE( TerrainRenderer2_drawTerrainMaterial, "TerrainRenderer2 DrawTerrainMaterial" );

extern bool g_drawTerrain;

namespace
{
	/**
	 * Flag adding of watches on first run of constructor.
	 */
	bool g_firstTime	= true;

	/**
	 *	This is a table of layer strings used in setting effects.
	 */
	static const char *layerText[4] =
	{
		"layer0",
		"layer1",
		"layer2",
		"layer3"
	};

	/**
	 *	This is a table of layer u-projection strings used in setting effects.
	 */
	static const char *layerUProjText[4] =
	{
		"layer0UProjection",
		"layer1UProjection",
		"layer2UProjection",
		"layer3UProjection"
	};

	/**
	 *	This is a table of layer v-projection strings used in setting effects.
	 */
	static const char *layerVProjText[4] =
	{
		"layer0VProjection",
		"layer1VProjection",
		"layer2VProjection",
		"layer3VProjection"
	};

	/**
	 *	This table is a mask for which indicies are used when there are zero,
	 *	one etc layers in a combined layer when we use as small a texture
	 *	size as possible.
	 */
	static const Vector4 layerMaskTable[] = 
	{									 // layers x  y  z  w
		Vector4(0.0f, 0.0f, 0.0f, 0.0f), // 0
		Vector4(1.0f, 0.0f, 0.0f, 0.0f), // 1      t0      
		Vector4(1.0f, 0.0f, 0.0f, 1.0f), // 2      t0       t1
		Vector4(1.0f, 1.0f, 1.0f, 0.0f), // 3      t2 t1 t0
		Vector4(1.0f, 1.0f, 1.0f, 1.0f)  // 4      t2 t1 t0 t3
	};

	/**
	 *	This table is a mask for which indicies are used when there are zero,
	 *	one etc layers in a combined layer when we use 32 bit textures.
	 */
	static const Vector4 layerMaskTable32bit[] = 
	{									 // layers x  y  z  w
		Vector4(0.0f, 0.0f, 0.0f, 0.0f), // 0
		Vector4(0.0f, 0.0f, 1.0f, 0.0f), // 1            t0      
		Vector4(0.0f, 1.0f, 1.0f, 0.0f), // 2         t1 t0 
		Vector4(1.0f, 1.0f, 1.0f, 0.0f), // 3      t2 t1 t0
		Vector4(1.0f, 1.0f, 1.0f, 1.0f)	 // 4      t2 t1 t0 t3
	};

	/**
	 *	Each row of the following map represents the order that combined layers
	 *	map to textures.  This is for the case when we compress textures to
	 *	8, 16 or 32 bit textures if possible.
	 */
	static const uint32 layerTexMap[5][4] =
	{
		{ 0, 1, 2, 3 },
		{ 0, 1, 2, 3 },
		{ 0, 3, 1, 2 }, 
		{ 2, 1, 0, 3 },
		{ 2, 1, 0, 3 }
	};

	/**
	 *	Each row of the following map represents the order that combined layers
	 *	map to textures.  This is for the case when we use 32 bit textures for 
	 *	the blends.
	 */
	static const uint32 layerTexMap32[5][4] =
	{
		{ 0, 1, 2, 3 },
		{ 2, 0, 1, 3 },
		{ 2, 1, 0, 3 }, 
		{ 2, 1, 0, 3 },
		{ 2, 1, 0, 3 }
	};

	/**
	* This class allows SunAngleConstant shader constant to be set.
	*/
	class SunAngleConstantSetter : public Moo::EffectConstantValue
	{
		bool SunAngleConstantSetter::operator()(ID3DXEffect* pEffect, 
			D3DXHANDLE constantHandle)
		{
			Vector4 value(0.5,0.5,0.5,0.5);

			Moo::LightContainer * pLC = Moo::rc().lightContainer().get();
			if (pLC && pLC->nDirectionals() > 0)
			{
				float f = 0.5f;
				f = -atan2f( -pLC->directionals()[0]->worldDirection().y,
					-pLC->directionals()[0]->worldDirection().x );


				f = float( ( f + ( MATH_PI * 1.5 ) ) / ( MATH_PI * 2 ) );
				f = fmodf( f, 1 );
				f = min( 1.f, max( (f * 2.f ) - 0.5f, 0.f ) );
				value.set(f,f,f,f);
			}

			pEffect->SetVector(constantHandle, &value);
			return true;
		}
	};

	/**
	* This class allows PenumbraSize shader constant to be set.
	*/
	float s_penumbraSize = 0.2f;

	class PenumbraSizeSetter : public Moo::EffectConstantValue
	{
		bool PenumbraSizeSetter::operator()(ID3DXEffect* pEffect, 
											D3DXHANDLE constantHandle)
		{		
			float psReciprocal = 1.f / s_penumbraSize;
			Vector4 value( psReciprocal, psReciprocal, psReciprocal, 
						psReciprocal );
			pEffect->SetVector(constantHandle, &value);
			return true;
		}
	};

	bool drawHeightmap = false;
	bool drawPlaceHolders = false;
} // anonymous namespace


namespace Terrain
{

// Reserved stencil value (and 64) for controlling pixel placement within a block.
const uint32 TERRAIN_STENCIL_OFFSET = 63;

// -----------------------------------------------------------------------------
// Section: TerrainRenderer2
// -----------------------------------------------------------------------------

TerrainRenderer2* TerrainRenderer2::s_instance_ = NULL;

/*
 * Default constructor
 */
TerrainRenderer2::TerrainRenderer2() :
	pDecl_( NULL ),
	frameTimestamp_( Moo::rc().frameTimestamp() ),
	nBlocksRendered_( 0 ),
	zBufferIsClear_( false ),
	useStencilMasking_( true )
{
	BW_GUARD;
	MF_ASSERT( Manager::pInstance() != NULL );

	texturedMaterial_.pEffect_ = NULL;
	lodTextureMaterial_.pEffect_ = NULL;
	zPassMaterial_.pEffect_ = NULL;
#ifdef EDITOR_ENABLED
	pPhotographer_ = new TerrainPhotographer; 
#endif
}

/**
 * Initialise renderer internals from values in given settings section.
 */
bool TerrainRenderer2::initInternal( DataSectionPtr pSettingsSection )
{
	BW_GUARD;
	sunAngleSetter_ = new SunAngleConstantSetter;
	penumbraSizeSetter_ = new PenumbraSizeSetter;

	// Validate section
	if ( !pSettingsSection )
	{
		ERROR_MSG( "Couldn't open settings section\n" );
		return false;
	}

	// Validate data
	std::string texturedEffect = pSettingsSection->readString( "texturedEffect" );
	if ( texturedEffect.empty() )
	{
		ERROR_MSG( "Couldn't get textured effect.\n" );
		return false;
	}

	std::string zPassEffect = pSettingsSection->readString( "zPassEffect" );
	if ( zPassEffect.empty() )
	{
		ERROR_MSG( "Couldn't get z pass effect.\n" );
		return false;
	}

	std::string lodTextureEffect = pSettingsSection->readString( "lodTextureEffect" );
	if ( lodTextureEffect.empty() )
	{
		ERROR_MSG( "Couldn't get lod texture effect.\n" );
		return false;
	}

	// Go ahead and create renderer
	texturedMaterial_.pEffect_ = new Moo::EffectMaterial;
	texturedMaterial_.pEffect_->initFromEffect( texturedEffect );
	texturedMaterial_.getHandles();

	zPassMaterial_.pEffect_ = new Moo::EffectMaterial;
	zPassMaterial_.pEffect_->initFromEffect( zPassEffect );
	zPassMaterial_.getHandles();

	lodTextureMaterial_.pEffect_ = new Moo::EffectMaterial;
	lodTextureMaterial_.pEffect_->initFromEffect( lodTextureEffect );
	lodTextureMaterial_.getHandles();

	pDecl_ = Moo::VertexDeclaration::get( "terrain" );

	// Bind textured material's specular constants to our local values
	ComObjectWrap<ID3DXEffect> pEffect = texturedMaterial_.pEffect_->pEffect()->pEffect();
	pEffect->GetFloat( "terrain2Specular.power",			&specularInfo_.power_);
	pEffect->GetFloat( "terrain2Specular.multiplier",		&specularInfo_.multiplier_ );
	pEffect->GetFloat( "terrain2Specular.fresnelExp",		&specularInfo_.fresnelExp_ );
	pEffect->GetFloat( "terrain2Specular.fresnelConstant",	&specularInfo_.fresnelConstant_ );

	// Are 8 and 16 bit textures supported?:
	supportSmallBlends_ = 
		Moo::rc().supportsTextureFormat(D3DFMT_L8)
		&&
		Moo::rc().supportsTextureFormat(D3DFMT_A8L8);

	useStencilMasking_ = true;

	if ( g_firstTime )
	{
	// init watchers
		MF_WATCH( "Render/Terrain/Terrain2/RenderedBlocks", nBlocksRendered_, 
			Watcher::WT_READ_ONLY );

		MF_WATCH( "Render/Terrain/Terrain2/VertexLODSizeMB", 
			VertexLodEntry::s_totalMB_, Watcher::WT_READ_ONLY  );

		MF_WATCH( "Render/Terrain/Terrain2/specular power", *this,
				MF_ACCESSORS( float, TerrainRenderer2, specularPower ),
				"Mathematical power of the specular reflectance function." );

		MF_WATCH( "Render/Terrain/Terrain2/specular multiplier", *this,
				MF_ACCESSORS( float, TerrainRenderer2, specularMultiplier ),
				"Overall multiplier on the terrain specular effect." );

		MF_WATCH( "Render/Terrain/Terrain2/specular fresnel constant", *this,
				MF_ACCESSORS( float, TerrainRenderer2, specularFresnelConstant ),
				"Fresnel constant for falloff calculations." );

		MF_WATCH( "Render/Terrain/Terrain2/specular fresnel falloff", *this,
				MF_ACCESSORS( float, TerrainRenderer2, specularFresnelExp ),
				"Fresnel falloff rate." );

		MF_WATCH( "Render/Performance/Enable Terrain Draw", g_drawTerrain,
			Watcher::WT_READ_WRITE,
			"Enable high level terrain renderering." );

		MF_WATCH( "Render/Terrain/Terrain2/Draw Heightmap", drawHeightmap,
			Watcher::WT_READ_WRITE,
			"Enable visualisation of terrain height map.");

		MF_WATCH( "Render/Terrain/Terrain2/Draw Placeholders", drawPlaceHolders,
			Watcher::WT_READ_WRITE,
			"Draw placeholders for missing or partially loaded terrain.");

#ifdef DEBUG_STENICL_MASKING
		// This allows developer to switch the feature off and on, only useful
		// if something related to it breaks.
		MF_WATCH( "Render/Terrain/Terrain2/useStencilMasking", useStencilMasking_,
			Watcher::WT_READ_WRITE );
#endif
		g_firstTime = false;
	}

	return true;
}

/**
 * Accessor for renderer instance.
 */
TerrainRenderer2* TerrainRenderer2::instance()
{
	MF_ASSERT( s_instance_ != NULL );
	return s_instance_;
}

/**
 * Initialise the renderer.
 */
bool TerrainRenderer2::init()
{
	BW_GUARD;
	if ( s_instance_ != NULL )
		return true;

	// Determine if current shader profile supports Terrain2
	if ( Moo::EffectManager::instance().PSVersionCap() < 2 )
	{
		ERROR_MSG("TerrainRenderer2::init() - Shader Version Cap must be 2 or "
					"above to render advanced terrain.\n");

#ifdef EDITOR_ENABLED
		Moo::EffectManager::instance().PSVersionCap( 2 );
#else//EDITOR_ENABLED
		return false;
#endif//EDITOR_ENABLED
	}

	s_instance_ = new TerrainRenderer2;

	// destroy the old terrain before creating the new one
	BaseTerrainRenderer::instance( s_instance_ );

	bool res = s_instance_->initInternal( Manager::instance().pTerrain2Defaults() );
	if (!res)
	{
		delete s_instance_;
		s_instance_ = NULL;
		BaseTerrainRenderer::instance( s_instance_ );
	}
	return res;
}

/**
 *  Destructor
 */
TerrainRenderer2::~TerrainRenderer2()
{
	BW_GUARD;
	s_instance_ = NULL;
	if (Manager::pInstance() != NULL && BaseTerrainRenderer::instance() == this)
		BaseTerrainRenderer::instance( NULL );

#ifdef EDITOR_ENABLED
	delete pPhotographer_;
	pPhotographer_ = NULL;
#endif

#ifdef EDITOR_ENABLED
	// Reset the layer cache whenever the renderer2 is destroyed.
	if (Manager::pInstance() != NULL)
		Manager::instance().ttl2Cache().clear();
#endif // EDITOR_ENABLED

	*Moo::EffectConstantValue::get( "SunAngle" ) = NULL;
	*Moo::EffectConstantValue::get( "PenumbraSize" ) = NULL;

}

/*virtual*/ void TerrainRenderer2::drawSingle(
									BaseRenderTerrainBlock* pBlock,
									const Matrix&			transform,
									Moo::EffectMaterialPtr	pOverrideMaterial,
									bool					useCachedLighting )
{
	*Moo::EffectConstantValue::get( "SunAngle" ) = sunAngleSetter_;
	*Moo::EffectConstantValue::get( "PenumbraSize" ) = penumbraSizeSetter_;

	Moo::EffectVisualContext::instance().isOutside( true );

	BaseMaterial* pOverrideBaseMaterial = NULL;
	if ( pOverrideMaterial )
	{
		overrideMaterial_.pEffect_ = pOverrideMaterial;
		overrideMaterial_.getHandles();
		pOverrideBaseMaterial = &overrideMaterial_;
	}

	drawSingleInternal( pBlock, transform, false, pOverrideBaseMaterial, useCachedLighting );
}

void TerrainRenderer2::drawPlaceHolderBlock(TerrainBlock2*	pBlock,
											uint32			colour,
											bool			lowDetail)
{
	BW_GUARD;


	Matrix	transform = Moo::rc().world();


	if ( lowDetail )
	{
		TerrainHeightMap2Ptr	hmp		= pBlock->heightMap2();
		uint32					size	= hmp->verticesWidth();

		float h[4] =
		{	
			hmp->heightAt( 0, 0 ),
			hmp->heightAt( 0, (int)size ),
			hmp->heightAt( (int)size, 0 ),
			hmp->heightAt( (int)size, (int)size )
		};
		Vector3 v[4];
		v[0].set( 0, h[0], 0 );
		v[1].set( 0, h[1], BLOCK_SIZE_METRES );
		v[2].set( BLOCK_SIZE_METRES, h[2], 0 );
		v[3].set( BLOCK_SIZE_METRES, h[3], BLOCK_SIZE_METRES );

		transform.applyPoint( v[0], v[0] );
		transform.applyPoint( v[1], v[1] );
		transform.applyPoint( v[2], v[2] );
		transform.applyPoint( v[3], v[3] );

		// draw diagonal cross
		LineHelper::instance().drawLine( v[0], v[3], colour );
		LineHelper::instance().drawLine( v[1], v[2], colour );
	}
	else
	{
		TerrainHeightMap2Ptr hmp =  pBlock->getHighestLodHeightMap();

		uint32	size = hmp->verticesWidth();
		float	step = BLOCK_SIZE_METRES / float(size);

		for ( uint32 x = 0; x < size; x++ )
		{
			for ( uint32 z = 0; z < size; z++ )
			{
				Vector3 p1( step * x, 
					hmp->heightAt(int(x), int(z)), 
					step * (z) );
				Vector3 p2( step * x, 
					hmp->heightAt(int(x), int(z+1)), 
					step * (z+1) );
				Vector3 p3( step * (x+1), 
					hmp->heightAt(int(x+1), int(z+1)), 
					step * (z+1) );
				transform.applyPoint( p1, p1 );
				transform.applyPoint( p2, p2 );
				transform.applyPoint( p3, p3 );

				LineHelper::instance().drawLine( p1, p2, colour );
				LineHelper::instance().drawLine( p1, p3, colour );
			}
		}
	}
}

/**
*	This method is the internal drawing method for a terrain block.
*	
*	@param pBlock				The block to draw.
*	@param transform			World transform of block.
*	@param pOverrideMaterial	An alternative material to use.
*	@param useCachedLighting	If true, use light state that was stored with
*								this block, otherwise use current lighting state.
*/
void TerrainRenderer2::drawSingleInternal(
	BaseRenderTerrainBlock* pBlock,
	const Matrix&			transform,
	bool					depthOnly,
	BaseMaterial*			pOverrideMaterial,
	bool					useCachedLighting )
{
	BW_GUARD;
	if (g_drawTerrain)
	{
		// Update world transform.
		TerrainBlock2* tb2 = static_cast<TerrainBlock2*>(pBlock);
		Moo::rc().world( transform );

		// Force draw of heightmap
		if ( drawHeightmap )
		{
			uint32 lod = tb2->getHighestLodHeightMap()->lodLevel();

			drawPlaceHolderBlock( tb2, 0xFFFFFFFF, lod > 0 );
		}

		// Draw big grid.
		if ( drawPlaceHolders )
			drawPlaceHolderBlock( tb2, 0xFFFF00FF, true );

		// update profiling
		if (!Moo::rc().frameDrawn( frameTimestamp_ ))
		{
			nBlocksRendered_ = 0;
		}

		// Prepare block for draw - always try to substitute, otherwise draw
		// a magenta placeholder.
		bool ok = tb2->preDraw( useCachedLighting, true );
		if ( !ok ) 
		{
			if ( drawPlaceHolders ) drawPlaceHolderBlock( tb2, 0xFFFF00FF );
			return;
		}

		// set the vertex declaration
		Moo::rc().setVertexDeclaration( pDecl_->declaration() );

		// get a copy of blends to use throughout draw
		TerrainBlendsPtr pBlends = tb2->currentDrawState_.blendsPtr_;

#if EDITOR_ENABLED
		// In the editor, the blends could have 
		if (!tb2->pBlendsResource_.hasObject() ||
			tb2->pBlendsResource_->getState() != RS_Loaded)
		{
			pBlends = NULL;
		}
#endif // EDITOR_ENABLED

		// Allow drawTerrainMaterial to keep track of how many passes have been done
		uint32 passCount = 0;

		if ( pOverrideMaterial )
		{
			// draw the block as override
			drawTerrainMaterial( tb2, pOverrideMaterial, DM_SINGLE_OVERRIDE, passCount );
		}
		else
		{
			// Determine blends availability
			bool drawBlend	= false;
			bool drawLod	= false;
			bool drawNormalLod = false;

			// Lod texture might be switched off (due to user settings) or 
			// missing (due to bad data).
			bool canDrawLodTexture = tb2->canDrawLodTexture();

			// We should have specified something to draw, or else there's a 
			// serious logic error.
			MF_ASSERT_DEBUG( tb2->lodRenderInfo_.renderTextureMask_ > 0 );

			// If blends are available, and we can't draw lod textures or we
			// should draw blends anyway, then draw blend.
			if ( pBlends && 
				( !canDrawLodTexture || 
				  tb2->lodRenderInfo_.renderTextureMask_ 
				    & TerrainRenderer2::RTM_DrawBlend ) )
			{
				drawBlend = true;
			}

			// If lod texture is available, and we don't have blends or we should
			// draw lod anyway, then draw lod.
			if ( canDrawLodTexture)
			{
				if ((!pBlends && 
						(tb2->lodRenderInfo_.renderTextureMask_ & 
						TerrainRenderer2::RTM_DrawBlend) ) || 
					(tb2->lodRenderInfo_.renderTextureMask_ & 
						TerrainRenderer2::RTM_DrawLOD ))
				{
					drawLod = true;
				}
				if (tb2->lodRenderInfo_.renderTextureMask_ & 
						TerrainRenderer2::RTM_DrawLODNormals)
				{
					drawNormalLod = true;
				}
			}

			if ( drawBlend || drawLod || drawNormalLod )
			{
				if ( depthOnly )
				{
					bool res = drawTerrainMaterial( tb2, &zPassMaterial_, DM_Z_ONLY, passCount );
					MF_ASSERT( res && "failed zpass draw" );
					tb2->depthPassMark( Moo::rc().renderTargetCount() );
				}
				else
				{
					// Determine if we need to do a z pass. A zpass is required if we have a
					// holes map. Even then, only do it if Umbra hasn't already done it
					// or if we are in wireframe mode, in which case the z buffer has been
					// cleared and the Umbra pass would be lost.

					bool drawZPass = false;

					if ( tb2->pHolesMap() )
					{
						if ( tb2->depthPassMark() != Moo::rc().renderTargetCount() )
							drawZPass = true;
						if ( Terrain::Manager::instance().wireFrame() )
							drawZPass = true;
						if ( zBufferIsClear_ )
							drawZPass = true;
					}

					// Draw a z pass if we need to.
					if ( drawZPass )
					{
						bool res = drawTerrainMaterial( tb2, &zPassMaterial_, DM_Z_ONLY, passCount );
						MF_ASSERT( res && "failed zpass draw" );
					}

					// Drawing policy : We "should" always have a lodTexture. 
					// 
					// When drawing, we can always fall back to this if blends 
					// (which are streamed) aren't available. This includes the 
					// situation when "useLodTexture" is set to false - that is, we 
					// should only draw blends (a high quality graphics option). If 
					// no lod texture or blend is available we don't get drawn.
					if ( drawBlend )
					{
						// Record whether blends rendered ok, so lod pass can avoid
						// blending to "blue".
						tb2->currentDrawState_.blendsRendered_ =  
							drawTerrainMaterial( tb2, &texturedMaterial_, DM_BLEND, passCount );				
						nBlocksRendered_++;
					}

					// Lod pass is done if the feature is enabled and if it has been 
					// flagged for this block.
					if ( drawLod )
					{
						bool res = drawTerrainMaterial( tb2, &lodTextureMaterial_, 
							DM_SINGLE_LOD, passCount );
						if ( !res && drawPlaceHolders ) 
							drawPlaceHolderBlock( tb2, 0xFFFFFFFF );
						tb2->currentDrawState_.lodsRendered_ = true;
						nBlocksRendered_++;
					}

					if (drawNormalLod)
					{
						bool res = drawTerrainMaterial( tb2, &lodTextureMaterial_, 
							DM_LOD_NORMALS, passCount );
						if ( !res && drawPlaceHolders ) 
							drawPlaceHolderBlock( tb2, 0xFFFF0000 );
						nBlocksRendered_++;
					}
				}
			}
			else
			{
				if ( drawPlaceHolders )
					drawPlaceHolderBlock( tb2, 0xFF0000FF );
			}
		}

		HRESULT hr = Moo::rc().device()->SetStreamSource(1, NULL, 0, 0);
		MF_ASSERT(hr == D3D_OK);
	}
}

/**
 *	This method sets the transform constants on a material
 *	@param pBlock the block to set the constants for
 *	@param pMaterial the material to set the constants on
 */
void TerrainRenderer2::setTransformConstants( BaseMaterial* pMaterial )
{
	BW_GUARD;
	pMaterial->SetParam( pMaterial->viewProj_, &Moo::rc().viewProjection() );
	pMaterial->SetParam( pMaterial->world_, &Moo::rc().world() );
	pMaterial->SetParam( pMaterial->cameraPosition_, &Moo::rc().invView().row(3) );
}

/**
 *	This method sets the normal map constants on a material
 *	@param pBlock the block to set the constants for
 *	@param pMaterial the material to set the constants on
 */
void TerrainRenderer2::setNormalMapConstants( const TerrainBlock2* pBlock, 
											  BaseMaterial* pMaterial,
											  bool lodNormals)
{
	BW_GUARD;
	TerrainNormalMap2Ptr pNormals = pBlock->normalMap2();
	if (lodNormals)
	{
		pMaterial->SetParam( pMaterial->normalMap_, pNormals->pLodMap().pComObject() );
		pMaterial->SetParam( pMaterial->normalMapSize_, (float)pNormals->lodSize() );
	}
	else
	{
		pMaterial->SetParam( pMaterial->normalMap_, pNormals->pMap().pComObject() );
		pMaterial->SetParam( pMaterial->normalMapSize_, (float)pNormals->size() );
	}
}

/**
 *	This method sets the horizon map constants on a material
 *	@param pBlock the block to set the constants for
 *	@param pMaterial the material to set the constants on
 */
void TerrainRenderer2::setHorizonMapConstants( const TerrainBlock2* pBlock, 
											   BaseMaterial* pMaterial )
{
	BW_GUARD;
	pMaterial->SetParam( pMaterial->horizonMap_, pBlock->pHorizonMap() );
	pMaterial->SetParam( pMaterial->horizonMapSize_, (float)pBlock->horizonMapSize() );
}

/**
 *	This method sets the holes map constants on a material
 *	@param pBlock the block to set the constants for
 *	@param pMaterial the material to set the constants on
 */
void TerrainRenderer2::setHolesMapConstants(const TerrainBlock2*	pBlock, 
											BaseMaterial*	pMaterial )
{
	BW_GUARD;
	pMaterial->SetParam( pMaterial->holesMap_, pBlock->pHolesMap() );
	pMaterial->SetParam( pMaterial->holesMapSize_, (float)pBlock->holesMapSize() );
	pMaterial->SetParam( pMaterial->holesSize_, (float)pBlock->holesSize() );
}

/**
 *	This sets up the constants for a single layer.
 *
 *	@param effect			The effect whose constants need setting.
 *	@param layerNo			The layerNo to set.
 *	@param combinedLayer	The combined layer that we are working with.
 */
void TerrainRenderer2::setSingleLayerConstants
(
	BaseMaterial*		 pMaterial,
	uint32				 layerNo,
	CombinedLayer const& combinedLayer
)
{
	BW_GUARD;
	// layerIdx is how we swizzle the order of textures from the packed texture
	uint32 layerIdx = 
		(supportSmallBlends_ && combinedLayer.smallBlended_) 
			? layerTexMap[combinedLayer.textureLayers_.size()][layerNo]
			: layerTexMap32[combinedLayer.textureLayers_.size()][layerNo];

	if (layerNo < combinedLayer.textureLayers_.size())
	{
		pMaterial->SetParam( pMaterial->layer_[layerIdx],
			combinedLayer.textureLayers_[layerNo]->texture()->pTexture() );

		pMaterial->SetParam( pMaterial->layerUProjection_[layerIdx],
			&combinedLayer.textureLayers_[layerNo]->uProjection() );

		pMaterial->SetParam( pMaterial->layerVProjection_[layerIdx],
			&combinedLayer.textureLayers_[layerNo]->vProjection() );
	}
}

/**
 *	This method sets the texture layer constants on a material
 *	@param pBlock the block to set the constants for
 *	@param pMaterial the material to set the constants on
 *	@param layer the index of the layer to set as active on the material
 */
void TerrainRenderer2::setTextureLayerConstants( TerrainBlendsPtr pBlends, 
		BaseMaterial* pMaterial, uint32 layer, bool blended )
{
	BW_GUARD;
	MF_ASSERT( pBlends.exists() );

	// Grab the layer and set up the blend texture
	const CombinedLayer& theLayer = pBlends->combinedLayers_[layer];

	Vector4 layerMask = 
		(supportSmallBlends_ && theLayer.smallBlended_)
			? layerMaskTable[theLayer.textureLayers_.size()]
			: layerMaskTable32bit[theLayer.textureLayers_.size()];

	// Grab the layer and set up the blend texture
	pMaterial->SetParam( pMaterial->blendMap_, theLayer.pBlendTexture_.pComObject() );
	pMaterial->SetParam( pMaterial->blendMapSize_, float(theLayer.width_) );
	pMaterial->SetParam( pMaterial->layerMask_, &layerMask );

	// Set up our texture layers
	setSingleLayerConstants( pMaterial, 0, theLayer );
	setSingleLayerConstants( pMaterial, 1, theLayer );
	setSingleLayerConstants( pMaterial, 2, theLayer );
	setSingleLayerConstants( pMaterial, 3, theLayer );

	pMaterial->SetParam( pMaterial->useMultipassBlending_, blended );
}

/**
 *	This method sets the texture layer constants on a material
 *
 *	@param pMaterial the material to set the constants on
 */
void TerrainRenderer2::setLodTextureConstants(	const TerrainBlock2*	pBlock,
												BaseMaterial*			pMaterial,
												bool					doBlends,
												bool					lodNormals )
{
	BW_GUARD;
	TerrainLodMap2Ptr pLodMap = pBlock->lodMap();
	if (pLodMap.hasObject())
	{
		TerrainSettingsPtr settings = pBlock->settings();

		pMaterial->SetParam( pMaterial->blendMap_, pLodMap->pTexture().pComObject() );
		pMaterial->SetParam( pMaterial->blendMapSize_, float(pLodMap->size()) );
		if (lodNormals)
		{
			pMaterial->SetParam( pMaterial->lodTextureStart_, settings->lodNormalStart() );
			pMaterial->SetParam( pMaterial->lodTextureDistance_, settings->lodNormalDistance()  );
		}
		else
		{
			pMaterial->SetParam( pMaterial->lodTextureStart_, settings->lodTextureStart() );
			pMaterial->SetParam( pMaterial->lodTextureDistance_, settings->lodTextureDistance()  );
		}
		pMaterial->SetParam( pMaterial->useMultipassBlending_, doBlends );
	}
}

/**
 *	This method updates the constants for our terrain block
 *	@param pBlock the block to update constants for
 */
void TerrainRenderer2::updateConstants( const TerrainBlock2*	pBlock,
									    TerrainBlendsPtr		pBlends,
										BaseMaterial*			pMaterial,
										DrawMethod				drawMethod )
{
	BW_GUARD_PROFILER( TerrainRenderer2_updateConstants );

	// always set transform constants
	setTransformConstants( pMaterial );

	if ( drawMethod == DM_Z_ONLY )
	{
		// Set up the holes constants
		if ( holeMapEnabled() )
		{
			setHolesMapConstants( pBlock, pMaterial );
		}
	}
	else
	{
		bool lodNormals = drawMethod == DM_LOD_NORMALS;

		// set normal and horizon for non-z only passes
		setNormalMapConstants( pBlock,	pMaterial, lodNormals );
		setHorizonMapConstants( pBlock, pMaterial );

		// set holes constant for non-z passes
		pMaterial->SetParam( pMaterial->hasHoles_,
			( pBlock->pHolesMap() && holeMapEnabled() ) );

		if ( drawMethod == DM_SINGLE_LOD || lodNormals )
		{
			// set lod constants if we are rendering with the lod texture .
			// if we are using multi-pass rendering set up blending.
			setLodTextureConstants( pBlock, pMaterial, 
								pBlock->currentDrawState_.blendsRendered_ || 
									pBlock->currentDrawState_.lodsRendered_,
								lodNormals);
		}
		else if ( drawMethod == DM_BLEND )
		{
			// set up first layer constants - others are set by draw
			if (pBlends->combinedLayers_.size())
			{
				setTextureLayerConstants( pBlends, pMaterial, 0 );
			}
		}
		else
		{
			MF_ASSERT( drawMethod == DM_SINGLE_OVERRIDE && "Unknown draw method.");
		}
	}
}

/**
 *	This method draws a terrain block with a given material and method. This
 *	assumes "set" has been called on the passed in block.
 * 
 *	@param pBlock the block to render.
 *	@param pMaterial the material to use.
 *	@param drawMethod draw as a Z pass, single layer or multi layer.
 *	@param passCount Keeps track of how many passes have been drawn.
 *  	caller should set to zero for first pass on a single block.
 */
bool TerrainRenderer2::drawTerrainMaterial(	TerrainBlock2*			pBlock,
											BaseMaterial*			pMaterial, 
											DrawMethod				drawMethod,
											uint32&					passCount )
{
	BW_GUARD_PROFILER( TerrainRenderer2_drawTerrainMaterial );

	bool result = true;

	MF_ASSERT( pBlock );
	MF_ASSERT( pMaterial );

	// Grab the blends to be used throughout the render
	TerrainBlendsPtr pBlends = pBlock->currentDrawState_.blendsPtr_;

	// Update constants for this draw method
	updateConstants( pBlock, pBlends, pMaterial, drawMethod );

	// If we're doing Z_ONLY then remember the color write state.
	if ( drawMethod == DM_Z_ONLY )
	{
		Moo::rc().pushRenderState( D3DRS_COLORWRITEENABLE );
		Moo::rc().pushRenderState( D3DRS_COLORWRITEENABLE1 );

		Moo::rc().setWriteMask( 0, 0 );
		Moo::rc().setWriteMask( 1, 0 );
	}

	// work out whether to do stencil draw or not
	bool doStencilDraw = !Moo::rc().reflectionScene() && useStencilMasking_ &&
		!Terrain::Manager::instance().wireFrame();

	Moo::EffectMaterialPtr pEffectMaterial = pMaterial->pEffect_;

	if ( pEffectMaterial->begin() )
	{
		for ( uint32 i = 0; i < pEffectMaterial->nPasses(); i++ )
		{
			if ( pEffectMaterial->beginPass(i) )
			{
				// save stencil state for this pass before it begins
				Moo::rc().pushRenderState(D3DRS_STENCILENABLE);		

				// setup for single material
				if ( DM_BLEND == drawMethod && doStencilDraw )
				{
					if ( passCount == 0)
					{
						// For first pass, write STENCIL_OFFSET to the buffer, without
						// testing. This is because the stencil value is relevant only for
						// this particular block.
						Moo::rc().setRenderState(D3DRS_STENCILENABLE, TRUE);
						Moo::rc().setRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
						Moo::rc().setRenderState(D3DRS_STENCILMASK, 0xff);
						Moo::rc().setRenderState(D3DRS_STENCILPASS , D3DSTENCILOP_REPLACE);
						Moo::rc().setRenderState(D3DRS_STENCILREF, TERRAIN_STENCIL_OFFSET);
					}
					else
					{
						// For other passes, test and write alternate values.
						Moo::rc().setRenderState(D3DRS_STENCILENABLE, TRUE);
						Moo::rc().setRenderState(D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL);
						Moo::rc().setRenderState(D3DRS_STENCILMASK, 0xff);

						Moo::rc().setRenderState(D3DRS_STENCILFAIL , D3DSTENCILOP_KEEP);
						Moo::rc().setRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
						Moo::rc().setRenderState(D3DRS_STENCILPASS , D3DSTENCILOP_REPLACE);

						Moo::rc().setRenderState(D3DRS_STENCILREF, ((passCount%2)+TERRAIN_STENCIL_OFFSET));
					}

					passCount++;
				}
				
				// draw single material
				pBlock->draw( pEffectMaterial );
				
				// setup for combined layers
				if (DM_BLEND == drawMethod )
				{
					if ( doStencilDraw )
					{		
						// setup stencil mask
						Moo::rc().setRenderState(D3DRS_STENCILENABLE, TRUE);
						Moo::rc().setRenderState(D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL);
						Moo::rc().setRenderState(D3DRS_STENCILMASK, 0xff);

						Moo::rc().setRenderState(D3DRS_STENCILFAIL , D3DSTENCILOP_KEEP);
						Moo::rc().setRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
						Moo::rc().setRenderState(D3DRS_STENCILPASS , D3DSTENCILOP_REPLACE);
					}

					for (uint32 j = 1; j < pBlends->combinedLayers_.size(); j++)
					{
						if ( doStencilDraw )
						{
							// update alternate values
							Moo::rc().setRenderState(D3DRS_STENCILREF, 
								((passCount%2)+TERRAIN_STENCIL_OFFSET));
							passCount++;
						}

						// draw combined layers
						setTextureLayerConstants( pBlends, pMaterial, j, true );
						pEffectMaterial->commitChanges();
						result = pBlock->draw( pEffectMaterial ) && result;
					}
				}

				// restore old stencil state
				Moo::rc().popRenderState();

				// end pass
				pEffectMaterial->endPass();
			} // if begin pass
		} // for each pass

		pEffectMaterial->end();
	}

	if ( drawMethod == DM_Z_ONLY )
	{
		// reset color write state
		Moo::rc().popRenderState();
		Moo::rc().popRenderState();
	}

	return result;
}

/**
 *	This method adds a terrain block to the render list.
 *
 *	@param pBlock     The block to render
 *	@param transform  Transformation matrix to use on the terrain block.
 */
void TerrainRenderer2::addBlock(BaseRenderTerrainBlock*	pBlock,	
								const Matrix&			transform )
{
	BW_GUARD;
	TerrainBlock2* pTerrainBlock2 = static_cast<TerrainBlock2*>(pBlock);

    //We only want to cache lighting from the normal draw call.             
    //draw overrides are used for all sorts of things; shadows,             
    //projecting textures etc. and all have indeterminate lighting.         
    if (Moo::Visual::s_pDrawOverride == NULL)                               
            pTerrainBlock2->cacheCurrentLighting( specularEnabled() );

	Block block;

	block.transform = transform;
	block.block = pTerrainBlock2;
	block.depthOnly = Moo::rc().depthOnly();

	blocks_.push_back( block );                             

	isVisible_ = true;
}

/**
 *	This method renders all our cached terrain blocks
 *	@param pOverride overridden material
 *	@param clearList whether or not to erase the blocks from the render list
 */
void TerrainRenderer2::drawAll( Moo::EffectMaterialPtr pOverride, bool clearList )
{
	BW_GUARD;
	*Moo::EffectConstantValue::get( "SunAngle" ) = sunAngleSetter_;
	*Moo::EffectConstantValue::get( "PenumbraSize" ) = penumbraSizeSetter_;

	Moo::EffectVisualContext::instance().isOutside( true );

	BlockVector::iterator it = blocks_.begin();
	BlockVector::iterator end = blocks_.end();

	Moo::rc().push();

	BaseMaterial* pOverrideBaseMaterial = NULL;
	if ( pOverride )
	{
		overrideMaterial_.pEffect_ = pOverride;
		overrideMaterial_.getHandles();
		pOverrideBaseMaterial = &overrideMaterial_;
	}

	// Render each terrain block
	while (it != end)
	{
		// draw a single block with given transform, override material, and
		// use cached lighting state.

		drawSingleInternal( it->block.getObject(), it->transform, it->depthOnly,
							pOverrideBaseMaterial, true );
		if (clearList)
			it->block = NULL;
		++it;
	}

	if (clearList)
	{
		blocks_.clear();	
	}

	Moo::rc().pop();
}


/**
 *	This method clears all our cached blocks.  Usually this is done
 *	automatically in ::drawAll, however in some cases clear needs
 *	to be called explicitly (for example entity shadow casting detects
 *	shadows falling on outdoor terrain blocks and caches them, even
 *	if the terrain is not visible and the shadows won't be drawn.)
 */
void TerrainRenderer2::clearBlocks()
{
	BW_GUARD;
	BlockVector::iterator it = blocks_.begin();
	BlockVector::iterator end = blocks_.end();
	while (it != end)
	{
		it->block = NULL;
		++it;
	}

	blocks_.clear();
}

/**
 * Watch accessor function for terrain specular power.
 */
void TerrainRenderer2::specularPower( float power )
{
	BW_GUARD;
	specularInfo_.power_ = power;
	ID3DXEffect* pEffect = texturedMaterial_.pEffect_->pEffect()->pEffect();
	pEffect->SetFloat( texturedMaterial_.specularPower_,		
		specularInfo_.power_ );
}

/**
* Watch accessor function for terrain specular power.
*/
void TerrainRenderer2::specularMultiplier( float multiplier )
{
	BW_GUARD;
	specularInfo_.multiplier_ = multiplier;
	ID3DXEffect* pEffect = texturedMaterial_.pEffect_->pEffect()->pEffect();
	pEffect->SetFloat( texturedMaterial_.specularMultiplier_,		
		specularEnabled() ? specularInfo_.multiplier_ : 0.0f );
}

/**
* Watch accessor function for Fresnel exponent.
*/
void TerrainRenderer2::specularFresnelExp( float exp )
{
	BW_GUARD;
	specularInfo_.fresnelExp_ = exp;
	ID3DXEffect* pEffect = texturedMaterial_.pEffect_->pEffect()->pEffect();
	pEffect->SetFloat( texturedMaterial_.specularFresnelExp_,		
		specularInfo_.fresnelExp_ );
}

/**
* Watch accessor function for terrain Fresnel constant.
*/
void TerrainRenderer2::specularFresnelConstant( float fresnelConstant )
{
	BW_GUARD;
	specularInfo_.fresnelConstant_ = fresnelConstant;
	ID3DXEffect* pEffect = texturedMaterial_.pEffect_->pEffect()->pEffect();
	pEffect->SetFloat( texturedMaterial_.specularFresnelConstant_,
		specularInfo_.fresnelConstant_ );
}


/**
 *	Classes that contain EffectMaterials and their parameter handles
 */

void TerrainRenderer2::BaseMaterial::getHandles()
{
	BW_GUARD;
	ID3DXEffect* pEffect = pEffect_->pEffect()->pEffect();

	viewProj_ =
		pEffect->GetParameterByName( NULL, "viewProj" );
	world_ =
		pEffect->GetParameterByName( NULL, "world" );
	cameraPosition_ =
		pEffect->GetParameterByName( NULL, "cameraPosition" );

	normalMap_ =
		pEffect->GetParameterByName( NULL, "normalMap" );
	normalMapSize_ =
		pEffect->GetParameterByName( NULL, "normalMapSize" );

	horizonMap_ =
		pEffect->GetParameterByName( NULL, "horizonMap" );
	horizonMapSize_ =
		pEffect->GetParameterByName( NULL, "horizonMapSize" );

	holesMap_ =
		pEffect->GetParameterByName( NULL, "holesMap" );
	holesMapSize_ =
		pEffect->GetParameterByName( NULL, "holesMapSize" );
	holesSize_ =
		pEffect->GetParameterByName( NULL, "holesSize" );

	for ( uint i = 0; i < 4; i++ )
	{
		layer_[i] =
			pEffect->GetParameterByName( NULL, layerText[i] );
		layerUProjection_[i] =
			pEffect->GetParameterByName( NULL, layerUProjText[i] );
		layerVProjection_[i] =
			pEffect->GetParameterByName( NULL, layerVProjText[i] );
	}

	blendMap_ =
		pEffect->GetParameterByName( NULL, "blendMap" );
	blendMapSize_ =
		pEffect->GetParameterByName( NULL, "blendMapSize" );
	layerMask_ =
		pEffect->GetParameterByName( NULL, "layerMask" );

	lodTextureStart_ =
		pEffect->GetParameterByName( NULL, "lodTextureStart" );
	lodTextureDistance_ =
		pEffect->GetParameterByName( NULL, "lodTextureDistance" );

	useMultipassBlending_ =
		pEffect->GetParameterByName( NULL, "useMultipassBlending" );
	hasHoles_ =
		pEffect->GetParameterByName( NULL, "hasHoles" );
}

void TerrainRenderer2::TexturedMaterial::getHandles()
{
	BW_GUARD;
	BaseMaterial::getHandles();

	ID3DXEffect* pEffect = pEffect_->pEffect()->pEffect();

	specularPower_ =
		pEffect->GetParameterByName( NULL, "terrain2Specular.power" );
	specularMultiplier_ =
		pEffect->GetParameterByName( NULL, "terrain2Specular.multiplier" );
	specularFresnelExp_ =
		pEffect->GetParameterByName( NULL, "terrain2Specular.fresnelExp" );
	specularFresnelConstant_ =
		pEffect->GetParameterByName( NULL, "terrain2Specular.fresnelConstant" );
}

/**
 * Sets material parameter if it exists.
 */
void TerrainRenderer2::BaseMaterial::SetParam( D3DXHANDLE param, bool b )
{
	BW_GUARD;
	if ( param )
		pEffect_->pEffect()->pEffect()->SetBool( param, b );
}

void TerrainRenderer2::BaseMaterial::SetParam( D3DXHANDLE param, float f )
{
	BW_GUARD;
	if ( param )
		pEffect_->pEffect()->pEffect()->SetFloat( param, f );
}

void TerrainRenderer2::BaseMaterial::SetParam( D3DXHANDLE param, const Vector4* v )
{
	BW_GUARD;
	if ( param )
		pEffect_->pEffect()->pEffect()->SetVector( param, v );
}

void TerrainRenderer2::BaseMaterial::SetParam( D3DXHANDLE param, const Matrix* m )
{
	BW_GUARD;
	if ( param )
		pEffect_->pEffect()->pEffect()->SetMatrix( param, m );
}

void TerrainRenderer2::BaseMaterial::SetParam( D3DXHANDLE param, DX::BaseTexture* t )
{
	BW_GUARD;
	if ( param )
		pEffect_->pEffect()->pEffect()->SetTexture( param, t );
}

/**  
 * This method takes a partial texture flag result (with preload flag possibly
 * set), and returns full result needed for draw. 
 */
uint8 TerrainRenderer2::getDrawFlag( const	uint8 partialResult,
								  float minDistance, float maxDistance,
								  const TerrainSettingsPtr pSettings)
{
	BW_GUARD;
	MF_ASSERT_DEBUG( partialResult != 0 );

	uint8 fullResult = partialResult;

	// If we can't use lod texture, then flag to draw blends
	if ( !TerrainSettings::useLodTexture() )
	{
		return partialResult | TerrainRenderer2::RTM_DrawBlend;
	}

	// Set texture rendering flags. If the block straddles or is above the 
	// lod texture start it needs lod flag set. Also, if block is inside
	// the blend zone ( start + distance ) then it needs blend flag set.

	// test within lod texture range
	if ( maxDistance >= pSettings->lodTextureStart() && 
		minDistance < (pSettings->lodNormalStart() + pSettings->lodNormalDistance()) )
	{
		fullResult |= TerrainRenderer2::RTM_DrawLOD;
	}

	if (maxDistance >= pSettings->lodNormalStart())
		fullResult |= TerrainRenderer2::RTM_DrawLODNormals;

	// If the partial result says to preload the blend, it should be tested
	// to see if drawing is required.
	if ( partialResult & TerrainRenderer2::RTM_PreLoadBlend )
	{
		// test within blend texture range
		if ( minDistance <= pSettings->lodTextureStart() 
							+ pSettings->lodTextureDistance() )
		{
			fullResult |= TerrainRenderer2::RTM_DrawBlend;
		}
	}

	return	fullResult;
}


};
