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
#include "terrain_renderer1.hpp"

#include "../manager.hpp"
#include "../terrain_texture_layer.hpp"
#include "moo/effect_visual_context.hpp"
#include "terrain_texture_setter.hpp"
#include "resmgr/auto_config.hpp"
#include "terrain_block1.hpp"

#include "cstdmf/watcher.hpp"

DECLARE_DEBUG_COMPONENT2( "Terrain", 0 )

static AutoConfigString s_fogGradientBmpName( "environment/fogGradientBmpName");
static BasicAutoConfig< float > s_terrainTextureSpacing( "environment/terrainTextureSpacing", 10.0f );
static AutoConfigString s_effectName( "system/terrainEffect" );


extern bool g_drawTerrain;

namespace
{
	bool g_firstTime_ = true;

	float s_penumbraSize = 0.2f;
	float s_terrainTilingFactor_ = -0.1f;

	/** 
	* Terrain 1 specular settings for watches.
	*/
	float s_specularDiffuseAmount	= 0.2f;
	float s_specularPower			= 16.f;
	float s_specularMultiplier		= 1.f;
	float s_specularFresnelExp		= 5.f;
	float s_specularFresnelConstant = 0.f;
} // anonymous namespace


namespace Terrain
{

// -----------------------------------------------------------------------------
// Section: Statics
// -----------------------------------------------------------------------------

TerrainRenderer1* TerrainRenderer1::s_instance_ = NULL;


// explicit casting method for float to dword bitwise conversion (ish).
inline DWORD toDWORD( float f )
{
	return *(DWORD*)(&f);
}

// -----------------------------------------------------------------------------
// Section: Helper Classes
// -----------------------------------------------------------------------------

class SunAngleConstantSetter : public Moo::EffectConstantValue
{
	bool SunAngleConstantSetter::operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
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

class PenumbraSizeSetter : public Moo::EffectConstantValue
{
	bool PenumbraSizeSetter::operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{		
		float psReciprocal = 1.f / s_penumbraSize;
		Vector4 value( psReciprocal, psReciprocal, psReciprocal, psReciprocal );
		pEffect->SetVector(constantHandle, &value);
		return true;
	}
};

class TextureTransformSetter : public Moo::EffectConstantValue
{
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{		
		Vector4 transform[2];
		transform[0].set( s_terrainTilingFactor_, 0.f, 0.f, 0.f );
		transform[1].set( 0.f, 0.f, s_terrainTilingFactor_, 0.f );
		pEffect->SetVectorArray(constantHandle, transform, 2);		
		return true;
	}
};

class FogTransformSetter : public Moo::EffectConstantValue
{
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{	
		// Calculate the fog transform matrix, we only want to worry about the z
		// component of the camera space position and map that onto our texture
		// according to fog begin and end.
		float fogBeg = Moo::rc().fogNear();
		float fogEnd = Moo::rc().fogFar();

		Matrix transform;
		transform.setIdentity();
		transform[0][0] = 0.f;
		transform[2][0] = 1.f / ( fogEnd - fogBeg );
		transform[3][0] = -fogBeg * transform[2][0];		

		pEffect->SetMatrix(constantHandle, &transform);
		return true;
	}
};

class FogTextureSetter : public Moo::EffectConstantValue
{
public:
	FogTextureSetter()
	{
		pFogGradientBmp_ = 
			Moo::TextureManager::instance()->get(s_fogGradientBmpName);
	}

	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{				
		if (pFogGradientBmp_ && pFogGradientBmp_->pTexture())
			pEffect->SetTexture(constantHandle,pFogGradientBmp_->pTexture());
		else
			WARNING_MSG( "Could not set fog gradient bitmap" );
		return true;
	}

	Moo::BaseTexturePtr pFogGradientBmp_;
};


// -----------------------------------------------------------------------------
// Section: EffectFileRenderer
// -----------------------------------------------------------------------------

/**
 * Constructor for the effect file renderer.
 */
TerrainRenderer1::EffectFileRenderer::EffectFileRenderer()
: material_( NULL ),
  textureSetter_( NULL )  
{
	BW_GUARD;
	// Create the effect.
	if ( Moo::rc().device() )
	{
		material_ = new Moo::EffectMaterial;
		DataSectionPtr pSect = BWResource::openSection( s_effectName );
		if (pSect && material_->load(pSect))
		{
			ComObjectWrap<ID3DXEffect> pEffect = material_->pEffect()->pEffect();
			pEffect->GetFloat( "specularDiffuseAmount", &s_specularDiffuseAmount );
			pEffect->GetFloat( "specularPower", &s_specularPower );
			pEffect->GetFloat( "specularMultiplier", &s_specularMultiplier );
			pEffect->GetFloat( "fresnelExp", &s_specularFresnelExp );
			pEffect->GetFloat( "fresnelConstant", &s_specularFresnelConstant );
			static bool watcherAdded = false;
			if ( !watcherAdded )
			{
				watcherAdded = true;
				MF_WATCH( "Render/Terrain/Terrain1/specular diffuse amount", s_specularDiffuseAmount,
						Watcher::WT_READ_WRITE,
						"Amount of diffuse colour to add to the specular colour." );
				MF_WATCH( "Render/Terrain/Terrain1/specular power", s_specularPower,
						Watcher::WT_READ_WRITE,
						"Mathematical power of the specular reflectance function." );
				MF_WATCH( "Render/Terrain/Terrain1/specular multiplier", s_specularMultiplier,
						Watcher::WT_READ_WRITE,
						"Overall multiplier on the terrain specular effect." );
				MF_WATCH( "Render/Terrain/Terrain1/specular fresnel constant", s_specularFresnelConstant,
						Watcher::WT_READ_WRITE,
						"Fresnel constant for falloff calculations." );
				MF_WATCH( "Render/Terrain/Terrain1/specular fresnel falloff", s_specularFresnelExp,
						Watcher::WT_READ_WRITE,
						"Fresnel falloff rate." );
			}
			textureSetter_ = new EffectFileTextureSetter(pEffect);
		}
		else
		{
			CRITICAL_MSG( "Moo::TerrainRenderer1::EffectFileRenderer::"
				"EffectFileRenderer - couldn't load effect material "
				"%s\n", s_effectName.value().c_str() );			
		}		
	}

	sunAngleSetter_ = new SunAngleConstantSetter;
	penumbraSizeSetter_ = new PenumbraSizeSetter;
	terrainTextureTransformSetter_ = new TextureTransformSetter;
	fogTextureTransformSetter_ = new FogTransformSetter;
	fogGradientTextureSetter_ = new FogTextureSetter;

	pDecl_ = Moo::VertexDeclaration::get( "xyznds" );
}


/**
 * Destructor.
 */
TerrainRenderer1::EffectFileRenderer::~EffectFileRenderer()
{
	BW_GUARD;
	if (textureSetter_)
		delete textureSetter_;	

	// clean up our auto variables
	*Moo::EffectConstantValue::get( "SunAngle" ) = NULL;
	*Moo::EffectConstantValue::get( "PenumbraSize" ) = NULL;
	*Moo::EffectConstantValue::get( "TerrainTextureTransform" ) = NULL;
	*Moo::EffectConstantValue::get( "FogTextureTransform" ) = NULL;
	*Moo::EffectConstantValue::get( "FogGradientTexture" ) = NULL;	
}

/**
 * This method is the implementation of Renderer::draw. Its the main draw function
 * for the terrain.
 * @param blocks vector of blocks to be rendered.
 * @param alternateMaterial an optional material with which to draw
 */
void TerrainRenderer1::EffectFileRenderer::draw( BlockVector& blocks,
	Moo::EffectMaterialPtr alternateMaterial )
{
	BW_GUARD;
	Moo::EffectMaterialPtr old = material_;
	if (alternateMaterial)
	{		
		material_ = alternateMaterial;
		textureSetter_->effect(material_->pEffect()->pEffect());
	}

	sortBlocks( blocks );	
	setInitialRenderStates();	
 	if (Moo::rc().mixedVertexProcessing())
 		Moo::rc().device()->SetSoftwareVertexProcessing(FALSE);
	renderBlocks( blocks );
 	if (Moo::rc().mixedVertexProcessing())
		Moo::rc().device()->SetSoftwareVertexProcessing(TRUE);

	
	if (alternateMaterial)
	{
		material_ = old;
		textureSetter_->effect(material_->pEffect()->pEffect());
	}
}


/**
 * This method will eventually sort the terrain blocks.
 */
void TerrainRenderer1::EffectFileRenderer::sortBlocks( BlockVector& blocks )
{
	//TODO: something clever!
}

/**
 *	This method sets render states that only need be set once for the whole
 *	terrain.
 */
void TerrainRenderer1::EffectFileRenderer::setInitialRenderStates()
{
	BW_GUARD;
	bool specular = TerrainRenderer1::instance()->specularEnabled();
	Moo::EffectVisualContext::instance().initConstants();
	Moo::rc().setVertexDeclaration( pDecl_->declaration() );
	ComObjectWrap<ID3DXEffect> pEffect = material_->pEffect()->pEffect();	
	SAFE_SET( pEffect, Matrix, "view", &Moo::rc().view() );
	SAFE_SET( pEffect, Matrix, "proj", &Moo::rc().projection() );
	SAFE_SET( pEffect, Float, "specularDiffuseAmount", s_specularDiffuseAmount );
	SAFE_SET( pEffect, Float, "specularPower", s_specularPower );
	SAFE_SET( pEffect, Float, "specularMultiplier", specular ? s_specularMultiplier : 0.f );
	SAFE_SET( pEffect, Float, "fresnelExp", specular ? s_specularFresnelExp : 0.f );
	SAFE_SET( pEffect, Float, "fresnelConstant", specular ? s_specularFresnelConstant : 0.f );
	if (Moo::rc().lightContainer())
		Moo::rc().lightContainer()->commitToFixedFunctionPipeline();
}

/**
 * This method calculates the texture transform for the fixed function pipeline.
 */
void TerrainRenderer1::EffectFileRenderer::textureTransform( const Matrix& world, Matrix& ret ) const
{
	BW_GUARD;
	ret.multiply( world, Moo::rc().view() );	
	ret.invert();
	Matrix rotatem;
	rotatem.setRotateX( DEG_TO_RAD( 90 ) );
	Matrix scalem;
	scalem.setScale( s_terrainTilingFactor_, s_terrainTilingFactor_, s_terrainTilingFactor_ );
	ret.postMultiply( rotatem );
	ret.postMultiply( scalem );
}

/**
 * This method iterates through the terrain blocks and renders them.
 * @param blocks vector of blocks to render.
 */
void TerrainRenderer1::EffectFileRenderer::renderBlocks( BlockVector& blocks )
{
	BW_GUARD;
	*Moo::EffectConstantValue::get( "SunAngle" ) = sunAngleSetter_;
	*Moo::EffectConstantValue::get( "PenumbraSize" ) = penumbraSizeSetter_;
	*Moo::EffectConstantValue::get( "TerrainTextureTransform" ) = terrainTextureTransformSetter_;
	*Moo::EffectConstantValue::get( "FogTextureTransform" ) = fogTextureTransformSetter_;
	*Moo::EffectConstantValue::get( "FogGradientTexture" ) = fogGradientTextureSetter_;

	ComObjectWrap<ID3DXEffect> pEffect = material_->pEffect()->pEffect();
	D3DXHANDLE wvp = pEffect->GetParameterByName(NULL,"worldViewProj");
	D3DXHANDLE vp = pEffect->GetParameterByName(NULL,"viewProj");
	D3DXHANDLE world = pEffect->GetParameterByName(NULL,"world");	
	D3DXHANDLE tex = pEffect->GetParameterByName(NULL,"textransform");

	if (material_->begin())
	{
		BlockVector::iterator it = blocks.begin();
		BlockVector::iterator end = blocks.end();
		
		while (it != end)
		{
			Matrix& m = it->first;

			/* for shader technique */
			Matrix full(m);
			full.postMultiply(Moo::rc().viewProjection());
			if (wvp)
				pEffect->SetMatrix( wvp, &full );

			/* for texture stage renderer*/
			if (world)
				pEffect->SetMatrix( world, &m );
			Matrix tt;
			this->textureTransform(m,tt);
			if (tex)
				pEffect->SetMatrix( tex, &tt );

			if (vp)
				pEffect->SetMatrix( vp, &Moo::rc().viewProjection() );

			pEffect->CommitChanges();
			
			for (uint32 i=0; i<material_->nPasses(); i++)
			{
				if (material_->beginPass(i))
				{
					it->second->draw(textureSetter_);
					material_->endPass();
				}
			}

			it++;			
		}				

		material_->end();		
	}
	blocks.clear();

	//have to do this because the fixed function effect file sets the transform
	//flags to 2, and must be disabled for every other object.
	Moo::rc().setTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	Moo::rc().setTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);

	//for shader 1.1 hardware, and the flora light map which also uses the alpha
	//channel and this renderer.
	Moo::rc().setRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_RED);
}



// -----------------------------------------------------------------------------
// Section: TerrainRenderer1
// -----------------------------------------------------------------------------

/*
 * Default constructor
 */
TerrainRenderer1::TerrainRenderer1()
{
	BW_GUARD;
	MF_ASSERT( Manager::pInstance() != NULL );
}


TerrainRenderer1::~TerrainRenderer1()
{
	BW_GUARD;
	s_instance_ = NULL;
	if (Manager::pInstance() != NULL && BaseTerrainRenderer::instance() == this)
		BaseTerrainRenderer::instance( NULL );
}


TerrainRenderer1* TerrainRenderer1::instance()
{
	BW_GUARD;
	MF_ASSERT( s_instance_ != NULL );
	return s_instance_;
}


bool TerrainRenderer1::init()
{
	BW_GUARD;
	if ( s_instance_ != NULL )
		return true;

	s_instance_ = new TerrainRenderer1;

	//destroy the previous terrain before initialising the new one.
	BaseTerrainRenderer::instance( s_instance_ );

	if ( !s_instance_->initInternal() )
	{
		delete s_instance_;
		s_instance_ = NULL;
		return false;
	}

	return true;
}

bool TerrainRenderer1::initInternal()
{
	BW_GUARD;
	if ( s_terrainTilingFactor_ < 0.f )
	{
		float scale = s_terrainTextureSpacing.value();
		scale = Math::clamp( 0.01f, scale, 100.f );
		s_terrainTilingFactor_ = 1.f / scale;
		if ( g_firstTime_ )
		{
			MF_WATCH( "Render/Terrain/Terrain1/texture tile frequency",
					s_terrainTilingFactor_, Watcher::WT_READ_WRITE,
					"Frequency of terrain textures, or, how many textures per metre. "
					"(e.g. 0.1 means 1 texture repeat every 10 metres)." );
			g_firstTime_ = false;
		}
	}

	// moved initialisation to constructor 
	// to avoid loading files while the game 
	// simulation is already running.
	renderer_ = new EffectFileRenderer;

	return true;
}

void TerrainRenderer1::drawSingle(BaseRenderTerrainBlock*	pBlock,	
								  const Matrix&				transform, 
								  Moo::EffectMaterialPtr	pOverrideMaterial,
								  bool )
{
	BW_GUARD;
	TerrainBlock1* pTerrainBlock1 = static_cast<TerrainBlock1*>(pBlock);

	if (g_drawTerrain)
	{
		BlockVector blocks;
		blocks.push_back( BlockInPlace( transform, pTerrainBlock1 ) );
		renderer_->draw( blocks, pOverrideMaterial );
	}
}

void TerrainRenderer1::addBlock(BaseRenderTerrainBlock*	pBlock,	
								const Matrix&			transform )
{
	BW_GUARD;
	TerrainBlock1* pTerrainBlock1 = static_cast<TerrainBlock1*>(pBlock);

    //We only want to cache lighting from the normal draw call.             
    //draw overrides are used for all sorts of things; shadows,             
    //projecting textures etc. and all have indeterminate lighting.         
    if (Moo::Visual::s_pDrawOverride == NULL)                               
            pTerrainBlock1->cacheCurrentLighting( specularEnabled() );

	blocks_.push_back( BlockInPlace( transform, pTerrainBlock1 ) );

	isVisible_ = true;
}

/**
 *	This method renders all our cached terrain blocks
 *	@param pOverride overridden material
 *	@param clearList whether or not to erase the blocks from the render list
 */
void TerrainRenderer1::drawAll( Moo::EffectMaterialPtr pOverride, bool clearList )
{
	BW_GUARD;
	MF_ASSERT( renderer_ != NULL );

	Moo::EffectVisualContext::instance().isOutside( true );

	if (blocks_.size() && Moo::rc().device() && g_drawTerrain)
	{
		renderer_->draw( blocks_, pOverride);
	}

	if ( clearList )
	{
		blocks_.clear();
	}
}


/**
 *	This method clears all our cached blocks.  Usually this is done
 *	automatically in ::drawAll, however in some cases clear needs
 *	to be called explicitly (for example entity shadow casting detects
 *	shadows falling on outdoor terrain blocks and caches them, even
 *	if the terrain is not visible and the shadows won't be drawn.)
 */
void TerrainRenderer1::clearBlocks()
{
	BW_GUARD;
	BlockVector::iterator it = blocks_.begin();
	BlockVector::iterator end = blocks_.end();
	while (it != end)
	{
		it->second = NULL;
		++it;
	}

	blocks_.clear();
}

};
