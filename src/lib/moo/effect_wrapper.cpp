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
#include "job_system/job_system.hpp"
#include "moo_dx.hpp"
#include "render_context.hpp"
#include <map>

DECLARE_DEBUG_COMPONENT2( "Effect Wrapper", 0 )

#define DX_STUB			{ MF_ASSERT( !"DX stub"	); return 0; }

static volatile LONG s_numEffects;

#ifdef ENABLE_DX_TRACKER
static ListNode s_EffectsList;
#endif

namespace DX
{
//-----------------------------------------------------------------------------

	class EffectWrapper : public ComInterface, public ID3DXEffect
	{
	public:
		EffectWrapper(ID3DXEffect* effectD3D, const char* name);
		virtual ~EffectWrapper();

		// ID3DXBaseEffect
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv) DX_STUB;
		virtual ULONG STDMETHODCALLTYPE AddRef();
		virtual ULONG STDMETHODCALLTYPE Release();

		// Descs
		virtual HRESULT STDMETHODCALLTYPE GetDesc(D3DXEFFECT_DESC* pDesc);
		virtual HRESULT STDMETHODCALLTYPE GetParameterDesc(D3DXHANDLE hParameter, D3DXPARAMETER_DESC* pDesc);
		virtual HRESULT STDMETHODCALLTYPE GetTechniqueDesc(D3DXHANDLE hTechnique, D3DXTECHNIQUE_DESC* pDesc);
		virtual HRESULT STDMETHODCALLTYPE GetPassDesc(D3DXHANDLE hPass, D3DXPASS_DESC* pDesc);
		virtual HRESULT STDMETHODCALLTYPE GetFunctionDesc(D3DXHANDLE hShader, D3DXFUNCTION_DESC* pDesc) DX_STUB;

		// Handle operations
		virtual D3DXHANDLE STDMETHODCALLTYPE GetParameter(D3DXHANDLE hParameter, UINT Index) DX_STUB;
		virtual D3DXHANDLE STDMETHODCALLTYPE GetParameterByName(D3DXHANDLE hParameter, LPCSTR pName);
		virtual D3DXHANDLE STDMETHODCALLTYPE GetParameterBySemantic(D3DXHANDLE hParameter, LPCSTR pSemantic);
		virtual D3DXHANDLE STDMETHODCALLTYPE GetParameterElement(D3DXHANDLE hParameter, UINT Index);
		virtual D3DXHANDLE STDMETHODCALLTYPE GetTechnique(UINT Index);
		virtual D3DXHANDLE STDMETHODCALLTYPE GetTechniqueByName(LPCSTR pName);
		virtual D3DXHANDLE STDMETHODCALLTYPE GetPass(D3DXHANDLE hTechnique, UINT Index);
		virtual D3DXHANDLE STDMETHODCALLTYPE GetPassByName(D3DXHANDLE hTechnique, LPCSTR pName) DX_STUB;
		virtual D3DXHANDLE STDMETHODCALLTYPE GetFunction(UINT Index) DX_STUB;
		virtual D3DXHANDLE STDMETHODCALLTYPE GetFunctionByName(LPCSTR pName) DX_STUB;
		virtual D3DXHANDLE STDMETHODCALLTYPE GetAnnotation(D3DXHANDLE hObject, UINT Index) DX_STUB;
		virtual D3DXHANDLE STDMETHODCALLTYPE GetAnnotationByName(D3DXHANDLE hObject, LPCSTR pName);

		// Get/Set Parameters
		virtual HRESULT STDMETHODCALLTYPE SetValue(D3DXHANDLE hParameter, LPCVOID pData, UINT Bytes);
		virtual HRESULT STDMETHODCALLTYPE GetValue(D3DXHANDLE hParameter, LPVOID pData, UINT Bytes) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE SetBool(D3DXHANDLE hParameter, BOOL b);
		virtual HRESULT STDMETHODCALLTYPE GetBool(D3DXHANDLE hParameter, BOOL* pb);
		virtual HRESULT STDMETHODCALLTYPE SetBoolArray(D3DXHANDLE hParameter, CONST BOOL* pb, UINT Count) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE GetBoolArray(D3DXHANDLE hParameter, BOOL* pb, UINT Count) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE SetInt(D3DXHANDLE hParameter, INT n);
		virtual HRESULT STDMETHODCALLTYPE GetInt(D3DXHANDLE hParameter, INT* pn);
		virtual HRESULT STDMETHODCALLTYPE SetIntArray(D3DXHANDLE hParameter, CONST INT* pn, UINT Count) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE GetIntArray(D3DXHANDLE hParameter, INT* pn, UINT Count) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE SetFloat(D3DXHANDLE hParameter, FLOAT f);
		virtual HRESULT STDMETHODCALLTYPE GetFloat(D3DXHANDLE hParameter, FLOAT* pf);
		virtual HRESULT STDMETHODCALLTYPE SetFloatArray(D3DXHANDLE hParameter, CONST FLOAT* pf, UINT Count) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE GetFloatArray(D3DXHANDLE hParameter, FLOAT* pf, UINT Count) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE SetVector(D3DXHANDLE hParameter, CONST D3DXVECTOR4* pVector);
		virtual HRESULT STDMETHODCALLTYPE GetVector(D3DXHANDLE hParameter, D3DXVECTOR4* pVector);
		virtual HRESULT STDMETHODCALLTYPE SetVectorArray(D3DXHANDLE hParameter, CONST D3DXVECTOR4* pVector, UINT Count);
		virtual HRESULT STDMETHODCALLTYPE GetVectorArray(D3DXHANDLE hParameter, D3DXVECTOR4* pVector, UINT Count) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE SetMatrix(D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix);
		virtual HRESULT STDMETHODCALLTYPE GetMatrix(D3DXHANDLE hParameter, D3DXMATRIX* pMatrix) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE SetMatrixArray(D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix, UINT Count);
		virtual HRESULT STDMETHODCALLTYPE GetMatrixArray(D3DXHANDLE hParameter, D3DXMATRIX* pMatrix, UINT Count) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE SetMatrixPointerArray(D3DXHANDLE hParameter, CONST D3DXMATRIX** ppMatrix, UINT Count) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE GetMatrixPointerArray(D3DXHANDLE hParameter, D3DXMATRIX** ppMatrix, UINT Count) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE SetMatrixTranspose(D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE GetMatrixTranspose(D3DXHANDLE hParameter, D3DXMATRIX* pMatrix) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE SetMatrixTransposeArray(D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix, UINT Count) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE GetMatrixTransposeArray(D3DXHANDLE hParameter, D3DXMATRIX* pMatrix, UINT Count) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE SetMatrixTransposePointerArray(D3DXHANDLE hParameter, CONST D3DXMATRIX** ppMatrix, UINT Count) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE GetMatrixTransposePointerArray(D3DXHANDLE hParameter, D3DXMATRIX** ppMatrix, UINT Count) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE SetString(D3DXHANDLE hParameter, LPCSTR pString) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE GetString(D3DXHANDLE hParameter, LPCSTR* ppString);
		virtual HRESULT STDMETHODCALLTYPE SetTexture(D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 pTexture);
		virtual HRESULT STDMETHODCALLTYPE GetTexture(D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 *ppTexture) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE GetPixelShader(D3DXHANDLE hParameter, LPDIRECT3DPIXELSHADER9 *ppPShader) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE GetVertexShader(D3DXHANDLE hParameter, LPDIRECT3DVERTEXSHADER9 *ppVShader) DX_STUB;

		//Set Range of an Array to pass to device
		virtual HRESULT STDMETHODCALLTYPE SetArrayRange(D3DXHANDLE hParameter, UINT uStart, UINT uEnd); 

		// Pool
		virtual HRESULT STDMETHODCALLTYPE GetPool(LPD3DXEFFECTPOOL* ppPool) DX_STUB;

		// Selecting and setting a technique
		virtual HRESULT STDMETHODCALLTYPE SetTechnique(D3DXHANDLE hTechnique);
		virtual D3DXHANDLE STDMETHODCALLTYPE GetCurrentTechnique() DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE ValidateTechnique(D3DXHANDLE hTechnique);
		virtual HRESULT STDMETHODCALLTYPE FindNextValidTechnique(D3DXHANDLE hTechnique, D3DXHANDLE *pTechnique) DX_STUB;
		virtual BOOL STDMETHODCALLTYPE IsParameterUsed(D3DXHANDLE hParameter, D3DXHANDLE hTechnique) DX_STUB;

		// Using current technique
		virtual HRESULT STDMETHODCALLTYPE Begin(UINT *pPasses, DWORD Flags);
		virtual HRESULT STDMETHODCALLTYPE BeginPass(UINT Pass);
		virtual HRESULT STDMETHODCALLTYPE CommitChanges();
		virtual HRESULT STDMETHODCALLTYPE EndPass();
		virtual HRESULT STDMETHODCALLTYPE End();

		// Managing D3D Device
		virtual HRESULT STDMETHODCALLTYPE GetDevice(LPDIRECT3DDEVICE9* ppDevice) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE OnLostDevice();
		virtual HRESULT STDMETHODCALLTYPE OnResetDevice();

		// Logging device calls
		virtual HRESULT STDMETHODCALLTYPE SetStateManager(LPD3DXEFFECTSTATEMANAGER pManager);
		virtual HRESULT STDMETHODCALLTYPE GetStateManager(LPD3DXEFFECTSTATEMANAGER *ppManager);

		// Parameter blocks
		virtual HRESULT STDMETHODCALLTYPE BeginParameterBlock() DX_STUB;
		virtual D3DXHANDLE STDMETHODCALLTYPE EndParameterBlock() DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE ApplyParameterBlock(D3DXHANDLE hParameterBlock) DX_STUB;
		virtual HRESULT STDMETHODCALLTYPE DeleteParameterBlock(D3DXHANDLE hParameterBlock) DX_STUB;

		// Cloning
		virtual HRESULT STDMETHODCALLTYPE CloneEffect(LPDIRECT3DDEVICE9 pDevice, LPD3DXEFFECT* ppEffect) DX_STUB;

		// Fast path for setting variables directly in ID3DXEffect
		virtual HRESULT STDMETHODCALLTYPE SetRawValue(D3DXHANDLE hParameter, LPCVOID pData, UINT ByteOffset, UINT Bytes) DX_STUB;

	//private:
		ID3DXEffect* effect_;
		D3DXHANDLE technique_;
		D3DXHANDLE techniqueWithNumPasses_;
		uint numPasses_;
		std::map<D3DXHANDLE, UINT> numPassesByTechnique_;
		LPD3DXEFFECTSTATEMANAGER stateManager_;

		const char* name_;
	};
}

//-----------------------------------------------------------------------------
// StateManager for the wrapped effects. It uses the unwrapped device.
//-----------------------------------------------------------------------------

bool gdx_mirroredTransform;
WrapperStateManager* WrapperStateManager::s_stateManager;

/*
 * Com QueryInterface method
 */
HRESULT __stdcall WrapperStateManager::QueryInterface( REFIID iid, LPVOID *ppv)
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
ULONG __stdcall WrapperStateManager::AddRef()
{
    this->incRef();
	return this->refCount();
}

/*
 * Com Release method
 */
ULONG __stdcall WrapperStateManager::Release()
{
    this->decRef();
	return this->refCount();
}

/*
 *	LightEnable
 */
HRESULT __stdcall WrapperStateManager::LightEnable( DWORD index, BOOL enable )
{
	BW_GUARD;
	MF_ASSERT( gdx_isDXThread );
	return device_->LightEnable( index, enable );
}

/*
 *	SetFVF
 */
HRESULT __stdcall WrapperStateManager::SetFVF( DWORD fvf )
{
	BW_GUARD;
	MF_ASSERT( gdx_isDXThread );
	return device_->SetFVF( fvf );
}

/*
 *	SetLight
 */
HRESULT __stdcall WrapperStateManager::SetLight( DWORD index, CONST D3DLIGHT9* pLight )
{
	BW_GUARD;
	MF_ASSERT( gdx_isDXThread );
	return device_->SetLight( index, pLight );
}

/*
 *	SetMaterial
 */
HRESULT __stdcall WrapperStateManager::SetMaterial( CONST D3DMATERIAL9* pMaterial )
{
	BW_GUARD;
	MF_ASSERT( gdx_isDXThread );
	return device_->SetMaterial( pMaterial );
}

/*
 *	SetNPatchMode
 */
HRESULT __stdcall WrapperStateManager::SetNPatchMode( FLOAT nSegments )
{
	BW_GUARD;
	MF_ASSERT( gdx_isDXThread );
	return device_->SetNPatchMode( nSegments );
}

/*
 *	SetPixelShader
 */
HRESULT __stdcall WrapperStateManager::SetPixelShader( LPDIRECT3DPIXELSHADER9 pShader )
{
	BW_GUARD;
//	MF_ASSERT( gdx_isDXThread );

	static DX::PixelShaderWrapper* current;

	if ( current )
	{
		current->Release();
	}

	DX::PixelShaderWrapper* pShaderWrapper = (DX::PixelShaderWrapper*)pShader;
	current = pShaderWrapper;

	if ( pShaderWrapper )
	{
		pShaderWrapper->AddRef();
	}

	return device_->SetPixelShader(pShaderWrapper ? pShaderWrapper->pixelShader_ : NULL);
}

/*
 *	SetPixelShaderConstantB
 */
HRESULT __stdcall WrapperStateManager::SetPixelShaderConstantB( UINT reg, CONST BOOL* pData, UINT count )
{
	BW_GUARD;
	MF_ASSERT( gdx_isDXThread );
	return device_->SetPixelShaderConstantB( reg, pData, count );
}

/*
 *	SetPixelShaderConstantF
 */
HRESULT __stdcall WrapperStateManager::SetPixelShaderConstantF( UINT reg, CONST FLOAT* pData, UINT count )
{
	BW_GUARD;
	MF_ASSERT( gdx_isDXThread );
	return device_->SetPixelShaderConstantF( reg, pData, count );

}

/*
 *	SetPixelShaderConstantI
 */
HRESULT __stdcall WrapperStateManager::SetPixelShaderConstantI( UINT reg, CONST INT* pData, UINT count )
{
	BW_GUARD;
	MF_ASSERT( gdx_isDXThread );
	return device_->SetPixelShaderConstantI( reg, pData, count );
}

/*
 *	SetVertexShader
 */
HRESULT __stdcall WrapperStateManager::SetVertexShader( LPDIRECT3DVERTEXSHADER9 pShader )
{
	BW_GUARD;
//	MF_ASSERT( gdx_isDXThread );

	static DX::VertexShaderWrapper* current;

	if ( current )
	{
		current->Release();
	}

	DX::VertexShaderWrapper* pShaderWrapper = (DX::VertexShaderWrapper*)pShader;
	current = pShaderWrapper;

	if ( pShaderWrapper )
	{
		pShaderWrapper->AddRef();
	}

	return device_->SetVertexShader(pShaderWrapper ? pShaderWrapper->vertexShader_ : NULL);
}

/*
 *	SetVertexShaderConstantB
 */
HRESULT __stdcall WrapperStateManager::SetVertexShaderConstantB( UINT reg, CONST BOOL* pData, UINT count )
{
	BW_GUARD;
	MF_ASSERT( gdx_isDXThread );
	return device_->SetVertexShaderConstantB( reg, pData, count );
}

/*
 *	SetVertexShaderConstantF
 */
HRESULT __stdcall WrapperStateManager::SetVertexShaderConstantF( UINT reg, CONST FLOAT* pData, UINT count )
{
	BW_GUARD;
	MF_ASSERT( gdx_isDXThread );
	return device_->SetVertexShaderConstantF( reg, pData, count );
}

/*
 *	SetVertexShaderConstantI
 */
HRESULT __stdcall WrapperStateManager::SetVertexShaderConstantI( UINT reg, CONST INT* pData, UINT count )
{
	BW_GUARD;
	MF_ASSERT( gdx_isDXThread );
	return device_->SetVertexShaderConstantI( reg, pData, count );
}

/*
 *	SetRenderState
 */
HRESULT __stdcall WrapperStateManager::SetRenderState( D3DRENDERSTATETYPE state, DWORD value )
{
	BW_GUARD;
	MF_ASSERT( gdx_isDXThread );

	if (gdx_mirroredTransform && state == D3DRS_CULLMODE)
	{
		value = value ^ (value >> 1);
	}

	if ( value != gdx_renderStateCache[state] )
	{
		gdx_renderStateCache[state] = value;
		return device_->SetRenderState(state,value);
	}

	return D3D_OK;
}

/*
 *	SetVertexSamplerState
 */
HRESULT __stdcall WrapperStateManager::SetSamplerState( DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value )
{
	BW_GUARD;
	MF_ASSERT( gdx_isDXThread );

	if ( value != gdx_samplerStateCache[sampler][type] )
	{
		gdx_samplerStateCache[sampler][type] = value;
		return device_->SetSamplerState( sampler, type, value );
	}

	return D3D_OK;
}

/*
 *	SetTexture
 */
HRESULT __stdcall WrapperStateManager::SetTexture( DWORD stage, LPDIRECT3DBASETEXTURE9 pTexture)
{
	BW_GUARD;
//	MF_ASSERT( gdx_isDXThread );

	static DX::BaseTextureWrapper* current[16];

	if ( current[stage] )
	{
		current[stage]->Release();
	}

	DX::BaseTextureWrapper* pTextureWrapper = (DX::BaseTextureWrapper*)pTexture;
	current[stage] = pTextureWrapper;

	if ( pTextureWrapper )
	{
		pTextureWrapper->AddRef();
	}

	if ( pTexture != gdx_textureCache[stage] )
	{
		gdx_textureCache[stage] = pTexture;
		return device_->SetTexture( stage, pTextureWrapper ? pTextureWrapper->baseTexture_ : NULL );
	}

	return D3D_OK;
}

/*
 *	SetTextureStageState
 */
HRESULT __stdcall WrapperStateManager::SetTextureStageState( DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value )
{
	BW_GUARD;
	MF_ASSERT( gdx_isDXThread );

	if ( value != gdx_textureStageStateCache[stage][type] )
	{
		gdx_textureStageStateCache[stage][type] = value;
		return device_->SetTextureStageState( stage, type, value );
	}

	return D3D_OK;
}

/*
 *	SetTransform
 */
HRESULT __stdcall WrapperStateManager::SetTransform( D3DTRANSFORMSTATETYPE state, CONST D3DMATRIX* pMatrix )
{
	BW_GUARD;
	MF_ASSERT( gdx_isDXThread );
	return device_->SetTransform( state, pMatrix );
}


WrapperStateManager::WrapperStateManager()
{
}


void WrapperStateManager::resetCache()
{
	BW_GUARD;

	for ( uint i = 0; i < 16; i++ )
	{
		SetTexture( i, NULL );
	}

	SetVertexShader( NULL );
	SetPixelShader( NULL );
}


//-----------------------------------------------------------------------------
// Create the wrapper's state manager
//-----------------------------------------------------------------------------

void DX::createEffectWrapperStateManager()
{
	WrapperStateManager::s_stateManager = new WrapperStateManager;
	DeviceWrapper* device = (DeviceWrapper*)gdx_device;
	WrapperStateManager::s_stateManager->device_ = device->device_;

#ifdef ENABLE_DX_TRACKER
	s_EffectsList.setAsRoot();
#endif

	MF_WATCH( "Render/D3D Wrapper/Effects", (LONG)s_numEffects, Watcher::WT_READ_ONLY );
}

//-----------------------------------------------------------------------------
// Create the wrapper
//-----------------------------------------------------------------------------

ID3DXEffect* DX::createEffectWrapper(ID3DXEffect* effectD3D, const char* name)
{
	return new DX::EffectWrapper( effectD3D, name );
}

//-----------------------------------------------------------------------------
// Effect Interface
//-----------------------------------------------------------------------------

DX::EffectWrapper::EffectWrapper(ID3DXEffect* effectD3D, const char* name)
:	ComInterface(effectD3D)
,	effect_(effectD3D)
,	technique_(0)
,	techniqueWithNumPasses_(0)
,	numPasses_(0)
,	stateManager_( NULL )
{
	BW_GUARD;

	MF_ASSERT( WrapperStateManager::s_stateManager )

	HRESULT res = effect_->SetStateManager( WrapperStateManager::s_stateManager );
	MF_ASSERT( res == D3D_OK );

	stateManager_ = WrapperStateManager::s_stateManager;

	InterlockedIncrement( &s_numEffects );

	name_ = name;

#ifdef ENABLE_DX_TRACKER
	SimpleMutexHolder smh( gdx_trackerMutex );
	node_.addThisBefore( &s_EffectsList );
#endif
}

//-----------------------------------------------------------------------------

DX::EffectWrapper::~EffectWrapper()
{
	BW_GUARD;
	InterlockedDecrement( &s_numEffects );
}

//-----------------------------------------------------------------------------
// IUnknown

ULONG DX::EffectWrapper::AddRef()
{
	BW_GUARD;

	LONG refCount = InterlockedIncrement( &refCount_ );
	return refCount;
}

//-----------------------------------------------------------------------------

static void playRelease()
{
	BW_GUARD;

	DX::ComInterface* comInterface = gdx_commandBuffer->read<DX::ComInterface*>();

	LONG refCount = InterlockedDecrement( &comInterface->refCount_ );

	if ( !refCount )
	{
		ULONG ref = comInterface->comInterface_->Release();
		MF_ASSERT( ref == 0 );
		delete comInterface;
	}
}

ULONG DX::EffectWrapper::Release()
{
	BW_GUARD;
	WRITE_LOCK;

	MF_ASSERT( !gdx_isDXThread );

	gdx_commandBuffer->write(playRelease,this);

	return refCount_;
}

//-----------------------------------------------------------------------------
// Effect

HRESULT DX::EffectWrapper::GetDesc(D3DXEFFECT_DESC* pDesc)
{
	BW_GUARD;
	return effect_->GetDesc(pDesc);
}

//-----------------------------------------------------------------------------

HRESULT DX::EffectWrapper::GetParameterDesc(D3DXHANDLE hParameter, D3DXPARAMETER_DESC* pDesc)
{
	BW_GUARD;
	return effect_->GetParameterDesc(hParameter, pDesc);
}

//-----------------------------------------------------------------------------

HRESULT DX::EffectWrapper::GetTechniqueDesc(D3DXHANDLE hTechnique, D3DXTECHNIQUE_DESC* pDesc)
{
	BW_GUARD;
	return effect_->GetTechniqueDesc(hTechnique, pDesc);
}

//-----------------------------------------------------------------------------

HRESULT DX::EffectWrapper::GetPassDesc(D3DXHANDLE hPass, D3DXPASS_DESC* pDesc)
{
	BW_GUARD;
	return effect_->GetPassDesc(hPass, pDesc);
}

//-----------------------------------------------------------------------------

D3DXHANDLE DX::EffectWrapper::GetParameterByName(D3DXHANDLE hParameter, LPCSTR pName)
{
	BW_GUARD;
	return effect_->GetParameterByName(hParameter, pName);
}

//-----------------------------------------------------------------------------

D3DXHANDLE DX::EffectWrapper::GetParameterBySemantic(D3DXHANDLE hParameter, LPCSTR pSemantic)
{
	BW_GUARD;
	return effect_->GetParameterBySemantic(hParameter, pSemantic);
}

//-----------------------------------------------------------------------------

D3DXHANDLE DX::EffectWrapper::GetParameterElement(D3DXHANDLE hParameter, UINT Index)
{
	BW_GUARD;
	return effect_->GetParameterElement(hParameter, Index);
}

//-----------------------------------------------------------------------------

D3DXHANDLE DX::EffectWrapper::GetTechnique(UINT Index)
{
	BW_GUARD;
	return effect_->GetTechnique(Index);
}

//-----------------------------------------------------------------------------

D3DXHANDLE DX::EffectWrapper::GetTechniqueByName(LPCSTR pName)
{
	BW_GUARD;
	return effect_->GetTechniqueByName(pName);
}

//-----------------------------------------------------------------------------

D3DXHANDLE DX::EffectWrapper::GetPass(D3DXHANDLE hTechnique, UINT Index)
{
	BW_GUARD;
	return effect_->GetPass(hTechnique, Index);
}

//-----------------------------------------------------------------------------

D3DXHANDLE DX::EffectWrapper::GetAnnotationByName(D3DXHANDLE hObject, LPCSTR pName)
{
	BW_GUARD;
	return effect_->GetAnnotationByName(hObject, pName);
}

//-----------------------------------------------------------------------------

static void playEffectSetValue()
{
	BW_GUARD;

	DX::EffectWrapper* effect = gdx_commandBuffer->read<DX::EffectWrapper*>();

	D3DXHANDLE hParameter = gdx_commandBuffer->read<D3DXHANDLE>();
	UINT Bytes = gdx_commandBuffer->read<UINT>();

	LPCVOID pData = (LPCVOID)gdx_commandBuffer->getCurrent();
	gdx_commandBuffer->seek(Bytes);

	effect->effect_->SetValue(hParameter, pData, Bytes);
}

HRESULT DX::EffectWrapper::SetValue(D3DXHANDLE hParameter, LPCVOID pData, UINT Bytes)
{
	BW_GUARD;
	MF_ASSERT( gdx_isMainThread );

	gdx_commandBuffer->write(playEffectSetValue, this, hParameter, Bytes);
	gdx_commandBuffer->writeRaw(pData, Bytes);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playEffectSetBool()
{
	BW_GUARD;

	DX::EffectWrapper* effect = gdx_commandBuffer->read<DX::EffectWrapper*>();

	D3DXHANDLE hParameter = gdx_commandBuffer->read<D3DXHANDLE>();
	BOOL b = gdx_commandBuffer->read<BOOL>();

	effect->effect_->SetBool(hParameter, b);
}

HRESULT DX::EffectWrapper::SetBool(D3DXHANDLE hParameter, BOOL b)
{
	BW_GUARD;
	MF_ASSERT( gdx_isMainThread );

	gdx_commandBuffer->write(playEffectSetBool, this, hParameter, b);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::EffectWrapper::GetBool(D3DXHANDLE hParameter, BOOL* pb)
{
	BW_GUARD;
	flushAndStall();
	return effect_->GetBool(hParameter, pb);
}

//-----------------------------------------------------------------------------

static void playEffectSetInt()
{
	BW_GUARD;

	DX::EffectWrapper* effect = gdx_commandBuffer->read<DX::EffectWrapper*>();

	D3DXHANDLE hParameter = gdx_commandBuffer->read<D3DXHANDLE>();
	INT n = gdx_commandBuffer->read<INT>();

	effect->effect_->SetInt(hParameter, n);
}

HRESULT DX::EffectWrapper::SetInt(D3DXHANDLE hParameter, INT n)
{
	BW_GUARD;
	MF_ASSERT( gdx_isMainThread );

	gdx_commandBuffer->write(playEffectSetInt, this, hParameter, n);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::EffectWrapper::GetInt(D3DXHANDLE hParameter, INT* pn)
{
	BW_GUARD;
	flushAndStall();
	return effect_->GetInt(hParameter, pn);
}

//-----------------------------------------------------------------------------

static void playEffectSetFloat()
{
	BW_GUARD;

	DX::EffectWrapper* effect = gdx_commandBuffer->read<DX::EffectWrapper*>();

	D3DXHANDLE hParameter = gdx_commandBuffer->read<D3DXHANDLE>();
	float f = gdx_commandBuffer->read<float>();

	effect->effect_->SetFloat(hParameter, f);
}

HRESULT DX::EffectWrapper::SetFloat(D3DXHANDLE hParameter, FLOAT f)
{
	BW_GUARD;
	MF_ASSERT( gdx_isMainThread );

	gdx_commandBuffer->write(playEffectSetFloat, this, hParameter, f);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::EffectWrapper::GetFloat(D3DXHANDLE hParameter, FLOAT* pf)
{
	BW_GUARD;
	flushAndStall();
	return effect_->GetFloat(hParameter, pf);
}

//-----------------------------------------------------------------------------

static void playEffectSetVector()
{
	BW_GUARD;

	DX::EffectWrapper* effect = gdx_commandBuffer->read<DX::EffectWrapper*>();

	D3DXHANDLE hParameter = gdx_commandBuffer->read<D3DXHANDLE>();
	D3DXVECTOR4 pVector = gdx_commandBuffer->read<D3DXVECTOR4>();

	effect->effect_->SetVector(hParameter, &pVector);
}

HRESULT DX::EffectWrapper::SetVector(D3DXHANDLE hParameter, CONST D3DXVECTOR4* pVector)
{
	BW_GUARD;
	MF_ASSERT( gdx_isMainThread );

	gdx_commandBuffer->write(playEffectSetVector, this, hParameter, *pVector);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::EffectWrapper::GetVector(D3DXHANDLE hParameter, D3DXVECTOR4* pVector)
{
	BW_GUARD;
	flushAndStall();
	return effect_->GetVector(hParameter, pVector);
}

//-----------------------------------------------------------------------------

static void playEffectSetVectorArray()
{
	BW_GUARD;

	DX::EffectWrapper* effect = gdx_commandBuffer->read<DX::EffectWrapper*>();

	D3DXHANDLE hParameter = gdx_commandBuffer->read<D3DXHANDLE>();
	UINT Count = gdx_commandBuffer->read<UINT>();

	CONST D3DXVECTOR4* pVector = (CONST D3DXVECTOR4*)gdx_commandBuffer->getCurrent();
	gdx_commandBuffer->seek(sizeof( D3DXVECTOR4 ) * Count);

	effect->effect_->SetVectorArray(hParameter, pVector, Count);
}

HRESULT DX::EffectWrapper::SetVectorArray(D3DXHANDLE hParameter, CONST D3DXVECTOR4* pVector, UINT Count)
{
	BW_GUARD;
	MF_ASSERT( gdx_isMainThread );

	uint size = sizeof( D3DXVECTOR4 ) * Count;

	gdx_commandBuffer->write(playEffectSetVectorArray, this, hParameter, Count );
	gdx_commandBuffer->writeRaw(pVector, size);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playEffectSetMatrix()
{
	BW_GUARD;

	DX::EffectWrapper* effect = gdx_commandBuffer->read<DX::EffectWrapper*>();

	D3DXHANDLE hParameter = gdx_commandBuffer->read<D3DXHANDLE>();
	D3DXMATRIX pMatrix = gdx_commandBuffer->read<D3DXMATRIX>();

	effect->effect_->SetMatrix(hParameter, &pMatrix);
}

HRESULT DX::EffectWrapper::SetMatrix(D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix)
{
	BW_GUARD;
	MF_ASSERT( gdx_isMainThread );

	gdx_commandBuffer->write(playEffectSetMatrix, this, hParameter, *pMatrix);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playEffectSetMatrixArray()
{
	BW_GUARD;

	DX::EffectWrapper* effect = gdx_commandBuffer->read<DX::EffectWrapper*>();

	D3DXHANDLE hParameter = gdx_commandBuffer->read<D3DXHANDLE>();
	UINT Count = gdx_commandBuffer->read<UINT>();

	CONST D3DXMATRIX* pMatrix = (CONST D3DXMATRIX*)gdx_commandBuffer->getCurrent();
	gdx_commandBuffer->seek(sizeof( D3DXMATRIX ) * Count);

	effect->effect_->SetMatrixArray(hParameter, pMatrix, Count);
}

HRESULT DX::EffectWrapper::SetMatrixArray(D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix, UINT Count)
{
	BW_GUARD;
	MF_ASSERT( gdx_isMainThread );

	uint size = sizeof( D3DXMATRIX ) * Count;

	gdx_commandBuffer->write(playEffectSetMatrixArray, this, hParameter, Count );
	gdx_commandBuffer->writeRaw(pMatrix, size);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::EffectWrapper::GetString(D3DXHANDLE hParameter, LPCSTR* ppString)
{
	BW_GUARD;
	flushAndStall();
	return effect_->GetString(hParameter, ppString);
}

//-----------------------------------------------------------------------------

static void playEffectSetTexture()
{
	BW_GUARD;

	DX::EffectWrapper* effect = gdx_commandBuffer->read<DX::EffectWrapper*>();

	D3DXHANDLE hParameter = gdx_commandBuffer->read<D3DXHANDLE>();
	LPDIRECT3DBASETEXTURE9 pTexture = gdx_commandBuffer->read<LPDIRECT3DBASETEXTURE9>();

	effect->effect_->SetTexture(hParameter, pTexture);
}

HRESULT DX::EffectWrapper::SetTexture(D3DXHANDLE hParameter, LPDIRECT3DBASETEXTURE9 pTexture)
{
	BW_GUARD;
	MF_ASSERT( gdx_isMainThread );

	gdx_commandBuffer->write(playEffectSetTexture, this, hParameter, pTexture);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playEffectSetArrayRange()
{
	BW_GUARD;

	DX::EffectWrapper* effect = gdx_commandBuffer->read<DX::EffectWrapper*>();

	D3DXHANDLE hParameter = gdx_commandBuffer->read<D3DXHANDLE>();
	UINT uStart = gdx_commandBuffer->read<UINT>();
	UINT uEnd = gdx_commandBuffer->read<UINT>();

	effect->effect_->SetArrayRange(hParameter, uStart, uEnd);
}

HRESULT DX::EffectWrapper::SetArrayRange(D3DXHANDLE hParameter, UINT uStart, UINT uEnd)
{
	BW_GUARD;
	MF_ASSERT( gdx_isMainThread );

	gdx_commandBuffer->write(playEffectSetArrayRange, this, hParameter, uStart, uEnd);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playEffectSetTechnique()
{
	BW_GUARD;

	DX::EffectWrapper* effect = gdx_commandBuffer->read<DX::EffectWrapper*>();

	D3DXHANDLE hTechnique = gdx_commandBuffer->read<D3DXHANDLE>();

	effect->effect_->SetTechnique(hTechnique);
}

HRESULT DX::EffectWrapper::SetTechnique(D3DXHANDLE hTechnique)
{
	BW_GUARD;
	MF_ASSERT( gdx_isMainThread );

	gdx_commandBuffer->write(playEffectSetTechnique, this, hTechnique);
	technique_ = hTechnique;

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::EffectWrapper::ValidateTechnique(D3DXHANDLE hTechnique)
{
	BW_GUARD;
	flushAndStall();
	return effect_->ValidateTechnique(hTechnique);
}

//-----------------------------------------------------------------------------

static void playEffectBegin()
{
	BW_GUARD;

	DX::EffectWrapper* effect = gdx_commandBuffer->read<DX::EffectWrapper*>();

	DWORD Flags = gdx_commandBuffer->read<DWORD>();
	gdx_mirroredTransform = gdx_commandBuffer->read<bool>();

	UINT passes;

	HRESULT res = effect->effect_->Begin(&passes, Flags);

	MF_ASSERT( res == D3D_OK );
	MF_ASSERT( passes > 0 );
}

HRESULT DX::EffectWrapper::Begin(UINT *pPasses, DWORD Flags)
{
	BW_GUARD;
	MF_ASSERT( gdx_isMainThread );

	gdx_commandBuffer->write(playEffectBegin, this, Flags, Moo::rc().mirroredTransform() );

	// If the technique has changed we need to update the number of passes
	if ( technique_ != techniqueWithNumPasses_ )
	{
		// If this is the first time this technique is being used we need
		// to cache the number of passes into a map
		if ( !numPassesByTechnique_.count( technique_ ) )
		{
			flushAndStall();

			UINT passes;
			HRESULT res = effect_->Begin( &passes, Flags );
			MF_ASSERT( res == D3D_OK );

			numPassesByTechnique_[technique_] = passes;

			techniqueWithNumPasses_ = technique_;
			numPasses_ = passes;
			*pPasses = numPasses_;

			return D3D_OK;
		}

		// We have used this technique before so get the number of passes
		// from the cache and store that its the current one
		techniqueWithNumPasses_ = technique_;
		numPasses_ = numPassesByTechnique_[technique_];
	}

	*pPasses = numPasses_;

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playEffectBeginPass()
{
	BW_GUARD;

	DX::EffectWrapper* effect = gdx_commandBuffer->read<DX::EffectWrapper*>();

	UINT Pass = gdx_commandBuffer->read<UINT>();

	HRESULT res = effect->effect_->BeginPass(Pass);

	MF_ASSERT( res == D3D_OK );
}

HRESULT DX::EffectWrapper::BeginPass(UINT Pass)
{
	BW_GUARD;
	MF_ASSERT( gdx_isMainThread );

	Moo::rc().invalidateStateCache();

	gdx_commandBuffer->write(playEffectBeginPass, this, Pass);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playEffectCommitChanges()
{
	BW_GUARD;

	DX::EffectWrapper* effect = gdx_commandBuffer->read<DX::EffectWrapper*>();

	effect->effect_->CommitChanges();
}

HRESULT DX::EffectWrapper::CommitChanges()
{
	BW_GUARD;
	MF_ASSERT( gdx_isMainThread );

	Moo::rc().invalidateStateCache();

	gdx_commandBuffer->write(playEffectCommitChanges, this);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playEffectEndPass()
{
	BW_GUARD;

	DX::EffectWrapper* effect = gdx_commandBuffer->read<DX::EffectWrapper*>();

	effect->effect_->EndPass();
}

HRESULT DX::EffectWrapper::EndPass()
{
	BW_GUARD;
	MF_ASSERT( gdx_isMainThread );

	gdx_commandBuffer->write(playEffectEndPass, this);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playEffectEnd()
{
	BW_GUARD;

	DX::EffectWrapper* effect = gdx_commandBuffer->read<DX::EffectWrapper*>();

	effect->effect_->End();
}

HRESULT DX::EffectWrapper::End()
{
	BW_GUARD;
	MF_ASSERT( gdx_isMainThread );

	gdx_commandBuffer->write(playEffectEnd, this);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::EffectWrapper::OnLostDevice()
{
	BW_GUARD;
	flushAndStall();
	return effect_->OnLostDevice();
}

//-----------------------------------------------------------------------------

HRESULT DX::EffectWrapper::OnResetDevice()
{
	BW_GUARD;
	flushAndStall();
	return effect_->OnResetDevice();
}

//-----------------------------------------------------------------------------

static void playEffectSetStateManager()
{
	BW_GUARD;

	DX::EffectWrapper* effect = gdx_commandBuffer->read<DX::EffectWrapper*>();

	LPD3DXEFFECTSTATEMANAGER pManager = gdx_commandBuffer->read<LPD3DXEFFECTSTATEMANAGER>();

	HRESULT res = effect->effect_->SetStateManager(pManager);
}

HRESULT DX::EffectWrapper::SetStateManager(LPD3DXEFFECTSTATEMANAGER pManager)
{
	BW_GUARD;
	MF_ASSERT( gdx_isMainThread );

	if ( stateManager_ )
	{
		stateManager_->Release();
	}

	pManager->AddRef();
	stateManager_ = pManager;

	gdx_commandBuffer->write(playEffectSetStateManager, this, pManager);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::EffectWrapper::GetStateManager(LPD3DXEFFECTSTATEMANAGER *ppManager)
{
	BW_GUARD;
	stateManager_->AddRef();
	*ppManager = stateManager_;
	return D3D_OK;
}
