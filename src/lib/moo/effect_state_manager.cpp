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
#include "effect_state_manager.hpp"

DECLARE_DEBUG_COMPONENT2( "Moo", 0 );

extern void * gRenderTgtTexture;

namespace Moo
{

PROFILER_DECLARE( StateManager_SetPixelShader, "StateManager SetPixelShader" );
PROFILER_DECLARE( StateManager_SetPixelShaderConstant, "StateManager SetPixelShaderConstant" );
PROFILER_DECLARE( StateManager_SetVertexShader, "StateManager SetVertexShader" );
PROFILER_DECLARE( StateManager_SetVertexShaderConstant, "StateManager SetVertexShaderConstant" );

/*
 * Com QueryInterface method
 */
HRESULT __stdcall StateManager::QueryInterface( REFIID iid, LPVOID *ppv)
{
    if (iid == IID_IUnknown || iid == IID_ID3DXEffectStateManager)
    {
        *ppv = static_cast<ID3DXEffectStateManager*>(this);
    } 
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}

/*
 * Com AddRef method
 */
ULONG __stdcall StateManager::AddRef()
{
    this->incRef();
	return this->refCount();
}

/*
 * Com Release method
 */
ULONG __stdcall StateManager::Release()
{
    this->decRef();
	return this->refCount();
}

/*
 *	LightEnable
 */
HRESULT __stdcall StateManager::LightEnable( DWORD index, BOOL enable )
{
	return rc().device()->LightEnable( index, enable );
}

/*
 *	SetFVF
 */
HRESULT __stdcall StateManager::SetFVF( DWORD fvf )
{
	return rc().setFVF( fvf );
}

/*
 *	SetLight
 */
HRESULT __stdcall StateManager::SetLight( DWORD index, CONST D3DLIGHT9* pLight )
{
	return rc().device()->SetLight( index, pLight );
}

/*
 *	SetMaterial
 */
HRESULT __stdcall StateManager::SetMaterial( CONST D3DMATERIAL9* pMaterial )
{
	return rc().device()->SetMaterial( pMaterial );
}

/*
 *	SetNPatchMode
 */
HRESULT __stdcall StateManager::SetNPatchMode( FLOAT nSegments )
{
	return rc().device()->SetNPatchMode( nSegments );
}

/*
 *	SetPixelShader
 */
HRESULT __stdcall StateManager::SetPixelShader( LPDIRECT3DPIXELSHADER9 pShader )
{
	PROFILER_SCOPED( StateManager_SetPixelShader );
	return rc().device()->SetPixelShader( pShader );
}

/*
 *	SetPixelShaderConstantB
 */
HRESULT __stdcall StateManager::SetPixelShaderConstantB( UINT reg, CONST BOOL* pData, UINT count )
{
	PROFILER_SCOPED( StateManager_SetPixelShaderConstant );
	return rc().device()->SetPixelShaderConstantB( reg, pData, count );
}

/*
 *	SetPixelShaderConstantF
 */
HRESULT __stdcall StateManager::SetPixelShaderConstantF( UINT reg, CONST FLOAT* pData, UINT count )
{
	PROFILER_SCOPED( StateManager_SetPixelShaderConstant );
	return rc().device()->SetPixelShaderConstantF( reg, pData, count );

}

/*
 *	SetPixelShaderConstantI
 */
HRESULT __stdcall StateManager::SetPixelShaderConstantI( UINT reg, CONST INT* pData, UINT count )
{
	PROFILER_SCOPED( StateManager_SetPixelShaderConstant );
	return rc().device()->SetPixelShaderConstantI( reg, pData, count );
}

/*
 *	SetVertexShader
 */
HRESULT __stdcall StateManager::SetVertexShader( LPDIRECT3DVERTEXSHADER9 pShader )
{
	PROFILER_SCOPED( StateManager_SetVertexShader );
	return rc().setVertexShader( pShader );
}

/*
 *	SetVertexShaderConstantB
 */
HRESULT __stdcall StateManager::SetVertexShaderConstantB( UINT reg, CONST BOOL* pData, UINT count )
{
	PROFILER_SCOPED( StateManager_SetVertexShaderConstant );
	return rc().device()->SetVertexShaderConstantB( reg, pData, count );
}

/*
 *	SetVertexShaderConstantF
 */
HRESULT __stdcall StateManager::SetVertexShaderConstantF( UINT reg, CONST FLOAT* pData, UINT count )
{
	PROFILER_SCOPED( StateManager_SetVertexShaderConstant );
	HRESULT hr;
	hr = rc().device()->SetVertexShaderConstantF( reg, pData, count );
	return hr;
}

/*
 *	SetVertexShaderConstantI
 */
HRESULT __stdcall StateManager::SetVertexShaderConstantI( UINT reg, CONST INT* pData, UINT count )
{
	PROFILER_SCOPED( StateManager_SetVertexShaderConstant );
	return rc().device()->SetVertexShaderConstantI( reg, pData, count );
}

/*
 *	SetRenderState
 */
HRESULT __stdcall StateManager::SetRenderState( D3DRENDERSTATETYPE state, DWORD value )
{
	if (Moo::rc().mirroredTransform() && state == D3DRS_CULLMODE)
		value = value ^ (value >> 1);
	return rc().setRenderState( state, value );
}

/*
 *	SetVertexSamplerState
 */
HRESULT __stdcall StateManager::SetSamplerState( DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value )
{
	return rc().setSamplerState( sampler, type, value );
}

/*
 *	SetTexture
 */
HRESULT __stdcall StateManager::SetTexture( DWORD stage, LPDIRECT3DBASETEXTURE9 pTexture)
{
	return rc().setTexture( stage, pTexture );
}

/*
 *	SetTextureStageState
 */
HRESULT __stdcall StateManager::SetTextureStageState( DWORD stage, D3DTEXTURESTAGESTATETYPE type,
	DWORD value )
{
	return rc().setTextureStageState( stage, type, value );
}

/*
 *	SetTransform
 */
HRESULT __stdcall StateManager::SetTransform( D3DTRANSFORMSTATETYPE state, CONST D3DMATRIX* pMatrix )
{
	return rc().device()->SetTransform( state, pMatrix );
}


// The constant allocator typedefs
typedef ConstantAllocator<BOOL> BoolAllocator;
typedef ConstantAllocator<Vector4> Vector4Allocator;
typedef ConstantAllocator<IntVector4> IntVector4Allocator;

/*
 *	Record LightEnable state
 */
HRESULT __stdcall StateRecorder::LightEnable( DWORD index, BOOL enable )
{
	BW_GUARD;

	lightEnable_.push_back( std::make_pair( index, enable ) );
	return S_OK;
}

/*
 *	Record FVF
 */
HRESULT __stdcall StateRecorder::SetFVF( DWORD fvf )
{
	BW_GUARD;

	fvf_.first = true;
	fvf_.second = fvf;
	return S_OK;
}

/*
 *	Record Light
 */
HRESULT __stdcall StateRecorder::SetLight( DWORD index, CONST D3DLIGHT9* pLight )
{
	BW_GUARD;

	lights_.push_back( std::make_pair( index, *pLight ) );
	return S_OK;
}

/*
 *	Record Material
 */
HRESULT __stdcall StateRecorder::SetMaterial( CONST D3DMATERIAL9* pMaterial )
{
	BW_GUARD;

	material_.first = true;
	material_.second = *pMaterial;
	return S_OK;
}

/*
 *	Record NPatchMode
 */
HRESULT __stdcall StateRecorder::SetNPatchMode( FLOAT nSegments )
{
	BW_GUARD;

	nPatchMode_.first = true;
	nPatchMode_.second = nSegments;
	return S_OK;
}

/*
 *	Record PixelShader
 */
HRESULT __stdcall StateRecorder::SetPixelShader( LPDIRECT3DPIXELSHADER9 pShader )
{
	BW_GUARD;

	pixelShader_.first = true;
	pixelShader_.second = pShader;
	return S_OK;
}

/*
 *	Record bool pixel shader constant
 */
HRESULT __stdcall StateRecorder::SetPixelShaderConstantB( UINT reg, CONST BOOL* pData, UINT count )
{
	BW_GUARD;

	pixelShaderConstantsB_.push_back( std::make_pair( reg, BoolAllocator::instance().init( pData, count ) ) );
	return S_OK;
}

/*
 *	Record float pixel shader constant
 */
HRESULT __stdcall StateRecorder::SetPixelShaderConstantF( UINT reg, CONST FLOAT* pData, UINT count )
{
	BW_GUARD;

	pixelShaderConstantsF_.push_back( std::make_pair( reg, Vector4Allocator::instance().init( (const Vector4*)pData, count ) ) );
	return S_OK;
}

/*
 *	Record int pixel shader constant
 */
HRESULT __stdcall StateRecorder::SetPixelShaderConstantI( UINT reg, CONST INT* pData, UINT count )
{
	BW_GUARD;

	pixelShaderConstantsI_.push_back( std::make_pair( reg, IntVector4Allocator::instance().init( (const IntVector4*)pData, count ) ) );
	return S_OK;
}

/*
 *	Record vertex shader
 */
HRESULT __stdcall StateRecorder::SetVertexShader( LPDIRECT3DVERTEXSHADER9 pShader )
{
	BW_GUARD;

	vertexShader_.first = true;
	vertexShader_.second = pShader;
	return S_OK;
}

/*
 *	Record bool vertex shader constant
 */
HRESULT __stdcall StateRecorder::SetVertexShaderConstantB( UINT reg, CONST BOOL* pData, UINT count )
{
	BW_GUARD;

	vertexShaderConstantsB_.push_back( std::make_pair( reg, BoolAllocator::instance().init( pData, count ) ) );
	return S_OK;
}

/*
 *	Record float vertex shader constant
 */
HRESULT __stdcall StateRecorder::SetVertexShaderConstantF( UINT reg, CONST FLOAT* pData, UINT count )
{
	BW_GUARD;

	vertexShaderConstantsF_.push_back( std::make_pair( reg, Vector4Allocator::instance().init( (const Vector4*)pData, count ) ) );
	return S_OK;
}

/*
 *	Record int vertex shader constant
 */
HRESULT __stdcall StateRecorder::SetVertexShaderConstantI( UINT reg, CONST INT* pData, UINT count )
{
	BW_GUARD;

	vertexShaderConstantsI_.push_back( std::make_pair( reg, IntVector4Allocator::instance().init( (const IntVector4*)pData, count ) ) );
	return S_OK;
}

/*
 *	Record render state
 */
HRESULT __stdcall StateRecorder::SetRenderState( D3DRENDERSTATETYPE state, DWORD value )
{
	BW_GUARD;

	static RenderState rs;

	if (Moo::rc().mirroredTransform() && state == D3DRS_CULLMODE)
		value = value ^ (value >> 1);

	rs.state = state;
	rs.value = value;
	renderStates_.push_back( rs );
	
	return S_OK;
}

/*
 *	Record sampler state
 */
HRESULT __stdcall StateRecorder::SetSamplerState( DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value )
{
	BW_GUARD;

	static SamplerState ss;
	ss.sampler = sampler;
	ss.type = type;
	ss.value = value;
	samplerStates_.push_back( ss );
	return S_OK;
}



/*
 *	Record texture
 */
HRESULT __stdcall StateRecorder::SetTexture( DWORD stage, LPDIRECT3DBASETEXTURE9 pTexture )
{
	BW_GUARD;

	textures_.push_back( std::make_pair( stage, pTexture ) );
	return S_OK;
}

/*
 *	Record texture stage state
 */
HRESULT __stdcall StateRecorder::SetTextureStageState( DWORD stage, D3DTEXTURESTAGESTATETYPE type,
	DWORD value )
{
	BW_GUARD;

	static TextureStageState tss;
	tss.stage = stage;
	tss.type = type;
	tss.value = value;
	textureStageStates_.push_back( tss );
	return S_OK;
}

/*
 *	Record transform
 */
HRESULT __stdcall StateRecorder::SetTransform( D3DTRANSFORMSTATETYPE state, CONST D3DMATRIX* pMatrix )
{
	BW_GUARD;

	transformStates_.push_back( std::make_pair( state, *pMatrix ) );
	return S_OK;
}

/*
 *	Helper method for setting shader constants
 */
template <class Container, class InputFormat >
void setConstants( const Container& container, 
	HRESULT ( __stdcall DX::Device::*functor)(UINT, InputFormat*, UINT) )
{
	BW_GUARD;

	Container::const_iterator it = container.begin();
	Container::const_iterator end = container.end();
	while (it != end)
	{
		(rc().device()->*functor)( it->first, (InputFormat*)it->second->data(), it->second->size() );
		it++;
	}
}

/*
 *	Flush the recorded states, constants etc.
 */
void StateRecorder::setStates()
{
	BW_GUARD;

	DX::Device* pDev = rc().device();

	if (fvf_.first) { rc().setFVF( fvf_.second ); }
	if (vertexShader_.first) { rc().setVertexShader( vertexShader_.second.pComObject() ); }
	if (pixelShader_.first) { pDev->SetPixelShader( pixelShader_.second.pComObject() ); }

	setConstants( vertexShaderConstantsF_, &DX::Device::SetVertexShaderConstantF );
	setConstants( vertexShaderConstantsI_, &DX::Device::SetVertexShaderConstantI );
	setConstants( vertexShaderConstantsB_, &DX::Device::SetVertexShaderConstantB );

	setConstants( pixelShaderConstantsF_, &DX::Device::SetPixelShaderConstantF );
	setConstants( pixelShaderConstantsI_, &DX::Device::SetPixelShaderConstantI );
	setConstants( pixelShaderConstantsB_, &DX::Device::SetPixelShaderConstantB );

	setRenderStates();
	setTextureStageStates();
	setSamplerStates();
	setTransforms();
	setTextures();
	setLights();

	if (material_.first) { pDev->SetMaterial( &material_.second ); };
	if (nPatchMode_.first) { pDev->SetNPatchMode( nPatchMode_.second ); }
}

/*
 *	Init the state recorder, 
 */
void StateRecorder::init()
{
	BW_GUARD;

	vertexShaderConstantsF_.clear();
	vertexShaderConstantsI_.clear();
	vertexShaderConstantsB_.clear();

	pixelShaderConstantsF_.clear();
	pixelShaderConstantsI_.clear();
	pixelShaderConstantsB_.clear();

	renderStates_.clear();
	textureStageStates_.clear();
	samplerStates_.clear();

	transformStates_.clear();
	textures_.clear();
	lightEnable_.clear();
	lights_.clear();

	vertexShader_.first = false;
	vertexShader_.second = NULL;
	pixelShader_.first = false;
	pixelShader_.second = NULL;
	fvf_.first = false;
	material_.first = false;
	nPatchMode_.first = false;
}

/*
 *	Flush the render states
 */
void StateRecorder::setRenderStates()
{
	BW_GUARD;

	RenderStates::iterator it = renderStates_.begin();
	RenderStates::iterator end = renderStates_.end();
	while (it != end)
	{
		rc().setRenderState( it->state, it->value );
		it++;
	}
}

/*
 *	Flush the texture stage states
 */
void StateRecorder::setTextureStageStates()
{
	BW_GUARD;

	TextureStageStates::iterator it = textureStageStates_.begin();
	TextureStageStates::iterator end = textureStageStates_.end();
	while (it != end)
	{
		rc().setTextureStageState( it->stage, it->type, it->value );
		it++;
	}
}

/*
 *	Flush the sampler states
 */
void StateRecorder::setSamplerStates()
{
	BW_GUARD;

	SamplerStates::iterator it = samplerStates_.begin();
	SamplerStates::iterator end = samplerStates_.end();
	while (it != end)
	{
		rc().setSamplerState( it->sampler, it->type, it->value );
		it++;
	}
}

/*
 *	Flush the transforms
 */
void StateRecorder::setTransforms()
{
	BW_GUARD;

	DX::Device* pDev = rc().device();
	TransformStates::iterator it = transformStates_.begin();
	TransformStates::iterator end = transformStates_.end();
	while (it != end)
	{
		pDev->SetTransform( it->first, &it->second );
		it++;
	}
}

/*
 *	Flush the textures
 */
void StateRecorder::setTextures()
{
	BW_GUARD;

	Textures::iterator it = textures_.begin();
	Textures::iterator end = textures_.end();
	while (it != end)
	{
		rc().setTexture( it->first, it->second.pComObject() );
		it++;
	}
}

/*
 *	Flush the lights
 */
void StateRecorder::setLights()
{
	BW_GUARD;

	DX::Device* pDev = rc().device();
	LightStates::iterator it = lightEnable_.begin();
	LightStates::iterator end = lightEnable_.end();

	while (it != end)
	{
		pDev->LightEnable( it->first, it->second );
		it++;
	}

	Lights::iterator lit = lights_.begin();
	Lights::iterator lend = lights_.end();

	while (lit != lend)
	{
		pDev->SetLight( lit->first, &lit->second );
		lit++;
	}
	

}

/**
 *	This static method gets a new staterecorder that is valid until the next frame.
 *	@return new state recorder
 */
StateRecorder* StateRecorder::get()
{
	BW_GUARD;

	static uint32 timeStamp = rc().frameTimestamp();
	if (!rc().frameDrawn( timeStamp ))
		s_nextAlloc_ = 0;

	if (s_nextAlloc_ == s_stateRecorders_.size())
		s_stateRecorders_.push_back( new StateRecorder );
	return s_stateRecorders_[s_nextAlloc_++].getObject();
}

/**
 *	This static method clears out any recorded state, and drops all the
 *	resource references that they hold.
 */
void StateRecorder::clear()
{
	BW_GUARD;

	s_stateRecorders_.clear();
	s_nextAlloc_ = 0;
}

uint32 StateRecorder::s_nextAlloc_ = 0;
std::vector< SmartPointer<StateRecorder> > StateRecorder::s_stateRecorders_;

//-----------------------------------------------------------------------------
// State recorder apply state functions
//-----------------------------------------------------------------------------

/*
 *	Record render state
 *  Override from normal state recorder so we use the deferred state of the
 *  mirror transform instead of the one from the render context
 */
HRESULT __stdcall WrapperStateRecorder::SetRenderState( D3DRENDERSTATETYPE state, DWORD value )
{
	BW_GUARD;

	static RenderState rs;

	if (gdx_mirroredTransform && state == D3DRS_CULLMODE)
		value = value ^ (value >> 1);

	rs.state = state;
	rs.value = value;
	renderStates_.push_back( rs );
	
	return S_OK;
}

/*
 *	Helper method for setting shader constants
 */
template <class Container, class InputFormat >
void wrapperSetConstants( const Container& container, 
	HRESULT ( __stdcall WrapperStateManager::*functor)(UINT, InputFormat*, UINT) )
{
	BW_GUARD;

	Container::const_iterator it = container.begin();
	Container::const_iterator end = container.end();
	while (it != end)
	{
		(WrapperStateManager::s_stateManager->*functor)( it->first, (InputFormat*)it->second->data(), it->second->size() );
		it++;
	}
}

static void playStateRecorderSetStates()
{
	BW_GUARD;

	WrapperStateRecorder* stateRecorder = gdx_commandBuffer->read<WrapperStateRecorder*>();
	stateRecorder->setStatesReal();

	// We don't need the state recorder any more so retire it
	WrapperStateRecorder::put( stateRecorder );
}

void WrapperStateRecorder::setStates()
{
	BW_GUARD;

	rc().invalidateStateCache();
	gdx_commandBuffer->write( playStateRecorderSetStates, this );
}

void WrapperStateRecorder::setStatesReal()
{
	BW_GUARD;

	if (fvf_.first) { WrapperStateManager::s_stateManager->SetFVF( fvf_.second ); }
	if (vertexShader_.first) { WrapperStateManager::s_stateManager->SetVertexShader( vertexShader_.second.pComObject() ); }
	if (pixelShader_.first) { WrapperStateManager::s_stateManager->SetPixelShader( pixelShader_.second.pComObject() ); }

	wrapperSetConstants( vertexShaderConstantsF_, &WrapperStateManager::SetVertexShaderConstantF );
	wrapperSetConstants( vertexShaderConstantsI_, &WrapperStateManager::SetVertexShaderConstantI );
	wrapperSetConstants( vertexShaderConstantsB_, &WrapperStateManager::SetVertexShaderConstantB );

	wrapperSetConstants( pixelShaderConstantsF_, &WrapperStateManager::SetPixelShaderConstantF );
	wrapperSetConstants( pixelShaderConstantsI_, &WrapperStateManager::SetPixelShaderConstantI );
	wrapperSetConstants( pixelShaderConstantsB_, &WrapperStateManager::SetPixelShaderConstantB );

	setRenderStates();
	setTextureStageStates();
	setSamplerStates();
	setTransforms();
	setTextures();
	setLights();

	if (material_.first) { WrapperStateManager::s_stateManager->SetMaterial( &material_.second ); };
	if (nPatchMode_.first) { WrapperStateManager::s_stateManager->SetNPatchMode( nPatchMode_.second ); }
}

/*
 *	Flush the render states
 */
void WrapperStateRecorder::setRenderStates()
{
	BW_GUARD;

	RenderStates::iterator it = renderStates_.begin();
	RenderStates::iterator end = renderStates_.end();
	while (it != end)
	{
		// Bypass the WrapperStateManager as we have already taken
		// the mirror flag into account.
		if ( it->value != gdx_renderStateCache[it->state] )
		{
			gdx_renderStateCache[it->state] = it->value;
			WrapperStateManager::s_stateManager->device_->SetRenderState(it->state,it->value);
		}

		it++;
	}
}

/*
 *	Flush the texture stage states
 */
void WrapperStateRecorder::setTextureStageStates()
{
	BW_GUARD;

	TextureStageStates::iterator it = textureStageStates_.begin();
	TextureStageStates::iterator end = textureStageStates_.end();
	while (it != end)
	{
		WrapperStateManager::s_stateManager->SetTextureStageState( it->stage, it->type, it->value );
		it++;
	}
}

/*
 *	Flush the sampler states
 */
void WrapperStateRecorder::setSamplerStates()
{
	BW_GUARD;

	SamplerStates::iterator it = samplerStates_.begin();
	SamplerStates::iterator end = samplerStates_.end();
	while (it != end)
	{
		WrapperStateManager::s_stateManager->SetSamplerState( it->sampler, it->type, it->value );
		it++;
	}
}

/*
 *	Flush the transforms
 */
void WrapperStateRecorder::setTransforms()
{
	BW_GUARD;

	TransformStates::iterator it = transformStates_.begin();
	TransformStates::iterator end = transformStates_.end();
	while (it != end)
	{
		WrapperStateManager::s_stateManager->SetTransform( it->first, &it->second );
		it++;
	}
}

/*
 *	Flush the textures
 */
void WrapperStateRecorder::setTextures()
{
	BW_GUARD;

	Textures::iterator it = textures_.begin();
	Textures::iterator end = textures_.end();
	while (it != end)
	{
		WrapperStateManager::s_stateManager->SetTexture( it->first, it->second.pComObject() );
		it++;
	}
}

/*
 *	Flush the lights
 */
void WrapperStateRecorder::setLights()
{
	BW_GUARD;

	LightStates::iterator it = lightEnable_.begin();
	LightStates::iterator end = lightEnable_.end();

	while (it != end)
	{
		WrapperStateManager::s_stateManager->LightEnable( it->first, it->second );
		it++;
	}

	Lights::iterator lit = lights_.begin();
	Lights::iterator lend = lights_.end();

	while (lit != lend)
	{
		WrapperStateManager::s_stateManager->SetLight( lit->first, &lit->second );
		lit++;
	}
}



/**
 *	This static method retires the recorders on the list
 */

void WrapperStateRecorder::retireRecorders()
{
	BW_GUARD;

	for ( uint i = 0; i < s_retireList_.size(); i++ )
	{
		s_freeRecorders_.push_back( s_retireList_[i] );
	}
	s_retireList_.clear();
}


/**
 *	This static method gets a new staterecorder that is valid until it applies its state.
 *	@return new state recorder
 */
WrapperStateRecorder* WrapperStateRecorder::get()
{
	BW_GUARD;

	SimpleMutexHolder smh( s_mutex_ );

	if ( s_freeRecorders_.empty() )
	{
		WrapperStateRecorder* recorder = new WrapperStateRecorder;
		s_allRecorders_.push_back( recorder );
		s_freeRecorders_.push_back( recorder );
	}

	WrapperStateRecorder* recorder = s_freeRecorders_.front();
	s_freeRecorders_.pop_front();

	recorder->needsToRetire_ = true;

	return recorder;
}

/**
 *	This static method puts a used state recorder into the retire list
 *  and flushes that list when the recorders are not longer needed
 */

void WrapperStateRecorder::put( WrapperStateRecorder* recorder )
{
	BW_GUARD;

	SimpleMutexHolder smh( s_mutex_ );

	static uint32 timeStamp;

	// If we're on a new timestamp retire the previous frame's recorders
	if ( DX::getCurrentFrame() != timeStamp )
	{
		timeStamp = DX::getCurrentFrame();
		retireRecorders();
	}

	// Add this recorder to the list for retirement next frame
	if ( recorder->needsToRetire_ )
	{
		recorder->needsToRetire_ = false;
		s_retireList_.push_back( recorder );
	}
}

/**
 *	This static method clears out any recorded state, and drops all the
 *	resource references that they hold.
 */
void WrapperStateRecorder::clear()
{
	BW_GUARD;

	flushAndStall();
	retireRecorders();
	flushAndStall();
	
	MF_ASSERT( s_retireList_.empty() );
	MF_ASSERT( s_allRecorders_.size() == s_freeRecorders_.size() );

	s_allRecorders_.clear();
	s_freeRecorders_.clear();
}

SimpleMutex WrapperStateRecorder::s_mutex_;
std::vector< SmartPointer<WrapperStateRecorder> > WrapperStateRecorder::s_allRecorders_;
std::list< WrapperStateRecorder* > WrapperStateRecorder::s_freeRecorders_;
std::vector< WrapperStateRecorder* > WrapperStateRecorder::s_retireList_;

}

// effect_state_manager.cpp
