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

#include "util.hpp"
#include "moo/render_context.hpp"
#include "ashes/simple_gui.hpp"


namespace Scaleform
{
	/**
	 *	This fuction converts from a python object to a GFxValue.
	 *
	 *	@param	PyObject*	incoming python object.
	 *	@param	GFxValue&	translated GFxValue representation.
	 */
	void objectToValue(PyObject* obj, GFxValue& value)
	{
		BW_GUARD;
		if ( !obj )
		{
			value.SetNull();
		}
		else if (PyUnicode_CheckExact(obj))
		{
			PyObject* utf8 = PyUnicode_AsUTF8String(obj);
			value.SetString(PyString_AsString(utf8));
		}
		else if (PyString_CheckExact(obj))
		{
			value.SetString(PyString_AsString(obj));
		}
		else if (PyInt_CheckExact(obj))
		{
			value.SetNumber(PyInt_AsLong(obj));
		}
		else if (PyFloat_CheckExact(obj))
		{
			value.SetNumber(PyFloat_AsDouble(obj));
		}
		else if (PyBool_Check(obj))
		{
			value.SetBoolean(obj == Py_True);
		}
		else
		{
			value.SetNull();
		}
	}


	/**
	 *	This fuction converts a GFxValue to a python object.
	 *
	 *	@param	GFxValue&	incoming GFxValue representation.
	 *	@param	PyObject*	translated as a python object.
	 */
	void valueToObject(const GFxValue& value, PyObject** obj)
	{
		BW_GUARD;
		switch (value.GetType())
		{
		case GFxValue::VT_Undefined:
		case GFxValue::VT_Null:
			*obj = Py_None;
			Py_INCREF(*obj);
			break;

		case GFxValue::VT_Boolean:
			*obj = PyBool_FromLong(value.GetBool());
			break;

		case GFxValue::VT_Number:
			*obj = PyFloat_FromDouble(value.GetNumber());
			break;

		case GFxValue::VT_String:
			*obj = PyString_FromString(value.GetString());
			break;

		case GFxValue::VT_StringW:
			*obj = PyUnicode_FromWideChar(value.GetStringW(), wcslen(value.GetStringW()) );
			break;
		}
	}


	/** 
	 *	This function initialises a GViewport structure with values
	 *	required to display a movie over the full screen.
	 *
	 *	@param	GViewport&	[out] filled GViewport structure representing the
	 *						entire back buffer.
	 */
	void fullScreenViewport( GViewport& outGv )
	{
		BW_GUARD;
		SInt w = (SInt)Moo::rc().backBufferDesc().Width;
		SInt h = (SInt)Moo::rc().backBufferDesc().Height;
	
		outGv = GViewport( w, h, 0, 0, w, h );
	}


	/** 
	 *	This function initialises a GViewport structure with values
	 *	required to display a movie in the area given by four corners
	 *	of a GUI component.  The corners should be created via the
	 *	SimpleGUIComponent::clibBounds() method.
	 *
	 *	@param	Vector2*	clip-space coordinates of the bounding rectangle.
	 *	@param	GViewport&	[out] filled GViewport structure representing the
	 *						desired view area.
	 */
	void viewportFromClipBounds( const Vector2* corners, GViewport& outGv )
	{
		BW_GUARD;
		//topleft..topRight..bottomLeft..bottomRight
		SInt x = (SInt)((corners[0].x + 1.f) * Moo::rc().halfScreenWidth());
		SInt y = (SInt)((1.f - corners[0].y) * Moo::rc().halfScreenHeight());
		SInt w = (SInt)((corners[3].x + 1.f) * Moo::rc().halfScreenWidth()) - x;
		SInt h = (SInt)((1.f - corners[3].y) * Moo::rc().halfScreenHeight()) - y;

		//work out runtime size of movie and apply
		D3DVIEWPORT9 pViewport;
		Moo::rc().device()->GetViewport( &pViewport );
		outGv = GViewport(
			(SInt)Moo::rc().backBufferDesc().Width,
			(SInt)Moo::rc().backBufferDesc().Height
			,x, y, w, h );
	}


	/** 
	 *	This function initialises a GRectF structure with values
	 *	required to display a movie in the area given by four corners
	 *	of a GUI component.  The corners should be created via the
	 *	SimpleGUIComponent::clibBounds() method.
	 *
	 *	@param	Vector2*	clip-space coordinates of the bounding rectangle.
	 *	@param	GRectF&		[out] filled GRectF structure representing the
	 *						desired view area.
	 */
	void rectFromClipBounds( const Vector2* corners, GRectF& outRect )
	{
		BW_GUARD;
		//topleft..topRight..bottomLeft..bottomRight
		outRect.Left = (corners[0].x + 1.f) * Moo::rc().halfScreenWidth();
		outRect.Top = (1.f - corners[0].y) * Moo::rc().halfScreenHeight();
		outRect.Right = (corners[3].x + 1.f) * Moo::rc().halfScreenWidth();
		outRect.Bottom = (1.f - corners[3].y) * Moo::rc().halfScreenHeight();
	}


	/**
	 *	This method takes 4 corner positions in GUI local space, and outputs
	 *	a clipped viewport/rect with which to draw a scaleform GUI component.
	 *
	 *	It additionally takes into account any clipping by current GUI windows.
	 *
	 *	@param	Vector2*	clip-space coordinates of the bounding rectangle.
	 *	@param	GViewport&	[out] filled GViewport structure representing the
	 *						desired view area.
	 *	@param	GRectF&		[out] filled GRectF structure representing the
	 *						desired view area.
	 */
	void runtimeGUIViewport( const Vector2* lcorners, GViewport& gv, GRectF& rect )
	{
		BW_GUARD;
		Vector2 corners[4];
		for ( size_t i=0; i<4; i++ )
		{
			Vector3 worldCorner =
				Moo::rc().world().applyPoint( Vector3(lcorners[i].x,lcorners[i].y,0.f) );
			corners[i].set( worldCorner.x, worldCorner.y );
		}

		fullScreenViewport( gv );

		//scissors rect gets disabled for some reason, so manually clip the text
		//viewport rectangle against the current gui clip region.
		Vector4 clippedViewport( SimpleGUI::instance().clipRegion() );		//clip region goes from -1,-1 .. 1,1
		clippedViewport.x = clippedViewport.x * Moo::rc().halfScreenWidth() + Moo::rc().halfScreenWidth();
		clippedViewport.y = (-clippedViewport.y) * Moo::rc().halfScreenHeight() + Moo::rc().halfScreenHeight();
		clippedViewport.z = clippedViewport.z * Moo::rc().halfScreenWidth() + Moo::rc().halfScreenWidth();
		clippedViewport.w = (-clippedViewport.w) * Moo::rc().halfScreenHeight() + Moo::rc().halfScreenHeight();

		gv.Left = (SInt)clippedViewport.x;
		gv.Width = (SInt)(clippedViewport.z - clippedViewport.x);
		gv.Top = (SInt)clippedViewport.y;
		gv.Height = (SInt)(clippedViewport.w - clippedViewport.y);

		rectFromClipBounds( corners, rect );

		//scaleform wants movie/text rect to be relative to the viewport rect.
		rect.Left -= (float)gv.Left;
		rect.Top -= (float)gv.Top;
		rect.Right -= (float)gv.Left;
		rect.Bottom -= (float)gv.Top;
	}


	/**
	 *	This function translates a BigWorld key event code to the
	 *	corresponding GFxKey code.
	 *
	 *	If there is no appropriate translation, the returned GFxKey::Code
	 *	will be set to GFxKey::VoidSymbol
	 *
	 *	@param	uint32			BigWorld key event code.
	 *	@return	GFxKey::Code	Translated key event code.
	 */
	GFxKey::Code translateKeyCode(uint32 eventChar)
	{
		BW_GUARD;
		UINT key = (UINT)eventChar;
		GFxKey::Code    c((GFxKey::Code)GFxKey::VoidSymbol);
		
		// many keys don't correlate, so just use a look-up table.
		static struct
		{
			int         vk;
			GFxKey::Code    gs;
		} table[] =
		{
			{ KeyCode::KEY_TAB,			GFxKey::Tab },
			{ KeyCode::KEY_RETURN,			GFxKey::Return },
			{ KeyCode::KEY_ESCAPE,			GFxKey::Escape },
			{ KeyCode::KEY_LEFTARROW,		GFxKey::Left },
			{ KeyCode::KEY_UPARROW,		GFxKey::Up },
			{ KeyCode::KEY_RIGHTARROW,		GFxKey::Right },
			{ KeyCode::KEY_DOWNARROW,		GFxKey::Down },
			{ KeyCode::KEY_HOME,			GFxKey::Home },
			{ KeyCode::KEY_END,			GFxKey::End },
			{ KeyCode::KEY_BACKSPACE,		GFxKey::Backspace },
			{ KeyCode::KEY_PGUP,			GFxKey::PageUp },
			{ KeyCode::KEY_PGDN,			GFxKey::PageDown },
			{ KeyCode::KEY_INSERT,			GFxKey::Insert },
			{ KeyCode::KEY_DELETE,			GFxKey::Delete },
				// @@ TODO fill this out some more
			{ 0, GFxKey::VoidSymbol }
		};

		for (int i = 0; table[i].vk != 0; i++)
		{
			if (key == (UInt)table[i].vk)
			{
				c = table[i].gs;
				break;
			}
		}

		return c;
	}


	/**
	 *	This function translates a BigWorld KeyEvent to a Scaleform Key Event.
	 *
	 *	@param	KeyEvent&		BigWorld KeyEvent.
	 *	@return	GFxKeyEvent		Scaleform KeyEvent.
	 */
	GFxKeyEvent translateKeyEvent(const KeyEvent & bwevent)
	{
		BW_GUARD;
		bool down = bwevent.isKeyDown();
		GFxKey::Code c = translateKeyCode(bwevent.key());
		GFxKeyEvent gfxevent(down ? GFxEvent::KeyDown : GFxKeyEvent::KeyUp, c, 0, 0);
		gfxevent.SpecialKeysState.SetShiftPressed(bwevent.isShiftDown());
		gfxevent.SpecialKeysState.SetCtrlPressed(bwevent.isCtrlDown());
		gfxevent.SpecialKeysState.SetAltPressed(bwevent.isAltDown());
		gfxevent.SpecialKeysState.SetCapsToggled((::GetKeyState(VK_NUMLOCK) & 0x1) ? 1: 0);
		gfxevent.SpecialKeysState.SetCapsToggled((::GetKeyState(VK_CAPITAL) & 0x1) ? 1: 0);
		gfxevent.SpecialKeysState.SetScrollToggled((::GetKeyState(VK_SCROLL) & 0x1) ? 1: 0);

		return gfxevent;
	}


	/**
	 *	This function fills a bit field desribing the current state of the mouse buttons.
	 *	The bitfield is used by various Scaleform mouse handling methods.
	 *
	 *	@param	KeyEvent&		BigWorld KeyEvent containing mouse button information.
	 *	@return int				Packed bitfield in Scaleform format.
	 */
	int mouseButtonsFromEvent(const KeyEvent &event )
	{
		BW_GUARD;
		KeyCode::Key keyEvent( KeyCode::KEY_NONE );

		if( event.isKeyDown() ) 
			keyEvent = event.key();
		else
			keyEvent = KeyCode::KEY_NONE;

		bool bLeftButtonDown = keyEvent == KeyCode::KEY_LEFTMOUSE;
		bool bRightButtonDown = keyEvent == KeyCode::KEY_RIGHTMOUSE;
		bool bMiddleButtonDown = keyEvent == KeyCode::KEY_MIDDLEMOUSE;
		bool bSideButton1Down = keyEvent == KeyCode::KEY_MOUSE3;
		bool bSideButton2Down = keyEvent == KeyCode::KEY_MOUSE4;

		return ( (bMiddleButtonDown << 2) + (bRightButtonDown << 1) + bLeftButtonDown );
	}
}	//namespace Scaleform

#endif // #if SCALEFORM_SUPPORT