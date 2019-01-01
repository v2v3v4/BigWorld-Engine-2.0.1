/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SCALEFORM_IME_HPP
#define SCALEFORM_IME_HPP

#include "config.hpp"

#if SCALEFORM_IME

namespace Scaleform
{
/**
 *	This class contains some utility methods enabling the use of Scaleform's
 *	in-game IME support.
 */
class IME
{
public:
	static bool init( const std::string& imeMovie );
	static bool handleIMMMessage( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	static bool handleIMEMessage( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	static void setFocussedMovie( class PyMovieView* m )
	{
		s_pFocussed = m;
	}
	static PyMovieView* pFocussedMovie()
	{
		return s_pFocussed;
	}
	static void onDeleteMovieView( class PyMovieView* m )
	{
		if ( s_pFocussed == m )
			s_pFocussed = NULL;
	}
	static class PyMovieView * s_pFocussed;
};

}	// namespace scaleform

#endif

#endif