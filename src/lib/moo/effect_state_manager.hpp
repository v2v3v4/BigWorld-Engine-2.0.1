/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EFFECT_STATE_MANAGER_HPP
#define EFFECT_STATE_MANAGER_HPP

#include "cstdmf/vectornodest.hpp"
#include "resmgr/bwresource.hpp"
#include "texture_manager.hpp"
#include "render_context_callback.hpp"
#include <d3dx9.h>

namespace Moo
{


/**
 *	This class implements the d3d effect state manager.
 *	It lets us use the render context state manager when using effects.
 *
 */
class StateManager : public ID3DXEffectStateManager, public SafeReferenceCount
{
public:
	StateManager() {}
	virtual ~StateManager() {}

	STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

	HRESULT __stdcall LightEnable( DWORD index, BOOL enable );
	HRESULT __stdcall SetFVF( DWORD fvf );
	HRESULT __stdcall SetLight( DWORD index, CONST D3DLIGHT9* pLight );
	HRESULT __stdcall SetMaterial( CONST D3DMATERIAL9* pMaterial );
	HRESULT __stdcall SetNPatchMode( FLOAT nSegments );
	HRESULT __stdcall SetPixelShader( LPDIRECT3DPIXELSHADER9 pShader );	
	HRESULT __stdcall SetPixelShaderConstantB( UINT reg, CONST BOOL* pData, UINT count );
	HRESULT __stdcall SetPixelShaderConstantF( UINT reg, CONST FLOAT* pData, UINT count );
	HRESULT __stdcall SetPixelShaderConstantI( UINT reg, CONST INT* pData, UINT count );
	HRESULT __stdcall SetVertexShader( LPDIRECT3DVERTEXSHADER9 pShader );
	HRESULT __stdcall SetVertexShaderConstantB( UINT reg, CONST BOOL* pData, UINT count );
	HRESULT __stdcall SetVertexShaderConstantF( UINT reg, CONST FLOAT* pData, UINT count );
	HRESULT __stdcall SetVertexShaderConstantI( UINT reg, CONST INT* pData, UINT count );
	HRESULT __stdcall SetRenderState( D3DRENDERSTATETYPE state, DWORD value );
	HRESULT __stdcall SetSamplerState( DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value );
	HRESULT __stdcall SetTexture( DWORD stage, LPDIRECT3DBASETEXTURE9 pTexture);
	HRESULT __stdcall SetTextureStageState( DWORD stage, D3DTEXTURESTAGESTATETYPE type, 
		DWORD value );
	HRESULT __stdcall SetTransform( D3DTRANSFORMSTATETYPE state, CONST D3DMATRIX* pMatrix );
};


/**
 * This class allocates memory that gets recycled every frame.
 */
template <int ElementSize>
class QuickAlloc : public RenderContextCallback
{
public:
	QuickAlloc()
	{

	}

	~QuickAlloc()
	{
		// Make sure fini was called and no further allocations were made.
		MF_ASSERT_DEV( memTable_.empty() );
	}

	void* alloc()
	{
		if ( !gdx_isDXThread )
		{
			static uint32 timeStamp = rc().frameTimestamp();
			if (!rc().frameDrawn( timeStamp ))
				reset();
		}
		else
		{
			static uint32 timeStamp;
			if ( timeStamp != DX::getCurrentFrame() )
			{
				timeStamp = DX::getCurrentFrame();
				reset();
			}
		}

		const int NUM_ALLOCS_PER_TABLE = 1024;
		const int ALLOC_MASK = NUM_ALLOCS_PER_TABLE - 1;
		const int TABLE_SHIFT = 10;

		uint32 table = offset_ >> TABLE_SHIFT;
		uint32 index = offset_ & ALLOC_MASK;
		offset_ += 1;
		if (table >= memTable_.size())
		{
			memTable_.push_back( new char[ElementSize * NUM_ALLOCS_PER_TABLE] );
		}
		return (void*)(memTable_[table] + (index * ElementSize));
	}

	void reset()
	{
		offset_ = 0;
	}

	static QuickAlloc& instance()
	{
		static QuickAlloc inst;
		return inst;
	}

	/*virtual*/ void renderContextFini()
	{
		for (size_t i = 0; i < memTable_.size(); ++i)
		{
			delete[] memTable_[i];
		}
		memTable_.clear();
	}

private:
	std::vector< char* > memTable_;
	uint32 offset_;
};

const uint32 RESERVE_SIZE = 512;
const uint32 GROW_SIZE = 512;


/**
 * This class allocates arrays of type ElemType that 
 * gets recycled every frame.
 */
template <class ElemType>
class ConstantAllocator
{
public:
	static ConstantAllocator& instance()
	{
		static ConstantAllocator inst;
		return inst;
	}

	typedef std::vector<ElemType> ElementPool;

	/**
	 *	An allocation helper class.
	 */
	class Allocation
	{
	public:
		Allocation( ElementPool& pool, uint32 first, uint32 size ) :
		  first_( first ),
		  size_( size )
		{
		}
		uint32 first() const { return first_; }
		uint32 size() const { return size_; }
		const ElemType* data() { return &ConstantAllocator<ElemType>::instance().pool()[first_]; }
		void* operator new( size_t s )
		{
			return QuickAlloc<sizeof(Allocation)>::instance().alloc();
		}

		void* operator new( size_t s, void* at )
		{
			return at;
		}

		void* operator new[]( size_t s )
		{
			return QuickAlloc<sizeof(Allocation)>::instance().alloc();
		}

		void operator delete( void* p, size_t s)
		{
		}

		void operator delete[]( void* p, size_t s )
		{
		}
	private:
		uint32 first_;
		uint32 size_;
	};

	Allocation* init( const ElemType* data, uint32 nConstants )
	{
		if ( !gdx_isDXThread )
		{
			static uint32 timeStamp = rc().frameTimestamp();
			if (!rc().frameDrawn( timeStamp ))
				reset();
		}
		else
		{
			static uint32 timeStamp;
			if ( timeStamp != DX::getCurrentFrame() )
			{
				timeStamp = DX::getCurrentFrame();
				reset();
			}
		}

		uint32 start = offset_;
		copyData( data, nConstants );
		return new Allocation( pool_, start, nConstants );
	}

	void reset()
	{
		offset_ = 0;
	}

    const ElementPool& pool() const     { return pool_; }

private:

	void copyData( const ElemType* pData, uint32 nConstants )
	{
		if ((offset_ + nConstants) > pool_.size())
		{
			pool_.resize( ( ( (offset_ + nConstants)/GROW_SIZE) + 1 ) * GROW_SIZE);
		}

		memcpy( &pool_[offset_], pData, nConstants * sizeof(ElemType) );
		offset_ += nConstants;
	}

	ConstantAllocator()
	: offset_( 0 )
	{
		pool_.reserve( RESERVE_SIZE );
	}

	ElementPool	pool_;
	uint32		offset_;
};


/**
 * Int vector4 definitition
 */
struct IntVector4
{
	int x,y,z,w;
};

/**
 * This class implements a d3d effect state manager that records all the
 * d3d states, vertex shaders, pixel shaders etc, so they can be played
 * back at a later time in the same frame.
 */
class StateRecorder : public StateManager
{
public:
	StateRecorder() {}
	virtual ~StateRecorder() {};

	HRESULT __stdcall LightEnable( DWORD index, BOOL enable );
	HRESULT __stdcall SetFVF( DWORD fvf );
	HRESULT __stdcall SetLight( DWORD index, CONST D3DLIGHT9* pLight );
	HRESULT __stdcall SetMaterial( CONST D3DMATERIAL9* pMaterial );
	HRESULT __stdcall SetNPatchMode( FLOAT nSegments );
	HRESULT __stdcall SetPixelShader( LPDIRECT3DPIXELSHADER9 pShader );
	HRESULT __stdcall SetPixelShaderConstantB( UINT reg, CONST BOOL* pData, UINT count );
	HRESULT __stdcall SetPixelShaderConstantF( UINT reg, CONST FLOAT* pData, UINT count );
	HRESULT __stdcall SetPixelShaderConstantI( UINT reg, CONST INT* pData, UINT count );
	HRESULT __stdcall SetVertexShader( LPDIRECT3DVERTEXSHADER9 pShader );
	HRESULT __stdcall SetVertexShaderConstantB( UINT reg, CONST BOOL* pData, UINT count );
	HRESULT __stdcall SetVertexShaderConstantF( UINT reg, CONST FLOAT* pData, UINT count );
	HRESULT __stdcall SetVertexShaderConstantI( UINT reg, CONST INT* pData, UINT count );
	HRESULT __stdcall SetRenderState( D3DRENDERSTATETYPE state, DWORD value );
	HRESULT __stdcall SetSamplerState( DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value );
	HRESULT __stdcall SetTexture( DWORD stage, LPDIRECT3DBASETEXTURE9 pTexture );
	HRESULT __stdcall SetTextureStageState( DWORD stage, D3DTEXTURESTAGESTATETYPE type, 
		DWORD value );
	HRESULT __stdcall SetTransform( D3DTRANSFORMSTATETYPE state, CONST D3DMATRIX* pMatrix );

	virtual void setStates();

	void init();

	static StateRecorder* get();
	static void clear();

protected:
	void setRenderStates();
	void setTextureStageStates();
	void setSamplerStates();
	void setTransforms();
	void setTextures();
	void setLights();

	typedef ConstantAllocator<Vector4>::Allocation F4Constant;
	typedef std::pair< UINT, F4Constant* > F4Mapping;
	typedef VectorNoDestructor<F4Mapping> F4Mappings;

	typedef ConstantAllocator<IntVector4>::Allocation I4Constant;
	typedef std::pair< UINT, I4Constant* > I4Mapping;
	typedef VectorNoDestructor<I4Mapping> I4Mappings;

	typedef ConstantAllocator<BOOL>::Allocation BConstant;
	typedef std::pair< UINT, BConstant* > BMapping;
	typedef VectorNoDestructor<BMapping> BMappings;

	typedef std::pair< D3DTRANSFORMSTATETYPE, D3DMATRIX > TransformState;
	typedef VectorNoDestructor< TransformState > TransformStates;

	typedef std::pair< DWORD, ComObjectWrap< DX::BaseTexture > > Texture;
	typedef VectorNoDestructor< Texture > Textures;

	typedef std::pair< DWORD, BOOL > LightState;
	typedef VectorNoDestructor< LightState > LightStates;

	typedef std::pair< DWORD, D3DLIGHT9 > Light;
	typedef VectorNoDestructor< Light > Lights;

	struct RenderState
	{
		D3DRENDERSTATETYPE state;
		DWORD value;
	};

	typedef std::vector< RenderState > RenderStates;

	struct TextureStageState
	{
		DWORD stage;
		D3DTEXTURESTAGESTATETYPE type;
		DWORD value;
	};

	typedef std::vector< TextureStageState > TextureStageStates;

	struct SamplerState
	{
		DWORD sampler;
		D3DSAMPLERSTATETYPE type;
		DWORD value;
	};

	typedef std::vector< SamplerState > SamplerStates;

	F4Mappings			vertexShaderConstantsF_;
	I4Mappings			vertexShaderConstantsI_;
	BMappings			vertexShaderConstantsB_;

	F4Mappings			pixelShaderConstantsF_;
	I4Mappings			pixelShaderConstantsI_;
	BMappings			pixelShaderConstantsB_;

	RenderStates		renderStates_;
	TextureStageStates	textureStageStates_;
	SamplerStates		samplerStates_;

	TransformStates		transformStates_;
	Textures			textures_;
	LightStates			lightEnable_;
	Lights				lights_;

	std::pair< bool, ComObjectWrap< DX::VertexShader > > vertexShader_;
	std::pair< bool, ComObjectWrap< DX::PixelShader > > pixelShader_;
	std::pair< bool, DWORD > fvf_;
	std::pair< bool, D3DMATERIAL9 > material_;
	std::pair< bool, float > nPatchMode_;

	static uint32 s_nextAlloc_;
	static std::vector< SmartPointer<StateRecorder> > s_stateRecorders_;
};

/**
 * This is the state recorder for use in the wrapper.
 */

 class WrapperStateRecorder : public StateRecorder
{
public:
	WrapperStateRecorder() {}
	virtual ~WrapperStateRecorder() {};

	static void retireRecorders();
	static WrapperStateRecorder* get();
	static void put( WrapperStateRecorder* recorder );
	static void clear();

	HRESULT __stdcall SetRenderState( D3DRENDERSTATETYPE state, DWORD value );

	virtual void setStates();

	void setStatesReal();

private:
	void setRenderStates();
	void setTextureStageStates();
	void setSamplerStates();
	void setTransforms();
	void setTextures();
	void setLights();

	bool needsToRetire_;

 	static SimpleMutex s_mutex_;
	static std::vector< SmartPointer<WrapperStateRecorder> > s_allRecorders_;
	static std::list< WrapperStateRecorder* > s_freeRecorders_;
	static std::vector< WrapperStateRecorder* > s_retireList_;
};

}

#endif // EFFECT_STATE_MANAGER_HPP
