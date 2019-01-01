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
#include "ime.hpp"

#if SCALEFORM_IME

#include "manager.hpp"
#include <GFxIMEManager.h>
#include <GFxIMEManagerWin32.h>
#include <GFxFontProviderWin32.h>

namespace Scaleform
{
	int IME_token = 0;
	PyMovieView * IME::s_pFocussed = NULL;

	bool IME::init( const std::string& imeMovie )
	{
		BW_GUARD;
		GFxIMEManager* ime;
		ime = new GFxIMEManagerWin32( Manager::instance().hwnd() );
		ime->Init( Manager::instance().pLogger() );
		ime->SetIMEMoviePath( imeMovie.c_str() );
		Manager::instance().pLoader()->SetIMEManager(ime);
		DEBUG_MSG( "ime manager ref count %d\n", ime->GetRefCount() );
		//ime->Release();
		//lifetime of ime object now controlled by loader
		return true;
	}


	bool IME::handleIMMMessage( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
	{
		BW_GUARD;
		GFxIMEWin32Event ev( GFxIMEEvent::IME_PreProcessKeyboard, (UPInt)hWnd, msg, wParam, lParam);
		if ( s_pFocussed )
		{
			int32 ret = s_pFocussed->pMovieView()->HandleEvent(ev);
			return !!(ret & GFxMovieView::HE_NoDefaultAction);
		}
		return false;
	}


	bool IME::handleIMEMessage( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
	{
		BW_GUARD;
		GFxIMEWin32Event ev(GFxIMEWin32Event::IME_Default, (UPInt)hWnd, msg, wParam, lParam);
		// Do not pass IME events handled by GFx to DefWindowProc.
		if ( s_pFocussed )
		{
			int32 ret = s_pFocussed->pMovieView()->HandleEvent(ev);
			return !!(ret & GFxMovieView::HE_NoDefaultAction);
		}
		return false;
	}

}	//namespace Scaleform


#endif // SCALEFORM_IME
#endif //#if SCALEFORM_SUPPORT