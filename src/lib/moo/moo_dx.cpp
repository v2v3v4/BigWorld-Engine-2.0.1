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
#include <map>

DECLARE_DEBUG_COMPONENT2( "DX Wrapper", 0 )

#ifdef ENABLE_DX_TRACKER
#include "cstdmf/stack_tracker.hpp"
#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#else
#undef MEMTRACKER_SCOPED
#undef PROFILER_SCOPED
#define MEMTRACKER_SCOPED(slot)
#define PROFILER_SCOPED(slot)
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

MEMTRACKER_DECLARE( DX, "DX Wrapper", 0 );
PROFILER_DECLARE2( DX, "DX Wrapper", Profiler::FLAG_WATCH );

//-----------------------------------------------------------------------------
// Rule: Dynamic vertex buffers cannot be locked from the loading thread
// This is because they defer, and we don't allow writes from the loading thread
//-----------------------------------------------------------------------------

typedef void ExecuteCommand();

//-----------------------------------------------------------------------------

// The COM interface for the device wrapper
IDirect3DDevice9* gdx_device;

// Render state caches
DWORD gdx_renderStateCache[D3DRS_MAX];
DWORD gdx_textureStageStateCache[D3DFFSTAGES_MAX][D3DTSS_MAX];
DWORD gdx_samplerStateCache[D3DSAMPSTAGES_MAX][D3DSAMP_MAX];
DX::BaseTexture* gdx_textureCache[D3DSAMPSTAGES_MAX];

// Cache for set and get so we don't have to flush
static DX::SurfaceWrapper* RenderTarget_[4];
static DX::SurfaceWrapper* DepthStencilSurface_;

static IDirect3DSurface9* RenderTargetD3D_[4];
static IDirect3DSurface9* DepthStencilSurfaceD3D_;

static RECT ScissorRect_;
static D3DVIEWPORT9 Viewport_;
static DX::IndexBufferWrapper* Indices_;

// Command buffer for deferred functions
THREADLOCAL( CommandBuffer* ) gdx_commandBuffer;

// Maintain a count of deferred locks. We cannot flush while there is an
// outstanding lock since this could mean that the flush will copy
// incomplete data into the resource
static THREADLOCAL( uint ) s_lockCount;

// Watcher stats
static volatile LONG s_numTextures;
static volatile LONG s_numCubeTextures;
static volatile LONG s_numSurfaces;
static volatile LONG s_numVertexBuffers;
static volatile LONG s_numIndexBuffers;
static volatile LONG s_numVertexShaders;
static volatile LONG s_numPixelShaders;
static volatile LONG s_numVertexDeclarations;
static volatile LONG s_numQueries;

static uint s_flushCount;
static bool s_forceFlush;
static uint s_skipFlush;

static uint s_commandObjectSize;
static uint s_commandRawSize;
static uint s_commandSeekSize;
static bool s_commandPrintTraces;

// List of all objects
#ifdef ENABLE_DX_TRACKER
SimpleMutex gdx_trackerMutex;

static ListNode s_TexturesList;
static ListNode s_CubeTexturesList;
static ListNode s_SurfacesList;
static ListNode s_VertexBuffersList;
static ListNode s_IndexBuffersList;
static ListNode s_VertexShadersList;
static ListNode s_PixelShadersList;
static ListNode s_VertexDeclarationsList;
static ListNode s_QueriesList;
static ListNode s_StateBlocksList;

#endif

class SpikeDetector
{
public:
	SpikeDetector()
	{
		startTime_ = timestamp();
	}

	~SpikeDetector()
	{
		if ( !gdx_isDXThread )
			return;
		uint64 endTime = timestamp();
		static const double stampsPerMs = stampsPerSecondD() / 1000.0;
		float t = ( float )( ( double )( endTime - startTime_ ) / stampsPerMs );
		if ( t > 50.0f )
		{
			DEBUG_MSG( "Spike! %f\n", t );
		}
	}

	uint64 startTime_;
};

//#undef BW_GUARD
//#define BW_GUARD SpikeDetector detector;

//-----------------------------------------------------------------------------
// Create the wrapper device
//-----------------------------------------------------------------------------

IDirect3DDevice9* DX::createDeviceWrapper(IDirect3DDevice9* deviceD3D)
{
#ifdef ENABLE_DX_TRACKER
	{
		SimpleMutexHolder smh( gdx_trackerMutex );

		s_TexturesList.setAsRoot();
		s_CubeTexturesList.setAsRoot();
		s_SurfacesList.setAsRoot();
		s_VertexBuffersList.setAsRoot();
		s_IndexBuffersList.setAsRoot();
		s_VertexShadersList.setAsRoot();
		s_PixelShadersList.setAsRoot();
		s_VertexDeclarationsList.setAsRoot();
		s_QueriesList.setAsRoot();
		s_StateBlocksList.setAsRoot();
	}
#endif

	// Init the job system
	new JobSystem;
	JobSystem* jobSystem = JobSystem::pInstance();
	jobSystem->init();

	gdx_commandBuffer = jobSystem->getConsumptionCommands();

	// Create the device wrapper
	return new DX::DeviceWrapper( deviceD3D );
}

//-----------------------------------------------------------------------------
// Tracking functionality
//-----------------------------------------------------------------------------

#ifdef ENABLE_DX_TRACKER

void printAllocatedObjects()
{
	DEBUG_MSG( "--------- DX Objects ---------\n" );
	DEBUG_MSG( "Textures: %d\n", s_numTextures );
	DEBUG_MSG( "CubeTextures: %d\n", s_numCubeTextures );
	DEBUG_MSG( "Surfaces: %d\n", s_numSurfaces );
	DEBUG_MSG( "VertexBuffers: %d\n", s_numVertexBuffers );
	DEBUG_MSG( "IndexBuffers: %d\n", s_numIndexBuffers );
	DEBUG_MSG( "VertexShaders: %d\n", s_numVertexShaders );
	DEBUG_MSG( "PixelShaders: %d\n", s_numPixelShaders );
	DEBUG_MSG( "VertexDeclarations: %d\n", s_numVertexDeclarations );
	DEBUG_MSG( "Queries: %d\n", s_numQueries );
}

uint getRefCount( IUnknown* unknown )
{
	unknown->AddRef();
	return unknown->Release();
}

namespace
{

PyObject* py_dxPrintSurfaces( PyObject * args )
{
	SimpleMutexHolder smh( gdx_trackerMutex );

	DEBUG_MSG( "Num = %d\n", s_numSurfaces );

	for ( ListNode* node = s_SurfacesList.getNext(); node != &s_SurfacesList; node = node->getNext() )
	{
		DX::ComInterface* object = CAST_NODE( node, DX::ComInterface, node_ );

		DEBUG_MSG( "(ref = %d, %d): %s\n",
			object->refCount_, getRefCount( object->comInterface_ ), object->stack_.c_str() );
	}

	Py_Return;
}
PY_MODULE_FUNCTION( dxPrintSurfaces, BigWorld )

}

//-----------------------------------------------------------------------------

class DXShutdown
{
public:
	DXShutdown();
	~DXShutdown();
};

DXShutdown::DXShutdown()
{
}

DXShutdown::~DXShutdown()
{
	printAllocatedObjects();
}

DXShutdown s_shutdown;

#endif

//-----------------------------------------------------------------------------

static THREADLOCAL( uint32 ) s_flags;

void DX::setWrapperFlags(uint32 flags)
{
	s_flags = flags;
}

uint32 DX::getWrapperFlags()
{
	return s_flags;
}


//-----------------------------------------------------------------------------
// Command buffer patching
//-----------------------------------------------------------------------------

static uint* s_patchPrimCount;

uint32* DX::getPatchForPrimCount()
{
	return s_patchPrimCount;
}


//-----------------------------------------------------------------------------
// Secondary threads
//-----------------------------------------------------------------------------

THREADLOCAL( ThreadInfo* ) gdx_threadInfo;
THREADLOCAL(bool) gdx_isMainThread;
THREADLOCAL(bool) gdx_isDXThread;
static std::vector<ThreadInfo*> s_secondaryThreads;

static void flushSecondaryThreads();

//-----------------------------------------------------------------------------

void DX::addFakeMainThread()
{
	gdx_isMainThread = true;
}

//-----------------------------------------------------------------------------

void DX::addSecondaryThread()
{
	gdx_threadInfo = new ThreadInfo;

	gdx_commandBuffer = &gdx_threadInfo->commandBuffer_;
	gdx_commandBuffer->init( 4*1024*1024 );
	gdx_commandBuffer->nextWrite();	// Force opposite read and write buffers

	InitializeCriticalSection( &gdx_threadInfo->mutex_ );

	s_secondaryThreads.push_back( gdx_threadInfo );
}

//-----------------------------------------------------------------------------
// DirectX thread (job system consumption)
//-----------------------------------------------------------------------------

class FlushCommands : public SyncBlock
{
public:
	virtual void consume();
};

//-----------------------------------------------------------------------------

void FlushCommands::consume()
{
	BW_GUARD;

	gdx_isDXThread = true;

	// Flush secondary threads first as the main thread can be dependent
	flushSecondaryThreads();

	// Set the thread local command buffer for reading
	gdx_commandBuffer = JobSystem::instance().getConsumptionCommands();

	// Execute the commands in this sync block
	while ( gdx_commandBuffer->getCurrent() < next_ )
	{
		ExecuteCommand* func = gdx_commandBuffer->read<ExecuteCommand*>();
		func();
	}
}

//-----------------------------------------------------------------------------

static void flushSecondaryThreads()
{
	// Now flush the secondary command buffers
	for ( std::vector<ThreadInfo*>::iterator it = s_secondaryThreads.begin();
		it != s_secondaryThreads.end(); ++it )
	{
		ThreadInfo* threadInfo = *it;

		gdx_commandBuffer = &threadInfo->commandBuffer_;

		// Flip command buffer
		EnterCriticalSection( &threadInfo->mutex_ );
		void* end = gdx_commandBuffer->getCurrentWrite();
		gdx_commandBuffer->nextWrite();
		gdx_commandBuffer->nextRead();
		LeaveCriticalSection( &threadInfo->mutex_ );

		// Execute command buffer
		while ( gdx_commandBuffer->getCurrent() < end )
		{
			ExecuteCommand* func = gdx_commandBuffer->read<ExecuteCommand*>();
			func();
		}
	}
}

//-----------------------------------------------------------------------------

class NullJob : public Job
{
public:
	virtual void execute();
};

//-----------------------------------------------------------------------------

void NullJob::execute()
{
}

//-----------------------------------------------------------------------------

static uint64 s_frameCount;
static uint s_subFrameCount;

static void beginFrame()
{
	JobSystem* jobSystem = JobSystem::pInstance();

	jobSystem->beginFrame();
	jobSystem->allocSyncBlock<FlushCommands>();
	jobSystem->allocJob<NullJob>();

	s_frameCount++;
	s_subFrameCount++;
}

//-----------------------------------------------------------------------------

void flush( bool flush )
{
	BW_GUARD;

	if ( !gdx_isMainThread )
		return;

	JobSystem* jobSystem = JobSystem::pInstance();

	jobSystem->endFrame();

	if ( flush || s_forceFlush )
	{
		jobSystem->flush();
		s_flushCount++;
	}
	else
	{
		s_flushCount = 0;
	}

	beginFrame();
}

//-----------------------------------------------------------------------------

void DX::newFrame()
{
	if ( gdx_device )
	{
		flush();
	}
}

void DX::newBlock()
{
	JobSystem* jobSystem = JobSystem::pInstance();

	jobSystem->allocSyncBlock<FlushCommands>();
	jobSystem->allocJob<NullJob>();
}

//-----------------------------------------------------------------------------

static uint32 s_currentFrame;

static void playSetCurrentFrame()
{
	BW_GUARD;
	s_currentFrame = gdx_commandBuffer->read<uint32>(); 
}

void DX::setCurrentFrame(uint32 currentFrame )
{
	gdx_commandBuffer->write(playSetCurrentFrame, currentFrame);
}

uint32 DX::getCurrentFrame()
{
	MF_ASSERT( gdx_isDXThread );
	return s_currentFrame;
}

//-----------------------------------------------------------------------------

char gdx_caller[1024];

inline void readCaller()
{
#ifdef ENABLE_DX_TRACKER
	uint len = gdx_commandBuffer->read<uint>();
	char* caller = (char*)gdx_commandBuffer->getCurrent();
	gdx_commandBuffer->seek( len );
	strncpy( gdx_caller, caller, sizeof( gdx_caller ) );
//	DEBUG_MSG("%s\n",caller);
#endif
}

inline void writeCaller()
{
#ifdef ENABLE_DX_TRACKER
	std::string report = StackTracker::buildReport();
	uint len = strlen(report.c_str()) + 1;
	gdx_commandBuffer->write(len);
	gdx_commandBuffer->writeRaw(report.c_str(), len);
#endif
}

//-----------------------------------------------------------------------------

static void invalidateStateCache()
{
	uint i;
	uint j;

	for ( i = 0; i < D3DFFSTAGES_MAX; i++ )
	{
		for( j = 0; j < D3DTSS_MAX; j++ )
		{
			gdx_textureStageStateCache[i][j] = 0xff81ff81;
		}
	}

	for( i = 0; i < D3DSAMPSTAGES_MAX; i++ )
	{
		for( j = 0; j < D3DSAMP_MAX; j++ )
		{
			gdx_samplerStateCache[i][j] = 0xff81ff81;
		}
		gdx_textureCache[i] = (DX::BaseTexture*)0xff81ff81;
	}

	for( i = 0; i < D3DRS_MAX; i++ )
	{
		gdx_renderStateCache[i] = 0xff81ff81;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

DX::DeviceWrapper::DeviceWrapper( IDirect3DDevice9* device )
:	ComInterface( device )
,	device_( device )
{
	gdx_device = this;

	createCache();
	invalidateStateCache();

	gdx_isMainThread = true;

	MF_WATCH( "Render/D3D Wrapper/Textures", (LONG)s_numTextures, Watcher::WT_READ_ONLY );
	MF_WATCH( "Render/D3D Wrapper/Cube Textures", (LONG)s_numCubeTextures, Watcher::WT_READ_ONLY );
	MF_WATCH( "Render/D3D Wrapper/Surfaces", (LONG)s_numSurfaces, Watcher::WT_READ_ONLY );
	MF_WATCH( "Render/D3D Wrapper/Vertex Buffers", (LONG)s_numVertexBuffers, Watcher::WT_READ_ONLY );
	MF_WATCH( "Render/D3D Wrapper/Index Buffers", (LONG)s_numIndexBuffers, Watcher::WT_READ_ONLY );
	MF_WATCH( "Render/D3D Wrapper/Vertex Shaders", (LONG)s_numVertexShaders, Watcher::WT_READ_ONLY );
	MF_WATCH( "Render/D3D Wrapper/Pixel Shaders", (LONG)s_numPixelShaders, Watcher::WT_READ_ONLY );
	MF_WATCH( "Render/D3D Wrapper/Vertex Declarations", (LONG)s_numVertexDeclarations, Watcher::WT_READ_ONLY );
	MF_WATCH( "Render/D3D Wrapper/Queries", (LONG)s_numQueries, Watcher::WT_READ_ONLY );

	MF_WATCH( "Render/D3D Wrapper/Flushes", s_flushCount, Watcher::WT_READ_ONLY );
	MF_WATCH( "Render/D3D Wrapper/Force Flush", s_forceFlush, Watcher::WT_READ_WRITE );
	MF_WATCH( "Render/D3D Wrapper/Skip Flush", s_skipFlush, Watcher::WT_READ_ONLY );

	MF_WATCH( "Render/D3D Wrapper/Command Object Size", s_commandObjectSize, Watcher::WT_READ_ONLY );
	MF_WATCH( "Render/D3D Wrapper/Command Raw Size", s_commandRawSize, Watcher::WT_READ_ONLY );
	MF_WATCH( "Render/D3D Wrapper/Command Seek Size", s_commandSeekSize, Watcher::WT_READ_ONLY );
	MF_WATCH( "Render/D3D Wrapper/Command Print Traces", s_commandPrintTraces, Watcher::WT_READ_WRITE );

	// Begin the first frame
	beginFrame();
}

DX::DeviceWrapper::~DeviceWrapper()
{
}

void DX::DeviceWrapper::createCache()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	HRESULT res;

	res = device_->GetRenderTarget(0, &RenderTargetD3D_[0]);
	MF_ASSERT( res == D3D_OK );
	RenderTarget_[0] = new SurfaceWrapper(RenderTargetD3D_[0]);

	res = device_->GetRenderTarget(1, &RenderTargetD3D_[1]);
	if ( res == D3D_OK )
	{
		RenderTarget_[1] = new SurfaceWrapper(RenderTargetD3D_[1]);
	}
	else
	{
		RenderTarget_[1] = NULL;
	}

	res = device_->GetDepthStencilSurface(&DepthStencilSurfaceD3D_);
	MF_ASSERT( res == D3D_OK );
	DepthStencilSurface_ = new SurfaceWrapper(DepthStencilSurfaceD3D_);

	device_->GetScissorRect(&ScissorRect_);
	device_->GetViewport(&Viewport_);
}

void DX::DeviceWrapper::destroyCache()
{
	if ( DepthStencilSurface_ )
	{
		DepthStencilSurface_->Release();
		DepthStencilSurface_ = NULL;
	}

	if ( RenderTarget_[0] )
	{
		RenderTarget_[0]->Release();
		RenderTarget_[0] = NULL;
	}

	if ( RenderTarget_[1] )
	{
		RenderTarget_[1]->Release();
		RenderTarget_[1] = NULL;
	}
}

//-----------------------------------------------------------------------------
// IUnknown

HRESULT DX::DeviceWrapper::QueryInterface(REFIID riid, void** ppvObj)
{
	return E_NOINTERFACE;
}

//-----------------------------------------------------------------------------

ULONG DX::DeviceWrapper::AddRef()
{
	return ComInterface::AddRef();
}

//-----------------------------------------------------------------------------

ULONG DX::DeviceWrapper::Release()
{
	return ComInterface::Release();
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::TestCooperativeLevel()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return device_->TestCooperativeLevel();
}

//-----------------------------------------------------------------------------

UINT DX::DeviceWrapper::GetAvailableTextureMem()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	flushAndStall();
	return device_->GetAvailableTextureMem();
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::EvictManagedResources()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	flushAndStall();
	return device_->EvictManagedResources();
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::SetCursorProperties(UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	SurfaceWrapper* pCursorBitmapWrapper = (SurfaceWrapper*)pCursorBitmap;

	return device_->SetCursorProperties(XHotSpot,YHotSpot,pCursorBitmapWrapper->surface_);
}

//-----------------------------------------------------------------------------

void DX::DeviceWrapper::SetCursorPosition(int X,int Y,DWORD Flags)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return device_->SetCursorPosition(X,Y,Flags);
}

//-----------------------------------------------------------------------------

BOOL DX::DeviceWrapper::ShowCursor(BOOL bShow)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return device_->ShowCursor(bShow);
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	flushAndStall();
	WrapperStateManager::s_stateManager->resetCache();
	destroyCache();
	flushAndStall();

	HRESULT res = device_->Reset(pPresentationParameters);

	if ( res == D3D_OK )
	{
		createCache();
		invalidateStateCache();
	}

	return res;
}

//-----------------------------------------------------------------------------

static void playPresent()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	device->Present(NULL,NULL,NULL,NULL);
}

HRESULT DX::DeviceWrapper::Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	MF_ASSERT(pSourceRect == NULL);
	MF_ASSERT(pDestRect == NULL);
	MF_ASSERT(hDestWindowOverride == NULL);
	MF_ASSERT(pDirtyRegion == NULL);

	gdx_commandBuffer->write(playPresent,device_);

	// Profile the command buffer to this point
	s_commandObjectSize = gdx_commandBuffer->writeObjectSize_;
	s_commandRawSize = gdx_commandBuffer->writeRawSize_;
	s_commandSeekSize = gdx_commandBuffer->writeSeekSize_;

	gdx_commandBuffer->writeObjectSize_ = 0;
	gdx_commandBuffer->writeRawSize_ = 0;
	gdx_commandBuffer->writeSeekSize_ = 0;

#if ENABLE_STACK_TRACKER
	if ( s_commandPrintTraces )
	{
		gdx_commandBuffer->traces_.print();
		s_commandPrintTraces = false;
	}

	gdx_commandBuffer->traces_.reset();
#endif

	// Flush
	flush();			// Deliberate flush

	JobSystem::instance().resetProfiling();
	s_subFrameCount = 0;

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::GetBackBuffer(UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	flushAndStall();

	IDirect3DSurface9* surfaceD3D;
	HRESULT res = device_->GetBackBuffer(iSwapChain,iBackBuffer,Type,&surfaceD3D);

	if ( res == D3D_OK )
		*ppBackBuffer = new SurfaceWrapper(surfaceD3D);

	return res;
}

//-----------------------------------------------------------------------------

static void playSetGammaRamp()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	UINT iSwapChain = gdx_commandBuffer->read<UINT>();
	DWORD Flags = gdx_commandBuffer->read<DWORD>();
	CONST D3DGAMMARAMP pRamp = gdx_commandBuffer->read<CONST D3DGAMMARAMP>();

	device->SetGammaRamp(iSwapChain,Flags,&pRamp);
}

void DX::DeviceWrapper::SetGammaRamp(UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playSetGammaRamp,device_,iSwapChain,Flags,*pRamp);
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	IDirect3DTexture9* textureD3D;
	HRESULT res = device_->CreateTexture(Width,Height,Levels,Usage,Format,Pool,&textureD3D,pSharedHandle);

	if ( res == D3D_OK )
	{
		*ppTexture = new TextureWrapper(textureD3D, Format, Width, Height);
	}

	return res;
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::CreateCubeTexture(UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	IDirect3DCubeTexture9* cubeTextureD3D;
	HRESULT res = device_->CreateCubeTexture(EdgeLength,Levels,Usage,Format,Pool,&cubeTextureD3D, pSharedHandle);

	if ( res == D3D_OK )
		*ppCubeTexture = new CubeTextureWrapper( cubeTextureD3D );

	return res;
}

//-----------------------------------------------------------------------------

#if 0

static void playCreateVertexBuffer()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	UINT Length = gdx_commandBuffer->read<UINT>();
	DWORD Usage = gdx_commandBuffer->read<DWORD>();
	DWORD FVF = gdx_commandBuffer->read<DWORD>();
	D3DPOOL Pool = gdx_commandBuffer->read<D3DPOOL>();
	DX::VertexBufferWrapper* vertexBufferWrapper = gdx_commandBuffer->read<DX::VertexBufferWrapper*>();

	IDirect3DVertexBuffer9* vertexBufferD3D;
	HRESULT res = device->CreateVertexBuffer(Length,Usage,FVF,Pool,&vertexBufferD3D,NULL);
	MF_ASSERT( res == D3D_OK );

	vertexBufferWrapper->set(vertexBufferD3D);
}

HRESULT DX::DeviceWrapper::CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	VertexBufferWrapper* vertexBuffer = new VertexBufferWrapper(NULL, Length, Usage);
	*ppVertexBuffer = vertexBuffer;

	gdx_commandBuffer->write(playCreateVertexBuffer,device_,Length,Usage,FVF,Pool,vertexBuffer);

	return D3D_OK;
}

#else

HRESULT DX::DeviceWrapper::CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	IDirect3DVertexBuffer9* vertexBufferD3D;
	HRESULT res = device_->CreateVertexBuffer(Length,Usage,FVF,Pool,&vertexBufferD3D,pSharedHandle);

	if ( res == D3D_OK )
		*ppVertexBuffer = new VertexBufferWrapper(vertexBufferD3D, Length, Usage);

	return res;
}

#endif

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	IDirect3DIndexBuffer9* indexBufferD3D;
	HRESULT res = device_->CreateIndexBuffer(Length,Usage,Format,Pool,&indexBufferD3D,pSharedHandle);

	if ( res == D3D_OK )
		*ppIndexBuffer = new IndexBufferWrapper(indexBufferD3D, Length, Usage);

	return res;
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::CreateRenderTarget(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	IDirect3DSurface9* surfaceD3D;
	HRESULT res = device_->CreateRenderTarget(Width,Height,Format,MultiSample,MultisampleQuality,Lockable,&surfaceD3D,pSharedHandle);

	if ( res == D3D_OK )
		*ppSurface = new SurfaceWrapper(surfaceD3D);

	return res;
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::CreateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	IDirect3DSurface9* surfaceD3D;
	HRESULT res = device_->CreateDepthStencilSurface(Width,Height,Format,MultiSample,MultisampleQuality,Discard,&surfaceD3D,pSharedHandle);

	if ( res == D3D_OK )
		*ppSurface = new SurfaceWrapper(surfaceD3D);

	return res;
}

//-----------------------------------------------------------------------------

static void playUpdateSurface()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	IDirect3DSurface9* pSourceSurface = gdx_commandBuffer->read<IDirect3DSurface9*>();
	IDirect3DSurface9* pDestinationSurface = gdx_commandBuffer->read<IDirect3DSurface9*>();

	RECT sourceRect;
	bool haveSourceRect = gdx_commandBuffer->read<bool>();
	if ( haveSourceRect )
	{
		sourceRect = gdx_commandBuffer->read<RECT>();
	}

	POINT destPoint;
	bool haveDestPoint = gdx_commandBuffer->read<bool>();
	if ( haveDestPoint )
	{
		destPoint = gdx_commandBuffer->read<POINT>();
	}

	device->UpdateSurface(pSourceSurface,haveSourceRect ? &sourceRect : NULL,
						  pDestinationSurface,haveDestPoint ? &destPoint : NULL);
}

HRESULT DX::DeviceWrapper::UpdateSurface(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	SurfaceWrapper* pSourceSurfaceWrapper = (SurfaceWrapper* )pSourceSurface;
	SurfaceWrapper* pDestinationSurfaceWrapper = (SurfaceWrapper* )pDestinationSurface;

	gdx_commandBuffer->write(playUpdateSurface,device_,pSourceSurfaceWrapper->surface_,pDestinationSurfaceWrapper->surface_);

	if ( pSourceRect )
	{
		gdx_commandBuffer->write( bool( true ), *pSourceRect );
	}
	else
	{
		gdx_commandBuffer->write( bool( false ) );
	}

	if ( pDestPoint )
	{
		gdx_commandBuffer->write( bool( true ), *pDestPoint );
	}
	else
	{
		gdx_commandBuffer->write( bool( false ) );
	}

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playUpdateTexture()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	IDirect3DBaseTexture9* pSourceTexture = gdx_commandBuffer->read<IDirect3DBaseTexture9*>();
	IDirect3DBaseTexture9* pDestinationTexture = gdx_commandBuffer->read<IDirect3DBaseTexture9*>();

	device->UpdateTexture(pSourceTexture,pDestinationTexture);
}

HRESULT DX::DeviceWrapper::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	BaseTextureWrapper* pSourceTextureWrapper = (BaseTextureWrapper*)pSourceTexture;
	BaseTextureWrapper* pDestinationTextureWrapper = (BaseTextureWrapper*)pDestinationTexture;

	gdx_commandBuffer->write(playUpdateTexture,device_,pSourceTextureWrapper->baseTexture_,pDestinationTextureWrapper->baseTexture_);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::GetRenderTargetData(IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	SurfaceWrapper* pRenderTargetWrapper = (SurfaceWrapper*)pRenderTarget;
	SurfaceWrapper* pDestSurfaceWrapper = (SurfaceWrapper*)pDestSurface;

	return device_->GetRenderTargetData(pRenderTargetWrapper->surface_,pDestSurfaceWrapper->surface_);
}

//-----------------------------------------------------------------------------

static void playStretchRect()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	IDirect3DSurface9* pSourceSurface = gdx_commandBuffer->read<IDirect3DSurface9*>();
	IDirect3DSurface9* pDestSurface = gdx_commandBuffer->read<IDirect3DSurface9*>();
	D3DTEXTUREFILTERTYPE Filter = gdx_commandBuffer->read<D3DTEXTUREFILTERTYPE>();

	RECT sourceRect;
	bool haveSourceRect = gdx_commandBuffer->read<bool>();
	if ( haveSourceRect )
	{
		sourceRect = gdx_commandBuffer->read<RECT>();
	}

	RECT destRect;
	bool haveDestRect = gdx_commandBuffer->read<bool>();
	if ( haveDestRect )
	{
		destRect = gdx_commandBuffer->read<RECT>();
	}

	device->StretchRect(pSourceSurface,haveSourceRect ? &sourceRect : NULL,
						pDestSurface,haveDestRect ? &destRect : NULL,Filter);
}

HRESULT DX::DeviceWrapper::StretchRect(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	SurfaceWrapper* pSourceSurfaceWrapper = (SurfaceWrapper*)pSourceSurface;
	SurfaceWrapper* pDestSurfaceWrapper = (SurfaceWrapper*)pDestSurface;

	gdx_commandBuffer->write(playStretchRect,device_,pSourceSurfaceWrapper->surface_,pDestSurfaceWrapper->surface_,Filter);

	if ( pSourceRect )
	{
		gdx_commandBuffer->write( bool( true ), *pSourceRect );
	}
	else
	{
		gdx_commandBuffer->write( bool( false ) );
	}

	if ( pDestRect )
	{
		gdx_commandBuffer->write( bool( true ), *pDestRect );
	}
	else
	{
		gdx_commandBuffer->write( bool( false ) );
	}

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::CreateOffscreenPlainSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	IDirect3DSurface9* surfaceD3D;
	HRESULT res = device_->CreateOffscreenPlainSurface(Width,Height,Format,Pool,&surfaceD3D,pSharedHandle);

	if ( res == D3D_OK )
		*ppSurface = new SurfaceWrapper(surfaceD3D);

	return res;
}

//-----------------------------------------------------------------------------

static void playSetRenderTarget()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	DWORD RenderTargetIndex = gdx_commandBuffer->read<DWORD>();
	IDirect3DSurface9* pRenderTarget = gdx_commandBuffer->read<IDirect3DSurface9*>();

	device->SetRenderTarget(RenderTargetIndex,pRenderTarget);
}

HRESULT DX::DeviceWrapper::SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	SurfaceWrapper* pRenderTargetWrapper = (SurfaceWrapper*)pRenderTarget;

	gdx_commandBuffer->write(playSetRenderTarget,device_,RenderTargetIndex,pRenderTargetWrapper ? pRenderTargetWrapper->surface_ : NULL);

	if ( pRenderTarget )
	{
		pRenderTarget->AddRef();
	}

	if ( RenderTarget_[RenderTargetIndex] )
	{
		RenderTarget_[RenderTargetIndex]->Release();
	}

	RenderTarget_[RenderTargetIndex] = pRenderTargetWrapper;

	// Update viewport cache
	if ( pRenderTarget )
	{
		D3DSURFACE_DESC desc;
		pRenderTarget->GetDesc( &desc );
		Viewport_.X      = 0;
		Viewport_.Y      = 0;
		Viewport_.Width  = desc.Width;
		Viewport_.Height = desc.Height;
		Viewport_.MinZ   = 0.0f;
		Viewport_.MaxZ   = 1.0f;
	}

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::GetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	MF_ASSERT( RenderTargetIndex < 4 );

	if ( RenderTarget_[RenderTargetIndex] )
	{
		RenderTarget_[RenderTargetIndex]->AddRef();
	}

	*ppRenderTarget = RenderTarget_[RenderTargetIndex];

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetDepthStencilSurface()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	IDirect3DSurface9* pNewZStencil = gdx_commandBuffer->read<IDirect3DSurface9*>();

	device->SetDepthStencilSurface(pNewZStencil);
}

HRESULT DX::DeviceWrapper::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	SurfaceWrapper* pNewZStencilWrapper = (SurfaceWrapper*)pNewZStencil;

	gdx_commandBuffer->write(playSetDepthStencilSurface,device_,pNewZStencilWrapper ? pNewZStencilWrapper->surface_ : NULL);

	if ( pNewZStencil )
	{
		pNewZStencil->AddRef();
	}

	if ( DepthStencilSurface_ )
	{
		DepthStencilSurface_->Release();
	}

	DepthStencilSurface_ = pNewZStencilWrapper;

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	if ( DepthStencilSurface_ )
	{
		DepthStencilSurface_->AddRef();
	}

	*ppZStencilSurface = DepthStencilSurface_;

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playBeginScene()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	device->BeginScene();
}

HRESULT DX::DeviceWrapper::BeginScene()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playBeginScene,device_);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playEndScene()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	device->EndScene();
}

HRESULT DX::DeviceWrapper::EndScene()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playEndScene,device_);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playClear()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	DWORD Count = gdx_commandBuffer->read<DWORD>();
	DWORD Flags = gdx_commandBuffer->read<DWORD>();
	D3DCOLOR Color = gdx_commandBuffer->read<D3DCOLOR>();
	float Z = gdx_commandBuffer->read<float>();
	DWORD Stencil = gdx_commandBuffer->read<DWORD>();

	CONST D3DRECT* pRects = (CONST D3DRECT*)gdx_commandBuffer->getCurrent();
	gdx_commandBuffer->seek( Count * sizeof( D3DRECT ) );

	device->Clear(Count,pRects,Flags,Color,Z,Stencil);
}

static void playClearNULL()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	DWORD Count = gdx_commandBuffer->read<DWORD>();
	DWORD Flags = gdx_commandBuffer->read<DWORD>();
	D3DCOLOR Color = gdx_commandBuffer->read<D3DCOLOR>();
	float Z = gdx_commandBuffer->read<float>();
	DWORD Stencil = gdx_commandBuffer->read<DWORD>();

	device->Clear(Count,NULL,Flags,Color,Z,Stencil);
}

HRESULT DX::DeviceWrapper::Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	if ( pRects )
	{
		gdx_commandBuffer->write(playClear,device_,Count,Flags,Color,Z,Stencil);
		gdx_commandBuffer->writeRaw(pRects, Count * sizeof( D3DRECT ) );
	}
	else
	{
		gdx_commandBuffer->write(playClearNULL,device_,Count,Flags,Color,Z,Stencil);
	}

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetTransform()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	D3DTRANSFORMSTATETYPE State = gdx_commandBuffer->read<D3DTRANSFORMSTATETYPE>();
	CONST D3DMATRIX pMatrix = gdx_commandBuffer->read<CONST D3DMATRIX>();

	device->SetTransform(State,&pMatrix);
}

HRESULT DX::DeviceWrapper::SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playSetTransform,device_,State,*pMatrix);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetViewport()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	CONST D3DVIEWPORT9 pViewport = gdx_commandBuffer->read<CONST D3DVIEWPORT9>();

	device->SetViewport(&pViewport);
}

HRESULT DX::DeviceWrapper::SetViewport(CONST D3DVIEWPORT9* pViewport)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playSetViewport,device_,*pViewport);

	Viewport_ = *pViewport;

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::GetViewport(D3DVIEWPORT9* pViewport)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	*pViewport = Viewport_;
	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetMaterial()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	CONST D3DMATERIAL9 pMaterial = gdx_commandBuffer->read<CONST D3DMATERIAL9>();

	device->SetMaterial(&pMaterial);
}

HRESULT DX::DeviceWrapper::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playSetMaterial,device_,*pMaterial);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetLight()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	DWORD Index = gdx_commandBuffer->read<DWORD>();
	CONST D3DLIGHT9 pLight = gdx_commandBuffer->read<CONST D3DLIGHT9>();

	device->SetLight(Index,&pLight);
}

HRESULT DX::DeviceWrapper::SetLight(DWORD Index,CONST D3DLIGHT9* pLight)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playSetLight,device_,Index,*pLight);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playLightEnable()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	DWORD Index = gdx_commandBuffer->read<DWORD>();
	BOOL Enable = gdx_commandBuffer->read<BOOL>();

	device->LightEnable(Index,Enable);
}

HRESULT DX::DeviceWrapper::LightEnable(DWORD Index,BOOL Enable)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playLightEnable,device_,Index,Enable);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetClipPlane()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	DWORD Index = gdx_commandBuffer->read<DWORD>();
	float pPlane[4];
	pPlane[0] = gdx_commandBuffer->read<float>();
	pPlane[1] = gdx_commandBuffer->read<float>();
	pPlane[2] = gdx_commandBuffer->read<float>();
	pPlane[3] = gdx_commandBuffer->read<float>();

	device->SetClipPlane(Index,pPlane);
}

HRESULT DX::DeviceWrapper::SetClipPlane(DWORD Index,CONST float* pPlane)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playSetClipPlane,device_,Index,pPlane[0],pPlane[1],pPlane[2],pPlane[3]);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetRenderState()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	D3DRENDERSTATETYPE State = gdx_commandBuffer->read<D3DRENDERSTATETYPE>();
	DWORD Value = gdx_commandBuffer->read<DWORD>();

	if ( Value != gdx_renderStateCache[State] )
	{
		gdx_renderStateCache[State] = Value;
		device->SetRenderState(State,Value);
	}
}

HRESULT DX::DeviceWrapper::SetRenderState(D3DRENDERSTATETYPE State,DWORD Value)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playSetRenderState,device_,State,Value);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetTexture()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	DWORD Stage = gdx_commandBuffer->read<DWORD>();
	IDirect3DBaseTexture9* pTexture = gdx_commandBuffer->read<IDirect3DBaseTexture9*>();

	if ( pTexture != gdx_textureCache[Stage] )
	{
		gdx_textureCache[Stage] = pTexture;
		device->SetTexture(Stage,pTexture);
	}
}

HRESULT DX::DeviceWrapper::SetTexture(DWORD Stage,IDirect3DBaseTexture9* pTexture)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	BaseTextureWrapper* pTextureWrapper = (BaseTextureWrapper*)pTexture;

	gdx_commandBuffer->write(playSetTexture,device_,Stage,pTextureWrapper ? pTextureWrapper->baseTexture_ : NULL);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetTextureStageState()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	DWORD Stage = gdx_commandBuffer->read<DWORD>();
	D3DTEXTURESTAGESTATETYPE Type = gdx_commandBuffer->read<D3DTEXTURESTAGESTATETYPE>();
	DWORD Value = gdx_commandBuffer->read<DWORD>();

	if ( Value != gdx_textureStageStateCache[Stage][Type] )
	{
		gdx_textureStageStateCache[Stage][Type] = Value;
		device->SetTextureStageState(Stage,Type,Value);
	}
}

HRESULT DX::DeviceWrapper::SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playSetTextureStageState,device_,Stage,Type,Value);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetSamplerState()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	DWORD Sampler = gdx_commandBuffer->read<DWORD>();
	D3DSAMPLERSTATETYPE Type = gdx_commandBuffer->read<D3DSAMPLERSTATETYPE>();
	DWORD Value = gdx_commandBuffer->read<DWORD>();

	if ( Value != gdx_samplerStateCache[Sampler][Type] )
	{
		gdx_samplerStateCache[Sampler][Type] = Value;
		device->SetSamplerState(Sampler,Type,Value);
	}
}

HRESULT DX::DeviceWrapper::SetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playSetSamplerState,device_,Sampler,Type,Value);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetScissorRect()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	CONST RECT pRect = gdx_commandBuffer->read<CONST RECT>();

	device->SetScissorRect(&pRect);
}

HRESULT DX::DeviceWrapper::SetScissorRect(CONST RECT* pRect)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playSetScissorRect,device_,*pRect);

	ScissorRect_ = *pRect;

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::GetScissorRect(RECT* pRect)
{
	BW_GUARD;

	*pRect = ScissorRect_;
	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetSoftwareVertexProcessing()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	BOOL bSoftware = gdx_commandBuffer->read<BOOL>();

	device->SetSoftwareVertexProcessing(bSoftware);
}

HRESULT DX::DeviceWrapper::SetSoftwareVertexProcessing(BOOL bSoftware)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playSetSoftwareVertexProcessing,device_,bSoftware);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetNPatchMode()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	float nSegments = gdx_commandBuffer->read<float>();

	device->SetNPatchMode(nSegments);
}

HRESULT DX::DeviceWrapper::SetNPatchMode(float nSegments)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playSetNPatchMode,device_,nSegments);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playDrawPrimitive()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	D3DPRIMITIVETYPE PrimitiveType = gdx_commandBuffer->read<D3DPRIMITIVETYPE>();
	UINT StartVertex = gdx_commandBuffer->read<UINT>();
	UINT PrimitiveCount = gdx_commandBuffer->read<UINT>();

	if ( PrimitiveCount )
	{
		device->DrawPrimitive(PrimitiveType,StartVertex,PrimitiveCount);
	}
}

HRESULT DX::DeviceWrapper::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playDrawPrimitive,device_,PrimitiveType,StartVertex,PrimitiveCount);
	s_patchPrimCount = (UINT*)gdx_commandBuffer->getCurrentWrite() - 1;

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playDrawIndexedPrimitive()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	D3DPRIMITIVETYPE PrimitiveType = gdx_commandBuffer->read<D3DPRIMITIVETYPE>();
	INT BaseVertexIndex = gdx_commandBuffer->read<INT>();
	UINT MinVertexIndex = gdx_commandBuffer->read<UINT>();
	UINT NumVertices = gdx_commandBuffer->read<UINT>();
	UINT startIndex = gdx_commandBuffer->read<UINT>();
	UINT primCount = gdx_commandBuffer->read<UINT>();

	device->DrawIndexedPrimitive(PrimitiveType,BaseVertexIndex,MinVertexIndex,NumVertices,startIndex,primCount);
}

HRESULT DX::DeviceWrapper::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playDrawIndexedPrimitive,device_,PrimitiveType,BaseVertexIndex,MinVertexIndex,NumVertices,startIndex,primCount);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static uint primCountToVertexCount( uint primCount, D3DPRIMITIVETYPE primType )
{
	switch( primType )
	{
		case D3DPT_POINTLIST:
			return primCount;

		case D3DPT_LINELIST:
			return primCount * 2;

		case D3DPT_LINESTRIP:
			return primCount + 1;

		case D3DPT_TRIANGLELIST:
			return primCount * 3;

		case D3DPT_TRIANGLESTRIP:
		case D3DPT_TRIANGLEFAN:
			return primCount + 2;
	}

	MF_ASSERT( !"Unknown primtive type" );
	return 0;
}

static void playDrawPrimitiveUP()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	D3DPRIMITIVETYPE PrimitiveType = gdx_commandBuffer->read<D3DPRIMITIVETYPE>();
	UINT PrimitiveCount = gdx_commandBuffer->read<UINT>();
	UINT VertexStreamZeroStride = gdx_commandBuffer->read<UINT>();

	uint size = primCountToVertexCount( PrimitiveCount, PrimitiveType ) * VertexStreamZeroStride;

	CONST void* pVertexStreamZeroData = (CONST void*)gdx_commandBuffer->getCurrent();
	gdx_commandBuffer->seek( size );

	device->DrawPrimitiveUP(PrimitiveType,PrimitiveCount,pVertexStreamZeroData,VertexStreamZeroStride);
}

HRESULT DX::DeviceWrapper::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	uint size = primCountToVertexCount( PrimitiveCount, PrimitiveType ) * VertexStreamZeroStride;

	gdx_commandBuffer->write(playDrawPrimitiveUP,device_,PrimitiveType,PrimitiveCount,VertexStreamZeroStride);
	gdx_commandBuffer->writeRaw( pVertexStreamZeroData, size );

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static uint primCountToIndexSize( uint primCount, D3DPRIMITIVETYPE primType, D3DFORMAT indexFormat )
{
	uint shift = 0;

	switch( indexFormat )
	{
		case D3DFMT_INDEX16:
			shift = 1;
			break;
		case D3DFMT_INDEX32:
			shift = 2;
			break;
		default:
			MF_ASSERT( !"Invalid index format" );
	}

	return primCountToVertexCount( primCount, primType ) << shift;
}

static void playDrawIndexedPrimitiveUP()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	D3DPRIMITIVETYPE PrimitiveType = gdx_commandBuffer->read<D3DPRIMITIVETYPE>();
	UINT MinVertexIndex = gdx_commandBuffer->read<UINT>();
	UINT NumVertices = gdx_commandBuffer->read<UINT>();
	UINT PrimitiveCount = gdx_commandBuffer->read<UINT>();
	D3DFORMAT IndexDataFormat = gdx_commandBuffer->read<D3DFORMAT>();
	UINT VertexStreamZeroStride = gdx_commandBuffer->read<UINT>();

	uint size = primCountToIndexSize( PrimitiveCount, PrimitiveType, IndexDataFormat );

	CONST void* pIndexData = (CONST void*)gdx_commandBuffer->getCurrent();
	gdx_commandBuffer->seek( size );

	CONST void* pVertexStreamZeroData = (CONST void*)gdx_commandBuffer->getCurrent();
	gdx_commandBuffer->seek( NumVertices * VertexStreamZeroStride );

	device->DrawIndexedPrimitiveUP(PrimitiveType,MinVertexIndex,NumVertices,PrimitiveCount,pIndexData,IndexDataFormat,pVertexStreamZeroData,VertexStreamZeroStride);
}

HRESULT DX::DeviceWrapper::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playDrawIndexedPrimitiveUP,device_,PrimitiveType,MinVertexIndex,NumVertices,PrimitiveCount,IndexDataFormat,VertexStreamZeroStride);

	uint size = primCountToIndexSize( PrimitiveCount, PrimitiveType, IndexDataFormat );

	gdx_commandBuffer->writeRaw( pIndexData, size );
	gdx_commandBuffer->writeRaw( pVertexStreamZeroData, NumVertices * VertexStreamZeroStride );

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetVertexDeclaration()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	DX::VertexDeclarationWrapper* pDecl = gdx_commandBuffer->read<DX::VertexDeclarationWrapper*>();

	device->SetVertexDeclaration(pDecl->vertexDeclaration_);
}

HRESULT DX::DeviceWrapper::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	VertexDeclarationWrapper* pDeclWrapper = (VertexDeclarationWrapper*)pDecl;

	gdx_commandBuffer->write(playSetVertexDeclaration,device_,pDeclWrapper);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	IDirect3DVertexDeclaration9* vertexDeclarationD3D;
	HRESULT res = device_->CreateVertexDeclaration(pVertexElements,&vertexDeclarationD3D);

	if ( res == D3D_OK )
		*ppDecl = new VertexDeclarationWrapper(vertexDeclarationD3D);

	return res;
}

//-----------------------------------------------------------------------------

static void playSetFVF()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	DWORD FVF = gdx_commandBuffer->read<DWORD>();

	device->SetFVF(FVF);
}

HRESULT DX::DeviceWrapper::SetFVF(DWORD FVF)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playSetFVF,device_,FVF);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::CreateVertexShader(CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	IDirect3DVertexShader9* vertexShaderD3D;
	HRESULT res = device_->CreateVertexShader(pFunction,&vertexShaderD3D);

	if ( res == D3D_OK )
		*ppShader = new VertexShaderWrapper(vertexShaderD3D);

	return res;
}

//-----------------------------------------------------------------------------

static void playSetVertexShader()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	DX::VertexShaderWrapper* pShader = gdx_commandBuffer->read<DX::VertexShaderWrapper*>();

	device->SetVertexShader(pShader ? pShader->vertexShader_ : NULL);
}

HRESULT DX::DeviceWrapper::SetVertexShader(IDirect3DVertexShader9* pShader)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	VertexShaderWrapper* pShaderWrapper = (VertexShaderWrapper*)pShader;

	gdx_commandBuffer->write(playSetVertexShader,device_,pShaderWrapper);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetVertexShaderConstantF()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	UINT StartRegister = gdx_commandBuffer->read<UINT>();
	UINT Vector4fCount = gdx_commandBuffer->read<UINT>();

	CONST float* pConstantData = (CONST float*)gdx_commandBuffer->getCurrent();
	gdx_commandBuffer->seek( Vector4fCount * 4 * sizeof( float ) );

	device->SetVertexShaderConstantF(StartRegister,pConstantData,Vector4fCount);
}

HRESULT DX::DeviceWrapper::SetVertexShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	uint size = Vector4fCount * 4 * sizeof( float );

	gdx_commandBuffer->write(playSetVertexShaderConstantF,device_,StartRegister,Vector4fCount);
	gdx_commandBuffer->writeRaw( pConstantData, size );

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetVertexShaderConstantI()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	UINT StartRegister = gdx_commandBuffer->read<UINT>();
	UINT Vector4iCount = gdx_commandBuffer->read<UINT>();

	CONST int* pConstantData = (CONST int*)gdx_commandBuffer->getCurrent();
	gdx_commandBuffer->seek( Vector4iCount * 4 * sizeof( int ) );

	device->SetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount);
}

HRESULT DX::DeviceWrapper::SetVertexShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	uint size = Vector4iCount * 4 * sizeof( int );

	gdx_commandBuffer->write(playSetVertexShaderConstantI,device_,StartRegister,Vector4iCount);
	gdx_commandBuffer->writeRaw( pConstantData, size );

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetVertexShaderConstantB()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	UINT StartRegister = gdx_commandBuffer->read<UINT>();
	UINT BoolCount = gdx_commandBuffer->read<UINT>();

	CONST BOOL* pConstantData = (CONST BOOL*)gdx_commandBuffer->getCurrent();
	gdx_commandBuffer->seek( BoolCount * sizeof( BOOL ) );

	device->SetVertexShaderConstantB(StartRegister,pConstantData,BoolCount);
}

HRESULT DX::DeviceWrapper::SetVertexShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT BoolCount)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	uint size = BoolCount * sizeof( BOOL );

	gdx_commandBuffer->write(playSetVertexShaderConstantB,device_,StartRegister,BoolCount);
	gdx_commandBuffer->writeRaw( pConstantData, size );

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetStreamSource()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	UINT StreamNumber = gdx_commandBuffer->read<UINT>();
	DX::VertexBufferWrapper* pStreamData = gdx_commandBuffer->read<DX::VertexBufferWrapper*>();
	UINT OffsetInBytes = gdx_commandBuffer->read<UINT>();
	UINT Stride = gdx_commandBuffer->read<UINT>();

	device->SetStreamSource(StreamNumber,pStreamData ? pStreamData->vertexBuffer_ : NULL,OffsetInBytes,Stride);
}

HRESULT DX::DeviceWrapper::SetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	VertexBufferWrapper* pStreamDataWrapper = (VertexBufferWrapper*)pStreamData;

	gdx_commandBuffer->write(playSetStreamSource,device_,StreamNumber,pStreamDataWrapper,OffsetInBytes,Stride);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetStreamSourceFreq()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	UINT StreamNumber = gdx_commandBuffer->read<UINT>();
	UINT Setting = gdx_commandBuffer->read<UINT>();

	device->SetStreamSourceFreq(StreamNumber,Setting);
}

HRESULT DX::DeviceWrapper::SetStreamSourceFreq(UINT StreamNumber,UINT Setting)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playSetStreamSourceFreq,device_,StreamNumber,Setting);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetIndices()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	DX::IndexBufferWrapper* pIndexData = gdx_commandBuffer->read<DX::IndexBufferWrapper*>();

	device->SetIndices(pIndexData ? pIndexData->indexBuffer_ : NULL);
}

HRESULT DX::DeviceWrapper::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	IndexBufferWrapper* pIndexDataWrapper = (IndexBufferWrapper*)pIndexData;

	gdx_commandBuffer->write(playSetIndices,device_,pIndexDataWrapper);

	Indices_ = pIndexDataWrapper;

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	Indices_->AddRef();
	*ppIndexData= Indices_;
	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::CreatePixelShader(CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	IDirect3DPixelShader9* pixelShaderD3D;
	HRESULT res = device_->CreatePixelShader(pFunction,&pixelShaderD3D);

	if ( res == D3D_OK )
		*ppShader = new PixelShaderWrapper(pixelShaderD3D);

	return res;
}

//-----------------------------------------------------------------------------

static void playSetPixelShader()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	DX::PixelShaderWrapper* pShader = gdx_commandBuffer->read<DX::PixelShaderWrapper*>();

	device->SetPixelShader(pShader ? pShader->pixelShader_ : NULL);
}

HRESULT DX::DeviceWrapper::SetPixelShader(IDirect3DPixelShader9* pShader)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	PixelShaderWrapper* pShaderWrapper = (PixelShaderWrapper*)pShader;

	gdx_commandBuffer->write(playSetPixelShader,device_,pShaderWrapper);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetPixelShaderConstantF()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	UINT StartRegister = gdx_commandBuffer->read<UINT>();
	UINT Vector4fCount = gdx_commandBuffer->read<UINT>();

	CONST float* pConstantData = (CONST float*)gdx_commandBuffer->getCurrent();
	gdx_commandBuffer->seek( Vector4fCount * 4 * sizeof( float ) );

	device->SetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount);
}

HRESULT DX::DeviceWrapper::SetPixelShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	uint size = Vector4fCount * 4 * sizeof( float );

	gdx_commandBuffer->write(playSetPixelShaderConstantF,device_,StartRegister,Vector4fCount);
	gdx_commandBuffer->writeRaw( pConstantData, size );

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetPixelShaderConstantI()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	UINT StartRegister = gdx_commandBuffer->read<UINT>();
	UINT Vector4iCount = gdx_commandBuffer->read<UINT>();

	CONST int* pConstantData = (CONST int*)gdx_commandBuffer->getCurrent();
	gdx_commandBuffer->seek( Vector4iCount * 4 * sizeof( int ) );

	device->SetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount);
}

HRESULT DX::DeviceWrapper::SetPixelShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	uint size = Vector4iCount * 4 * sizeof( int );

	gdx_commandBuffer->write(playSetPixelShaderConstantI,device_,StartRegister,Vector4iCount);
	gdx_commandBuffer->writeRaw( pConstantData, size );

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playSetPixelShaderConstantB()
{
	BW_GUARD;

	IDirect3DDevice9* device = gdx_commandBuffer->read<IDirect3DDevice9*>(); 

	UINT StartRegister = gdx_commandBuffer->read<UINT>();
	UINT BoolCount = gdx_commandBuffer->read<UINT>();

	CONST BOOL* pConstantData = (CONST BOOL*)gdx_commandBuffer->getCurrent();
	gdx_commandBuffer->seek( BoolCount * sizeof( BOOL ) );

	device->SetPixelShaderConstantB(StartRegister,pConstantData,BoolCount);
}

HRESULT DX::DeviceWrapper::SetPixelShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT BoolCount)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	uint size = BoolCount * sizeof( BOOL );

	gdx_commandBuffer->write(playSetPixelShaderConstantB,device_,StartRegister,BoolCount);
	gdx_commandBuffer->writeRaw( pConstantData, size );

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::CreateQuery(D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	if ( !ppQuery )
		return device_->CreateQuery(Type,NULL);

	IDirect3DQuery9* queryD3D;
	HRESULT res = device_->CreateQuery(Type,&queryD3D);

	if ( res == D3D_OK )
		*ppQuery = new QueryWrapper(queryD3D);

	return res;
}

//-----------------------------------------------------------------------------
// DeviceWrapper but only for D3DX, it doesn't defer.
//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::GetDirect3D(IDirect3D9** ppD3D9)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return device_->GetDirect3D(ppD3D9);
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::GetDeviceCaps(D3DCAPS9* pCaps)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return device_->GetDeviceCaps(pCaps);
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::GetDisplayMode(UINT iSwapChain,D3DDISPLAYMODE* pMode)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return device_->GetDisplayMode(iSwapChain,pMode);
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	flushAndStall();
	return device_->GetCreationParameters(pParameters);
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::BeginStateBlock()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	flushAndStall();
	return device_->BeginStateBlock();
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::EndStateBlock(IDirect3DStateBlock9** ppSB)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	flushAndStall();

	IDirect3DStateBlock9* stateBlockD3D;
	HRESULT res = device_->EndStateBlock(&stateBlockD3D);

	if ( res == D3D_OK )
		*ppSB = new StateBlockWrapper(stateBlockD3D);

	return res;
}

//-----------------------------------------------------------------------------

HRESULT DX::DeviceWrapper::ValidateDevice(DWORD* pNumPasses)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	flushAndStall();
	return device_->ValidateDevice(pNumPasses);
}

//-----------------------------------------------------------------------------
// ComInterface
//-----------------------------------------------------------------------------

DX::ComInterface::ComInterface(IUnknown* unknown)
:	comInterface_(unknown)
,	refCount_(1)
{
#ifdef ENABLE_DX_TRACKER
	static volatile LONG comCount;
	id_ = (uint)InterlockedIncrement( &comCount ) - 1;
	stack_ = StackTracker::buildReport();
#endif
}

//-----------------------------------------------------------------------------

DX::ComInterface::~ComInterface()
{
	MF_ASSERT( refCount_ == 0 );

#ifdef ENABLE_DX_TRACKER
	SimpleMutexHolder smh( gdx_trackerMutex );
	node_.remove();
#endif
}

//-----------------------------------------------------------------------------

void DX::ComInterface::set(IUnknown* unknown)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	comInterface_ = unknown;
}

//-----------------------------------------------------------------------------

#if 0

static void playAddRef()
{
	BW_GUARD;

	DX::ComInterface* comInterface = gdx_commandBuffer->read<DX::ComInterface*>();

	ULONG ref = comInterface->comInterface_->AddRef();

	InterlockedIncrement( &comInterface->refCount_ );
}

ULONG DX::ComInterface::AddRef()
{
	BW_GUARD;
	WRITE_LOCK;

	MF_ASSERT( refCount_ > 0 );

	gdx_commandBuffer->write(playAddRef,this);

	return refCount_;
}

#else

ULONG DX::ComInterface::AddRef()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	MF_ASSERT( refCount_ > 0 );
	InterlockedIncrement( &refCount_ );

	return comInterface_->AddRef();
}

#endif

//-----------------------------------------------------------------------------

static void playRelease()
{
	BW_GUARD;

	DX::ComInterface* comInterface = gdx_commandBuffer->read<DX::ComInterface*>();

	ULONG ref = comInterface->comInterface_->Release();

	LONG refCount = InterlockedDecrement( &comInterface->refCount_ );

	if ( !refCount )
	{
		delete comInterface;
	}
	else
	{
		MF_ASSERT( ref );
	}
}

ULONG DX::ComInterface::Release()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	if ( gdx_isDXThread )
	{
		ULONG ref = comInterface_->Release();

		LONG refCount = InterlockedDecrement( &refCount_ );

		if ( !refCount )
		{
			delete this;
			return 0;
		}
		else
		{
			MF_ASSERT( ref );
		}

		return refCount_;
	}

	WRITE_LOCK;

	gdx_commandBuffer->write(playRelease,this);

	MF_ASSERT( refCount_ > 0 );

	return refCount_;
}

//-----------------------------------------------------------------------------
// ResourceWrapper
//-----------------------------------------------------------------------------

DX::ResourceWrapper::ResourceWrapper(IDirect3DResource9* resourceD3D)
:	ComInterface(resourceD3D)
,	resource_(resourceD3D)
{
}

//-----------------------------------------------------------------------------

DX::ResourceWrapper::~ResourceWrapper()
{
}

//-----------------------------------------------------------------------------

void DX::ResourceWrapper::set(IDirect3DResource9* resourceD3D)
{
	BW_GUARD;

	ComInterface::set( resourceD3D );
	resource_ = resourceD3D;
}

//-----------------------------------------------------------------------------

HRESULT DX::ResourceWrapper::GetDevice(IDirect3DDevice9** ppDevice)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	gdx_device->AddRef();
	*ppDevice = gdx_device;

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playResourcePreLoad()
{
	BW_GUARD;

	DX::ResourceWrapper* resource = gdx_commandBuffer->read<DX::ResourceWrapper*>();

	resource->resource_->PreLoad();
}

void DX::ResourceWrapper::PreLoad()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playResourcePreLoad,this);
}

//-----------------------------------------------------------------------------

D3DRESOURCETYPE DX::ResourceWrapper::GetType()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return resource_->GetType();
}

//-----------------------------------------------------------------------------
// BaseTextureWrapper
//-----------------------------------------------------------------------------

DX::BaseTextureWrapper::BaseTextureWrapper(IDirect3DBaseTexture9* baseTextureD3D)
:	ResourceWrapper(baseTextureD3D)
,	baseTexture_(baseTextureD3D)
{
}

//-----------------------------------------------------------------------------

DX::BaseTextureWrapper::~BaseTextureWrapper()
{
}

//-----------------------------------------------------------------------------
// IUnknown

ULONG DX::BaseTextureWrapper::AddRef()
{
	return ComInterface::AddRef();
}

//-----------------------------------------------------------------------------

ULONG DX::BaseTextureWrapper::Release()
{
	return ComInterface::Release();
}

//-----------------------------------------------------------------------------
// IDirect3DResource9

HRESULT DX::BaseTextureWrapper::GetDevice(IDirect3DDevice9** ppDevice)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return ResourceWrapper::GetDevice(ppDevice);
}

//-----------------------------------------------------------------------------

void DX::BaseTextureWrapper::PreLoad()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	ResourceWrapper::PreLoad();
}

//-----------------------------------------------------------------------------

D3DRESOURCETYPE DX::BaseTextureWrapper::GetType()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return ResourceWrapper::GetType();
}

//-----------------------------------------------------------------------------
// Methods

DWORD DX::BaseTextureWrapper::SetLOD(DWORD LODNew)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	flushAndStall();
	return baseTexture_->SetLOD(LODNew);
}

//-----------------------------------------------------------------------------

DWORD DX::BaseTextureWrapper::GetLevelCount()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return baseTexture_->GetLevelCount();
}

//-----------------------------------------------------------------------------

static void playBaseTextureSetAutoGenFilterType()
{
	BW_GUARD;

	DX::BaseTextureWrapper* baseTexture = gdx_commandBuffer->read<DX::BaseTextureWrapper*>();
	D3DTEXTUREFILTERTYPE FilterType = gdx_commandBuffer->read<D3DTEXTUREFILTERTYPE>();

	baseTexture->baseTexture_->SetAutoGenFilterType(FilterType);
}

HRESULT DX::BaseTextureWrapper::SetAutoGenFilterType(D3DTEXTUREFILTERTYPE FilterType)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playBaseTextureSetAutoGenFilterType,this,FilterType);

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playBaseTextureGenerateMipSubLevels()
{
	BW_GUARD;

	DX::BaseTextureWrapper* baseTexture = gdx_commandBuffer->read<DX::BaseTextureWrapper*>();

	baseTexture->baseTexture_->GenerateMipSubLevels();
}

void DX::BaseTextureWrapper::GenerateMipSubLevels()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playBaseTextureGenerateMipSubLevels,this);
}

//-----------------------------------------------------------------------------
// Texture
//-----------------------------------------------------------------------------

DX::TextureWrapper::TextureWrapper(IDirect3DTexture9* textureD3D, D3DFORMAT format, uint width, uint height)
:	BaseTextureWrapper(textureD3D)
,	texture_(textureD3D)
,	format_(format)
,	width_(width)
,	height_(height)
,	locked_(false)
{
	for ( uint i = 0; i < ARRAY_SIZE( surface_ ); i++ )
	{
		surface_[i] = NULL;
	}

	InterlockedIncrement( &s_numTextures );

#ifdef ENABLE_DX_TRACKER
	SimpleMutexHolder smh( gdx_trackerMutex );
	node_.addThisBefore( &s_TexturesList );
#endif
}

//-----------------------------------------------------------------------------

DX::TextureWrapper::~TextureWrapper()
{
	for ( uint i = 0; i < ARRAY_SIZE( surface_ ); i++ )
	{
		if ( surface_[i] )
		{
			surface_[i]->Release();
		}
	}

	InterlockedDecrement( &s_numTextures );
}

//-----------------------------------------------------------------------------
// IUnknown

HRESULT DX::TextureWrapper::QueryInterface(REFIID riid, void** ppvObj)
{
	if ( riid == IID_IDirect3DTexture9 )
	{
		IDirect3DTexture9* textureWrapper = this;
		AddRef();
		*ppvObj = textureWrapper;
		return S_OK;
	}

	return E_NOINTERFACE;
}

//-----------------------------------------------------------------------------

ULONG DX::TextureWrapper::AddRef()
{
	return ComInterface::AddRef();
}

//-----------------------------------------------------------------------------

ULONG DX::TextureWrapper::Release()
{
	return ComInterface::Release();
}

//-----------------------------------------------------------------------------
// IDirect3DResource9

HRESULT DX::TextureWrapper::GetDevice(IDirect3DDevice9** ppDevice)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return ResourceWrapper::GetDevice(ppDevice);
}

//-----------------------------------------------------------------------------

void DX::TextureWrapper::PreLoad()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	ResourceWrapper::PreLoad();
}

//-----------------------------------------------------------------------------

D3DRESOURCETYPE DX::TextureWrapper::GetType()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return ResourceWrapper::GetType();
}

//-----------------------------------------------------------------------------
// IDirect3DBaseTexture9

DWORD DX::TextureWrapper::GetLevelCount()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return BaseTextureWrapper::GetLevelCount();
}

//-----------------------------------------------------------------------------

HRESULT DX::TextureWrapper::SetAutoGenFilterType(D3DTEXTUREFILTERTYPE FilterType)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return BaseTextureWrapper::SetAutoGenFilterType(FilterType);
}

//-----------------------------------------------------------------------------

void DX::TextureWrapper::GenerateMipSubLevels()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	BaseTextureWrapper::GenerateMipSubLevels();
}

//-----------------------------------------------------------------------------
// Methods

HRESULT DX::TextureWrapper::GetLevelDesc(UINT Level,D3DSURFACE_DESC *pDesc)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return texture_->GetLevelDesc(Level,pDesc);
}

//-----------------------------------------------------------------------------

HRESULT DX::TextureWrapper::GetSurfaceLevel(UINT Level,IDirect3DSurface9** ppSurfaceLevel)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	IDirect3DSurface9* surfaceD3D;
	HRESULT res = texture_->GetSurfaceLevel(Level,&surfaceD3D);

	if ( res == D3D_OK )
		*ppSurfaceLevel = new SurfaceWrapper(surfaceD3D);

	return res;

	// FIXME
	if ( !surface_[Level] )
	{
		IDirect3DSurface9* surfaceLevelD3D;
		HRESULT res = texture_->GetSurfaceLevel(Level,&surfaceLevelD3D);

		if ( res != D3D_OK )
		{
			return res;
		}

		surface_[Level] = new SurfaceWrapper(surfaceLevelD3D);
	}

	surface_[Level]->AddRef();
	*ppSurfaceLevel = surface_[Level];

	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playTextureLockRect()
{
	BW_GUARD;

	DX::TextureWrapper* texture = gdx_commandBuffer->read<DX::TextureWrapper*>();

	UINT Level = gdx_commandBuffer->read<UINT>();
	DWORD Flags = gdx_commandBuffer->read<DWORD>();
	D3DLOCKED_RECT pLockedRect;

	bool hasRect = gdx_commandBuffer->read<bool>();

	uint width;
	uint height;
	HRESULT res;

	// Set width and height and lock
	if ( hasRect )
	{
		RECT pRect = gdx_commandBuffer->read<CONST RECT>();

		width = gdx_commandBuffer->read<uint>();
		height = gdx_commandBuffer->read<uint>();

		res = texture->texture_->LockRect(Level,&pLockedRect,&pRect,Flags);
	}
	else
	{
		width = texture->width_;
		height = texture->height_;

		res = texture->texture_->LockRect(Level,&pLockedRect,NULL,Flags);
	}

	// Get the texture source data (its either in the command buffer or a job's output)
	uint size = gdx_commandBuffer->read<uint>();
	void* textureData = gdx_commandBuffer->read<void*>();

	if ( !textureData )
	{
		gdx_commandBuffer->skipPadding();
		textureData = gdx_commandBuffer->getCurrent();
		gdx_commandBuffer->seek( size );
	}

	// If we didn't lock, return now
	if ( res != D3D_OK )
	{
		return;
	}

	// Copy to the texture
	uint lineSize = size / height;

	if ( pLockedRect.Pitch != lineSize )
	{
		char* dest = (char*)pLockedRect.pBits;
		char* src = (char*)textureData;
		for ( uint i = 0; i < height; i++ )
		{
			memcpy( dest, src, lineSize );
			dest += pLockedRect.Pitch;
			src += lineSize;
		}
	}
	else
	{
		memcpy( pLockedRect.pBits, textureData, size );
	}

	// Unlock
	texture->texture_->UnlockRect(Level);
}

HRESULT DX::TextureWrapper::LockRect(UINT Level,D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	if (	!gdx_isMainThread
		||	( Flags & D3DLOCK_READONLY )
		||	( s_flags & WRAPPER_FLAG_IMMEDIATE_LOCK )
		||	format_ == D3DFMT_DXT1
		||	format_ == D3DFMT_DXT2
		||	format_ == D3DFMT_DXT3
		||	format_ == D3DFMT_DXT4
		||	format_ == D3DFMT_DXT5 )
	{
		flushAndStall();
		locked_ = true;
		return texture_->LockRect(Level,pLockedRect,pRect,Flags);
	}

	lockCommandBuffer();	// Don't allow the D3D thread to flip until its unlocked

	gdx_commandBuffer->write(playTextureLockRect,this,Level,Flags);

	uint size;
	
	if ( pRect )
	{
		uint width = pRect->right - pRect->left;
		uint height = pRect->bottom - pRect->top;

		size = surfaceSize( width, height, format_ );
		pLockedRect->Pitch = size / height;

		gdx_commandBuffer->write( bool ( true ), *pRect, width, height );
	}
	else
	{
		size = surfaceSize( width_, height_, format_ );
		pLockedRect->Pitch = size / height_;

		gdx_commandBuffer->write( bool ( false ) );
	}

	gdx_commandBuffer->write( size );

	if ( s_flags & WRAPPER_FLAG_DEFERRED_LOCK )
	{
		pLockedRect->pBits = JobSystem::instance().allocOutput<uint8>( size );
		gdx_commandBuffer->write( pLockedRect->pBits );
	}
	else
	{
		gdx_commandBuffer->write( (void*)NULL );
		gdx_commandBuffer->writePadding();
		pLockedRect->pBits = gdx_commandBuffer->getCurrentWrite();
		gdx_commandBuffer->seekWrite( size );
	}

	if ( s_flags & WRAPPER_FLAG_ZERO_TEXTURE_LOCK )
	{
		memset( pLockedRect->pBits, 0, size );
	}

	s_lockCount++;

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::TextureWrapper::UnlockRect(UINT Level)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	if ( locked_ )
	{
		locked_ = false;
		return texture_->UnlockRect(Level);
	}

	s_lockCount--;
	unlockCommandBuffer();	// Allow the D3D thread to flip now
	return D3D_OK;
}

//-----------------------------------------------------------------------------

static void playTextureAddDirtyRect()
{
	BW_GUARD;

	DX::TextureWrapper* texture = gdx_commandBuffer->read<DX::TextureWrapper*>();

	texture->texture_->AddDirtyRect( NULL );
}

HRESULT DX::TextureWrapper::AddDirtyRect(CONST RECT* pDirtyRect)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playTextureAddDirtyRect,this);

	return D3D_OK;
}

//-----------------------------------------------------------------------------
// CubeTexture
//-----------------------------------------------------------------------------

DX::CubeTextureWrapper::CubeTextureWrapper(IDirect3DCubeTexture9* cubeTextureD3D)
:	BaseTextureWrapper(cubeTextureD3D)
,	cubeTexture_(cubeTextureD3D)
{
	for ( uint i = 0; i < ARRAY_SIZE( surface_ ); i++ )
	{
		for ( uint j = 0; j < ARRAY_SIZE( surface_[i] ); j++ )
		{
			surface_[i][j] = NULL;
		}
	}

	InterlockedIncrement( &s_numCubeTextures );

#ifdef ENABLE_DX_TRACKER
	SimpleMutexHolder smh( gdx_trackerMutex );
	node_.addThisBefore( &s_CubeTexturesList );
#endif
}

//-----------------------------------------------------------------------------

DX::CubeTextureWrapper::~CubeTextureWrapper()
{
	for ( uint i = 0; i < ARRAY_SIZE( surface_ ); i++ )
	{
		for ( uint j = 0; j < ARRAY_SIZE( surface_[i] ); j++ )
		{
			if ( surface_[i][j] )
			{
				surface_[i][j]->Release();
			}
		}
	}

	InterlockedDecrement( &s_numCubeTextures );
}

//-----------------------------------------------------------------------------
// IUnknown

ULONG DX::CubeTextureWrapper::AddRef()
{
	return ComInterface::AddRef();
}

//-----------------------------------------------------------------------------

ULONG DX::CubeTextureWrapper::Release()
{
	return ComInterface::Release();
}

//-----------------------------------------------------------------------------
// IDirect3DResource9

HRESULT DX::CubeTextureWrapper::GetDevice(IDirect3DDevice9** ppDevice)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return ResourceWrapper::GetDevice(ppDevice);
}

//-----------------------------------------------------------------------------

void DX::CubeTextureWrapper::PreLoad()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	ResourceWrapper::PreLoad();
}

//-----------------------------------------------------------------------------

D3DRESOURCETYPE DX::CubeTextureWrapper::GetType()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return ResourceWrapper::GetType();
}

//-----------------------------------------------------------------------------
// IDirect3DBaseTexture9

DWORD DX::CubeTextureWrapper::SetLOD(DWORD LODNew)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return BaseTextureWrapper::SetLOD(LODNew);
}

//-----------------------------------------------------------------------------

DWORD DX::CubeTextureWrapper::GetLevelCount()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return BaseTextureWrapper::GetLevelCount();
}

//-----------------------------------------------------------------------------
// Methods

HRESULT DX::CubeTextureWrapper::GetLevelDesc(UINT Level,D3DSURFACE_DESC *pDesc)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return cubeTexture_->GetLevelDesc(Level,pDesc);
}

//-----------------------------------------------------------------------------

HRESULT DX::CubeTextureWrapper::GetCubeMapSurface(D3DCUBEMAP_FACES FaceType,UINT Level,IDirect3DSurface9** ppCubeMapSurface)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	IDirect3DSurface9* cubeMapSurfaceD3D;
	HRESULT res = cubeTexture_->GetCubeMapSurface(FaceType,Level,&cubeMapSurfaceD3D);

	if ( res == D3D_OK )
		*ppCubeMapSurface = new SurfaceWrapper(cubeMapSurfaceD3D);

	return res;

	// FIXME
	if ( !surface_[FaceType][Level] )
	{
		IDirect3DSurface9* cubeMapSurfaceD3D;
		HRESULT res = cubeTexture_->GetCubeMapSurface(FaceType,Level,&cubeMapSurfaceD3D);

		if ( res != D3D_OK )
		{
			return res;
		}

		surface_[FaceType][Level] = new SurfaceWrapper(cubeMapSurfaceD3D);
	}

	surface_[FaceType][Level]->AddRef();
	*ppCubeMapSurface = surface_[FaceType][Level];

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::CubeTextureWrapper::LockRect(D3DCUBEMAP_FACES FaceType,UINT Level,D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	flushAndStall();
	return cubeTexture_->LockRect(FaceType,Level,pLockedRect,pRect,Flags);
}

//-----------------------------------------------------------------------------

HRESULT DX::CubeTextureWrapper::UnlockRect(D3DCUBEMAP_FACES FaceType,UINT Level)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	flushAndStall();
	return cubeTexture_->UnlockRect(FaceType,Level);
}

//-----------------------------------------------------------------------------
// Surface
//-----------------------------------------------------------------------------

DX::SurfaceWrapper::SurfaceWrapper(IDirect3DSurface9* surfaceD3D)
:	ResourceWrapper(surfaceD3D)
,	surface_(surfaceD3D)
{
	InterlockedIncrement( &s_numSurfaces );

#ifdef ENABLE_DX_TRACKER
	SimpleMutexHolder smh( gdx_trackerMutex );
	node_.addThisBefore( &s_SurfacesList );
#endif
}

//-----------------------------------------------------------------------------

DX::SurfaceWrapper::~SurfaceWrapper()
{
	InterlockedDecrement( &s_numSurfaces );
}

//-----------------------------------------------------------------------------
// IUnknown

ULONG DX::SurfaceWrapper::AddRef()
{
	return ComInterface::AddRef();
}

//-----------------------------------------------------------------------------

ULONG DX::SurfaceWrapper::Release()
{
	return ComInterface::Release();
}

//-----------------------------------------------------------------------------
// IDirect3DResource9

HRESULT DX::SurfaceWrapper::GetDevice(IDirect3DDevice9** ppDevice)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return ResourceWrapper::GetDevice(ppDevice);
}

//-----------------------------------------------------------------------------

void DX::SurfaceWrapper::PreLoad()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	ResourceWrapper::PreLoad();
}

//-----------------------------------------------------------------------------

D3DRESOURCETYPE DX::SurfaceWrapper::GetType()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return ResourceWrapper::GetType();
}

//-----------------------------------------------------------------------------
// Methods

HRESULT DX::SurfaceWrapper::GetDesc(D3DSURFACE_DESC *pDesc)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return surface_->GetDesc(pDesc);
}

//-----------------------------------------------------------------------------

HRESULT DX::SurfaceWrapper::GetContainer(REFIID riid,void** ppContainer)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	flushAndStall();

	void* containerD3D;
	HRESULT res = surface_->GetContainer(riid,&containerD3D);

	if ( res != D3D_OK )
		return res;

	if ( riid == IID_IDirect3DTexture9 )
	{
		// FIXME - we need the correct format, width and height
		*ppContainer = (IDirect3DTexture9*)
			(new TextureWrapper((IDirect3DTexture9*)containerD3D, (D3DFORMAT)0, 0, 0));
	}
	else
	{
		MF_ASSERT( !"Unsupported type requested from SurfaceWrapper::GetContainer" );
	}

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::SurfaceWrapper::LockRect(D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	flushAndStall();

	return surface_->LockRect(pLockedRect,pRect,Flags);
}

//-----------------------------------------------------------------------------

HRESULT DX::SurfaceWrapper::UnlockRect()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	flushAndStall();
	return surface_->UnlockRect();
}

//-----------------------------------------------------------------------------
// VertexBuffer
//-----------------------------------------------------------------------------

DX::VertexBufferWrapper::VertexBufferWrapper(IDirect3DVertexBuffer9* vertexBufferD3D, uint size, DWORD usage)
:	ResourceWrapper(vertexBufferD3D)
,	vertexBuffer_(vertexBufferD3D)
,	size_(size)
,	usage_(usage)
{
	InterlockedIncrement( &s_numVertexBuffers );

#ifdef ENABLE_DX_TRACKER
	SimpleMutexHolder smh( gdx_trackerMutex );
	node_.addThisBefore( &s_VertexBuffersList );
#endif
}

//-----------------------------------------------------------------------------

DX::VertexBufferWrapper::~VertexBufferWrapper()
{
	InterlockedDecrement( &s_numVertexBuffers );
}

//-----------------------------------------------------------------------------

void DX::VertexBufferWrapper::set(IDirect3DVertexBuffer9* vertexBufferD3D)
{
	BW_GUARD;

	ResourceWrapper::set( vertexBufferD3D );
	vertexBuffer_ = vertexBufferD3D;
}

//-----------------------------------------------------------------------------
// IUnknown

ULONG DX::VertexBufferWrapper::AddRef()
{
	return ComInterface::AddRef();
}

//-----------------------------------------------------------------------------

ULONG DX::VertexBufferWrapper::Release()
{
	return ComInterface::Release();
}

//-----------------------------------------------------------------------------
// IDirect3DResource9

HRESULT DX::VertexBufferWrapper::GetDevice(IDirect3DDevice9** ppDevice)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return ResourceWrapper::GetDevice(ppDevice);
}

//-----------------------------------------------------------------------------

void DX::VertexBufferWrapper::PreLoad()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	ResourceWrapper::PreLoad();
}

//-----------------------------------------------------------------------------

D3DRESOURCETYPE DX::VertexBufferWrapper::GetType()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return ResourceWrapper::GetType();
}

//-----------------------------------------------------------------------------
// Methods

static void playVertexBufferLock()
{
	BW_GUARD;

	DX::VertexBufferWrapper* vertexBuffer = gdx_commandBuffer->read<DX::VertexBufferWrapper*>();

	UINT OffsetToLock = gdx_commandBuffer->read<UINT>();
	UINT SizeToLock = gdx_commandBuffer->read<UINT>();
	DWORD Flags = gdx_commandBuffer->read<DWORD>();

	void* ppbData = NULL;

	HRESULT res = vertexBuffer->vertexBuffer_->Lock(OffsetToLock,SizeToLock,&ppbData,Flags);

	void* vertexData = gdx_commandBuffer->read<void*>();
	uint size = SizeToLock ? SizeToLock : vertexBuffer->size_;

	if ( vertexData )
	{
		if ( ppbData )
		{
			memcpy( ppbData, vertexData, size );
		}
	}
	else
	{
		gdx_commandBuffer->skipPadding();
		if ( ppbData )
		{
			memcpy( ppbData, gdx_commandBuffer->getCurrent(), size );
		}
		gdx_commandBuffer->seek( size );
	}

	if ( res == D3D_OK )
	{
		vertexBuffer->vertexBuffer_->Unlock();
	}
}

HRESULT DX::VertexBufferWrapper::Lock(UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	if ( gdx_isMainThread && !( s_flags & WRAPPER_FLAG_IMMEDIATE_LOCK ) )
	{
		lockCommandBuffer();	// Don't allow the D3D thread to flip until its unlocked

		gdx_commandBuffer->write(playVertexBufferLock,this,OffsetToLock,SizeToLock,Flags);

		uint size = SizeToLock ? SizeToLock : size_;

		if ( s_flags & WRAPPER_FLAG_DEFERRED_LOCK )
		{
			*ppbData = JobSystem::instance().allocOutput<uint8>( size );
			gdx_commandBuffer->write( *ppbData );
		}
		else
		{
			gdx_commandBuffer->write( (void*)NULL );
			gdx_commandBuffer->writePadding();
			*ppbData = gdx_commandBuffer->getCurrentWrite();
			gdx_commandBuffer->seekWrite( size );
		}

		s_lockCount++;

		return D3D_OK;
	}
	else
	{
		flushAndStall();
		return vertexBuffer_->Lock(OffsetToLock,SizeToLock,ppbData,Flags);
	}
}

//-----------------------------------------------------------------------------

HRESULT DX::VertexBufferWrapper::Unlock()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	if ( gdx_isMainThread && !( s_flags & WRAPPER_FLAG_IMMEDIATE_LOCK ) )
	{
		s_lockCount--;
		unlockCommandBuffer();	// Allow the D3D thread to flip now
		return D3D_OK;
	}
	else
	{
		return vertexBuffer_->Unlock();
	}
}

//-----------------------------------------------------------------------------

HRESULT DX::VertexBufferWrapper::GetDesc(D3DVERTEXBUFFER_DESC *pDesc)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return vertexBuffer_->GetDesc(pDesc);
}

//-----------------------------------------------------------------------------
// Index Buffer
//-----------------------------------------------------------------------------

DX::IndexBufferWrapper::IndexBufferWrapper(IDirect3DIndexBuffer9* indexBufferD3D, uint size, DWORD usage)
:	ResourceWrapper(indexBufferD3D)
,	indexBuffer_(indexBufferD3D)
,	size_(size)
,	usage_(usage)
{
	InterlockedIncrement( &s_numIndexBuffers );

#ifdef ENABLE_DX_TRACKER
	SimpleMutexHolder smh( gdx_trackerMutex );
	node_.addThisBefore( &s_IndexBuffersList );
#endif
}

//-----------------------------------------------------------------------------

DX::IndexBufferWrapper::~IndexBufferWrapper()
{
	InterlockedDecrement( &s_numIndexBuffers );
}

//-----------------------------------------------------------------------------
// IUnknown

ULONG DX::IndexBufferWrapper::AddRef()
{
	return ComInterface::AddRef();
}

//-----------------------------------------------------------------------------

ULONG DX::IndexBufferWrapper::Release()
{
	return ComInterface::Release();
}

//-----------------------------------------------------------------------------
// IDirect3DResource9

HRESULT DX::IndexBufferWrapper::GetDevice(IDirect3DDevice9** ppDevice)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return ResourceWrapper::GetDevice(ppDevice);
}

//-----------------------------------------------------------------------------

void DX::IndexBufferWrapper::PreLoad()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	ResourceWrapper::PreLoad();
}

//-----------------------------------------------------------------------------

D3DRESOURCETYPE DX::IndexBufferWrapper::GetType()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	return ResourceWrapper::GetType();
}

//-----------------------------------------------------------------------------
// Methods

static void playIndexBufferLock()
{
	BW_GUARD;

	DX::IndexBufferWrapper* indexBuffer = gdx_commandBuffer->read<DX::IndexBufferWrapper*>();

	UINT OffsetToLock = gdx_commandBuffer->read<UINT>();
	UINT SizeToLock = gdx_commandBuffer->read<UINT>();
	DWORD Flags = gdx_commandBuffer->read<DWORD>();

	void* ppbData = NULL;

	HRESULT res = indexBuffer->indexBuffer_->Lock(OffsetToLock,SizeToLock,&ppbData,Flags);

	uint size = SizeToLock ? SizeToLock : indexBuffer->size_;

	gdx_commandBuffer->skipPadding();
	if ( ppbData )
	{
		memcpy( ppbData, gdx_commandBuffer->getCurrent(), size );
	}
	gdx_commandBuffer->seek( size );

	if ( res == D3D_OK )
	{
		indexBuffer->indexBuffer_->Unlock();
	}
}

HRESULT DX::IndexBufferWrapper::Lock(UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	if ( gdx_isMainThread && !( s_flags & WRAPPER_FLAG_IMMEDIATE_LOCK ) )
	{
		lockCommandBuffer();	// Don't allow the D3D thread to flip until its unlocked

		gdx_commandBuffer->write(playIndexBufferLock,this,OffsetToLock,SizeToLock,Flags);

		uint size = SizeToLock ? SizeToLock : size_;

		gdx_commandBuffer->writePadding();
		*ppbData = gdx_commandBuffer->getCurrentWrite();
		gdx_commandBuffer->seekWrite( size );

		s_lockCount++;

		return D3D_OK;
	}
	else
	{
		flushAndStall();
		return indexBuffer_->Lock(OffsetToLock,SizeToLock,ppbData,Flags);
	}
}

//-----------------------------------------------------------------------------

HRESULT DX::IndexBufferWrapper::Unlock()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	if ( gdx_isMainThread && !( s_flags & WRAPPER_FLAG_IMMEDIATE_LOCK ) )
	{
		s_lockCount--;
		unlockCommandBuffer();	// Allow the D3D thread to flip now
		return D3D_OK;
	}
	else
	{
		return indexBuffer_->Unlock();
	}
}

//-----------------------------------------------------------------------------
// Vertex Shader
//-----------------------------------------------------------------------------

DX::VertexShaderWrapper::VertexShaderWrapper(IDirect3DVertexShader9* vertexShaderD3D)
:	ComInterface(vertexShaderD3D)
,	vertexShader_(vertexShaderD3D)
{
	InterlockedIncrement( &s_numVertexShaders );

#ifdef ENABLE_DX_TRACKER
	SimpleMutexHolder smh( gdx_trackerMutex );
	node_.addThisBefore( &s_VertexShadersList );
#endif
}

//-----------------------------------------------------------------------------

DX::VertexShaderWrapper::~VertexShaderWrapper()
{
	InterlockedDecrement( &s_numVertexShaders );
}

//-----------------------------------------------------------------------------
// IUnknown

ULONG DX::VertexShaderWrapper::AddRef()
{
	return ComInterface::AddRef();
}

//-----------------------------------------------------------------------------

ULONG DX::VertexShaderWrapper::Release()
{
	return ComInterface::Release();
}

//-----------------------------------------------------------------------------
// Methods

HRESULT DX::VertexShaderWrapper::GetDevice(IDirect3DDevice9** ppDevice)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	gdx_device->AddRef();
	*ppDevice = gdx_device;

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::VertexShaderWrapper::GetFunction(void* pData,UINT* pSizeOfData)
{
	return vertexShader_->GetFunction(pData,pSizeOfData);
}

//-----------------------------------------------------------------------------
// Pixel Shader
//-----------------------------------------------------------------------------

DX::PixelShaderWrapper::PixelShaderWrapper(IDirect3DPixelShader9* pixelShaderD3D)
:	ComInterface(pixelShaderD3D)
,	pixelShader_(pixelShaderD3D)
{
	InterlockedIncrement( &s_numPixelShaders );

#ifdef ENABLE_DX_TRACKER
	SimpleMutexHolder smh( gdx_trackerMutex );
	node_.addThisBefore( &s_PixelShadersList );
#endif
}

//-----------------------------------------------------------------------------

DX::PixelShaderWrapper::~PixelShaderWrapper()
{
	InterlockedDecrement( &s_numPixelShaders );
}

//-----------------------------------------------------------------------------
// IUnknown

ULONG DX::PixelShaderWrapper::AddRef()
{
	return ComInterface::AddRef();
}

//-----------------------------------------------------------------------------

ULONG DX::PixelShaderWrapper::Release()
{
	return ComInterface::Release();
}

//-----------------------------------------------------------------------------
// Methods

HRESULT DX::PixelShaderWrapper::GetDevice(IDirect3DDevice9** ppDevice)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	gdx_device->AddRef();
	*ppDevice = gdx_device;

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::PixelShaderWrapper::GetFunction(void* pData,UINT* pSizeOfData)
{
	return pixelShader_->GetFunction(pData,pSizeOfData);
}

//-----------------------------------------------------------------------------
// VertexDeclarationWrapper
//-----------------------------------------------------------------------------

DX::VertexDeclarationWrapper::VertexDeclarationWrapper(IDirect3DVertexDeclaration9* vertexDeclarationD3D)
:	ComInterface(vertexDeclarationD3D)
,	vertexDeclaration_(vertexDeclarationD3D)
{
	InterlockedIncrement( &s_numVertexDeclarations );

#ifdef ENABLE_DX_TRACKER
	SimpleMutexHolder smh( gdx_trackerMutex );
	node_.addThisBefore( &s_VertexDeclarationsList );
#endif
}

//-----------------------------------------------------------------------------

DX::VertexDeclarationWrapper::~VertexDeclarationWrapper()
{
	InterlockedDecrement( &s_numVertexDeclarations );
}

//-----------------------------------------------------------------------------
// IUnknown

ULONG DX::VertexDeclarationWrapper::AddRef()
{
	return ComInterface::AddRef();
}

//-----------------------------------------------------------------------------

ULONG DX::VertexDeclarationWrapper::Release()
{
	return ComInterface::Release();
}

//-----------------------------------------------------------------------------
// Methods

HRESULT DX::VertexDeclarationWrapper::GetDevice(IDirect3DDevice9** ppDevice)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	gdx_device->AddRef();
	*ppDevice = gdx_device;

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::VertexDeclarationWrapper::GetDeclaration(D3DVERTEXELEMENT9* pElement,UINT* pNumElements)
{
	flushAndStall();
	return vertexDeclaration_->GetDeclaration(pElement,pNumElements);
}

//-----------------------------------------------------------------------------
// QueryWrapper
//-----------------------------------------------------------------------------

DX::QueryWrapper::QueryWrapper(IDirect3DQuery9* queryD3D)
:	ComInterface(queryD3D)
,	query_(queryD3D)
,	issueQueued_(0)
,	issueCookie_(0)
{
	InterlockedIncrement( &s_numQueries );

#ifdef ENABLE_DX_TRACKER
	SimpleMutexHolder smh( gdx_trackerMutex );
	node_.addThisBefore( &s_QueriesList );
#endif
}

//-----------------------------------------------------------------------------

DX::QueryWrapper::~QueryWrapper()
{
	InterlockedDecrement( &s_numQueries );
}

//-----------------------------------------------------------------------------
// IUnknown

ULONG DX::QueryWrapper::AddRef()
{
	return ComInterface::AddRef();
}

//-----------------------------------------------------------------------------

ULONG DX::QueryWrapper::Release()
{
	return ComInterface::Release();
}

//-----------------------------------------------------------------------------
// Methods

static void playQueryIssue()
{
	BW_GUARD;

	DX::QueryWrapper* query = gdx_commandBuffer->read<DX::QueryWrapper*>();
	DWORD dwIssueFlags = gdx_commandBuffer->read<DWORD>();

	query->query_->Issue(dwIssueFlags);
	InterlockedDecrement( &query->issueQueued_ );
}

HRESULT DX::QueryWrapper::Issue(DWORD dwIssueFlags)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );
	WRITE_LOCK;

	gdx_commandBuffer->write(playQueryIssue,this,dwIssueFlags);
	InterlockedIncrement( &issueQueued_ );

	issueCookie_ = s_frameCount;

	if ( ( s_flags & WRAPPER_FLAG_QUERY_ISSUE_FLUSH ) && JobSystem::instance().canBegin() )
	{
		flush();
	}

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::QueryWrapper::GetData(void* pData,DWORD dwSize,DWORD dwGetDataFlags)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	MF_ASSERT( issueCookie_ <= s_frameCount );

	if ( dwGetDataFlags & D3DGETDATA_FLUSH )
	{
		while ( s_frameCount < ( issueCookie_ + CommandBuffer::NUM_BUFFERS ) )
		{
			flush();
		}
	}

	// If the issue is still in the command buffer we know the results aren't
	// ready. Calling GetData() would return the previous results so we don't.
	if ( issueQueued_ )
		return S_FALSE;

	return query_->GetData(pData,dwSize,dwGetDataFlags);
}

//-----------------------------------------------------------------------------
// StateBlockWrapper
//-----------------------------------------------------------------------------

DX::StateBlockWrapper::StateBlockWrapper(IDirect3DStateBlock9* stateBlockD3D)
:	ComInterface(stateBlockD3D)
,	stateBlock_(stateBlockD3D)
{
#ifdef ENABLE_DX_TRACKER
	SimpleMutexHolder smh( gdx_trackerMutex );
	node_.addThisBefore( &s_StateBlocksList );
#endif
}

//-----------------------------------------------------------------------------

DX::StateBlockWrapper::~StateBlockWrapper()
{
}

//-----------------------------------------------------------------------------
// IUnknown

ULONG DX::StateBlockWrapper::AddRef()
{
	return ComInterface::AddRef();
}

//-----------------------------------------------------------------------------

ULONG DX::StateBlockWrapper::Release()
{
	return ComInterface::Release();
}

//-----------------------------------------------------------------------------
// Methods

HRESULT DX::StateBlockWrapper::GetDevice(IDirect3DDevice9** ppDevice)
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	gdx_device->AddRef();
	*ppDevice = gdx_device;

	return D3D_OK;
}

//-----------------------------------------------------------------------------

HRESULT DX::StateBlockWrapper::Capture()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	flushAndStall();
	invalidateStateCache();
	return stateBlock_->Capture();
}

//-----------------------------------------------------------------------------

HRESULT DX::StateBlockWrapper::Apply()
{
	BW_GUARD;
	PROFILER_SCOPED( DX );
	MEMTRACKER_SCOPED( DX );

	flushAndStall();
	invalidateStateCache();
	return stateBlock_->Apply();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

uint32	DX::surfaceSize( uint width, uint height, D3DFORMAT format )
{
	float scale = 0.0f;
	switch ( format )
	{
		case D3DFMT_R8G8B8:			scale =  3.0f; break;
		case D3DFMT_A8R8G8B8:		scale =  4.0f; break;
		case D3DFMT_X8R8G8B8:		scale =  4.0f; break;
		case D3DFMT_R5G6B5:			scale =  2.0f; break;
		case D3DFMT_X1R5G5B5:		scale =  2.0f; break;
		case D3DFMT_A1R5G5B5:		scale =  2.0f; break;
		case D3DFMT_A4R4G4B4:		scale =  2.0f; break;
		case D3DFMT_R3G3B2:			scale =  1.0f; break;
		case D3DFMT_A8:				scale =  1.0f; break;
		case D3DFMT_A8R3G3B2:		scale =  2.0f; break;
		case D3DFMT_X4R4G4B4:		scale =  2.0f; break;
		case D3DFMT_A2B10G10R10:	scale =  4.0f; break;
		case D3DFMT_A8B8G8R8:		scale =  4.0f; break;
		case D3DFMT_X8B8G8R8:		scale =  4.0f; break;
		case D3DFMT_G16R16:			scale =  4.0f; break;
		case D3DFMT_A2R10G10B10:	scale =  4.0f; break;
		case D3DFMT_A16B16G16R16:	scale =  8.0f; break;
		case D3DFMT_A8P8:			scale =  2.0f; break;
		case D3DFMT_P8:				scale =  1.0f; break;
		case D3DFMT_L8:				scale =  1.0f; break;
		case D3DFMT_A8L8:			scale =  2.0f; break;
		case D3DFMT_A4L4:			scale =  1.0f; break;
		case D3DFMT_V8U8:			scale =  2.0f; break;
		case D3DFMT_L6V5U5:			scale =  2.0f; break;
		case D3DFMT_X8L8V8U8:		scale =  4.0f; break;
		case D3DFMT_Q8W8V8U8:		scale =  4.0f; break;
		case D3DFMT_V16U16:			scale =  4.0f; break;
		case D3DFMT_A2W10V10U10:	scale =  4.0f; break;
		case D3DFMT_UYVY:			scale =  1.0f; break;
		case D3DFMT_R8G8_B8G8:		scale =  2.0f; break;
		case D3DFMT_YUY2:			scale =  1.0f; break;
		case D3DFMT_G8R8_G8B8:		scale =  2.0f; break;
		case D3DFMT_DXT1: 			scale =  0.5f; break;
		case D3DFMT_DXT2: 			scale =  1.0f; break;
		case D3DFMT_DXT3: 			scale =  1.0f; break;
		case D3DFMT_DXT4: 			scale =  1.0f; break;
		case D3DFMT_DXT5: 			scale =  1.0f; break;
		case D3DFMT_D16_LOCKABLE:	scale =  2.0f; break;
		case D3DFMT_D32:			scale =  4.0f; break;
		case D3DFMT_D15S1: 			scale =  2.0f; break;
		case D3DFMT_D24S8: 			scale =  4.0f; break;
		case D3DFMT_D24X8: 			scale =  4.0f; break;
		case D3DFMT_D24X4S4:		scale =  4.0f; break;
		case D3DFMT_D16:			scale =  2.0f; break;
		case D3DFMT_D32F_LOCKABLE:	scale =  4.0f; break;
		case D3DFMT_D24FS8:			scale =  4.0f; break;
		case D3DFMT_L16:			scale =  2.0f; break;
		case D3DFMT_Q16W16V16U16: 	scale =  8.0f; break;
		case D3DFMT_MULTI2_ARGB8: 	scale =  0.0f; break;
		case D3DFMT_R16F:			scale =  2.0f; break;
		case D3DFMT_G16R16F:		scale =  4.0f; break;
		case D3DFMT_A16B16G16R16F:	scale =  8.0f; break;
		case D3DFMT_R32F:			scale =  4.0f; break;
		case D3DFMT_G32R32F:		scale =  8.0f; break;
		case D3DFMT_A32B32G32R32F:	scale = 16.0f; break;
		case D3DFMT_CxV8U8:			scale =  2.0f; break;
	}

	return uint32(scale * (float)(width * height));
}

uint32	DX::surfaceSize( const D3DSURFACE_DESC& desc )
{
	return surfaceSize(desc.Width, desc.Height, desc.Format);
}


uint32 DX::textureSize( const Texture *constTexture )
{
	BW_GUARD;
	if (constTexture == NULL)
		return 0;

	// The operations on the texture are really const operations
	Texture *texture = const_cast<Texture *>(constTexture);

	// Determine the mip-map texture size scaling factor
	double mipmapScaler = ( 4 - pow(0.25, (double)texture->GetLevelCount() - 1 ) ) / 3;

	// Get a surface to determine the width, height, and format
	D3DSURFACE_DESC surfaceDesc;
	texture->GetLevelDesc(0, &surfaceDesc);

	// Get the surface size
	uint32 surfaceSize = DX::surfaceSize(surfaceDesc);

	// Track memory usage
	return (uint32)(surfaceSize * mipmapScaler);
}


#define ERRORSTRING( code, ext )											\
	if (hr == code)															\
	{																		\
		bw_snprintf( res, sizeof(res), #code "(0x%08x) : %s", code, ext );	\
	}


std::string DX::errorAsString( HRESULT hr )
{
	char res[1024];
	     ERRORSTRING( D3D_OK, "No error occurred." )
	else ERRORSTRING( D3DOK_NOAUTOGEN, "This is a success code. However, the autogeneration of mipmaps is not supported for this format. This means that resource creation will succeed but the mipmap levels will not be automatically generated." )
	else ERRORSTRING( D3DERR_CONFLICTINGRENDERSTATE, "The currently set render states cannot be used together." )
	else ERRORSTRING( D3DERR_CONFLICTINGTEXTUREFILTER, "The current texture filters cannot be used together." )
	else ERRORSTRING( D3DERR_CONFLICTINGTEXTUREPALETTE, "The current textures cannot be used simultaneously." )
	else ERRORSTRING( D3DERR_DEVICELOST, "The device has been lost but cannot be reset at this time. Therefore, rendering is not possible." )
	else ERRORSTRING( D3DERR_DEVICENOTRESET, "The device has been lost but can be reset at this time." )
	else ERRORSTRING( D3DERR_DRIVERINTERNALERROR, "Internal driver error. Applications should destroy and recreate the device when receiving this error. For hints on debugging this error, see Driver Internal Errors (Direct3D 9)." )
	else ERRORSTRING( D3DERR_DRIVERINVALIDCALL, "Not used." )
	else ERRORSTRING( D3DERR_INVALIDCALL, "The method call is invalid. For example, a method's parameter may not be a valid pointer." )
	else ERRORSTRING( D3DERR_INVALIDDEVICE, "The requested device type is not valid." )
	else ERRORSTRING( D3DERR_MOREDATA, "There is more data available than the specified buffer size can hold." )
	else ERRORSTRING( D3DERR_NOTAVAILABLE, "This device does not support the queried technique." )
	else ERRORSTRING( D3DERR_NOTFOUND, "The requested item was not found." )
	else ERRORSTRING( D3DERR_OUTOFVIDEOMEMORY, "Direct3D does not have enough display memory to perform the operation." )
	else ERRORSTRING( D3DERR_TOOMANYOPERATIONS, "The application is requesting more texture-filtering operations than the device supports." )
	else ERRORSTRING( D3DERR_UNSUPPORTEDALPHAARG, "The device does not support a specified texture-blending argument for the alpha channel." )
	else ERRORSTRING( D3DERR_UNSUPPORTEDALPHAOPERATION, "The device does not support a specified texture-blending operation for the alpha channel." )
	else ERRORSTRING( D3DERR_UNSUPPORTEDCOLORARG, "The device does not support a specified texture-blending argument for color values." )
	else ERRORSTRING( D3DERR_UNSUPPORTEDCOLOROPERATION, "The device does not support a specified texture-blending operation for color values." )
	else ERRORSTRING( D3DERR_UNSUPPORTEDFACTORVALUE, "The device does not support the specified texture factor value. Not used; provided only to support older drivers." )
	else ERRORSTRING( D3DERR_UNSUPPORTEDTEXTUREFILTER, "The device does not support the specified texture filter." )
	else ERRORSTRING( D3DERR_WASSTILLDRAWING, "The previous blit operation that is transferring information to or from this surface is incomplete." )
	else ERRORSTRING( D3DERR_WRONGTEXTUREFORMAT, "The pixel format of the texture surface is not valid." )

	else ERRORSTRING( D3DXERR_CANNOTMODIFYINDEXBUFFER, "The index buffer cannot be modified." )
	else ERRORSTRING( D3DXERR_INVALIDMESH, "The mesh is invalid." )
	else ERRORSTRING( D3DXERR_CANNOTATTRSORT, "Attribute sort (D3DXMESHOPT_ATTRSORT) is not supported as an optimization technique." )
	else ERRORSTRING( D3DXERR_SKINNINGNOTSUPPORTED, "Skinning is not supported." )
	else ERRORSTRING( D3DXERR_TOOMANYINFLUENCES, "Too many influences specified." )
	else ERRORSTRING( D3DXERR_INVALIDDATA, "The data is invalid." )
	else ERRORSTRING( D3DXERR_LOADEDMESHASNODATA, "The mesh has no data." )
	else ERRORSTRING( D3DXERR_DUPLICATENAMEDFRAGMENT, "A fragment with that name already exists." )
	else ERRORSTRING( D3DXERR_CANNOTREMOVELASTITEM, "The last item cannot be deleted." )

	else ERRORSTRING( E_FAIL, "An undetermined error occurred inside the Direct3D subsystem." )
	else ERRORSTRING( E_INVALIDARG, "An invalid parameter was passed to the returning function." )
//	else ERRORSTRING( E_INVALIDCALL, "The method call is invalid. For example, a method's parameter may have an invalid value." )
	else ERRORSTRING( E_NOINTERFACE, "No object interface is available." )
	else ERRORSTRING( E_NOTIMPL, "Not implemented." )
	else ERRORSTRING( E_OUTOFMEMORY, "Direct3D could not allocate sufficient memory to complete the call." )
	else ERRORSTRING( S_OK, "No error occurred." )
	else
	{
		bw_snprintf( res, sizeof(res), "Unknown(0x%08x)", hr );
	}

	return std::string(res);
}
