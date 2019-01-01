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
#include "flash_gui_component.hpp"
#include "manager.hpp"
#include "util.hpp"
#include "ashes/simple_gui.hpp"
#include "cstdmf/debug.hpp"
#include "moo/render_context.hpp"
#include "moo/viewport_setter.hpp"
#include "moo/scissors_setter.hpp"

DECLARE_DEBUG_COMPONENT2( "2DComponents", 0 )

namespace Scaleform
{

PY_TYPEOBJECT( FlashGUIComponent )

PY_BEGIN_METHODS( FlashGUIComponent )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( FlashGUIComponent )
	/*~ attribute FlashGUIComponent.movie
	 *
	 *	The movie associated with this gui component.
	 *
	 *	@type	PyMovieView
	 */
	PY_ATTRIBUTE( movie )
PY_END_ATTRIBUTES()

/*~ function GUI.Flash
 *
 *	This function creates a new FlashGUIComponent which allows a flash movie to be
 *	inserted into the GUI hierarchy. If this component has an attached script, then
 *	the script must take responsibility for passing mouse and key events into the
 *	PyMovieView object.
 *
 *	@param	movie	A PyMovieView object containing the movie to associate with
 *					the component.
 *
 *	@return			the newly created component.
 *
 */
PY_FACTORY_NAMED( FlashGUIComponent, "Flash", GUI )


FlashGUIComponent::FlashGUIComponent( PyMovieView* pMovie, PyTypePlus * pType )
:SimpleGUIComponent( "", pType ),
 pMovieView_( pMovie )

{
	BW_GUARD;
	pMovieView_->showAsGlobalMovie( false );

	//Now the gui component controls the drawing/ticking of the movie.
	pMovieView_->visible( true );
}


FlashGUIComponent::~FlashGUIComponent()
{
	BW_GUARD;	
}


/// Get an attribute for python
PyObject * FlashGUIComponent::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();
	return SimpleGUIComponent::pyGetAttribute( attr );
}


/// Set an attribute for python
int FlashGUIComponent::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();
	return SimpleGUIComponent::pySetAttribute( attr, value );
}


/**
 *	Static python factory method
 */
PyObject * FlashGUIComponent::pyNew( PyObject * args )
{
	BW_GUARD;
	PyObject * obj = NULL;
	if (!PyArg_ParseTuple( args, "O", &obj ))
	{
		PyErr_SetString( PyExc_TypeError, "GUI.Flash: "
			"Argument parsing error: Expected a PyMovieView object" );
		return NULL;
	}

	if (!PyMovieView::Check(obj))
	{
		PyErr_SetString( PyExc_TypeError, "GUI.Flash: "
			"Argument parsing error: Expected a PyMovieView object" );
		return NULL;
	}
	
	FlashGUIComponent* pFlash = new FlashGUIComponent(static_cast<PyMovieView*>(obj));
	return pFlash;
}


void
FlashGUIComponent::update( float dTime, float relativeParentWidth, float relativeParentHeight )
{
	BW_GUARD;
	SimpleGUIComponent::update( dTime, relativeParentWidth, relativeParentHeight );
	pMovieView_->pMovieView()->Advance( dTime, 2 );
}


void FlashGUIComponent::drawSelf( bool reallyDraw, bool overlay )
{
	BW_GUARD;
	if (!reallyDraw)
		return;

	//This scoped viewport/scissor setter will remember the original settings,
	//and reapply them when we leave the function.
	Moo::ViewportSetter viewportState;
	Moo::ScissorsSetter scissorState;

	Vector2 corners[4];
	SimpleGUIComponent::clipBounds(corners[0], corners[1], corners[2], corners[3] );

	GViewport gv;
	GRectF rect;
	runtimeGUIViewport( corners, gv, rect );

	//The component draw region is in rect, the parent window
	//draw region is in gv.  We want the parent window region
	//in the scissors rectangle, and our draw region in the
	//viewport.
	gv.SetScissorRect( gv.Left, gv.Top, gv.Width, gv.Height );

	//Now set the draw region into the viewport part of gv
	gv.Left = (SInt)(gv.Left + rect.Left);
	gv.Top = (SInt)(gv.Top + rect.Top);
	gv.Width = (SInt)(rect.Right - rect.Left);
	gv.Height = (SInt)(rect.Bottom - rect.Top);

	pMovieView_->pMovieView()->SetViewport( gv );
	pMovieView_->pMovieView()->Display();

	//TODO - take this out when we have re-enabled the D3D wrapper,
	//when we use the wrapper, scaleform will be calling via our
	//own setRenderState methods, instead of going straight to the
	//device and causing issues with later draw calls.
	Moo::rc().initRenderStates();
}

bool FlashGUIComponent::handleMouseEvent( const MouseEvent & event )
{
	BW_GUARD;
	bool handled = SimpleGUIComponent::handleMouseEvent( event );
	
	// If we don't have a script, automatically call it
	if (!handled && !script() && pMovieView_)
	{
		handled = pMovieView_->handleMouseEvent( event );
	}

	return handled;
}

bool FlashGUIComponent::handleKeyEvent( const KeyEvent & event )
{
	BW_GUARD;
	bool handled = SimpleGUIComponent::handleKeyEvent( event );
	
	// If we don't have a script, automatically call it
	if (!handled && !script() && pMovieView_)
	{
		handled = pMovieView_->handleKeyEvent( event );
	}

	return handled;
}

bool FlashGUIComponent::handleMouseButtonEvent( const KeyEvent & event )
{
	BW_GUARD;
	bool handled = SimpleGUIComponent::handleMouseButtonEvent( event );
	
	// If we don't have a script, automatically call it
	if (!handled && !script() && pMovieView_)
	{
		handled = pMovieView_->handleMouseButtonEvent( event );
	}

	return handled;
}

} //namespace Scaleform


#endif //#if SCALEFORM_SUPPORT
// flash_gui_component.cpp