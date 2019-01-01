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
#if SCALEFORM_SUPPORT

#pragma comment(lib, "libgfx.lib")
#pragma comment(lib, "libgrenderer_d3d9.lib")
#pragma comment(lib, "libgfx_expat.lib")
#pragma comment(lib, "libjpeg.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "winmm.lib")

#include "manager.hpp"
#include "py_movie_view.hpp"
#include "util.hpp"
#include "moo/render_context.hpp"
#include <GFxPlayer.h>
#include <GFxEvent.h>
#include <GFxFontLib.h>
#include <GFxFontProviderWin32.h>
#include "ime.hpp"

DECLARE_DEBUG_COMPONENT2( "Scaleform", 0 )

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

BW_INIT_SINGLETON_STORAGE( Scaleform::Manager )

namespace Scaleform
{

	extern int PyMovieDef_token;
	extern int FlashGUIComponent_token;
	extern int FlashTextGUIComponent_token;

	int tokenSet = PyMovieDef_token | FlashGUIComponent_token | FlashTextGUIComponent_token;

#if SCALEFORM_IME
	extern int IME_token;
	int imeTokenSet = IME_token;
#endif


//-----------------------------------------------------------------------------
// Section - Manager
//-----------------------------------------------------------------------------
	Manager::Manager():
		pRenderer_( NULL ),
		pRenderConfig_( NULL ),
		pLoader_( NULL ),
		pLogger_( NULL )
	{
		BW_GUARD;
	}


	Manager::~Manager()
	{
		BW_GUARD;
	}


	bool Manager::doInit()
	{
		BW_GUARD;
		INFO_MSG( "Scaleform::Manager initialised\n" );
		pLoader_ = new Loader();

		// Setup logging
		pLogger_ = *new Log();
		//pLoader_->SetLog( pLogger_ );
		// This would give us A LOT more debugging information - (while GFX/SWF parsing)
		//GPtr<GFxParseControl> pparseControl = *new GFxParseControl();
		//pparseControl->SetParseFlags(GFxParseControl::VerboseParse);
		//impl.SetParseControl(pparseControl);

		/*
		impl.SetUserEventHandler(GPtr<GFxUserEventHandler>(*new pygfxUserEventHandler()));
		*/

		// Setup default font provider.  In practice we should be loading these
		// from .swf font libraries, ie. using the font lib below
		GPtr<GFxFontProviderWin32> fontProvider = *new GFxFontProviderWin32(::GetDC(0));
		pLoader_->SetFontProvider(fontProvider);

		pFontMap_ = *new GFxFontMap();
		pLoader_->SetFontMap(pFontMap_);

		pFontLib_ = *new GFxFontLib();
		pLoader_->SetFontLib(pFontLib_);

		return true;
	}


	bool Manager::doFini()
	{
	   BW_GUARD;
	   pFontMap_ = NULL;
	   pFontLib_ = NULL;
	   pLoader_->SetRenderConfig((GFxRenderConfig*)NULL);
	   pDrawTextManager_->SetRenderConfig((GFxRenderConfig*)NULL);
	   pRenderConfig_ = NULL;
	   pDrawTextManager_ = NULL;
	   delete pLoader_;
	   pRenderer_ = NULL;
	   pLoader_ = NULL;
	   pLogger_ = NULL;
	   INFO_MSG( "Scaleform::Manager finalised\n" );
	   return true;
	} 


	struct AdvanceFn
	{
		Float time_;
		UINT catchup_;

		AdvanceFn(Float time, UINT catchup): time_(time), catchup_(catchup) {}

		void operator () ( PyMovieView* elem )
		{
			BW_GUARD;
			GFxMovieView * mv = elem->pMovieView();
			if ( mv && mv->GetVisible() )
			{
				mv->Advance(time_, catchup_);
			}
		}
	};


	void Manager::tick(float elapsedTime, uint32 frameCatchUp)
	{
		AdvanceFn afn( elapsedTime, (UINT)frameCatchUp );
		std::for_each(list_.begin(), list_.end(), afn);
	}	


	struct FullScreenDisplayFn
	{
		FullScreenDisplayFn()
		{
			Scaleform::fullScreenViewport( gv_ );
		}


		void operator () ( PyMovieView* elem )
		{
			BW_GUARD;
			elem->pMovieView()->SetViewport( gv_ );
			elem->pMovieView()->Display();
		}

		GViewport gv_;
	};


	void Manager::draw()
	{
		BW_GUARD;
		if (!pRenderer_)
			return;

		if (pRenderer_->CheckDisplayStatus() != GRendererD3D9::DisplayStatus_Ok)
		{
			WARNING_MSG( "Scaleform renderer - display status not ok\n" );
			return;
		}

		FullScreenDisplayFn fn;
		std::for_each(list_.begin(), list_.end(), fn );

		//TODO - take this out when we have re-enabled the D3D wrapper,
		//when we use the wrapper, scaleform will be calling via our
		//own setRenderState methods, instead of going straight to the
		//device and causing issues with later draw calls.
		Moo::rc().initRenderStates();
	}


	void Manager::deleteUnmanagedObjects()
	{
		TRACE_MSG( "Scaleform::Manager::deleteUnmanagedObjects\n" );
		BW_GUARD;
		if (!pRenderer_)
			return;

		pRenderer_->ResetVideoMode();
	}


	void Manager::deleteManagedObjects()
	{
		TRACE_MSG( "Scaleform::Manager::deleteManagedObjects\n" );
		BW_GUARD;
		if (!pRenderer_)
			return;

		SAFE_RELEASE(pRenderer_);
	}


	void Manager::createUnmanagedObjects()
	{
		TRACE_MSG( "Scaleform::Manager::createUnmanagedObjects\n" );
		BW_GUARD;
		if (!pRenderer_)
			return;

		D3DPRESENT_PARAMETERS presentParams = Moo::rc().presentParameters();

		// Configure renderer in "Dependent mode", honoring externally configured settings.
		if (pRenderer_->CheckDisplayStatus() == GRendererD3D9::DisplayStatus_NoModeSet)
			pRenderer_->SetDependentVideoMode(Moo::rc().device(), &presentParams, configFlag(), hwndModule_ );
	}


	void Manager::createManagedObjects()
	{
		TRACE_MSG( "Scaleform::Manager::createManagedObjects\n" );
		BW_GUARD;
		D3DPRESENT_PARAMETERS presentParams = Moo::rc().presentParameters();
		this->createRenderer(Moo::rc().device(), &presentParams, false, hwndModule_ );
		//pygfx::Module::GetInstance()->SetLogHandler( PyGfxLogHandler );  <<<< Needed here ? 
	}


	void Manager::createRenderer(DX::Device* device,
		D3DPRESENT_PARAMETERS* presentParams,
		bool noSceneCalls,
		HWND hwnd)
	{
		BW_GUARD;
		if (pRenderer_)
			return;

		// Create the renderer.
		pRenderer_ = *GRendererD3D9::CreateRenderer();

		configFlag_ = noSceneCalls ? GRendererD3D9::VMConfig_NoSceneCalls : 
			GRendererD3D9::VMConfigFlags(0);
		hwndModule_ = hwnd;

		// Configure renderer in "Dependent mode", honoring externally configured settings.
		bool success = pRenderer_->SetDependentVideoMode(device, presentParams, configFlag_, hwnd);
		//TODO - check return value

		pRenderConfig_  = *new GFxRenderConfig(pRenderer_);
		pRenderConfig_->SetRenderFlags( GFxRenderConfig::RF_EdgeAA | GFxRenderConfig::RF_StrokeNormal );
		pLoader_->SetRenderConfig(pRenderConfig_);

		pRenderStats_ = *new GFxRenderStats();
		pLoader_->SetRenderStats(pRenderStats_);

		//Set the loader as our provider of fonts.  When fonts are added to the loader,
		//we inherit these as well.
		pDrawTextManager_ = *new GFxDrawTextManager( pLoader_ );
		pDrawTextManager_->SetRenderConfig( pRenderConfig_ );
	}


	void Manager::setDrawTextManager( GPtr<GFxDrawTextManager> dtm )
	{
		BW_GUARD;
		pDrawTextManager_ = dtm;
		pDrawTextManager_->SetRenderConfig( pRenderConfig_ );
	}


	/*~ module Scaleform
	 *	@components{ client }
	 *
	 *	The Scaleform module provides access to the Scaleform integration, if
	 *	it is enabled.
	 */
	/*~ function Scaleform.mapFont
	 *	This function provides a means by which an alias can be set up for
	 *	font names.  Any fonts mapped to names via this function will be
	 *	used by movies by being loaded.
	 *
	 *	@param	String		font name embedded in movies.
	 *	@param	String		actual font to use at runtime.
	 */
	void mapFont( const std::string fontA, const std::string& fontB )
	{
		BW_GUARD;
		Scaleform::Manager::instance().pFontMap_->MapFont( fontA.c_str(), fontB.c_str() );
	}
	PY_AUTO_MODULE_FUNCTION( RETVOID, mapFont, ARG( std::string, ARG( std::string, END )), _Scaleform )


	//-------------------------------------------------------------------------
	// section - Global movie views
	//-------------------------------------------------------------------------
	void Manager::addMovieView(PyMovieView* obj)
	{
		BW_GUARD;
		//we don't increment the refcount, we are storing
		//raw pointers to these objects, so that the holder
		//of the python movie view object controls its lifetime.
		//removeMovieView is called when the movie view is destructed.
		list_.push_back( obj );

		// This would give us A LOT more debugging information - (for ActionScript)
		//GPtr<GFxActionControl> pactionControl = *new GFxActionControl();
		//pactionControl->SetVerboseAction( true );
		//pactionControl->SetActionErrorSuppress( false );
		//obj->impl->SetActionControl(pactionControl);
	}


	void Manager::removeMovieView(PyMovieView* obj)
	{
		BW_GUARD;
		std::list<PyMovieView*>::iterator itr = std::find(list_.begin(), list_.end(), obj);
		if (itr != list_.end())
		{
			list_.erase(itr);
		}
	}


	//-------------------------------------------------------------------------
	// section - Key Handling
	//-------------------------------------------------------------------------
	HandleEventFn::HandleEventFn(const GFxEvent& gfxevent):
			gfxevent_(gfxevent),
			handled_(false)
	{
		BW_GUARD;
	}


	void HandleEventFn::operator () ( PyMovieView* elem )
	{
		BW_GUARD;
		GFxMovieView* mv = elem->pMovieView();
		if ( !handled_ && mv && mv->GetVisible() )
		{
			handled_ = ( mv->HandleEvent(gfxevent_) == GFxMovieView::HE_Completed );
		}
	}


	bool Manager::onKeyEvent(const KeyEvent & event)
	{
		BW_GUARD;
		bool handled = false;

		//BigWorld key events also contain mouse key events.  Scaleform requires
		//key and mouse events to be separate.
		if (event.key() >= KeyCode::KEY_MINIMUM_MOUSE && event.key() <= KeyCode::KEY_MAXIMUM_MOUSE)
		{
			POINT mousePos;
			::GetCursorPos( &mousePos );
			::ScreenToClient( hwndModule_, &mousePos );
			//TODO - remove below.  build mouse key state dynamically as needed.
			lastMouseKeyEvent_ = event;
			int buttons = mouseButtonsFromEvent( event );
			handled = this->onMouse(  buttons, 0, mousePos.x, mousePos.y );
		}
		else
		{
			// keydown events are piggybacked with the char event.
			// keyup events come without a char.
			GFxKeyEvent gfxevent = translateKeyEvent( event );
			if (gfxevent.KeyCode != GFxKey::VoidSymbol)
			{
				HandleEventFn fn(gfxevent);
				//note - std::for_each copies the fn object passed in, and
				//uses the copy.  thankfully it returns the copy that it used.
				handled = std::for_each(list_.begin(), list_.end(), fn).handled();
			}

			if ( event.utf16Char()[0] != 0 )
			{
				GFxCharEvent gfxevent( * (const UInt32*)(event.utf16Char()) );
				HandleEventFn fn(gfxevent);
				handled |= std::for_each(list_.begin(), list_.end(), fn).handled();
			}
		}

		return handled;
	}


	//-------------------------------------------------------------------------
	// section - Mouse handling
	//-------------------------------------------------------------------------
	NotifyMouseStateFn::NotifyMouseStateFn(float x_, float y_, int buttons_, Float scrollDelta_)
			: x(x_), y(y_), buttons(buttons_), scrollDelta(scrollDelta_), handled_( false )
	{
	}


	void NotifyMouseStateFn::operator () ( PyMovieView* elem )
	{
		BW_GUARD;
		if ( handled_ )
			return;

		GFxMovieView* movie = elem->pMovieView();

		if( !movie->GetVisible() )
			return;

		//TODO - can maybe hittest against the scaleform movie before continuing
		GViewport view;
		movie->GetViewport(&view);

		x -= view.Left;
		y -= view.Top;

		// Adjust x, y to viewport.
		GSizeF  s;
		s.Width     = ( movie->GetMovieDef()->GetFrameRect().Width() / view.Width );
		s.Height    = ( movie->GetMovieDef()->GetFrameRect().Height() / view.Height );
		Float	mX = ( x * s.Width );
		Float	mY = ( y * s.Height );
		GRenderer::Matrix m;
		// apply viewport transforms here...
		GRenderer::Point p = m.TransformByInverse( GRenderer::Point( mX, mY ) );
		float vx = p.x / s.Width;
		float vy = p.y / s.Height;

		if (scrollDelta == 0.0f)
		{
			// Fire event
			movie->NotifyMouseState(vx, vy, buttons);
		}
		else
		{
			// TODO - should be vx, vy?
			GFxMouseEvent gfxevent(GFxEvent::MouseWheel, buttons, x, y, scrollDelta);
			handled_ |= ( movie->HandleEvent(gfxevent) == GFxMovieView::HE_Completed );
		}
	}


	bool Manager::onMouse(int buttons, int nMouseWheelDelta, int xPos, int yPos)
	{
		BW_GUARD;
		bool handled = false;
		NotifyMouseStateFn msFn = NotifyMouseStateFn( (float)xPos, (float)yPos, buttons, (Float)((nMouseWheelDelta / WHEEL_DELTA) * 3) );
		handled = std::for_each(list_.begin(), list_.end(), msFn).handled();
		return handled;
	}


	/*
	//TODO - what is this bit of code not yet integrated?
	void serEventHandler::HandleEvent(GFxMovieView* pmovie, const GFxEvent& event)
	{
		switch(event.Type)
		{
		case GFxEvent::DoShowMouse:
			pApp->ShowCursor(true);
			break;
		case GFxEvent::DoHideMouse:
			pApp->ShowCursor(false);
			break;
		case GFxEvent::DoSetMouseCursor:
			{
				const GFxMouseCursorEvent& mcEvent = static_cast<const GFxMouseCursorEvent&>(event);
				switch(mcEvent.CursorShape)
				{
				case GFxMouseCursorEvent::ARROW:
					pApp->SetCursor(::LoadCursor(NULL, IDC_ARROW));
					break;
				case GFxMouseCursorEvent::HAND:
					pApp->SetCursor(::LoadCursor(NULL, IDC_HAND));
					break;
				case GFxMouseCursorEvent::IBEAM:
					pApp->SetCursor(::LoadCursor(NULL, IDC_IBEAM));
					break;
				}
			}
			break;
		}
	}*/

} // namespace Scaleform
#endif //#if SCALEFORM_SUPPORT