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
#include "flora_renderer.hpp"
#include "flora_texture.hpp"
#include "flora_light_map.hpp"
#include "resmgr/auto_config.hpp"
#include "resmgr/bwresource.hpp"
#include "flora_constants.hpp"
#include "flora.hpp"
#include "time_of_day.hpp"
#include "enviro_minder.hpp"
#include "texture_feeds.hpp"
#include "math/colour.hpp"
#include "moo/fog_helper.hpp" 

DECLARE_DEBUG_COMPONENT2( "romp", 0 );

static AutoConfigString s_falloffTexture( "environment/floraFalloff" );
static AutoConfigString s_floraMFM( "environment/floraMaterial" );
static AutoConfigString s_floraShadowMFM( "environment/floraShadowMaterial" );

FloraRenderer::FloraRenderer( Flora* flora )
: capacity_( 0 ),
	pLocked_( NULL ),
	pContainer_( NULL ),
    animation_( 8, 8 ),
	material_( NULL ),
	shadowMaterial_( NULL ),
	lastBlockID_(-1),
	bufferSize_(0),
	lockFlags_(0),
	parameters_(NULL),
	flora_( flora ),
	lightMap_( flora )
{
	// Set up the materials
	material_ = new Moo::EffectMaterial;
	shadowMaterial_ = new Moo::EffectMaterial;
	DataSectionPtr pSection = BWResource::openSection( s_floraMFM );
	if (pSection)
	{
		material_->load(pSection);
		standardParams_.effect(material_->pEffect()->pEffect());
	}

	pSection = BWResource::openSection( s_floraShadowMFM );
	if (pSection)
	{
		if (shadowMaterial_->load(pSection))
		{
			shadowParams_.effect(shadowMaterial_->pEffect()->pEffect());
		}
		else
		{
			ERROR_MSG( "Flora Renderer : Could not load shadow material.\n" );
			shadowMaterial_ = NULL;
		}
	}
}


FloraRenderer::~FloraRenderer()
{
	deleteUnmanagedObjects();
}


void FloraRenderer::deleteUnmanagedObjects( )
{
	this->delVB();
}


void FloraRenderer::createUnmanagedObjects( )
{
	if (bufferSize_ != 0)
		this->createVB( bufferSize_, fvf_, lockFlags_ );
}


/** 
 * helper to create the vb and setting flags for locking it
 * 
 * @param bufferSize size of the vertex buffer
 * @param fvf the flexible vertex format to associate with the buffer
 * @param lockFlags the flags to use when locking the buffer
 * @return true if the successful
 */
bool FloraRenderer::createVB( uint32 bufferSize, DWORD fvf, DWORD lockFlags )
{
	if (pVB_.valid())
	{
		delVB();
	}

	HRESULT hr = pVB_.create( bufferSize, 
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, fvf, 
		D3DPOOL_DEFAULT, "vertex buffer/flora" );
	if (FAILED(hr))
	{
		if (hr ==  D3DERR_OUTOFVIDEOMEMORY)
			ERROR_MSG( "FloraRenderer::createVB - unable to create vertex buffer.  Out of Video Memory\n", hr );
		else
			ERROR_MSG( "FloraRenderer::createVB - unable to create vertex buffer.  Error code %lx\n", hr );
		pVB_.release();
		return false;
	}

	fvf_ = fvf;
	lockFlags_ = lockFlags;
	bufferSize_ = bufferSize;
	clear();
	return true;
}

/**
 * helper to delete the vb
 */
void FloraRenderer::delVB()
{
	pVB_.release();
}

/**
 * helper to clear the vb or a section of it
 * @param offset the first byte in the vb to clear
 * @param size the number of bytes to clear
 */
void FloraRenderer::clear( uint32 offset, uint32 size )
{
	uint32 oldFlags = DX::getWrapperFlags();
	DX::setWrapperFlags( oldFlags | DX::WRAPPER_FLAG_IMMEDIATE_LOCK );

	if (pVB_.valid())
	{
		if (size == 0 || (offset + size) > bufferSize_ )
			size = bufferSize_ - offset;
		Moo::SimpleVertexLock vl( pVB_, 0, 0, lockFlags_ );
        if (vl)
		{
			ZeroMemory( (char*)(void*)vl + offset, size );
		}
		else
		{
			ERROR_MSG( "FloraRenderer::clear - unable to lock vertex buffer\n" );
		}
	}
	else
	{
		ERROR_MSG( "FloraRenderer::clear - No vertex buffer to clear\n" );
	}

	DX::setWrapperFlags( oldFlags );
}

FloraVertexContainer* FloraRenderer::lock( uint32 firstVertex, uint32 nVertices )
{
	if (!pContainer_)
	{
		FloraVertexContainer* pVC = new FloraVertexContainer;

		void* pData;
		pVB_.lock( sizeof( FloraVertex ) * firstVertex, sizeof( FloraVertex ) * nVertices,
			&pData, lockFlags_ );
		FloraVertex* pVerts = (FloraVertex*)pData;

		pVC->init( pVerts, nVertices );
		pContainer_ = pVC;

		return pContainer_;
	}
	else
	{
		ERROR_MSG( "FloraVertexContainer::lock - Only allowed to lock once\n" );
	}
	return pContainer_;
}

bool FloraRenderer::unlock( FloraVertexContainer* pContainer )
{
	if (pContainer == pContainer_)
	{
		pContainer_ = NULL;
		delete pContainer;
		pVB_.unlock();
	}
	return true;
}

uint32 FloraRenderer::capacity()
{
	return capacity_;
}

bool FloraRenderer::preDraw( EnviroMinder& enviro )
{
	TimeOfDay* timeOfDay = enviro.timeOfDay();
	DX::Device* pDevice = Moo::rc().device();

	Moo::rc().setVertexDeclaration( pDecl_->declaration() );

	if (!material_->begin())
		return false;
	material_->beginPass(0);

	parameters_ = &standardParams_;
		
	uint32 ambient = Colour::getUint32FromNormalised( timeOfDay->lighting().ambientColour );
	Moo::rc().setRenderState( D3DRS_TEXTUREFACTOR, ambient );	
	parameters_->setVector( "ambient", &timeOfDay->lighting().ambientColour );

	pVB_.set( 0, 0, sizeof( FloraVertex ) );	

	// Set up fog start and end, so that the vertex shader output will be interpreted
	// correctly.

	Moo::FogHelper::setFog( Moo::rc().fogNear(), Moo::rc().fogFar(), 
 			D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR );


	if (Moo::rc().mixedVertexProcessing())
			Moo::rc().device()->SetSoftwareVertexProcessing(FALSE);

	Moo::rc().setRenderState( D3DRS_COLORWRITEENABLE, 
								D3DCOLORWRITEENABLE_RED |
								D3DCOLORWRITEENABLE_GREEN |
								D3DCOLORWRITEENABLE_BLUE );

	lastBlockID_ = -1;
	return true;
}

void FloraRenderer::postDraw()
{
	material_->endPass();
	material_->end();
	if (Moo::rc().mixedVertexProcessing())
			Moo::rc().device()->SetSoftwareVertexProcessing(TRUE);
}

void FloraRenderer::beginAlphaTestPass( float drawDistance, float fadePercent, uint32 alphaTestRef )
{
	//5 - visibility constants
	float vizFar = drawDistance;
	float vizNear = vizFar * (fadePercent /100.f);
	float vizRange = vizFar - vizNear;
	parameters_->setVector( "VISIBILITY", &Vector4( 1.f / vizRange, - vizNear / vizRange, 0, 0.f ) );		

	Moo::rc().setRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	Moo::rc().setRenderState( D3DRS_ALPHAREF, alphaTestRef );
	Moo::rc().setRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	Moo::rc().setRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	Moo::rc().setRenderState( D3DRS_ZWRITEENABLE, TRUE );		
	lastBlockID_ = -1;
}

void FloraRenderer::beginAlphaBlendPass( float drawDistance, float fadePercent )
{
	float vizFar = VISIBILITY;
	float vizNear = vizFar * (fadePercent /100.f);
	float vizRange = vizFar - vizNear;
	parameters_->setVector( "VISIBILITY", &Vector4( 1.f / vizRange, - vizNear / vizRange, 0, 0.f ) );		

	Moo::rc().setRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	Moo::rc().setRenderState( D3DRS_ALPHAREF, 0x00000001 );
	Moo::rc().setRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	Moo::rc().setRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	Moo::rc().setRenderState( D3DRS_ZWRITEENABLE, FALSE );
	lastBlockID_ = -1;
}

bool FloraRenderer::beginShadowPass( ShadowCasterPtr pShadowCaster, uint32 alphaTestRef )
{
	if (!shadowMaterial_)
		return false;

	Moo::rc().setVertexDeclaration( pDecl_->declaration() );
	pVB_.set( 0, 0, sizeof( FloraVertex ) );

	if (Moo::rc().mixedVertexProcessing())
		Moo::rc().device()->SetSoftwareVertexProcessing(FALSE);

	Moo::rc().setRenderState( D3DRS_COLORWRITEENABLE, 
								D3DCOLORWRITEENABLE_RED |
								D3DCOLORWRITEENABLE_GREEN |
								D3DCOLORWRITEENABLE_BLUE );

	pShadowCaster->setupConstants( *shadowMaterial_ );

	//The alpha blend, src blend etc. states are setup in the fx file for this.		
	if (!shadowMaterial_->begin())
		return false;
	shadowMaterial_->beginPass(0);	
	Moo::rc().setRenderState( D3DRS_ALPHAREF, alphaTestRef );	
	lastBlockID_ = -1;
	parameters_ = &shadowParams_;
	return true;
}

void FloraRenderer::endShadowPass()
{
	shadowMaterial_->endPass();
	shadowMaterial_->end();
}

void FloraRenderer::drawBlock( uint32 firstVertex, uint32 nVertices, 
	const FloraBlock& block )
{		
	if ( block.blockID() != lastBlockID_ )
	{				
		lastBlockID_ = block.blockID();		
		Matrix tr( flora_->transform(block) );
		parameters_->setMatrix( "vertexToWorld", &tr );
		parameters_->commitChanges();		
	}

	Moo::rc().drawPrimitive(
		D3DPT_TRIANGLELIST, firstVertex, nVertices / 3 );
}

bool FloraRenderer::init( uint32 bufferSize )
{
	bool result = this->createVB( bufferSize, 0, D3DLOCK_NOOVERWRITE);
	if (result)
	{
		this->clear();
		capacity_ = bufferSize / sizeof( FloraVertex );
	}
	pDecl_ = Moo::VertexDeclaration::get( "flora" );
	return result;
}

void FloraRenderer::fini( )
{
	this->delVB();
	capacity_ = 0;
}

void FloraRenderer::update( float dTime, EnviroMinder& enviro )
{
	animation_.update( dTime, enviro );
	lightMap_.update( enviro.timeOfDay()->gameTime() );
}

static bool canRender()
{
	return Moo::rc().vsVersion() >= 0x100 && 
		Moo::rc().deviceInfo( Moo::rc().deviceIndex() ).caps_.MaxSimultaneousTextures > 1;
}


/**
 *	Constructor
 */
FloraVertexContainer::FloraVertexContainer( FloraVertex* pVerts, uint32 nVerts )
{
    init( pVerts, nVerts );
}


/**
* This method inits the vertex container
*/
void FloraVertexContainer::init( FloraVertex* pVerts, uint32 nVerts )
{
	pVertsBase_ = pVerts;
	pVerts_ = pVerts;
	nVerts_ = nVerts;
}


/**
* This method adds a vertex to the vertex container
*/
void FloraVertexContainer::addVertex( const FloraVertex* pVertex, const Matrix* pTransform )
{
	MF_ASSERT( uint32(pVerts_ - pVertsBase_) < nVerts_ );
	pVerts_->set( *pVertex, pTransform );
    pVerts_->uv_[0] += uOffset_;
	pVerts_->uv_[1] += vOffset_;
	pVerts_->animation_[0] = pVertex->animation_[0];
	pVerts_->animation_[1] = (ANIM_TYPE)blockNum_;
	pVerts_++;
}

/**
* This method adds a list of vertices to the vertex container
*/
void FloraVertexContainer::addVertices( const FloraVertex* pVertices, uint32 count, const Matrix* pTransform )
{
	MF_ASSERT( uint32(pVerts_ - pVertsBase_) <= (nVerts_ - count) );
	const FloraVertex* it = pVertices;
	const FloraVertex* end = pVertices + count;
	while (it != end)
	{
		addVertex( it++, pTransform );
	}
}

void FloraVertexContainer::clear( uint32 nVertices )
{
	MF_ASSERT( nVertices <= (nVerts_ - (pVerts_ - pVertsBase_)) );
	ZeroMemory( pVerts_, nVertices * sizeof( FloraVertex ) );
	pVerts_ += nVertices;
}
