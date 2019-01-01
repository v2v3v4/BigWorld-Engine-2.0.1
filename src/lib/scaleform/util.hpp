/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SCALEFORM_UTIL_HPP
#define SCALEFORM_UTIL_HPP
#if SCALEFORM_SUPPORT

#include <GRenderer.h>
#include <GFxEvent.h>
#include <GFxPlayer.h>
#include "input/input.hpp"
#include "math/vector2.hpp"
#include "pyscript/script.hpp"
#include "pyscript/pyobject_plus.hpp"

namespace Scaleform
{
	//-------------------------------------------------------------------------
	// data translation utilities
	//-------------------------------------------------------------------------
	void objectToValue(PyObject* obj, GFxValue& value);
	void valueToObject(const GFxValue& value, PyObject** obj);

	//-------------------------------------------------------------------------
	// input handling utilities
	//-------------------------------------------------------------------------
	struct NotifyMouseStateFn
	{
		float x;
		float y;
		int buttons;
		Float scrollDelta;
		bool handled_;

		NotifyMouseStateFn(float x_, float y_, int buttons_, Float scrollDelta_);
		void operator () ( class PyMovieView* elem );
		bool handled() const { return handled_; }
	};

	struct HandleEventFn
	{
		HandleEventFn(const GFxEvent& gfxevent);
		void operator () ( class PyMovieView* elem );
		bool handled() const { return handled_; }
		const GFxEvent& gfxevent_;
		bool handled_;
	};

	int mouseButtonsFromEvent(const KeyEvent& event );
	GFxKey::Code translateKeyCode(uint32 eventChar);
	GFxKeyEvent translateKeyEvent(const KeyEvent & bwevent);

	//-------------------------------------------------------------------------
	// viewport utilities
	//-------------------------------------------------------------------------
	void fullScreenViewport( GViewport& outGv );
	void viewportFromClipBounds( const Vector2* corners, GViewport& outGv );
	void rectFromClipBounds( const Vector2* corners, GRectF& outRect );
	void runtimeGUIViewport( const Vector2* corners, GViewport& gv, GRectF& rect );

}	//namespace scaleform

#endif //#if SCALEFORM_SUPPORT
#endif //#ifndef SCALEFORM_UTIL_HPP