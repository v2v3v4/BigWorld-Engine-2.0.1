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
#include "render_context.hpp"

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <d3dx9.h>

#include "cstdmf/dogwatch.hpp"
#include "cstdmf/watcher.hpp"
#include "cstdmf/string_utils.hpp"
#include "cstdmf/processor_affinity.hpp"

#include "device_callback.hpp"

#include "vertex_formats.hpp"
#include "dynamic_vertex_buffer.hpp"
#include "dynamic_index_buffer.hpp"
#include "effect_state_manager.hpp"
#include "effect_visual_context.hpp"
#include "mrt_support.hpp"
#include "visual_manager.hpp"
#include "primitive_manager.hpp"
#include "vertices_manager.hpp"
#include "animation_manager.hpp"

PROFILER_DECLARE( drawPrimitive, "Context DrawPrimitive" );
PROFILER_DECLARE( setState, "Context SetState" );

#ifndef CODE_INLINE
#include "render_context.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "Moo", 0 )

PROFILER_DECLARE( GPU_Wait, "GPU Wait" );
PROFILER_DECLARE( PreloadResources, "D3D Resources Preload" );

namespace // anonymous
{
bool s_lost = false;
bool s_changingMode = false;

// Flie scope variables
HCURSOR s_OriginalMouseCursor = NULL;

/**
 * Output a string error message for some DirectX errors.
 */
void printErrorMsg_( HRESULT hr, const std::string & msg )
{	
	switch ( hr )
	{
	case D3DERR_DRIVERINTERNALERROR:
		CRITICAL_MSG( "%s %s\n", msg.c_str(), DX::errorAsString( hr ).c_str() );
		break;
	default:
		WARNING_MSG( "%s %s\n", msg.c_str(), DX::errorAsString( hr ).c_str() );
		break;
	}
}

uint32 s_numPreloadResources = 0;
const size_t MAX_PRELOAD_COUNT = 2000;

} // namespace anonymous

// comparison operators for d3ddisplaymodes
inline bool operator == ( const D3DDISPLAYMODE& dm1, const D3DDISPLAYMODE& dm2 )
{
	return ( ( dm1.Width == dm2.Width ) &&
		( dm1.Height == dm2.Height ) &&
		( dm1.Format == dm2.Format ) &&
		( dm1.RefreshRate == dm2.RefreshRate ) );
}

inline bool operator != ( const D3DDISPLAYMODE& dm1, const D3DDISPLAYMODE& dm2 )
{
	return !( dm1 == dm2 );
}

inline bool operator < ( const D3DDISPLAYMODE& dm1, const D3DDISPLAYMODE& dm2 )
{
	if( dm1.Width < dm2.Width )
		return true;
	if( dm1.Width > dm2.Width )
		return false;
	if( dm1.Height < dm2.Height )
		return true;
	if( dm1.Height > dm2.Height )
		return false;
	if( dm1.Format < dm2.Format )
		return true;
	if( dm1.Format > dm2.Format )
		return false;
	if( dm1.RefreshRate < dm2.RefreshRate )
		return true;
	if( dm1.RefreshRate > dm2.RefreshRate )
		return false;
	return false;
}

namespace Moo
{

extern class RenderContext* g_RC;

const float RenderContext::VP_MAXZ = 0.999f;

/**
 *
 */
class OcclusionQuery
{
public:
	int					index;
	IDirect3DQuery9*	queryObject;
private:
};

THREADLOCAL( bool ) g_renderThread = false;

static void clearStateRecorders()
{
	if ( rc().usingWrapper() )
	{
		WrapperStateRecorder::clear();
	}
	else
	{
		StateRecorder::clear();
	}
}

/**
 *	This method sets up the gamma correction value on the device.
 */
void RenderContext::setGammaCorrection()
{
	BW_GUARD;
	gammaCorrection_ = max( 0.5f, min( 6.f, gammaCorrection_ ) );
	if (device_.hasComObject())
	{
		D3DGAMMARAMP ramp;

		for (uint32 i = 0; i < 256; i++)
		{
			float f = /*1.f -*/ (float(i) / 255.f);
			ramp.red[i] = ramp.green[i] = ramp.blue[i] = WORD( ( /*1.f -*/ powf( f, 1.f / gammaCorrection_ ) ) * 65535.f );
		}

		device_->SetGammaRamp( 0, D3DSGR_CALIBRATE, &ramp );
	}
}


/**
 *	The render contexts constructor, prepares to create a dx device, and
 *	sets default values.
 */
RenderContext::RenderContext()
: camera_( 0.25, 500, DEG_TO_RAD( 60 ), 4.f/3.f ),
  windowed_( true ),
  hideCursor_( true ),
  windowedStyle_( WS_OVERLAPPEDWINDOW ),
  currentFrame_( 0 ),
  primitiveGroupCount_( 0 ),
  halfScreenWidth_( 320.f ),
  halfScreenHeight_( 240.f ),
  fullScreenAspectRatio_( 4.f/3.f ),
  alphaOverride_( 0xff000000 ),
  depthOnly_( false ),
  lodValue_( 1.f ),
  lodFar_( 400 ),
  lodPower_( 10 ),
  zoomFactor_( 1 ),
  stencilWanted_( false ),
  stencilAvailable_( false ),
  gammaCorrection_( 1.f ),
  maxZ_( 1.f ),
  cacheValidityId_( 0 ),
  waitForVBL_( true ),
  tripleBuffering_( true ),
  vsVersion_( 0 ),
  psVersion_( 0 ),
  mixedVertexProcessing_( false ),
  mrtSupported_( false ),
  mirroredTransform_(false),
  reflectionScene_(false),
  paused_( false ),
  memoryCritical_( false ),
  currentObjectID_( 0.f ),
  renderTargetCount_( 0 ),
  backBufferWidthOverride_( 0 ),
  beginSceneCount_(0),
  fogEnabled_( false ),
  fogColour_( 0x000000FF),
  fogNear_(0),
  fogFar_(500),
  isValid_( false ),
  pDynamicIndexBufferInterface_( NULL )
{
	BW_GUARD;
	view_.setIdentity();
	projection_.setIdentity();
	viewProjection_.setIdentity();
	lastViewProjection_.setIdentity();
	invView_.setIdentity();
	world_.push_back( projection_ );

	renderTarget_[0] = NULL;
	renderTarget_[1] = NULL;
}

bool RenderContext::init()
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( isValid_ == false && "RenderContext already initialised")
	{
		return true;
	}

	// Create the directx9 interface
	d3d_.pComObject( Direct3DCreate9( D3D_SDK_VERSION ) );
	if( d3d_.pComObject() != NULL )
	{
		// Reduce the reference count, because d3ds_.pComobject( ) increases it.
		d3d_->Release();

		// Iterate through the adapters.
		for (uint32 adapterIndex = 0; adapterIndex < d3d_->GetAdapterCount(); adapterIndex++)
		{
			DeviceInfo deviceInfo;
			deviceInfo.windowed_ = false;
			deviceInfo.adapterID_ = adapterIndex;
			if (D3D_OK == d3d_->GetAdapterIdentifier( adapterIndex, 0, &deviceInfo.identifier_ ))
			{
				if (D3D_OK == d3d_->GetDeviceCaps( adapterIndex, D3DDEVTYPE_HAL, &deviceInfo.caps_ ))
				{
					const uint32 VENDOR_ID_ATI = 0x1002;
					const uint32 VENDOR_ID_NVIDIA = 0x10de;

					// set up compatibilty flags for special rendering codepaths
					deviceInfo.compatibilityFlags_ = 0;
					if (strstr(deviceInfo.identifier_.Description, "GeForce4 MX") != NULL)
					{
						// this card does not support D3DLOCK_NOOVERWRITE in the way flora uses it
						deviceInfo.compatibilityFlags_ |= COMPATIBILITYFLAG_NOOVERWRITE;
						NOTICE_MSG("GeForce4 MX compatibility selected\n");
					}

					if (deviceInfo.identifier_.VendorId == VENDOR_ID_NVIDIA)
					{
						// this card does not support D3DLOCK_NOOVERWRITE in the way flora uses it
						deviceInfo.compatibilityFlags_ |= COMPATIBILITYFLAG_NVIDIA;
						NOTICE_MSG("nVidia compatibility selected\n");
					}

					if (deviceInfo.identifier_.VendorId == VENDOR_ID_ATI)
					{
						// this card does not support D3DLOCK_NOOVERWRITE in the way flora uses it
						deviceInfo.compatibilityFlags_ |= COMPATIBILITYFLAG_ATI;
						NOTICE_MSG("ATI compatibility selected\n");
					}


					D3DFORMAT formats[] = { D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8, D3DFMT_A2B10G10R10, D3DFMT_X1R5G5B5, D3DFMT_A1R5G5B5, D3DFMT_R5G6B5 };
					uint32 nFormats = sizeof( formats ) / sizeof(D3DFORMAT);

					for (uint32 fi = 0; fi < nFormats; fi++)
					{
						D3DFORMAT format = formats[fi];
						// Iterate through adapter modes
						for (uint32 displayModeIndex = 0; displayModeIndex < d3d_->GetAdapterModeCount( adapterIndex, format ); displayModeIndex++)
						{
							D3DDISPLAYMODE mode;
							if (D3D_OK == d3d_->EnumAdapterModes( adapterIndex, format, displayModeIndex, &mode ))
							{
								mode.RefreshRate = 0;
								if( std::find( deviceInfo.displayModes_.begin(), deviceInfo.displayModes_.end(), mode ) == deviceInfo.displayModes_.end() )
								{
									if( mode.Width >= 640 && mode.Height >= 480 )
									{
										deviceInfo.displayModes_.push_back( mode );
									}
								}
							}
						}
					}
					deviceInfo.windowed_ = true; //( deviceInfo.caps_.Caps2 & D3DCAPS2_CANRENDERWINDOWED ) != 0;
				}

				if (deviceInfo.displayModes_.size() != 0 || deviceInfo.windowed_ == true)
				{

					if( deviceInfo.windowed_ )
					{
						d3d_->GetAdapterDisplayMode( deviceInfo.adapterID_, &deviceInfo.windowedDisplayMode_ );
					}

					std::sort( deviceInfo.displayModes_.begin(), deviceInfo.displayModes_.end() );
					devices_.push_back( deviceInfo );
				}
			}
		}

		if (devices_.size())
		{
			// We succedeed in initialising the RenderContext.
			isValid_ = true;
		}
		else
		{
			ERROR_MSG( "Moo::RenderContext::RenderContext: No hardware rasterisation devices found.\n" );
		}
	}
	else
	{
		ERROR_MSG( "Moo::RenderContext::RenderContext: Unable to create Directx interface. Is Directx9c installed?\n" );
	}

	return isValid_;
}

void RenderContext::releaseUnmanaged()
{
	BW_GUARD;
	renderTargetStack_.clear();
	renderTarget_[0] = NULL;
	renderTarget_[1] = NULL;
	secondRenderTargetTexture_ = NULL;

	clearStateRecorders();
	DeviceCallback::deleteAllUnmanaged();
	releaseQueryObjects();

	SimpleMutexHolder smh(preloadResourceMutex_);
	while (preloadResourceList_.size())
	{
		preloadResourceList_.back()->Release();
		preloadResourceList_.pop_back();
	}

	memoryCritical_=false;
}

void RenderContext::createUnmanaged()
{
	BW_GUARD;
	DeviceCallback::createAllUnmanaged();
	createQueryObjects();
	if (!renderTarget_[0].hasComObject())
	{
		device_->GetRenderTarget(0, &renderTarget_[0]);
		device_->SetRenderTarget(0, renderTarget_[0].pComObject());
	}
	if (MRTSupport::instance().isEnabled())
	{
		this->createSecondSurface();
		device_->SetRenderTarget(1, renderTarget_[1].pComObject());
	}
}

/**
 *	This method creates the second surface if we are doing MRT.
 */
void RenderContext::createSecondSurface()
{
	renderTarget_[1] = NULL;
	secondRenderTargetTexture_ = NULL;

	//The PC can only create standard 32bit colour render targets.
	secondRenderTargetTexture_ =  this->createTexture(
		(UINT)this->screenWidth(),
		(UINT)this->screenHeight(),
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8, 
		D3DPOOL_DEFAULT,
		"texture/render target/second_main_surface" );

	if (secondRenderTargetTexture_.hasComObject())
	{
		HRESULT hr = (secondRenderTargetTexture_->GetSurfaceLevel( 0, &renderTarget_[1] ));
		if ( FAILED( hr ) )
		{
			WARNING_MSG( "Failed to get second RT surface\n" );
			return;
		}
		else
		{
			INFO_MSG( "Created Second RT Surface\n" );
		}
	}
	else
	{
		WARNING_MSG( "Failed to create texture for second RT surface\n" );
	}
}


/**
 *	This method changes the current display mode or windowed/fullscreen
 *	mode on the current device.
 *
 *	@param modeIndex is the index of the displaymode to choose.
 *	@param windowed tells the rendercontext whether to use windowed mode or not.
 *	@param testCooperative if true, tests the device's cooperative level before
 *		   changing the mode. The mode is not changed if the device is lost.
 *	@param backBufferWidthOverride	Width to set the back buffer to. 0
 *									indicates same width as the visible display.
 *
 *
 *	@return true if the operation was successful, false otherwise.
 */
bool RenderContext::changeMode( uint32 modeIndex, bool windowed, bool testCooperative, uint32 backBufferWidthOverride )
{
	BW_GUARD;
	// Avoid changeMode to be called recursively from 
	// App::resizeWindow. resizeWindow will be triggered 
	// by the SetWindowPos call when going windowed.
	IF_NOT_MF_ASSERT_DEV(!s_changingMode)
	{
		return false;
	}

	s_changingMode = true;

	bool goingFullScreen = windowed != windowed_ && windowed == false;
	bool goingWindowMode = windowed != windowed_ && windowed == true;

	if (goingFullScreen)
	{
		// from windowed to fullscreen
		// save windows geometry info
		GetWindowRect( windowHandle_, &windowedRect_ );
		// save the windowed style to restore it later, and set it to
		// WS_POPUP so the fullscreen window doesn't have a caption.
		windowedStyle_ = GetWindowLong( windowHandle_, GWL_STYLE );
		SetWindowLong( windowHandle_, GWL_STYLE, WS_POPUP );
	}
	else if (goingWindowMode)
	{
		// restore the window's windowed style
		SetWindowLong( windowHandle_, GWL_STYLE, windowedStyle_ );
	}

	// switch 
	releaseUnmanaged();
	bool result = this->changeModePriv(
		modeIndex, windowed, testCooperative, 
		backBufferWidthOverride);

	if (result && goingWindowMode)
	{
		// If going back to windowed mode, resize it.
		SetWindowPos( windowHandle_, HWND_NOTOPMOST,
					  windowedRect_.left, windowedRect_.top,
					  ( windowedRect_.right - windowedRect_.left ),
					  ( windowedRect_.bottom - windowedRect_.top ),
					  SWP_SHOWWINDOW );

		// finally, adjust backbuffer size to window
		// equivalent to calling resetDevice, but we
		// don't want to call createAllUnmanaged and 
		// createAllUnmanaged again.
		result = this->changeModePriv(modeIndex_, windowed_, true, 0);
	}
	if (result)
	{
		createUnmanaged();
	}

	s_changingMode = false;
	return result;
}

/**
 *	This method resets the device to the exact same current modeIndex and windowed
 *	mode. Use it whenever the underlying window is resized to adjust the size of the 
 *	frame buffers.
 */
bool RenderContext::resetDevice()
{
	BW_GUARD;
	BWResource::WatchAccessFromCallingThreadHolder holder( false );
	if (s_changingMode)
	{
		return false;
	}

	releaseUnmanaged();

	bool result = this->changeModePriv(modeIndex_, windowed_, true, 0);
	if (result)
	{
		createUnmanaged();
	}

	return result;
}

/**
 *	This method resets the current display mode or windowed/fullscreen
 *	mode on the current device. It differs from changeMode by not reseting the 
 *
 *	@param modeIndex is the index of the displaymode to choose.
 *	@param windowed tells the rendercontext whether to use windowed mode or not.
 *	@param testCooperative if true, tests the device's cooperative level before
 *		   changing the mode. The mode is not changed if the device is lost.
 *	@param backBufferWidthOverride	Width to set the back buffer to. 0
 *									indicates same width as the visible display.
 *
 *	@return true if the operation was successful.
 */
bool RenderContext::changeModePriv( uint32 modeIndex, bool windowed, bool testCooperative, uint32 backBufferWidthOverride )
{
	BW_GUARD;
	if (testCooperative && 
		Moo::rc().device()->TestCooperativeLevel() == D3DERR_DEVICELOST)
	{
		return false;
	}

	screenCopySurface_ = NULL;
	clearStateRecorders();
	IF_NOT_MF_ASSERT_DEV( device_.pComObject() )
	{
		return false;
	}

	if (modeIndex != -1)
	{
		modeIndex_ = modeIndex;
		windowed_  = windowed;
	}
	backBufferWidthOverride_ = backBufferWidthOverride;

	// are we changing windowed status?
	fillPresentationParameters();

    // Some drivers have difficulty in restoring a device for a minimised 
    // window unless the width and height of the window are set in the 
    // presentation parameters.
    WINDOWPLACEMENT placement;
    GetWindowPlacement(windowHandle_, &placement);
    if (backBufferWidthOverride_ == 0 && placement.showCmd == SW_SHOWMINIMIZED)
    {
        RECT &rect = placement.rcNormalPosition;
        presentParameters_.BackBufferWidth  = abs(rect.right  - rect.left);
        presentParameters_.BackBufferHeight = abs(rect.bottom - rect.top );
    }

	HRESULT hr = D3DERR_DEVICELOST;
	while( hr == D3DERR_DEVICELOST )
	{
		INFO_MSG( "RenderContext::changeMode - trying to reset \n" );
		hr = device_->Reset( &presentParameters_ );

		s_lost = true;
		if( FAILED( hr ) && hr != D3DERR_DEVICELOST )
		{
			WARNING_MSG( "Moo::RenderContext::changeMode: Unable to reset device:: %s", 
				DX::errorAsString( hr ).c_str() );
			return false;
		}
		Sleep( 100 );
	}

	s_lost = false;

	beginSceneCount_ = 0;
	UINT availTexMem = device_->GetAvailableTextureMem();
	INFO_MSG( "RenderContext::changeMode - available texture memory after reset: %d\n", availTexMem );

	updateDeviceInfo();
	updateProjectionMatrix();
	initRenderStates();
	if (mixedVertexProcessing_)
		device_->SetSoftwareVertexProcessing( TRUE );

    // Set the affinity of the app.  There were some 
    // problems previously with multi-core systems.
	ProcessorAffinity::update();

	// save windowed mode size
	if (windowed_)
	{
		RECT clientRect;
		GetClientRect( windowHandle_, &clientRect );
		this->windowedSize_ = Vector2(
			float(clientRect.right - clientRect.left),
            float(clientRect.bottom - clientRect.top));
	}

	return true;
}


/*
 *	this method updates all the info that can be queried from
 *	outside the rendercontext.
 */
void RenderContext::updateDeviceInfo( )
{
	BW_GUARD;
	DX::Surface*  pBackBuffer;
	device_->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
	pBackBuffer->GetDesc( &backBufferDesc_ );
	pBackBuffer->Release();

	halfScreenWidth_ = float( backBufferDesc_.Width ) * 0.5f;
	halfScreenHeight_ = float( backBufferDesc_.Height ) * 0.5f;

	DX::Surface* surf = NULL;

	D3DDISPLAYMODE smode;
	d3d_->GetAdapterDisplayMode( devices_[ deviceIndex_ ].adapterID_, &smode );

	HRESULT hr;
	if ( SUCCEEDED( hr = device_->CreateOffscreenPlainSurface( backBufferDesc_.Width, backBufferDesc_.Height, 
		D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &surf, NULL ) ) )
	{
		screenCopySurface_.pComObject( surf );
		surf->Release();
	}
	else
	{
		screenCopySurface_.pComObject( NULL );
	}

	DEBUG_MSG( "Moo: Changed mode: %d %d\n", backBufferDesc_.Width, backBufferDesc_.Height );
}


/*
 *	This method sets up the presentation parameters, for a reset or a createdevice call.
 */
void RenderContext::fillPresentationParameters( void )
{
	BW_GUARD;
	ZeroMemory( &presentParameters_, sizeof( presentParameters_ ) );

	if( windowed_ )
	{
		presentParameters_.BackBufferWidth = backBufferWidthOverride_;
		presentParameters_.BackBufferHeight = static_cast<uint32>( backBufferWidthOverride_ * ( screenHeight() / screenWidth() ) );
		presentParameters_.BackBufferFormat = devices_[ deviceIndex_ ].windowedDisplayMode_.Format;
		presentParameters_.PresentationInterval = waitForVBL_ ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	}
	else
	{
		D3DDISPLAYMODE mode = devices_[ deviceIndex_ ].displayModes_[ modeIndex_ ];
		presentParameters_.BackBufferWidth = mode.Width;
		presentParameters_.BackBufferHeight = mode.Height;
		presentParameters_.BackBufferFormat = mode.Format;
		presentParameters_.PresentationInterval = waitForVBL_ ? D3DPRESENT_INTERVAL_DEFAULT : D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	if ( presentParameters_.BackBufferFormat == D3DFMT_X8R8G8B8 )
	{
		if (SUCCEEDED(d3d_->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8, windowed_)))
		{
			presentParameters_.BackBufferFormat = D3DFMT_A8R8G8B8;
			INFO_MSG( "This adapter supports A8R8G8B8 back buffers\n" );
		}	
		else
		{
			INFO_MSG( "This adapter does not support A8R8G8B8 back buffers\n" );
		}
	}

	presentParameters_.Windowed = windowed_;
	presentParameters_.BackBufferCount = tripleBuffering_ ? 2 : 1;
	presentParameters_.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters_.MultiSampleType = D3DMULTISAMPLE_NONE;
	presentParameters_.EnableAutoDepthStencil = TRUE;
	if (!s_lost)
		presentParameters_.AutoDepthStencilFormat = 
			getMatchingZBufferFormat( presentParameters_.BackBufferFormat, 
									  stencilWanted_, stencilAvailable_ );
	else if (stencilAvailable_)
		presentParameters_.AutoDepthStencilFormat = D3DFMT_D24S8;
	else
		presentParameters_.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters_.Flags = 0;
	presentParameters_.hDeviceWindow = windowHandle_;
}


/**
 *	This method releases the rendercontexts reference to a d3d device, and
 *	tries to clean up all references to d3d objects.
 */
void RenderContext::releaseDevice( void )
{
	BW_GUARD;
	if ( hideCursor_ )
	{
		ShowCursor( TRUE );
		SetCursor( s_OriginalMouseCursor );
	}

	lightContainer_ = NULL;
	specularLightContainer_ = NULL;

	screenCopySurface_ = NULL;
	releaseUnmanaged();	
	DeviceCallback::deleteAllManaged();

	// Release all device objects
	if( device_.pComObject() != NULL )
	{
		clearBindings();

		//reset window if it is not the correct size
		if( !windowed_ )
		{
			SetWindowLong( windowHandle_, GWL_STYLE, windowedStyle_ );
            SetWindowPos( windowHandle_, HWND_NOTOPMOST,
                          windowedRect_.left, windowedRect_.top,
                          ( windowedRect_.right - windowedRect_.left ),
                          ( windowedRect_.bottom - windowedRect_.top ),
                          SWP_SHOWWINDOW );
		}
	}

	// Release the render device
	device_.pComObject( NULL );
}

/**
 *	Gets amount of available texture memory.
 */
uint RenderContext::getAvailableTextureMem( ) const
{
	BW_GUARD;
	return device_->GetAvailableTextureMem();
}

/**
 *	Get a zbuffer format that is allowed with the current render format.
 */
D3DFORMAT RenderContext::getMatchingZBufferFormat( D3DFORMAT colourFormat, bool stencilWanted, bool & stencilAvailable )
{
	BW_GUARD;
	uint32 adapter = devices_[ deviceIndex_ ].adapterID_;
	D3DFORMAT format = this->adapterFormat();

	if( colourFormat == D3DFMT_R8G8B8 || colourFormat == D3DFMT_X8R8G8B8 || colourFormat == D3DFMT_A8R8G8B8 )
	{
		if ( !stencilWanted )
		{
			if( SUCCEEDED( d3d_->CheckDeviceFormat( adapter, deviceType_, format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D32 ) ) &&
				SUCCEEDED( d3d_->CheckDepthStencilMatch( adapter, deviceType_, format, colourFormat, D3DFMT_D32 ) ) )
				return D3DFMT_D32;
			if( SUCCEEDED( d3d_->CheckDeviceFormat( adapter, deviceType_, format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X8 ) ) &&
				SUCCEEDED( d3d_->CheckDepthStencilMatch( adapter, deviceType_, format, colourFormat, D3DFMT_D24X8 ) ) )
				return D3DFMT_D24X8;
			if( SUCCEEDED( d3d_->CheckDeviceFormat( adapter, deviceType_, format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8 ) ) &&
				SUCCEEDED( d3d_->CheckDepthStencilMatch( adapter, deviceType_, format, colourFormat, D3DFMT_D24S8 ) ) )
				return D3DFMT_D24S8;
			if( SUCCEEDED( d3d_->CheckDeviceFormat( adapter, deviceType_, format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X4S4 ) ) &&
				SUCCEEDED( d3d_->CheckDepthStencilMatch( adapter, deviceType_, format, colourFormat, D3DFMT_D24X4S4 ) ) )
				return D3DFMT_D24X4S4;
		}
		else
		{
			if( SUCCEEDED( d3d_->CheckDeviceFormat( adapter, deviceType_, format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8 ) ) &&
				SUCCEEDED( d3d_->CheckDepthStencilMatch( adapter, deviceType_, format, colourFormat, D3DFMT_D24S8 ) ) )
			{
				stencilAvailable = true;
				return D3DFMT_D24S8;
			}
			else
			{
				stencilAvailable = false;
			}
		}
	}

	if ( !stencilWanted )
	{
		if( SUCCEEDED( d3d_->CheckDeviceFormat( adapter, deviceType_, format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16 ) ) &&
			SUCCEEDED( d3d_->CheckDepthStencilMatch( adapter, deviceType_, format, colourFormat, D3DFMT_D16 ) ) )
			return D3DFMT_D16;
		if( SUCCEEDED( d3d_->CheckDeviceFormat( adapter, deviceType_, format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D15S1 ) ) &&
			SUCCEEDED( d3d_->CheckDepthStencilMatch( adapter, deviceType_, format, colourFormat, D3DFMT_D15S1 ) ) )
			return D3DFMT_D15S1;
		if( SUCCEEDED( d3d_->CheckDeviceFormat( adapter, deviceType_, format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X8 ) ) &&
			SUCCEEDED( d3d_->CheckDepthStencilMatch( adapter, deviceType_, format, colourFormat, D3DFMT_D24X8 ) ) )
			return D3DFMT_D24X8;
		if( SUCCEEDED( d3d_->CheckDeviceFormat( adapter, deviceType_, format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D32 ) ) &&
			SUCCEEDED( d3d_->CheckDepthStencilMatch( adapter, deviceType_, format, colourFormat, D3DFMT_D32 ) ) )
			return D3DFMT_D32;
		if( SUCCEEDED( d3d_->CheckDeviceFormat( adapter, deviceType_, format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8 ) ) &&
			SUCCEEDED( d3d_->CheckDepthStencilMatch( adapter, deviceType_, format, colourFormat, D3DFMT_D24S8 ) ) )
			return D3DFMT_D24S8;
		if( SUCCEEDED( d3d_->CheckDeviceFormat( adapter, deviceType_, format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X4S4 ) ) &&
			SUCCEEDED( d3d_->CheckDepthStencilMatch( adapter, deviceType_, format, colourFormat, D3DFMT_D24X4S4 ) ) )
			return D3DFMT_D24X4S4;
	}
	else
	{
		if( SUCCEEDED( d3d_->CheckDeviceFormat( adapter, deviceType_, format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8 ) ) &&
			SUCCEEDED( d3d_->CheckDepthStencilMatch( adapter, deviceType_, format, colourFormat, D3DFMT_D24S8 ) ) )
		{
			stencilAvailable = true;
			return D3DFMT_D24S8;
		}
		else
		{
			stencilAvailable = false;
		}
	}
	return D3DFMT_D16;
}

/**
 *	This method updates the projection matrix based on the current camera.
 */
void RenderContext::updateProjectionMatrix( bool detectAspectRatio )
{
	BW_GUARD;
	if ( detectAspectRatio && !renderTargetStack_.nStackItems() )
	{
		if( !windowed_ )
		{
			camera_.aspectRatio( fullScreenAspectRatio_ );
		}
		else
		{
			camera_.aspectRatio( float( backBufferDesc_.Width ) / float( backBufferDesc_.Height ) );
		}
	}

	zoomFactor_ = camera_.fov() / DEG_TO_RAD( 60 );

	projection_.perspectiveProjection( camera_.fov(), camera_.aspectRatio(), camera_.nearPlane(), camera_.farPlane() );
}

/**
 *	This method checks the device's cooperative level. It returns true if
 *	the device is operational. If it has been lost, it tries to reset it,
 *	returning true if successful. It returns false if the device was lost 
 *	cannot be reset immediatly. CRITICAL_MSG will be triggered if the 
 *	device has entered an unrecoverable state.	
 *
 *  @param      reset set to true if the device was reset.
 *	@return		true if device is operational, false otherwise.
 */
bool RenderContext::checkDevice(bool *reset /*=NULL*/)
{
	BW_GUARD;
	bool result = false;
    if (reset != NULL)
        *reset = false;
	HRESULT hr = Moo::rc().device()->TestCooperativeLevel();
	switch (hr)
	{
		case D3D_OK:
			result = true;
			break;
		case D3DERR_DEVICELOST:
			result = false;
			break;
		case D3DERR_DEVICENOTRESET:
		{
			s_lost = true;
			result = Moo::rc().resetDevice();
            if (reset != NULL)
                *reset = result;
			break;
		}
		case D3DERR_DRIVERINTERNALERROR:
			CRITICAL_MSG("Internal Driver Error");
			break;
		case D3DERR_OUTOFVIDEOMEMORY:
			CRITICAL_MSG("D3DERR_OUTOFVIDEOMEMORY");
			break;
		default:
			INFO_MSG( "RenderContext::checkDevice - UNHANDLED D3D RESULT : %d\n", hr );
			break;
	}
	s_lost = !result;
	return result;
}

/**
 *	Tests the cooperative level of the device, seizing control if the 
 *	device has been lost. In this case, starts retesting the device 
 *	cooperative level, at a fixed time intervals, until it can be reset. 
 *	When that finally happens, resets the device and returns true. 
 *
 *	Messages requesting the application to close will make the function 
 *	return false. All Other messages are translated and dispatched to the 
 *	registered WinProc function. 
 *
 *	CRITICAL_MSG will be triggered if the device enters an unrecoverable 
 *	state.	
 *
 *  @param      reset set to true if the device was reset.
 *	@return		true if device is operational. False if a WM_QUIT 
 *				message or ALT+F4 key event has been intercepted.
 */ 
bool RenderContext::checkDeviceYield(bool *reset /*= NULL*/)
{
	BW_GUARD;
	bool result = true;
	while (!(result = this->checkDevice(reset)))
	{
		MSG msg;
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if ((msg.message == WM_QUIT) ||
				(msg.message == WM_KEYDOWN && 
					msg.wParam == VK_F4 && ::GetKeyState(VK_MENU)))
			{
				result = false;
				break;
			}

			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		::Sleep(50);
	}
	return result;
}

/**
 *	Creates the d3d device and binds it to a window.
 *
 *	@param hWnd is the windowhandle to bind the d3d device to.
 *	@param deviceIndex is the index of the device to create.
 *	@param modeIndex is the index of the displaymode to initialise.
 *	@param windowed says whether to create a windowed or fullscreen device.
 *	@param requireStencil says whether a stencil buffer is required.
 *	@param windowedSize windowed size.
 *	@param hideCursor says whether the cursor should be hidden.
 *	@param forceRef Boolean indicating whether to force the Direct3D
 *			software reference rasteriser.
 *	@return true if the operation was successful.
 */
bool RenderContext::createDevice( HWND hWnd, uint32 deviceIndex,
								 uint32 modeIndex, bool windowed,
								 bool requireStencil, 
								 const Vector2 & windowedSize,
								 bool hideCursor,
								 bool forceRef,
								 bool useWrapper )
{
	BW_GUARD;
	// save windowed mode size
	windowedSize_ = windowedSize;
	hideCursor_ = hideCursor;

	g_renderThread = true;
	if( deviceIndex > devices_.size() )
	{
		CRITICAL_MSG( "Moo::RenderContext::createDevice: Tried to select a device that does not exist\n" );
		return false;
	}

	// Have we created a device before?
	if( device_.pComObject() != NULL )
	{
		// If the device is the same as the current device, just change the display mode, else release the device.
		if( deviceIndex == deviceIndex_ && windowHandle_ == hWnd )
		{
			return this->changeMode( modeIndex, windowed );
		}
		else
		{
			releaseDevice();
		}
	}

	// Set up the current device parameters.
	windowHandle_ = hWnd;
	// Assumes that the style the window has when creating the device is the
	// style desired when in windowed mode, and will be restored when the
	// device is released.
	windowedStyle_ = GetWindowLong( windowHandle_, GWL_STYLE );
	if (forceRef)
		deviceType_ = D3DDEVTYPE_REF;
	else
		deviceType_ = D3DDEVTYPE_HAL;
	deviceIndex_ = deviceIndex;
	modeIndex_ = modeIndex;
	windowed_ = windowed;
	stencilWanted_ = requireStencil;

	fillPresentationParameters();
	DX::Device* deviceD3D;
	DX::Device* device;

	if( windowed_ == false )
	{
	    GetWindowRect( windowHandle_, &windowedRect_ );
		SetWindowLong( windowHandle_, GWL_STYLE, WS_POPUP );
	}

	uint32 vertexProcessing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	mixedVertexProcessing_ = false;

	if (devices_[deviceIndex].caps_.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		// Treat hardware with no pixel or vertex shader support as fixed
		// function cards.
		if (uint16(devices_[deviceIndex].caps_.VertexShaderVersion) > 0x100 &&
			uint16(devices_[deviceIndex].caps_.PixelShaderVersion) > 0x100)
		{
			vertexProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		}
		else
		{
			vertexProcessing = D3DCREATE_MIXED_VERTEXPROCESSING;
			mixedVertexProcessing_ = true;
		}
	}

	D3DDEVTYPE devType = deviceType_;
	if (forceRef)
		devType = D3DDEVTYPE_NULLREF;
	else 
	{
		// Force reference if using NVIDIA NVPerfHUD.
		std::string description 
			= devices_[ deviceIndex_ ].identifier_.Description;

		if ( description.find("PerfHUD") != std::string::npos )
			devType = D3DDEVTYPE_REF;
	}

	int retries = 5;
	HRESULT hr = D3D_OK;
	while ( retries-- )
	{
		hr = d3d_->CreateDevice( devices_[ deviceIndex_ ].adapterID_, devType, 
			windowHandle_,
 			vertexProcessing | D3DCREATE_DISABLE_DRIVER_MANAGEMENT | 
 				D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE, 
			&presentParameters_, &deviceD3D );

		if( SUCCEEDED( hr ) )
		{
			usingWrapper_ = useWrapper;
			if ( useWrapper )
			{
				device = DX::createDeviceWrapper( deviceD3D );
				DX::createEffectWrapperStateManager();
			}
			else
			{
				device = deviceD3D;
			}
			break;
		}

		WARNING_MSG( "Moo::RenderContext::createDevice: Unable to create device, retrying (%d retries left)\n", retries );
		// Wait a couple of seconds before reattempting, to give a chance to
		// other D3D apps to free there resources and/or quit properly
		Sleep( 2000 );
	}

	if ( !SUCCEEDED( hr ) )
	{
		printErrorMsg_( hr, "Moo::RenderContext::createDevice: Unable to create device" );
		return false;
	}

	// succeeded
	vsVersion_ = uint16( devices_[ deviceIndex_ ].caps_.VertexShaderVersion );
	psVersion_ = uint16( devices_[ deviceIndex_ ].caps_.PixelShaderVersion );
	maxSimTextures_ = uint16( devices_[ deviceIndex_ ].caps_.MaxSimultaneousTextures );
	maxAnisotropy_ = uint16( devices_[ deviceIndex_ ].caps_.MaxAnisotropy );
	beginSceneCount_ = 0;

	mrtSupported_ = (psVersion_ >= 0x300) && 
		(devices_[ deviceIndex_ ].caps_.NumSimultaneousRTs > 1) &&
		(devices_[ deviceIndex_ ].caps_.PrimitiveMiscCaps & D3DPMISCCAPS_INDEPENDENTWRITEMASKS);

	device_.pComObject( device );
	device->Release();
	setGammaCorrection();
	UINT availTexMem = device_->GetAvailableTextureMem();
	INFO_MSG( "RenderContext::createDevice - available texture memory after create: %d\n", availTexMem );

	updateDeviceInfo();
	updateProjectionMatrix();
	initRenderStates();
	if (mixedVertexProcessing_)
		device_->SetSoftwareVertexProcessing( TRUE );

	pDynamicIndexBufferInterface_ = new DynamicIndexBufferInterface();

	if ( hideCursor_ )
	{
		s_OriginalMouseCursor = SetCursor( NULL );
		ShowCursor( FALSE );
	}
	else
	{
		s_OriginalMouseCursor = GetCursor();
	}

	if (!MRTSupport::pInstance())
	{
		MRTSupport::init();
	}

	// init managers if they have not been inited.
	if (!EffectVisualContext::pInstance())
	{
		new EffectVisualContext();
	}
	if (!EffectManager::pInstance())
	{
		new EffectManager();
	}
	if (!TextureManager::instance())
		TextureManager::init();
	if (!PrimitiveManager::instance())
		PrimitiveManager::init();
	if (!VerticesManager::instance())
		VerticesManager::init();
	if (!VisualManager::instance())
		VisualManager::init();

	if (!NodeCatalogue::pInstance())
	{
		new NodeCatalogue();
	}

	if (!AnimationManager::pInstance())
	{
		new AnimationManager();
	}	

	DeviceCallback::createAllManaged();
	createUnmanaged();

    // Set the affinity of the app.  There were some problems previously with
    // multi-core systems.
	ProcessorAffinity::update();

	MF_WATCH(	"Render/backBufferWidthOverride", 
				*this, 
				MF_ACCESSORS( uint32, RenderContext, backBufferWidthOverride ),
				"Resize the back buffer. Only works in windowed mode. 0 matches window size. Aspect ratio is maintained." );

	MF_WATCH( "Render/numPreloadResources",
		s_numPreloadResources, Watcher::WT_READ_ONLY,
		"The number of managed pool resources not yet preloaded" );

	MF_WATCH( "Render/Draw Calls",
		lastFrameProfilingData_.nDrawcalls_,  Watcher::WT_READ_ONLY,
		"DrawCalls of the previous frame." );
	MF_WATCH( "Render/Primitives",
		lastFrameProfilingData_.nPrimitives_,  Watcher::WT_READ_ONLY,
		"Primitives drawn in the previous frame." );

	return true;
}

/**
*	This method initialises all the render states and TS states in the cache.
*/
void RenderContext::initRenderStates()
{
	BW_GUARD;
	int i,j;

	for ( i = 0; i < D3DFFSTAGES_MAX; i++ )
	{
		for( j = 0; j < D3DTSS_MAX; j++ )
		{
			tssCache_[i][j].Id = 0;
		}
	}

	for( i = 0; i < D3DSAMPSTAGES_MAX; i++ )
	{
		for( j = 0; j < D3DSAMP_MAX; j++ )
		{
			sampCache_[i][j].Id = 0;
		}
		textureCache_[i].Id = 0;
	}

	for( i = 0; i < D3DRS_MAX; i++ )
	{
		rsCache_[i].Id = 0;
	}

	vertexDeclarationId_ = 0;

	// Make all cached states invalid
	cacheValidityId_ = 1;

	// Turn on clipping
	Moo::rc().setRenderState( D3DRS_CLIPPING, TRUE );
}

/**
 *	This method updates the viewprojection and the inverse view matrix,
 *	should be called whenever the view transform or the projection transform
 *	gets changed.
 */
void RenderContext::updateViewTransforms()
{
	viewProjection_.multiply( view_, projection_ );
	invView_.invert( view_ );
}

/**
 * This method sets the screen width in pixels.
 */
void RenderContext::screenWidth( int width )
{
	halfScreenWidth_ = float(width) * 0.5f;
}

/**
* This method sets the screen height in pixels.
*/
void RenderContext::screenHeight( int height )
{
	halfScreenHeight_ = float(height) * 0.5f;
}

/**
 * This function overrides the dimensions of the back buffer when in windowed mode 
 * using a given width.
 * 
 * Given values are first clamped to the maximum width supported by the device. 
 * The height is set so as to maintain the aspect ratio of the front buffer. 
 * A value of 0 means that this feature is disabled and the back buffer will 
 * have the same dimensions as the front.
 * This functionality can be used to produce screenshots at resolutions 
 * greater than that that can be displayed on a monitor.
 * 
 *	@param backBufferWidthOverride	Width to set the back buffer to. 0
 *									indicates same width as the visible display.
 */
void RenderContext::backBufferWidthOverride( uint32 backBufferWidthOverride )
{
	BW_GUARD;
	uint32 maxWidth = this->deviceInfo(0).caps_.MaxTextureWidth;

	backBufferWidthOverride = std::min( maxWidth, backBufferWidthOverride );

	if( this->windowed() == false )
		backBufferWidthOverride = 0;

    if( backBufferWidthOverride != this->backBufferWidthOverride_ )
		this->changeMode(this->modeIndex(), this->windowed(), true, backBufferWidthOverride );
}

uint32 RenderContext::backBufferWidthOverride() const
{
	return backBufferWidthOverride_;
}

const Vector2 & RenderContext::windowedModeSize() const
{
	return this->windowedSize_;
}

/**
 *	This method increases the current frame counter.
 */
void RenderContext::nextFrame( )
{
	BW_GUARD;
	DynamicVertexBufferBase::resetLockAll();
	this->dynamicIndexBufferInterface().resetLocks();
//	ZeroMemory( primitiveHistogram_, sizeof( primitiveHistogram_ ) );
	currentFrame_++;
	if ( usingWrapper_ )
	{
		DX::setCurrentFrame( currentFrame_ );
	}
}


/**
* This method enables or disables fog.
*/
void RenderContext::fogEnabled( bool enabled )
{
	fogEnabled_ = enabled;

	// set the fog onto the direct 3D device
	setRenderState( D3DRS_FOGENABLE, enabled );
}

/**
* This method sets the current fog colour.
*/
void RenderContext::fogColour( Colour colour )
{
	fogColour_ = colour;

	// Set hardware fog
	setRenderState( D3DRS_FOGCOLOR, colour);
}

/**
 * Takes a screenshot
 *
 * @param fileExt - the extension to save the screenshot as, this can be "bmp", "jpg", "tga", "png" or "dds".
 *
 * @param fileName - the root name of the file to save out
 *
 * @param autoNumber - is this is true then try to postpend a unique identifying number to the shot name(e.g. shot_666.bmp)
 */
const std::string RenderContext::screenShot( const std::string& fileExt /*= "bmp"*/, const std::string& fileName /*= "shot"*/, bool autoNumber /*= true*/ )
{
	BW_GUARD;
	static std::map< std::string, D3DXIMAGE_FILEFORMAT > format;
	if (format.size() == 0)
	{
		format["dds"] = D3DXIFF_DDS;
		format["bmp"] = D3DXIFF_BMP;
		format["jpg"] = D3DXIFF_JPG;
		format["tga"] = D3DXIFF_TGA;
		format["png"] = D3DXIFF_PNG;
	}

	//Convert uppercase to lowercase
	std::string ext = fileExt;
	std::transform( ext.begin(), ext.end(), ext.begin(), tolower );

	//It the extension specified cannot be found then use "bmp"
	if (format.find( ext ) == format.end())
	{
		ext = "bmp";
	}

	std::string name;

	if (autoNumber)
	{	
		static uint32 sequence = 1;

		// go through filenames until we find one that has not been created yet.
		bool foundEmptyFile = false;
		while( !foundEmptyFile )
		{
			// Create the filename
			std::ostringstream findName;
			findName	<< fileName 
						<< "_" << std::setfill('0') << std::setw(3) << std::right << sequence 
						<< "." << ext;

			// is there such a file?
			if( !BWResource::fileAbsolutelyExists( findName.str() ) )
			{
				// nope, we have a winner.
				foundEmptyFile = true;
				name = findName.str();
			}
			else
			{
				// try the next file.
				sequence++;
			}
		}
	}
	else
	{
		std::ostringstream oss;
		oss << fileName << "." << ext;
		name = oss.str();
	}

	ComObjectWrap<DX::Surface> backBuffer;

	if( SUCCEEDED( Moo::rc().device()->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer ) ) )
	{
		std::wstring wname;
		bw_utf8tow( name, wname );
		if (SUCCEEDED( D3DXSaveSurfaceToFile( wname.c_str(), format[ext], backBuffer.pComObject(), NULL, NULL) ) )
		{
			INFO_MSG( "Moo::RenderContext::screenShot - saved image %s\n", name.c_str() );
		}
		else
		{
			D3DSURFACE_DESC backBufferDesc;
			IDirect3DSurface9* systemMemorySurface;

			backBuffer->GetDesc( &backBufferDesc );

			if( SUCCEEDED( Moo::rc().device()->CreateOffscreenPlainSurface(	backBufferDesc.Width,
																			backBufferDesc.Height,
																			backBufferDesc.Format,
																			D3DPOOL_SYSTEMMEM,
																			&systemMemorySurface,
																			NULL ) ) )
			{
				if( SUCCEEDED( Moo::rc().device()->GetRenderTargetData( backBuffer.pComObject(), systemMemorySurface ) ) )
				{
					if( SUCCEEDED( D3DXSaveSurfaceToFile( wname.c_str(), format[ext], systemMemorySurface, NULL, NULL) ) )
					{
						INFO_MSG( "Moo::RenderContext::screenShot - saved image %s\n", name.c_str() );
					}
				}

				systemMemorySurface->Release();
			}
		}
	}
	else
	{
		ERROR_MSG( "Moo::RenderContext::screenShot - unable to get backbuffer surface\n" );
	}

	return name;
}

/**
 * This method copies the current back buffer into and internal surface, then
 * returns a pointer to the internal surface.
 */
DX::Surface* RenderContext::getScreenCopy()
{
	BW_GUARD;
	DX::Surface* ret = NULL;
	if (screenCopySurface_.hasComObject())
	{
		ComObjectWrap<DX::Surface> backBuffer;
		if( SUCCEEDED( device_->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer ) ) )
		{
			if (SUCCEEDED( D3DXLoadSurfaceFromSurface( screenCopySurface_.pComObject(), NULL, NULL,
				backBuffer.pComObject(), NULL, NULL, D3DX_FILTER_NONE, 0 ) ) )
			{
				ret = screenCopySurface_.pComObject();
			}
		}
		else
		{
			ERROR_MSG( "RenderContext::getScreenCopy - unable to get backbuffer\n" );
		}
	}
	else
	{
		ERROR_MSG( "RenderContext::getScreenCopy - no screencopy surface available\n" );
	}
	return ret;
}


void RenderContext::fini()
{
	BW_GUARD;
	// Destroy managers if they have not been destroyed.
	if (MRTSupport::pInstance() != NULL)
		MRTSupport::fini();

	clearStateRecorders();
	if (VisualManager::instance())
		VisualManager::fini();

	delete EffectManager::pInstance();

	if (PrimitiveManager::instance())
		PrimitiveManager::fini();
	if (VerticesManager::instance())
		VerticesManager::fini();

	delete AnimationManager::pInstance();

	AnimationChannel::fini();

	delete EffectVisualContext::pInstance();

	if (TextureManager::instance())
		TextureManager::fini();

	delete pDynamicIndexBufferInterface_;
	pDynamicIndexBufferInterface_ = NULL;

	DynamicVertexBufferBase::fini();
	DeviceCallback::fini();

	delete NodeCatalogue::pInstance();

	RenderContextCallback::fini();

	isValid_ = false;
}


/**
 * Destruct render context.
 */
RenderContext::~RenderContext()
{
	if ( isValid_ )
		fini();
}

bool RenderContext::supportsTextureFormat( D3DFORMAT fmt ) const
{
	BW_GUARD;
	if (device_.hasComObject())
	{
		if (SUCCEEDED( d3d_->CheckDeviceFormat( devices_[ deviceIndex_ ].adapterID_,
					deviceType_,
					adapterFormat(),
					0,
					D3DRTYPE_TEXTURE,
					fmt) ) )
			return true;
	}

	return false;
}

/**
 *	This method pushes a render target from the supplied RenderContext
 *	onto the stack.
 */
bool RenderContext::RenderTargetStack::push( RenderContext* rc )
{
	BW_GUARD;
	MF_ASSERT_DEV( rc );

	if (!rc)
		return false;

	DX::Device* device = rc->device();
	MF_ASSERT_DEV( device );

	if (!device)
		return false;

	StackItem si;
	si.cam_ = rc->camera();
	si.projection_ = rc->projection();
	si.view_ = rc->view();
	HRESULT hr;
	ComObjectWrap< DX::Surface > ts = rc->getRenderTarget( 0 );
	if (!ts.hasComObject())
	{
		CRITICAL_MSG( "Moo::RenderContext::RenderTargetStack::push: couldn't get current render target\n" );
		return false;
	}

	si.renderSurfaces_[0] = ts;
	ts = NULL;

	for ( size_t i=1; i<MAX_CONCURRENT_RTS; i++ )
	{	
		ts = rc->getRenderTarget(i);
		if ( ts )
		{
			si.renderSurfaces_[i] = ts;
			ts = NULL;
		}
	}

	hr = device->GetDepthStencilSurface( &ts );
	if( FAILED( hr ) )
	{
		CRITICAL_MSG( "Moo::RenderContext::RenderTargetStack::push: couldn't get current depth/stencil buffer\n" );
		return false;
	}
	si.zbufferSurface_ = ts;
	ts = NULL;

	rc->getViewport( &si.viewport_ );

	stackItems_.push_back( si );
	return true;
}

/**
 *	This method pops a render target into the supplied render context.
 */
bool RenderContext::RenderTargetStack::pop( RenderContext* rc )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( rc )
	{
		return false;
	}

	MF_ASSERT_DEV( stackItems_.size() );
	if (!stackItems_.size())
	{
		CRITICAL_MSG( "Tried to pop the render target when no render target was there.\n" );
		return false;
	}

	DX::Device* device = rc->device();
	MF_ASSERT_DEV( device );
	if (!device)
		return false;

	StackItem si = stackItems_.back();
	stackItems_.pop_back();

	IF_NOT_MF_ASSERT_DEV(si.renderSurfaces_[0])
	{
		return false;
	}

	HRESULT hr = rc->setRenderTarget( 0, si.renderSurfaces_[0] );
	if (SUCCEEDED(hr))
	{
		for ( size_t i=1; i<MAX_CONCURRENT_RTS; i++ )
		{
			if ( si.renderSurfaces_[i].hasComObject() )
				hr = rc->setRenderTarget( i, si.renderSurfaces_[i] );
			else
				rc->setRenderTarget( i, NULL );
		}
	}

	HRESULT hr2 = device->SetDepthStencilSurface( &*si.zbufferSurface_ );
	if( FAILED( hr ) || FAILED( hr2 ) )
	{
		CRITICAL_MSG( "Moo::RenderContext::RenderTargetStack::pop: couldn't set rendertarget/depth buffer\n" );
		return false;
	}
	rc->setViewport( &si.viewport_ );

	rc->camera( si.cam_ );
	rc->view( si.view_ );
	rc->projection( si.projection_ );
	rc->updateViewTransforms();

	rc->screenWidth( si.viewport_.Width );
	rc->screenHeight( si.viewport_.Height );

	return true;
}


/**
 *	This method returns the rectangle that is the intersection of the near-plane
 *	with the view frustum.
 *
 *	@param corner		The position of the bottom-left corner of the rectangle.
 *	@param xAxis		The length of the horizontal edges.
 *	@param yAxis		The length of the vertical edges.
 *
 *	@note	The invView matrix must be correct before this method is called. You
 *			may need to call updateViewTransforms.
 *
 *	@see invView
 *	@see updateViewTransforms
 */
void RenderContext::getNearPlaneRect( Vector3 & corner,
		Vector3 & xAxis, Vector3 & yAxis ) const
{
	BW_GUARD;
	const Matrix & matrix = this->invView();

	// zAxis is the vector from the camera position to the centre of the
	// near plane of the camera.
	Vector3 zAxis = matrix.applyToUnitAxisVector( Z_AXIS );
	zAxis.normalise();

	// xAxis is the vector from the centre of the near plane to its right edge.
	xAxis = matrix.applyToUnitAxisVector( X_AXIS );
	xAxis.normalise();

	// yAxis is the vector from the centre of the near plane to its top edge.
	yAxis = matrix.applyToUnitAxisVector( Y_AXIS );
	yAxis.normalise();

	const float fov = camera_.fov();
	const float nearPlane = camera_.nearPlane();
	const float aspectRatio = camera_.aspectRatio();

	const float yLength = nearPlane * tanf( fov / 2.0f );
	const float xLength = yLength * aspectRatio;

	xAxis *= xLength;
	yAxis *= yLength;
	zAxis *= nearPlane;

	Vector3 nearPlaneCentre( matrix.applyToOrigin() + zAxis );
	corner = nearPlaneCentre - xAxis - yAxis;
	xAxis *= 2.f;
	yAxis *= 2.f;
}

/**
 *	Determines whether a hardware mouse cursor is available.
 *
 *	@return	Whether or not a hardware cursor is available.
 */
bool RenderContext::hardwareCursor() const
{
	return this->windowed() || devices_[ deviceIndex_ ].caps_.CursorCaps != 0;
}


/**
 *	Restores the windows cursor state, hidding or showing it depending on the 
 *	value passed.
 *
 *	@param	state	True will show the windows cursor. False will hide it.
 */
void RenderContext::restoreCursor( bool state )
{
	BW_GUARD;
	if (state)
	{
		while (!(::ShowCursor(true) >= 0));
		if (s_OriginalMouseCursor)
			SetCursor( s_OriginalMouseCursor );
	}
	else
	{
		while (!(::ShowCursor(false) < 0));
		if (device_.hasComObject())
			SetCursor( NULL );
	}
}


/**
 *	Frees DX resources temporarily.
 *	
 *	@see resume
 *	@see paused
 */
void RenderContext::pause()
{
	BW_GUARD;
	if ( Moo::rc().device()->TestCooperativeLevel() == D3D_OK )
	{
		if ( !paused_ )
			releaseUnmanaged();
		device_->EvictManagedResources();
	}
	paused_ = true;
}


/**
 *	Reallocates DX resources freed in pause.
 *	
 *	@see pause
 *	@see paused
 */
void RenderContext::resume()
{
	BW_GUARD;
	if ( !paused_ )
		return;

    bool reset = false;
	if ( checkDevice(&reset) )
	{
        if (!reset)
			createUnmanaged();
		paused_ = false;
	}
}


/**
 *	Checks if the device is paused.
 *	
 *	@return true if the device is paused, false otherwise
 *	
 *	@see pause
 *	@see resume
 */
bool RenderContext::paused() const
{
	return paused_;
}

/**
 *
 */

static std::vector< std::pair< D3DRENDERSTATETYPE, uint32 > > s_renderStateStackDX;
static std::vector< std::pair< D3DRENDERSTATETYPE, uint32 > > s_renderStateStackMain;

static void playPushRenderState()
{
	D3DRENDERSTATETYPE state = gdx_commandBuffer->read<D3DRENDERSTATETYPE>();

	s_renderStateStackDX.push_back( std::make_pair( state, gdx_renderStateCache[state] ) );
}

void RenderContext::pushRenderState( D3DRENDERSTATETYPE state )
{
	if ( usingWrapper_ )
	{
		gdx_commandBuffer->write( playPushRenderState, state );
	}

	s_renderStateStackMain.push_back( std::make_pair( state, rsCache_[state].currentValue ) );
}

/**
 *
 */

static void playPopRenderState()
{
	DX::DeviceWrapper* deviceWrapper = (DX::DeviceWrapper*)gdx_device;
	IDirect3DDevice9* device = deviceWrapper->device_;

	D3DRENDERSTATETYPE state = s_renderStateStackDX.back().first;
	uint32 value = s_renderStateStackDX.back().second;
	s_renderStateStackDX.pop_back();

	if ( value != gdx_renderStateCache[state] )
	{
		gdx_renderStateCache[state] = value;
		device->SetRenderState(state,value);
	}
}

void RenderContext::popRenderState()
{
	if ( usingWrapper_ )
	{
		rsCache_[s_renderStateStackMain.back().first].Id = 0;
		gdx_commandBuffer->write( playPopRenderState );
	}
	else
	{
		setRenderState( s_renderStateStackMain.back().first, s_renderStateStackMain.back().second );
	}

	s_renderStateStackMain.pop_back();
}

/**
 *	Wrapper for D3D BeginScene call, this reference counts the Begin/End
 *	Scene pairs, so that we can nest calls to begin and end scene.
 *	@return D3D_OK if successful d3d error code otherwise
 */
HRESULT RenderContext::beginScene()
{
	BW_GUARD;
	HRESULT hr = D3D_OK;
	if (beginSceneCount_ == 0)
	{

		#ifdef EDITOR_ENABLED
			// The editors can spend 10ms preloading resources.
			preloadDeviceResources( 10 );
		#else
			// Spend 2 milliseconds preloading device resources
			preloadDeviceResources( 2 );
		#endif

		hr = device_->BeginScene();
	}

	++beginSceneCount_;
	return hr;
}

/**
 *	Wrapper for D3D endScene call, this reference counts the Begin/End
 *	Scene pairs, so that we can nest calls to begin and end scene.
 *	@return D3D_OK if successful D3DERR_INVALIDCALL otherwise
 */
HRESULT	RenderContext::endScene()
{
	BW_GUARD;
	HRESULT hr = D3D_OK;

	--beginSceneCount_;
	if (beginSceneCount_ == 0)
	{
		hr = device_->EndScene();
		lastFrameProfilingData_ = liveProfilingData_;
		ZeroMemory(&liveProfilingData_, sizeof(liveProfilingData_));
	}
	else if (beginSceneCount_ < 0)
	{
		hr = D3DERR_INVALIDCALL;
		beginSceneCount_ = 0;
	}
	return hr;
}

/**
 *	Clears the currently bound device resources. 
 *	It is D3D recommended behaviour to unbind all resource states after every frame.
 *
 */
void RenderContext::clearBindings()
{
	BW_GUARD;
	// unbind all the texture/vert/ind
	setIndices( NULL );
	for( uint32 i = 0; ( i < devices_[ deviceIndex_ ].caps_.MaxStreams ) || ( i < 1 ); i++ )
	{
		device_->SetStreamSource( i, NULL, 0, 0 );
	}

	// release all textures
	for( uint32 i = 0; i < devices_[ deviceIndex_ ].caps_.MaxSimultaneousTextures; i++ )
	{
		setTexture( i, NULL );
	}

	//release shaders
	setPixelShader( NULL );
	setVertexShader( NULL );

	//consider invalidating the render states as well?
}

/**
 *	Wrapper for D3D Present call.
 *
 *	@return D3D_OK if successful D3DERR_INVALIDCALL otherwise
 */

HRESULT RenderContext::present()
{
	BW_GUARD;

	if (beginSceneCount_ != 0)
		return D3DERR_INVALIDCALL;

	HRESULT hr = NULL;
	{
		PROFILER_SCOPED( GPU_Wait );

		clearBindings();

		hr = device_->Present( NULL, NULL, NULL, NULL );
	}
	return hr;
}


void RenderContext::releaseQueryObjects(void)
{
	BW_GUARD;
	int nQueries = queryList_.size();
	for (int i = 0; i < nQueries; i++)
	{
		OcclusionQuery* query = queryList_[i];
		if (query && query->queryObject)
		{
			IDirect3DQuery9* queryObject = query->queryObject;			
			queryObject->Release();
			query->queryObject = NULL;
		}
	}
}

/*-------------------------------------------------------------------*//*!
 * \brief	
 *
 *//*-------------------------------------------------------------------*/

void RenderContext::createQueryObjects(void)
{
	BW_GUARD;
	int nQueries = queryList_.size();
	for (int i = 0; i < nQueries; i++)
	{
		OcclusionQuery* query = queryList_[i];

		IDirect3DQuery9* queryObject = NULL;
		device_->CreateQuery( D3DQUERYTYPE_OCCLUSION, &queryObject );
		query->queryObject = queryObject;
	}
}

/**
 * 
 */
OcclusionQuery* RenderContext::createOcclusionQuery()
{
	BW_GUARD;
	IDirect3DQuery9* pOcclusionQuery = NULL;
	HRESULT res = device_->CreateQuery( D3DQUERYTYPE_OCCLUSION, &pOcclusionQuery );

	if (res != S_OK) // creation failed
		return NULL;

	OcclusionQuery* q = new OcclusionQuery();
	q->queryObject = pOcclusionQuery;
	q->index = queryList_.size();
	queryList_.push_back( q );
	return q;
}

/**
 * 
 */
void RenderContext::destroyOcclusionQuery(OcclusionQuery* query)
{
	BW_GUARD;
	// swap query with the last one in the list and delete it
	OcclusionQuery* last = queryList_.back();
	if (query)
	{
		queryList_[query->index] = last;
		last->index = query->index;
		queryList_.pop_back();
		if (query->queryObject)
			query->queryObject->Release();
		delete query;
	}
}

/**
 * 
 */
void RenderContext::beginQuery(OcclusionQuery* query)
{
	BW_GUARD;
	query->queryObject->Issue( D3DISSUE_BEGIN );
}

/**
 * 
 */
void RenderContext::endQuery(OcclusionQuery* query)
{
	BW_GUARD;
	query->queryObject->Issue( D3DISSUE_END );
}

/**
 * @return true is query result was available, false otherwise
 */
bool RenderContext::getQueryResult(int& visiblePixels, OcclusionQuery* query, bool wait)
{
	BW_GUARD;
	IDirect3DQuery9* pOcclusionQuery = query->queryObject;

	if (wait)
	{
		HRESULT hres = S_FALSE;
		while (S_FALSE == 
			(hres = pOcclusionQuery->GetData( &visiblePixels, sizeof(DWORD), D3DGETDATA_FLUSH )))
		{
		}
		return S_OK == hres;
	}
	else
	{
		return S_OK == pOcclusionQuery->GetData( &visiblePixels, sizeof(DWORD), D3DGETDATA_FLUSH );
	}
}
/**
 * This method wraps the DX CreateTexture call.
 */
ComObjectWrap<DX::Texture>	RenderContext::createTexture( UINT Width, UINT Height, UINT Levels, DWORD Usage,
	D3DFORMAT Format, D3DPOOL Pool, const char* allocator )
{
	BW_GUARD;
	ComObjectWrap<DX::Texture> tex;
	HRESULT hr = device_->CreateTexture( Width, Height, Levels, Usage, Format, Pool, &tex, NULL );
	if( SUCCEEDED( hr ) )
	{
#if ENABLE_RESOURCE_COUNTERS
		tex.addAlloc( allocator );
#endif
		return tex;
	}
	else
	{
		if ( hr == E_OUTOFMEMORY )
			memoryCritical_ = true;

		WARNING_MSG( "RenderContext::createTexture - could not create texture map "
		"size %dx%dx%d.\n ( error %x:%s )\n", 
		Width, Height, Levels,
		hr, DX::errorAsString( hr ).c_str() );
	}
	return ComObjectWrap<DX::Texture>();
}

/**
* This method wraps the D3DXCreateTextureFromFileInMemoryEx call.
*/
ComObjectWrap<DX::Texture>	RenderContext::createTextureFromFileInMemoryEx(
	LPCVOID pSrcData, UINT SrcDataSize, UINT Width, UINT Height,
	UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
	DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO* pSrcInfo,
	PALETTEENTRY* pPalette, const char* allocator )
{
	BW_GUARD;
	SimpleMutexHolder smh( d3dxCreateMutex_ );

	ComObjectWrap<DX::Texture> tex;
	HRESULT hr = D3DXCreateTextureFromFileInMemoryEx( device_.pComObject(), pSrcData, SrcDataSize, Width, Height,
		MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, &tex );
	if( SUCCEEDED( hr ) )
	{
#if ENABLE_RESOURCE_COUNTERS
		tex.addAlloc( allocator );
#endif
		return tex;
	}
	else
	{
		if ( hr == E_OUTOFMEMORY )
			memoryCritical_ = true;

		WARNING_MSG( "RenderContext::createTextureFromFileInMemoryEx - could not create texture map ( error %x:%s )\n", hr, DX::errorAsString( hr ).c_str() );
	}
	return ComObjectWrap<DX::Texture>();
}

/**
* This method wraps the D3DXCreateTextureFromFileEx call.
*/
ComObjectWrap<DX::Texture>	RenderContext::createTextureFromFileEx( LPCTSTR pSrcFile, UINT Width, UINT Height, UINT MipLevels,
	DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter,
	D3DCOLOR ColorKey, D3DXIMAGE_INFO * pSrcInfo, PALETTEENTRY * pPalette, const char* allocator )
{
	BW_GUARD;
	SimpleMutexHolder smh( d3dxCreateMutex_ );

	ComObjectWrap<DX::Texture> tex;
	HRESULT hr = D3DXCreateTextureFromFileEx( device_.pComObject(), pSrcFile, Width, Height, MipLevels,
		Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, &tex );
	if( SUCCEEDED( hr ) )
	{
#if ENABLE_RESOURCE_COUNTERS
		tex.addAlloc( allocator );
#endif
		return tex;
	}
	else
	{
		if ( hr == E_OUTOFMEMORY )
			memoryCritical_ = true;

		WARNING_MSG( "RenderContext::createTextureFromFileEx - could not create texture map ( error %x:%s )\n", hr, DX::errorAsString( hr ).c_str() );
	}
	return ComObjectWrap<DX::Texture>();
}

/**
* This method wraps the D3DXCreateCubeTextureFromFileInMemoryEx call.
*/
ComObjectWrap<DX::CubeTexture>	RenderContext::createCubeTextureFromFileInMemoryEx(
	LPCVOID pSrcData, UINT SrcDataSize, UINT Size, UINT MipLevels, DWORD Usage,
	D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey,
	D3DXIMAGE_INFO* pSrcInfo, PALETTEENTRY* pPalette, const char* allocator )
{
	BW_GUARD;
	SimpleMutexHolder smh( d3dxCreateMutex_ );

	ComObjectWrap<DX::CubeTexture> tex;
	HRESULT hr = D3DXCreateCubeTextureFromFileInMemoryEx( device_.pComObject(), pSrcData, SrcDataSize, Size,
		MipLevels, Usage, Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, &tex );
	if( SUCCEEDED( hr ) )
	{
#if ENABLE_RESOURCE_COUNTERS
		tex.addAlloc( allocator );
#endif
		return tex;
	}
	else
	{
		if ( hr == E_OUTOFMEMORY )
			memoryCritical_ = true;

		WARNING_MSG( "RenderContext::createCubeTextureFromFileInMemoryEx - could not create texture map ( error %x:%s )\n", hr, DX::errorAsString( hr ).c_str() );
	}
	return ComObjectWrap<DX::CubeTexture>();
}

RenderContext& rc()
{
	MF_ASSERT_DEBUG( g_RC && g_RC->isValid() && "RenderContext not initialised." );
	return *g_RC;
}

/**
 * This method preloads resources from the preloadlist.
 * It spends up to timeLimit time doing this
 * @param timeLimitMs the max time in milliseconds we want to spend
 * on preloading
 */
void RenderContext::preloadDeviceResources( uint32 timeLimitMs )
{
	BW_GUARD;
	if (preloadResourceList_.size() && 
		preloadResourceMutex_.grabTry())
	{
		PROFILER_SCOPED( PreloadResources );

		// Calculate the timelimit in stamps
		uint64 timeLimit = (stampsPerSecond() * timeLimitMs) / uint64(1000);
		uint64 beginStamp = timestamp();

		// We limit the preloadlist to MAX_PRELOAD_COUNT entries so that
		// we don't hold on to too many potentially deleted resources
		// We delete the entries off the front of the list as they are more
		// likely to have been deleted or already preloaded
		while (preloadResourceList_.size() > MAX_PRELOAD_COUNT)
		{
			preloadResourceList_.front()->Release();
			preloadResourceList_.pop_front();
		}

		// Iterate over the list and preload each entry, once the time limit
		// has been reached or the list is empty, continue, leave any resource
		// that has not been preloaded in the list.
		while ( (timestamp() - beginStamp) < timeLimit && 
			preloadResourceList_.size())
		{
			IDirect3DResource9* pResource = preloadResourceList_.front();
			preloadResourceList_.pop_front();

			// Only preload the resource if the current reference count is
			// above 1 (if the reference count is 1, the preload list is the
			// only remaining reference to the resource and we can release it)
			// Since AddRef and Release are the only ways of getting
			// the reference count of an IUnknown object we use these methods.
			pResource->AddRef();
			ULONG refCount = pResource->Release();
			if (refCount > 1)
			{
				pResource->PreLoad();
			}
			pResource->Release();
		}

		preloadResourceMutex_.give();
		s_numPreloadResources = preloadResourceList_.size();
	}
}

/**
 *	This method adds a resource to the preload list
 *	@param preloadResource the d3d resource to add to the list
 */
void RenderContext::addPreloadResource(IDirect3DResource9* preloadResource)
{
	BW_GUARD;
	SimpleMutexHolder smh( preloadResourceMutex_ );
	preloadResource->AddRef();
	preloadResourceList_.push_back( preloadResource );
}


}
