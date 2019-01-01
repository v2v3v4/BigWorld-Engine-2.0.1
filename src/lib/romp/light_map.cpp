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
#include "light_map.hpp"
#include "cstdmf/debug.hpp"
#include "moo/effect_material.hpp"

DECLARE_DEBUG_COMPONENT2( "romp", 0 )


/**
 *	This class exposes the LightMap to the effect file engine. 
 */
class LightMapSetter : public Moo::EffectConstantValue
{
public:
	LightMapSetter( const std::string& tfn = "" )
	{
		this->textureFeedName( tfn );
	}

	~LightMapSetter()
	{
		value_ = NULL;
	}

	void textureFeedName( const std::string& tfn )
	{
		if (!tfn.empty())
			value_ = TextureFeeds::get( tfn );
		else
			value_ = NULL;
	}

	bool LightMapSetter::operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{
		DX::BaseTexture* pTex = NULL;
		if (value_)
		{
			pTex = value_->pTexture();
		}

		pEffect->SetTexture(constantHandle, pTex);
		
		return true;
	}

	Moo::BaseTexturePtr value_;	
};


/**
 *	Constructor.
 */
LightMap::LightMap( const std::string& className ):
	lastTime_( -1000.f ),
	timeTolerance_( 0.0167f ),	//roughly 1 minute game time
	width_( 64 ),
	height_( 64 ),
	pRT_( NULL ),
	lightMapSetter_( NULL ),
	transformSetter_( NULL ),
	textureProvider_( NULL ),
	renderTargetName_( className ),
	effectTextureName_( "LightMapEffectTexture" ),
	effectTransformName_( "LightMapEffectTransform" ),
	active_( false )
{	
}


/**
 * Destructor.
 */
LightMap::~LightMap()
{
	if ( textureProvider_.hasObject() )
	{
		Py_DECREF(textureProvider_.getObject());
		textureProvider_ = NULL;
	}

	pRT_=NULL;

	if (lightMapSetter_)
	{
		lightMapSetter_ = NULL;
	}

	DEBUG_MSG( "DELETING LIGHT MAP %lx\n", (long)this );
}


/**
 *	This method calculates the orthogonal projection matrix and returns
 *	it in the passed in Matrix parameter.
 */
void LightMap::orthogonalProjection(float xExtent, float zExtent, Matrix& m)
{	
	m.row( 0, Vector4(  2.f / xExtent ,  0,  0,  0 ) );
	m.row( 1, Vector4(  0,  0,  0,  0 ) );
	m.row( 2, Vector4(  0, -2.f / zExtent,  0,  0 ) );
	m.row( 3, Vector4( -1, 1,  0.1f,  1 ) );
}

void LightMap::linkTextures()
{
	//Note : must add texture feed before creating light setter.
	if (textureProvider_ && !renderTargetName_.empty())
	{
		TextureFeeds::addTextureFeed(renderTargetName_,textureProvider_);
	}
	if (!lightMapSetter_ && !effectTextureName_.empty())
	{
		this->createLightMapSetter( renderTargetName_ );
	}
	if (lightMapSetter_)
	{
		*Moo::EffectConstantValue::get( effectTextureName_ ) = lightMapSetter_;	
	}
}

void LightMap::deLinkTextures()
{
	if (textureProvider_ && !renderTargetName_.empty())
	{
		TextureFeeds::delTextureFeed(renderTargetName_);
	}

	if (lightMapSetter_)
	{
		*Moo::EffectConstantValue::get( effectTextureName_ ) = NULL;
		lightMapSetter_ = NULL;
	}
}


void LightMap::deleteUnmanagedObjects()
{
	if (active_)
	{
		deLinkTextures();
	}

	if ( textureProvider_.hasObject() )
	{
		Py_DECREF(textureProvider_.getObject());
		textureProvider_ = NULL;
	}

	pRT_ = NULL;
}

void LightMap::createUnmanagedObjects()
{
	if (pRT_ == NULL)
		pRT_ = new Moo::RenderTarget( renderTargetName_ );
	else
		pRT_->release();

	pRT_->clearOnRecreate( true, (DWORD)0x00000000 );
	pRT_->create(width_,height_);

	if (!textureProvider_ && pRT_)
	{
		textureProvider_ = new PyTextureProvider( NULL, pRT_ );
	}

	if (active_)
	{
		linkTextures();
	}
}

/**
 *	This method initialises the render target material for a
 *	light map.
 */
bool LightMap::init(const DataSectionPtr pSection)
{	
	if (!pSection)
	{
		ERROR_MSG( "LightMap::init - data section passed in was NULL\n" );
		return false;
	}

	width_ = pSection->readInt("width", width_);
	height_ = pSection->readInt("height", height_);
	float tts = timeTolerance_ * 3600.f;
	tts = pSection->readFloat("timeToleranceSecs", tts);	
	timeTolerance_ = tts / 3600.f;
	renderTargetName_ = pSection->readString("textureFeedName",renderTargetName_);

	if (!renderTargetName_.empty())
	{
		effectTextureName_ = pSection->readString( "effectTextureName",effectTextureName_ );		
	}
	effectTransformName_ = pSection->readString( "effectTransformName",effectTransformName_ );

	createUnmanagedObjects();

	return (pRT_->pTexture() != NULL);
}


/**
 *	This method sets the light map and transform setters on Moo,
 *	exposing this light map for use by effects.
 */
void LightMap::activate()
{	
	active_=true;

	linkTextures();

	if (!transformSetter_ && !effectTransformName_.empty())
	{
		this->createTransformSetter();
	}

	if (transformSetter_)
	{
		*Moo::EffectConstantValue::get( effectTransformName_ ) = transformSetter_;
	}
}


void LightMap::deactivate()
{
	active_ = false;

	deLinkTextures();

	if (transformSetter_)
	{
		*Moo::EffectConstantValue::get( effectTransformName_ ) = NULL;
		transformSetter_ = NULL;
	}
}


void LightMap::createLightMapSetter( const std::string& textureName )
{
	lightMapSetter_ = new LightMapSetter( textureName );
}


/**
 *	This method updates the known game time, and returns true
 *	if the light map needs updating.
 */
bool LightMap::needsUpdate(float gameTime)
{
	float diff = fabsf(gameTime-lastTime_);
	if (diff > timeTolerance_)
	{
		lastTime_ = gameTime;
		return true;
	}
	return false;
}


/**
 *	Constructor.
 */
EffectLightMap::EffectLightMap( const std::string& className ):
	LightMap( className ),
	material_(NULL)
{
}


/**
 *	This method initialises the render target and effect material for an
 *	effect-based light map.
 */
bool EffectLightMap::init( DataSectionPtr pSection )	
{
	if (!LightMap::init(pSection))
		return false;

	std::string mfmName = pSection->readString( "material" );
	if ( mfmName.empty() )
	{
		ERROR_MSG( "Missing mfm section in EffectLightMap data\n" );
		return false;
	}

	Moo::EffectMaterialPtr	material( new Moo::EffectMaterial() );
	DataSectionPtr pSect = BWResource::openSection( mfmName );
	if ( pSect )
	{
		material_ = material;
		return material_->load( pSect );		
	}
	else
	{
		ERROR_MSG( "Could not load light map effect %s", mfmName.c_str() );
		return false;
	}
}


/**
 *	This method sets the given projection matrix on the effect, using
 *	standard light map parameter naming.
 */
void EffectLightMap::setLightMapProjection(const Matrix& m)
{
	//for shader hardware
	ComObjectWrap<ID3DXEffect> pEffect = material_->pEffect()->pEffect();
	D3DXHANDLE h = pEffect->GetParameterByName(NULL,"lightMapProjection");
	if (h)
		pEffect->SetMatrix( h, &m );

	//and for fixed function hardware...
	h = pEffect->GetParameterByName(NULL,"lightMapWorld");
	if (h)
		pEffect->SetMatrix( h, &Matrix::identity );
	h = pEffect->GetParameterByName(NULL,"lightMapView");
	if (h)
		pEffect->SetMatrix( h, &Matrix::identity );
	h = pEffect->GetParameterByName(NULL,"lightMapProj");
	if (h)
		pEffect->SetMatrix( h, &m );

	pEffect->CommitChanges();
}
