/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MOO_DX_HPP
#define MOO_DX_HPP

#include "cstdmf/stdmf.hpp"
#include "cstdmf/list_node.hpp"
#include "cstdmf/smartpointer.hpp"
#include "job_system/command_buffer.hpp"

#include <d3d9.h>
#include <d3d9types.h>
#include <d3dx9effect.h>
#include <string>

// If we have a stack tracker we can also enable DX tracking.
// Should normally be off as it requires a mutex when maintaining its lists.
#if ENABLE_STACK_TRACKER
//#define ENABLE_DX_TRACKER
#endif


#ifndef D3DRS_MAX
// Must be larger than the maximum valid value of the enum D3DRENDERSTATETYPE
#define D3DRS_MAX       210
#endif D3DRS_MAX

#ifndef D3DTSS_MAX
// Must be larger than the maximum valid value of the enum D3DTEXTURESTAGESTATETYPE
#define D3DTSS_MAX      33
#endif D3DTSS_MAX

#ifndef D3DSAMP_MAX
// Must be larger than the maximum valid value of the enum D3DSAMPLERSTATETYPE
#define D3DSAMP_MAX		14
#endif D3DSAMP_MAX

#ifndef D3DFFSTAGES_MAX
// Always 8, the maximum number of stages supported by the fixed function pipeline
#define D3DFFSTAGES_MAX	8
#endif D3DFFSTAGES_MAX

#ifndef D3DSAMPSTAGES_MAX
// Must be larger than the maximum valid sampler stage, ie. D3DVERTEXTEXTURESAMPLER3
#define D3DSAMPSTAGES_MAX	261
#endif D3DSAMPSTAGES_MAX

namespace DX
{
	typedef IDirect3D9				Interface;

	typedef IDirect3DDevice9		Device;
	typedef IDirect3DResource9		Resource;
	typedef IDirect3DBaseTexture9	BaseTexture;
	typedef IDirect3DTexture9		Texture;
	typedef IDirect3DCubeTexture9	CubeTexture;
	typedef IDirect3DSurface9		Surface;
	typedef IDirect3DVertexBuffer9	VertexBuffer;
	typedef IDirect3DIndexBuffer9	IndexBuffer;
	typedef IDirect3DPixelShader9	PixelShader;
	typedef IDirect3DVertexShader9	VertexShader;
	typedef IDirect3DVertexDeclaration9 VertexDeclaration;
	typedef IDirect3DQuery9			Query;

	typedef D3DLIGHT9				Light;
	typedef D3DVIEWPORT9			Viewport;
	typedef D3DMATERIAL9			Material;

	IDirect3DDevice9* createDeviceWrapper(IDirect3DDevice9* deviceD3D);
	void createEffectWrapperStateManager();
	ID3DXEffect* createEffectWrapper(ID3DXEffect* effectD3D, const char* name);

	uint32	surfaceSize( uint width, uint height, D3DFORMAT format );
	uint32	surfaceSize( const D3DSURFACE_DESC& desc );
	uint32	textureSize( const Texture *texture );
	std::string errorAsString( HRESULT hr );

	enum
	{
		WRAPPER_FLAG_IMMEDIATE_LOCK		= ( 1 << 0 ),
		WRAPPER_FLAG_DEFERRED_LOCK		= ( 1 << 1 ),
		WRAPPER_FLAG_ZERO_TEXTURE_LOCK	= ( 1 << 2 ),
		WRAPPER_FLAG_QUERY_ISSUE_FLUSH	= ( 1 << 3 ),
	};

	void setWrapperFlags(uint32 flags);
	uint32 getWrapperFlags();

	uint32* getPatchForPrimCount();

	class ScopedWrapperFlags
	{
	public:
		ScopedWrapperFlags( uint32 newFlags )
		{
			oldFlags_ = getWrapperFlags();
			setWrapperFlags( newFlags );
		}

		~ScopedWrapperFlags()
		{
			setWrapperFlags( oldFlags_ );
		}

	private:
		uint32 oldFlags_;
	};

	void newBlock();
	void newFrame();

	// The main thread tells us what the current frame is so the D3D thread
	// can know what frame the main thread was in when it wrote to the
	// command buffer
	void setCurrentFrame( uint32 currentFrame );	// Call from main thread
	uint32 getCurrentFrame();						// Call from D3D thread

	// Allow secondary threads to submit deferred commands into the wrapper
	void addSecondaryThread();

	// This function is a work around for the fact that the tools use a
	// present thread. The wrapper needs to think that this is the main
	// thread. It works provided that the threads provide mutual exclusion
	// and therefore appear as one thread to the wrapper.
	void addFakeMainThread();
}

//-----------------------------------------------------------------------------
// Private for wrapper only
//-----------------------------------------------------------------------------

#define DX_STUB			{ MF_ASSERT( !"DX stub"	); return 0; }
#define DX_STUB_T(T)	{ MF_ASSERT( !"DX stub"	); return (T)0; }

namespace DX
{
	class DeviceWrapper;
	class ResourceWrapper;
	class BaseTextureWrapper;
	class TextureWrapper;
	class CubeTextureWrapper;
	class SurfaceWrapper;
	class VertexBufferWrapper;
	class IndexBufferWrapper;
	class VertexShaderWrapper;
	class PixelShaderWrapper;
	class VertexDeclarationWrapper;
	class QueryWrapper;
	class StateBlockWrapper;

//-----------------------------------------------------------------------------

	class ComInterface
	{
	public:
		ComInterface(IUnknown* unknown);
		virtual ~ComInterface();

		void set(IUnknown* unknown);

		ULONG AddRef();
		ULONG Release();

	//private:
		IUnknown* comInterface_;
		volatile LONG refCount_;

#ifdef ENABLE_DX_TRACKER
		uint id_;
		ListNode node_;
		std::string stack_;
#endif
	};
	
//-----------------------------------------------------------------------------

	class DeviceWrapper : public ComInterface, public IDirect3DDevice9
	{
	public:
		DeviceWrapper( IDirect3DDevice9* device );
		virtual ~DeviceWrapper();

		void createCache();
		void destroyCache();

		// IUnknown
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj);
		virtual ULONG STDMETHODCALLTYPE AddRef();
		virtual ULONG STDMETHODCALLTYPE Release();

		// Methods
		virtual HRESULT STDMETHODCALLTYPE TestCooperativeLevel();
		virtual UINT STDMETHODCALLTYPE GetAvailableTextureMem();
		virtual HRESULT STDMETHODCALLTYPE EvictManagedResources();
		virtual HRESULT STDMETHODCALLTYPE SetCursorProperties(UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap);
		virtual void STDMETHODCALLTYPE SetCursorPosition(int X,int Y,DWORD Flags);
		virtual BOOL STDMETHODCALLTYPE ShowCursor(BOOL bShow);
		virtual HRESULT STDMETHODCALLTYPE Reset(D3DPRESENT_PARAMETERS* pPresentationParameters);
		virtual HRESULT STDMETHODCALLTYPE Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion);
		virtual HRESULT STDMETHODCALLTYPE GetBackBuffer(UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer);
		virtual void STDMETHODCALLTYPE SetGammaRamp(UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp);
		virtual HRESULT STDMETHODCALLTYPE CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle);
		virtual HRESULT STDMETHODCALLTYPE CreateCubeTexture(UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle);
		virtual HRESULT STDMETHODCALLTYPE CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle);
		virtual HRESULT STDMETHODCALLTYPE CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle);
		virtual HRESULT STDMETHODCALLTYPE CreateRenderTarget(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
		virtual HRESULT STDMETHODCALLTYPE CreateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
		virtual HRESULT STDMETHODCALLTYPE UpdateSurface(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint);
		virtual HRESULT STDMETHODCALLTYPE UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture);
		virtual HRESULT STDMETHODCALLTYPE GetRenderTargetData(IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface);
		virtual HRESULT STDMETHODCALLTYPE StretchRect(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter);
		virtual HRESULT STDMETHODCALLTYPE CreateOffscreenPlainSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
		virtual HRESULT STDMETHODCALLTYPE SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget);
		virtual HRESULT STDMETHODCALLTYPE GetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget);
		virtual HRESULT STDMETHODCALLTYPE SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil);
		virtual HRESULT STDMETHODCALLTYPE GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface);
		virtual HRESULT STDMETHODCALLTYPE BeginScene();
		virtual HRESULT STDMETHODCALLTYPE EndScene();
		virtual HRESULT STDMETHODCALLTYPE Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil);
		virtual HRESULT STDMETHODCALLTYPE SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix);
		virtual HRESULT STDMETHODCALLTYPE SetViewport(CONST D3DVIEWPORT9* pViewport);
		virtual HRESULT STDMETHODCALLTYPE GetViewport(D3DVIEWPORT9* pViewport);
		virtual HRESULT STDMETHODCALLTYPE SetMaterial(CONST D3DMATERIAL9* pMaterial);
		virtual HRESULT STDMETHODCALLTYPE SetLight(DWORD Index,CONST D3DLIGHT9* pLight);
		virtual HRESULT STDMETHODCALLTYPE LightEnable(DWORD Index,BOOL Enable);
		virtual HRESULT STDMETHODCALLTYPE SetClipPlane(DWORD Index,CONST float* pPlane);
		virtual HRESULT STDMETHODCALLTYPE SetRenderState(D3DRENDERSTATETYPE State,DWORD Value);
		virtual HRESULT STDMETHODCALLTYPE SetTexture(DWORD Stage,IDirect3DBaseTexture9* pTexture);
		virtual HRESULT STDMETHODCALLTYPE SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value);
		virtual HRESULT STDMETHODCALLTYPE SetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value);
		virtual HRESULT STDMETHODCALLTYPE SetScissorRect(CONST RECT* pRect);
		virtual HRESULT STDMETHODCALLTYPE GetScissorRect(RECT* pRect);
		virtual HRESULT STDMETHODCALLTYPE SetSoftwareVertexProcessing(BOOL bSoftware);
		virtual HRESULT STDMETHODCALLTYPE SetNPatchMode(float nSegments);
		virtual HRESULT STDMETHODCALLTYPE DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount);
		virtual HRESULT STDMETHODCALLTYPE DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount);
		virtual HRESULT STDMETHODCALLTYPE DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride);
		virtual HRESULT STDMETHODCALLTYPE DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride);
		virtual HRESULT STDMETHODCALLTYPE SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl);
		virtual HRESULT STDMETHODCALLTYPE CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl);
		virtual HRESULT STDMETHODCALLTYPE SetFVF(DWORD FVF);
		virtual HRESULT STDMETHODCALLTYPE CreateVertexShader(CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader);
		virtual HRESULT STDMETHODCALLTYPE SetVertexShader(IDirect3DVertexShader9* pShader);
		virtual HRESULT STDMETHODCALLTYPE SetVertexShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount);
		virtual HRESULT STDMETHODCALLTYPE SetVertexShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount);
		virtual HRESULT STDMETHODCALLTYPE SetVertexShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount);
		virtual HRESULT STDMETHODCALLTYPE SetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride);
	    virtual HRESULT STDMETHODCALLTYPE SetStreamSourceFreq(UINT StreamNumber,UINT Setting);
		virtual HRESULT STDMETHODCALLTYPE SetIndices(IDirect3DIndexBuffer9* pIndexData);
		virtual HRESULT STDMETHODCALLTYPE GetIndices(IDirect3DIndexBuffer9** ppIndexData);
		virtual HRESULT STDMETHODCALLTYPE CreatePixelShader(CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader);
		virtual HRESULT STDMETHODCALLTYPE SetPixelShader(IDirect3DPixelShader9* pShader);
		virtual HRESULT STDMETHODCALLTYPE SetPixelShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount);
		virtual HRESULT STDMETHODCALLTYPE SetPixelShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount);
		virtual HRESULT STDMETHODCALLTYPE SetPixelShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount);
		virtual HRESULT STDMETHODCALLTYPE CreateQuery(D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery);

		// Stubs
	virtual HRESULT STDMETHODCALLTYPE GetDirect3D(IDirect3D9** ppD3D9);
	virtual HRESULT STDMETHODCALLTYPE GetDeviceCaps(D3DCAPS9* pCaps);
	virtual HRESULT STDMETHODCALLTYPE GetDisplayMode(UINT iSwapChain,D3DDISPLAYMODE* pMode);
	virtual HRESULT STDMETHODCALLTYPE GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters);
		virtual HRESULT STDMETHODCALLTYPE CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS *,IDirect3DSwapChain9 **) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetSwapChain(UINT,IDirect3DSwapChain9 **) DX_STUB
		virtual UINT STDMETHODCALLTYPE GetNumberOfSwapChains(void) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetRasterStatus(UINT,D3DRASTER_STATUS *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE SetDialogBoxMode(BOOL) DX_STUB
		virtual void STDMETHODCALLTYPE GetGammaRamp(UINT,D3DGAMMARAMP *) DX_STUB_T(void)
		virtual HRESULT STDMETHODCALLTYPE CreateVolumeTexture(UINT,UINT,UINT,UINT,DWORD,D3DFORMAT,D3DPOOL,IDirect3DVolumeTexture9 **,HANDLE *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetFrontBufferData(UINT,IDirect3DSurface9 *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE ColorFill(IDirect3DSurface9 *,const RECT *,D3DCOLOR) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetTransform(D3DTRANSFORMSTATETYPE,D3DMATRIX *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE MultiplyTransform(D3DTRANSFORMSTATETYPE,const D3DMATRIX *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetMaterial(D3DMATERIAL9 *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetLight(DWORD,D3DLIGHT9 *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetLightEnable(DWORD,BOOL *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetClipPlane(DWORD,float *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetRenderState(D3DRENDERSTATETYPE,DWORD *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE CreateStateBlock(D3DSTATEBLOCKTYPE,IDirect3DStateBlock9 **) DX_STUB
	virtual HRESULT STDMETHODCALLTYPE BeginStateBlock();
	virtual HRESULT STDMETHODCALLTYPE EndStateBlock(IDirect3DStateBlock9** ppSB);
		virtual HRESULT STDMETHODCALLTYPE SetClipStatus(const D3DCLIPSTATUS9 *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetClipStatus(D3DCLIPSTATUS9 *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetTexture(DWORD,IDirect3DBaseTexture9 **) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetTextureStageState(DWORD,D3DTEXTURESTAGESTATETYPE,DWORD *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetSamplerState(DWORD,D3DSAMPLERSTATETYPE,DWORD *) DX_STUB
	virtual HRESULT STDMETHODCALLTYPE ValidateDevice(DWORD* pNumPasses);
		virtual HRESULT STDMETHODCALLTYPE SetPaletteEntries(UINT,const PALETTEENTRY *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetPaletteEntries(UINT,PALETTEENTRY *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE SetCurrentTexturePalette(UINT) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetCurrentTexturePalette(UINT *) DX_STUB
		virtual BOOL STDMETHODCALLTYPE GetSoftwareVertexProcessing(void) DX_STUB
		virtual float STDMETHODCALLTYPE GetNPatchMode(void) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE ProcessVertices(UINT,UINT,UINT,IDirect3DVertexBuffer9 *,IDirect3DVertexDeclaration9 *,DWORD) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetVertexDeclaration(IDirect3DVertexDeclaration9 **) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetFVF(DWORD *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetVertexShader(IDirect3DVertexShader9 **) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetVertexShaderConstantF(UINT,float *,UINT) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetVertexShaderConstantI(UINT,int *,UINT) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetVertexShaderConstantB(UINT,BOOL *,UINT) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetStreamSource(UINT,IDirect3DVertexBuffer9 **,UINT *,UINT *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetStreamSourceFreq(UINT,UINT *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetPixelShader(IDirect3DPixelShader9 **) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetPixelShaderConstantF(UINT,float *,UINT) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetPixelShaderConstantI(UINT,int *,UINT) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetPixelShaderConstantB(UINT,BOOL *,UINT) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE DrawRectPatch(UINT,const float *,const D3DRECTPATCH_INFO *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE DrawTriPatch(UINT,const float *,const D3DTRIPATCH_INFO *) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE DeletePatch(UINT) DX_STUB

	//private:
		IDirect3DDevice9* device_;
	};

//-----------------------------------------------------------------------------

	class ResourceWrapper : public ComInterface
	{
	public:
		ResourceWrapper(IDirect3DResource9* resourceD3D);
		virtual ~ResourceWrapper();

		void set(IDirect3DResource9* resourceD3D);

		HRESULT GetDevice(IDirect3DDevice9** ppDevice);
		void PreLoad();
		D3DRESOURCETYPE GetType();

	//private:
		IDirect3DResource9* resource_;
	};

//-----------------------------------------------------------------------------

	class BaseTextureWrapper : public ResourceWrapper, public IDirect3DBaseTexture9
	{
		friend class DeviceWrapper;

	public:
		BaseTextureWrapper(IDirect3DBaseTexture9* baseTextureD3D);
		virtual ~BaseTextureWrapper();

		// IUnknown
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) DX_STUB
		virtual ULONG STDMETHODCALLTYPE AddRef();
		virtual ULONG STDMETHODCALLTYPE Release();

		// Resource
		virtual HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice9** ppDevice);
		virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid,void* pData,DWORD* pSizeOfData) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID refguid) DX_STUB
		virtual DWORD STDMETHODCALLTYPE SetPriority(DWORD PriorityNew) DX_STUB
		virtual DWORD STDMETHODCALLTYPE GetPriority() DX_STUB
		virtual void STDMETHODCALLTYPE PreLoad();
		virtual D3DRESOURCETYPE STDMETHODCALLTYPE GetType();

		// BaseTexture
		virtual DWORD STDMETHODCALLTYPE SetLOD(DWORD LODNew);
		virtual DWORD STDMETHODCALLTYPE GetLOD() DX_STUB
		virtual DWORD STDMETHODCALLTYPE GetLevelCount();
		virtual HRESULT STDMETHODCALLTYPE SetAutoGenFilterType(D3DTEXTUREFILTERTYPE FilterType);
		virtual D3DTEXTUREFILTERTYPE STDMETHODCALLTYPE GetAutoGenFilterType() DX_STUB_T(D3DTEXTUREFILTERTYPE)
		virtual void STDMETHODCALLTYPE GenerateMipSubLevels();

	//private:
		IDirect3DBaseTexture9* baseTexture_;
	};

//-----------------------------------------------------------------------------

	class TextureWrapper : public BaseTextureWrapper, public IDirect3DTexture9
	{
		friend class DeviceWrapper;

	public:
		TextureWrapper(IDirect3DTexture9* textureD3D, D3DFORMAT format, uint width, uint height );
		virtual ~TextureWrapper();

		// IUnknown
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj);
		virtual ULONG STDMETHODCALLTYPE AddRef();
		virtual ULONG STDMETHODCALLTYPE Release();

		// Resource
		virtual HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice9** ppDevice);
		virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid,void* pData,DWORD* pSizeOfData) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID refguid) DX_STUB
		virtual DWORD STDMETHODCALLTYPE SetPriority(DWORD PriorityNew) DX_STUB
		virtual DWORD STDMETHODCALLTYPE GetPriority() DX_STUB
		virtual void STDMETHODCALLTYPE PreLoad();
		virtual D3DRESOURCETYPE STDMETHODCALLTYPE GetType();

		// BaseTexture
		virtual DWORD STDMETHODCALLTYPE SetLOD(DWORD LODNew) DX_STUB
		virtual DWORD STDMETHODCALLTYPE GetLOD() DX_STUB
		virtual DWORD STDMETHODCALLTYPE GetLevelCount();
		virtual HRESULT STDMETHODCALLTYPE SetAutoGenFilterType(D3DTEXTUREFILTERTYPE FilterType);
		virtual D3DTEXTUREFILTERTYPE STDMETHODCALLTYPE GetAutoGenFilterType() DX_STUB_T(D3DTEXTUREFILTERTYPE)
		virtual void STDMETHODCALLTYPE GenerateMipSubLevels();

		// Methods
		virtual HRESULT STDMETHODCALLTYPE GetLevelDesc(UINT Level,D3DSURFACE_DESC *pDesc);
		virtual HRESULT STDMETHODCALLTYPE GetSurfaceLevel(UINT Level,IDirect3DSurface9** ppSurfaceLevel);
		virtual HRESULT STDMETHODCALLTYPE LockRect(UINT Level,D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags);
		virtual HRESULT STDMETHODCALLTYPE UnlockRect(UINT Level);
		virtual HRESULT STDMETHODCALLTYPE AddDirtyRect(CONST RECT* pDirtyRect);

	//private:
		IDirect3DTexture9* texture_;
		SurfaceWrapper* surface_[16];
		D3DFORMAT format_;
		uint width_;
		uint height_;

		bool locked_;
	};

//-----------------------------------------------------------------------------

	class CubeTextureWrapper : public BaseTextureWrapper, public IDirect3DCubeTexture9
	{
		friend class DeviceWrapper;

	public:
		CubeTextureWrapper(IDirect3DCubeTexture9* cubeTextureD3D);
		virtual ~CubeTextureWrapper();

		// IUnknown
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) DX_STUB
		virtual ULONG STDMETHODCALLTYPE AddRef();
		virtual ULONG STDMETHODCALLTYPE Release();

		// Resource
		virtual HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice9** ppDevice);
		virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid,void* pData,DWORD* pSizeOfData) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID refguid) DX_STUB
		virtual DWORD STDMETHODCALLTYPE SetPriority(DWORD PriorityNew) DX_STUB
		virtual DWORD STDMETHODCALLTYPE GetPriority() DX_STUB
		virtual void STDMETHODCALLTYPE PreLoad();
		virtual D3DRESOURCETYPE STDMETHODCALLTYPE GetType();

		// BaseTexture
		virtual DWORD STDMETHODCALLTYPE SetLOD(DWORD LODNew);
		virtual DWORD STDMETHODCALLTYPE GetLOD() DX_STUB
		virtual DWORD STDMETHODCALLTYPE GetLevelCount();
		virtual HRESULT STDMETHODCALLTYPE SetAutoGenFilterType(D3DTEXTUREFILTERTYPE FilterType) DX_STUB
		virtual D3DTEXTUREFILTERTYPE STDMETHODCALLTYPE GetAutoGenFilterType() DX_STUB_T(D3DTEXTUREFILTERTYPE)
		virtual void STDMETHODCALLTYPE GenerateMipSubLevels() DX_STUB_T(void)

		// Methods
		virtual HRESULT STDMETHODCALLTYPE GetLevelDesc(UINT Level,D3DSURFACE_DESC *pDesc);
		virtual HRESULT STDMETHODCALLTYPE GetCubeMapSurface(D3DCUBEMAP_FACES FaceType,UINT Level,IDirect3DSurface9** ppCubeMapSurface);
		virtual HRESULT STDMETHODCALLTYPE LockRect(D3DCUBEMAP_FACES FaceType,UINT Level,D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags);
		virtual HRESULT STDMETHODCALLTYPE UnlockRect(D3DCUBEMAP_FACES FaceType,UINT Level);

		// Stubs
		virtual HRESULT STDMETHODCALLTYPE AddDirtyRect(D3DCUBEMAP_FACES FaceType,CONST RECT* pDirtyRect) DX_STUB

	//private:
		IDirect3DCubeTexture9* cubeTexture_;
		SurfaceWrapper* surface_[6][16];
	};

//-----------------------------------------------------------------------------

	class SurfaceWrapper : public ResourceWrapper, public IDirect3DSurface9
	{
		friend class DeviceWrapper;

	public:
		SurfaceWrapper(IDirect3DSurface9* Surface);
		virtual ~SurfaceWrapper();

		// IUnknown
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) DX_STUB
		virtual ULONG STDMETHODCALLTYPE AddRef();
		virtual ULONG STDMETHODCALLTYPE Release();

		// Resource
		virtual HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice9** ppDevice);
		virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid,void* pData,DWORD* pSizeOfData) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID refguid) DX_STUB
		virtual DWORD STDMETHODCALLTYPE SetPriority(DWORD PriorityNew) DX_STUB
		virtual DWORD STDMETHODCALLTYPE GetPriority() DX_STUB
		virtual void STDMETHODCALLTYPE PreLoad();
		virtual D3DRESOURCETYPE STDMETHODCALLTYPE GetType();

		// Methods
		virtual HRESULT STDMETHODCALLTYPE GetDesc(D3DSURFACE_DESC *pDesc);
		virtual HRESULT STDMETHODCALLTYPE GetContainer(REFIID riid,void** ppContainer);
		virtual HRESULT STDMETHODCALLTYPE LockRect(D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags);
		virtual HRESULT STDMETHODCALLTYPE UnlockRect();

		// Stubs
		virtual HRESULT STDMETHODCALLTYPE GetDC(HDC *phdc) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE ReleaseDC(HDC hdc) DX_STUB

	//private:
		IDirect3DSurface9* surface_;
	};

//-----------------------------------------------------------------------------

	class VertexBufferWrapper : public ResourceWrapper, public IDirect3DVertexBuffer9
	{
		friend class DeviceWrapper;

	public:
		VertexBufferWrapper(IDirect3DVertexBuffer9* vertexBufferD3D, uint size, DWORD usage);
		virtual ~VertexBufferWrapper();

		void set(IDirect3DVertexBuffer9* vertexBufferD3D);

		// IUnknown
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) DX_STUB
		virtual ULONG STDMETHODCALLTYPE AddRef();
		virtual ULONG STDMETHODCALLTYPE Release();

		// Resource
		virtual HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice9** ppDevice);
		virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid,void* pData,DWORD* pSizeOfData) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID refguid) DX_STUB
		virtual DWORD STDMETHODCALLTYPE SetPriority(DWORD PriorityNew) DX_STUB
		virtual DWORD STDMETHODCALLTYPE GetPriority() DX_STUB
		virtual void STDMETHODCALLTYPE PreLoad();
		virtual D3DRESOURCETYPE STDMETHODCALLTYPE GetType();

		// Methods
		virtual HRESULT STDMETHODCALLTYPE Lock(UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags);
		virtual HRESULT STDMETHODCALLTYPE Unlock();

		// Stubs
		virtual HRESULT STDMETHODCALLTYPE GetDesc(D3DVERTEXBUFFER_DESC *pDesc);

	//private:
		IDirect3DVertexBuffer9* vertexBuffer_;
		uint size_;
		DWORD usage_;
	};

//-----------------------------------------------------------------------------

	class IndexBufferWrapper : public ResourceWrapper, public IDirect3DIndexBuffer9
	{
		friend class DeviceWrapper;

	public:
		IndexBufferWrapper(IDirect3DIndexBuffer9* indexBufferD3D, uint size, DWORD usage);
		virtual ~IndexBufferWrapper();

		// IUnknown
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) DX_STUB
		virtual ULONG STDMETHODCALLTYPE AddRef();
		virtual ULONG STDMETHODCALLTYPE Release();

		// Resource
		virtual HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice9** ppDevice);
		virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid,void* pData,DWORD* pSizeOfData) DX_STUB
		virtual HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID refguid) DX_STUB
		virtual DWORD STDMETHODCALLTYPE SetPriority(DWORD PriorityNew) DX_STUB
		virtual DWORD STDMETHODCALLTYPE GetPriority() DX_STUB
		virtual void STDMETHODCALLTYPE PreLoad();
		virtual D3DRESOURCETYPE STDMETHODCALLTYPE GetType();

		// Methods
		virtual HRESULT STDMETHODCALLTYPE Lock(UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags);
		virtual HRESULT STDMETHODCALLTYPE Unlock();

		// Stubs
		virtual HRESULT STDMETHODCALLTYPE GetDesc(D3DINDEXBUFFER_DESC *pDesc) DX_STUB

	//private:
		IDirect3DIndexBuffer9* indexBuffer_;
		uint size_;
		DWORD usage_;
	};

//-----------------------------------------------------------------------------

	class VertexShaderWrapper : public ComInterface, public IDirect3DVertexShader9
	{
		friend class DeviceWrapper;

	public:
		VertexShaderWrapper(IDirect3DVertexShader9* vertexShader);
		virtual ~VertexShaderWrapper();

		// IUnknown
	    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) DX_STUB
		virtual ULONG STDMETHODCALLTYPE AddRef();
		virtual ULONG STDMETHODCALLTYPE Release();

		// Methods
		virtual HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice9** ppDevice);
		virtual HRESULT STDMETHODCALLTYPE GetFunction(void*,UINT* pSizeOfData);

	//private:
		IDirect3DVertexShader9* vertexShader_;
	};

//-----------------------------------------------------------------------------

	class PixelShaderWrapper : public ComInterface, public IDirect3DPixelShader9
	{
		friend class DeviceWrapper;

	public:
		PixelShaderWrapper(IDirect3DPixelShader9* PixelShader);
		virtual ~PixelShaderWrapper();

		// IUnknown
	    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) DX_STUB
		virtual ULONG STDMETHODCALLTYPE AddRef();
		virtual ULONG STDMETHODCALLTYPE Release();

		// Methods
		virtual HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice9** ppDevice);
		virtual HRESULT STDMETHODCALLTYPE GetFunction(void*,UINT* pSizeOfData);

	//private:
		IDirect3DPixelShader9* pixelShader_;
	};

//-----------------------------------------------------------------------------

	class VertexDeclarationWrapper : public ComInterface, public IDirect3DVertexDeclaration9
	{
		friend class DeviceWrapper;

	public:
		VertexDeclarationWrapper(IDirect3DVertexDeclaration9* vertexDeclaration);
		virtual ~VertexDeclarationWrapper();

		// IUnknown
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) DX_STUB
		virtual ULONG STDMETHODCALLTYPE AddRef();
		virtual ULONG STDMETHODCALLTYPE Release();

		// Methods
		virtual HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice9** ppDevice);
		virtual HRESULT STDMETHODCALLTYPE GetDeclaration(D3DVERTEXELEMENT9* pElement,UINT* pNumElements);

	//private:
		IDirect3DVertexDeclaration9* vertexDeclaration_;
	};

//-----------------------------------------------------------------------------

	class QueryWrapper : public ComInterface, public IDirect3DQuery9
	{
		friend class DeviceWrapper;

	public:
		QueryWrapper(IDirect3DQuery9* query);
		virtual ~QueryWrapper();

		// IUnknown
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) DX_STUB
		virtual ULONG STDMETHODCALLTYPE AddRef();
		virtual ULONG STDMETHODCALLTYPE Release();

		// Methods
		virtual HRESULT STDMETHODCALLTYPE Issue(DWORD dwIssueFlags);
		virtual HRESULT STDMETHODCALLTYPE GetData(void* pData,DWORD dwSize,DWORD dwGetDataFlags);

		// Stubs
		virtual HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice9** ppDevice) DX_STUB
		virtual D3DQUERYTYPE STDMETHODCALLTYPE GetType() DX_STUB_T(D3DQUERYTYPE)
		virtual DWORD STDMETHODCALLTYPE GetDataSize() DX_STUB

	//private:
		IDirect3DQuery9* query_;
		volatile LONG issueQueued_;
		uint64 issueCookie_;
	};

//-----------------------------------------------------------------------------

	class StateBlockWrapper : public ComInterface, public IDirect3DStateBlock9
	{
		friend class DeviceWrapper;

	public:
		StateBlockWrapper(IDirect3DStateBlock9* query);
		virtual ~StateBlockWrapper();

		// IUnknown
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) DX_STUB
		virtual ULONG STDMETHODCALLTYPE AddRef();
		virtual ULONG STDMETHODCALLTYPE Release();

		// Methods
		virtual HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice9** ppDevice);
		virtual HRESULT STDMETHODCALLTYPE Capture();
		virtual HRESULT STDMETHODCALLTYPE Apply();

	//private:
		IDirect3DStateBlock9* stateBlock_;
	};
}

//-----------------------------------------------------------------------------
// StateManager for the wrapped effects. It uses the unwrapped device.
//-----------------------------------------------------------------------------

class WrapperStateManager : public ID3DXEffectStateManager, public SafeReferenceCount
{
public:
	WrapperStateManager();
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
	HRESULT __stdcall SetTextureStageState( DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value );
	HRESULT __stdcall SetTransform( D3DTRANSFORMSTATETYPE state, CONST D3DMATRIX* pMatrix );

	IDirect3DDevice9* device_;

	static WrapperStateManager* s_stateManager;

	void resetCache();
};

//-----------------------------------------------------------------------------

extern DWORD gdx_renderStateCache[D3DRS_MAX];
extern DWORD gdx_textureStageStateCache[D3DFFSTAGES_MAX][D3DTSS_MAX];
extern DWORD gdx_samplerStateCache[D3DSAMPSTAGES_MAX][D3DSAMP_MAX];
extern DX::BaseTexture* gdx_textureCache[D3DSAMPSTAGES_MAX];

extern IDirect3DDevice9* gdx_device;

class ThreadInfo
{
public:
	CRITICAL_SECTION mutex_;
	CommandBuffer commandBuffer_;
};

extern THREADLOCAL( ThreadInfo* ) gdx_threadInfo;
extern THREADLOCAL(bool) gdx_isMainThread;
extern THREADLOCAL(bool) gdx_isDXThread;
extern uint32 gdx_currentFrame;

extern THREADLOCAL( CommandBuffer* ) gdx_commandBuffer;

extern bool gdx_mirroredTransform;

//-----------------------------------------------------------------------------

void flush( bool flush = false );

//#define flushAndStall()	\
//	{ WARNING_MSG( "D3D Performance: Flush & Stall in %s\n", __FUNCTION__ ); flush( true ); }

#define flushAndStall()														\
	flush( true )

#if ENABLE_STACK_TRACKER
extern SimpleMutex gdx_trackerMutex;
#endif

//-----------------------------------------------------------------------------
// Secondary thread write mutex
//-----------------------------------------------------------------------------

static inline void lockCommandBuffer()
{
	if ( gdx_isMainThread )
		return;

	EnterCriticalSection( &gdx_threadInfo->mutex_ );
}

static inline void unlockCommandBuffer()
{
	if ( gdx_isMainThread )
		return;

	LeaveCriticalSection( &gdx_threadInfo->mutex_ );
}

class CommandBufferWriteLock
{
public:
	CommandBufferWriteLock()
	{
		lockCommandBuffer();
	}

	~CommandBufferWriteLock()
	{
		unlockCommandBuffer();
	}
};

#define WRITE_LOCK CommandBufferWriteLock writeLock;


#endif // MOO_DX_HPP
