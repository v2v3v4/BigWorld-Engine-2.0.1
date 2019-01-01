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
#include "py_movie_view.hpp"
#include "util.hpp"
#include "manager.hpp"
#if SCALEFORM_IME
#include "ime.hpp"
#endif

PY_ENUM_MAP( Scaleform::PyMovieView::eScaleModeType );
PY_ENUM_CONVERTERS_CONTIGUOUS( Scaleform::PyMovieView::eScaleModeType );

namespace Scaleform
{
	PY_TYPEOBJECT( PyMovieView )

	#undef PY_ATTR_SCOPE
	#define PY_ATTR_SCOPE PyMovieView::
	
	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( eScaleModeType, scaleMode, scaleMode );

	PY_BEGIN_METHODS( PyMovieView )
		/*~ function PyMovieView.showAsGlobalMovie
		 *
		 * This function allows you to run the movie as part of a global
		 * movie list.  The alternative is to wrap the movie in a
		 * FlashGUIComponent.  Key/Mouse handling is better handled as
		 * part of the GUI system, so creating a FlashGUIComponent
		 * wrapper is preferred.  The Global movie list is for convenience.
		 *
		 * @param	Boolean		[optional] show (default) or hide the movie.
		 */
		PY_METHOD( showAsGlobalMovie )
		/*~ function PyMovieView.invoke
		 *
		 * This function calls a method in Action Script.
		 */
		PY_METHOD( invoke )
		/*~ function PyMovieView.setFSCommandCallback
		 *
		 * This function sets up the callback function used
		 * to handle ActionScript's fscommand.
		 */
		PY_METHOD( setFSCommandCallback )
		/*~ function PyMovieView.setExternalInterfaceCallback
		 *
		 * This function sets up the callback function used
		 * to handle ActionScript's external interface command.
		 */
		PY_METHOD( setExternalInterfaceCallback )
		/*~ function PyMovieView.gotoFrame
		 *
		 * This function moves the playahead to the specified frame. Note that
		 * calling this function may cause ActionScript tags attached to the
		 * frame to be bypassed.
		 *
		 * @param Integer	The frame number to go to.
		 */
		PY_METHOD( gotoFrame )
		/*~ function PyMovieView.gotoLabeledFrame
		 *
		 * This function moves the playahead to the frame with the given label.
		 * If the label is not found, the function will return false.
		 *
		 * @param String	The frame label to go to.
		 * @param Integer	Offset from the label.
		 * @return Boolean	Whether or not the label was found.
		 */
		PY_METHOD( gotoLabeledFrame )
		/*~ function PyMovieView.restart
		 *
		 * This function restarts the flash movie.
		 */
		PY_METHOD( restart )
		/*~ function PyMovieView.setPause
		 *
		 * This function sets the pause mode of the movie.
		 *
		 * @param Boolean	Flag specifying whether or not to pause the movie.
		 *
		 */
		PY_METHOD( setPause )
		/*~ function PyMovieView.setFocussed
		 *
		 * This function selects this movie as having IME focus, and
		 * finalises IME support for the previously focussed movie.
		 *
		 * Once focussed, the movie will internally tell IME which
		 * dynamic text fields are focussed, and IME will appear
		 * accordingly.
		 */
		PY_METHOD( setFocussed )
		/*~ function PyMovieView.handleMouseEvent
		 *
		 *	Call this method to pass a mouse move event into the underlying flash movie. If the 
		 *	movie handles the event, this method will return True.
		 *
		 *	@param	event	A PyMouseEvent object.
		 *
		 *	@return			True if the event was handled, False if it was not
		 */
		PY_METHOD( handleMouseEvent )
		/*~ function PyMovieView.handleKeyEvent
		 *
		 *	Call this method to pass a key event into the underlying flash movie. If the 
		 *	movie handles the event, this method will return True.
		 *	
		 *	@param	event	A PyKeyEvent object.
		 *
		 *	@return			True if the event was handled, False if it was not
		 */
		PY_METHOD( handleKeyEvent )
		/*~ function PyMovieView.handleMouseButtonEvent
		 *
		 *	Call this method to pass a mouse button event into the underlying flash movie. If the 
		 *	movie handles the event, this method will return True.
		 *	
		 *	@param	event	A PyKeyEvent object.
		 *
		 *	@return			True if the event was handled, False if it was not
		 */
		PY_METHOD( handleMouseButtonEvent )

	PY_END_METHODS()

	PY_BEGIN_ATTRIBUTES( PyMovieView )
		/*~ attribute PyMovieView.backgroundAlpha
		 * Sets the background color alpha applied to the movie clip.
		 * Set to 0.0f if you do not want the movie to render its background
		 * (ex. if you are displaying a hud on top of the game scene).
		 * The default value is 1.0f, indication full opacity.
		 * @type Read-Write Float.
		 */
		PY_ATTRIBUTE( backgroundAlpha )
		/*~ attribute PyMovieView.visible
		 * Sets the visibility state of the movie view.  Movie Views that
		 * are invisible do not draw, and do not advance.
		 */
		PY_ATTRIBUTE( visible )
		/*~ attribute PyMovieView.userData
		 *
		 * This attribute stores the User data object associated
		 * with the movie.  The data can be any object.
		 */
		PY_ATTRIBUTE( userData )
		/*~ attribute PyMovieView.scaleMode
		 *
		 * This attribute stores the movie view scale mode,
		 * which describes how the movie should display when the
		 * viewing dimensions are not the same as the original
		 * flash movie size.
		 */
		PY_ATTRIBUTE( scaleMode )

	PY_END_ATTRIBUTES()


	///Constructor
	PyMovieView::PyMovieView( GFxMovieView* pMovieView, PyTypePlus *pType ):
		PyObjectPlus( pType ),
		userData_( NULL ),
		fsCommandHandler_( NULL ),
		pMovieView_( *pMovieView )
	{
		BW_GUARD;
	}	


	PyMovieView::~PyMovieView()
	{
		BW_GUARD;
		Manager::instance().removeMovieView(this);
#if SCALEFORM_IME
		IME::onDeleteMovieView(this);
#endif
		
		// Release PyObjects.
		Py_CLEAR(userData_);

		if ( fsCommandHandler_ )
		{
			pMovieView_->SetFSCommandHandler( NULL );
			fsCommandHandler_ = NULL;
		}

		if ( externalInterface_ )
		{
			pMovieView_->SetExternalInterface( NULL );
			externalInterface_ = NULL;
		}

		// Release Gfx reference.
		pMovieView_ = NULL;
	}


	void PyMovieView::userData(PyObject* args)
	{
		BW_GUARD;
		Py_CLEAR(this->userData_);
		MF_ASSERT( this->userData_ == NULL );

		if (args != Py_None)
		{
			Py_XINCREF(args);
			this->userData_ = args;
		}
	}


	PyObject* PyMovieView::userData()
	{
		BW_GUARD;
		if (!this->userData_)
			Py_RETURN_NONE;

		Py_INCREF(this->userData_);
		return this->userData_;	
	}


	void PyMovieView::setFSCommandCallback(PyObjectPtr args)
	{
		BW_GUARD;
		if ( fsCommandHandler_ )
		{
			pMovieView_->SetFSCommandHandler(NULL);
			fsCommandHandler_ = NULL;
		}

		if (args != Py_None)
		{
			fsCommandHandler_ = *new PyFSCommandHandler( args.get() );
			pMovieView_->SetFSCommandHandler(fsCommandHandler_);
		}
	}


	void PyMovieView::setExternalInterfaceCallback(PyObjectPtr args)
	{
		BW_GUARD;
		if ( externalInterface_ )
		{
			pMovieView_->SetExternalInterface(NULL);
			externalInterface_ = NULL;
		}

		if (args != Py_None)
		{
			externalInterface_ = *new PyExternalInterface( args.get() );
			pMovieView_->SetExternalInterface( externalInterface_ );
		}
	}


	PyObject* PyMovieView::invoke(PyObjectPtr spArgs)
	{
		BW_GUARD;
		PyObject * args = spArgs.get();

		const char* methodName;
		Py_ssize_t numArgs = PyTuple_Size(args);

		if (numArgs == -1)
		{
			methodName = PyString_AsString(args);
		}
		if (numArgs == 0)
		{
			PyErr_SetString( PyExc_Exception, "PyMovieView.invoke expected atleast 1 argument.");
			return NULL;
		}
		else if (numArgs > 0)
		{
			PyObject* arg0 = PyTuple_GetItem(args, 0);

			if (!PyString_CheckExact(arg0))
			{
				PyErr_SetString( PyExc_Exception, "PyMovieView.invoke - The first argument must be a String.");
				return NULL;
			}

			methodName = PyString_AsString(arg0);
		}

		// Call result.
		PyObject* pyResult = NULL;
		GFxValue gfxResult( false );

		// No arguments.
		if (numArgs <= 1)
		{
			// Make the call.
			if (!this->pMovieView_->Invoke(methodName, &gfxResult, NULL, 0))
			{
				PyErr_SetString(PyExc_Exception, "PyMovieView.invoke - Failed to invoke method.");
				return NULL;
			}
		}

		// Arguments stored in a list (2nd argument of the tuple).
		else if (numArgs == 2 && PyList_CheckExact(PyTuple_GetItem(args, 1)))
		{
			PyObject* argList = PyTuple_GetItem(args, 1);
			numArgs = PyList_Size(argList);

			// Convert remaining arguments to GFxValues.
			std::vector<GFxValue> gfxArgs(numArgs);
			for (Py_ssize_t i = 0; i < numArgs; ++i)
				objectToValue(PyList_GetItem(argList, i), gfxArgs[i]);

			// Make the call.
			if (!this->pMovieView_->Invoke(methodName, &gfxResult, &gfxArgs[0], static_cast<UInt>(numArgs)))
			{
				PyErr_SetString(PyExc_Exception, "PyMovieView.invoke - Failed to invoke method.");
				return NULL;
			}
		}
		// Arguments stored in the tuple of args passed to this function.
		else
		{
			// Convert remaining arguments to GFxValues.
			std::vector<GFxValue> gfxArgs(numArgs-1);
			for (Py_ssize_t i = 1; i < numArgs; ++i)
			{
				objectToValue(PyTuple_GetItem(args, i), gfxArgs[i-1]);
			}

			// Make the call.
			if (!this->pMovieView_->Invoke(methodName, &gfxResult, &gfxArgs[0], static_cast<UInt>(numArgs-1)))
			{
				PyErr_SetString(PyExc_Exception, "PyMovieView.invoke - Failed to invoke method.");
				return NULL;
			}
		}

		// Convert the return value.
		valueToObject(gfxResult, &pyResult);

		return pyResult;
	}


	void PyMovieView::setFocussed()
	{
		BW_GUARD;
		pMovieView_->HandleEvent(GFxEvent::SetFocus);
#if SCALEFORM_IME
		IME::setFocussedMovie(this);
#endif
	}


	PyMovieView::eScaleModeType PyMovieView::scaleMode() const
	{
		BW_GUARD;
		return (PyMovieView::eScaleModeType)this->pMovieView_->GetViewScaleMode();
	}


	void PyMovieView::scaleMode( PyMovieView::eScaleModeType s )
	{
		BW_GUARD;
		if( this->pMovieView_ )
			this->pMovieView_->SetViewScaleMode( (GFxMovieView::ScaleModeType)s );
	}


	void PyMovieView::gotoFrame( uint32 frameNumber)
	{
		BW_GUARD;
		this->pMovieView_->GotoFrame(frameNumber);
	}


	bool PyMovieView::gotoLabeledFrame(const std::string& plabel, int32 offset)
	{
		BW_GUARD;
		return this->pMovieView_->GotoLabeledFrame(plabel.c_str(), (SInt)offset);
	}


	void PyMovieView::showAsGlobalMovie( bool state )
	{
		BW_GUARD;
		if ( state )
		{
			Manager::instance().addMovieView(this);
		}
		else
		{
			Manager::instance().removeMovieView(this);
		}
	}

	bool PyMovieView::handleMouseEvent( const MouseEvent & event )
	{
 		BW_GUARD;
		int buttons = mouseButtonsFromEvent( lastMouseKeyEvent_ );
		
		Vector2 mousePos = event.cursorPosition();

		RECT rect;
		::GetClientRect( Manager::instance().hwnd(), &rect );
		mousePos.x = ( (mousePos.x + 1.0f) / 2.0f ) * (float)rect.right;
		mousePos.y = ( (-mousePos.y + 1.0f) / 2.0f ) * (float)rect.bottom;

		NotifyMouseStateFn msFn(mousePos.x, mousePos.y, buttons, (Float)((event.dz() / WHEEL_DELTA) * 3));
		msFn( this );
		
		return msFn.handled();
	}


	bool PyMovieView::handleKeyEvent( const KeyEvent & event )
	{
		BW_GUARD;
		bool handled = false;

		if (event.isMouseButton())
		{
			lastMouseKeyEvent_ = event;
			int buttons = mouseButtonsFromEvent(event);
			POINT mousePos;
			::GetCursorPos( &mousePos );
			::ScreenToClient( Manager::instance().hwnd(), &mousePos );
			NotifyMouseStateFn msFn((float)mousePos.x, (float)mousePos.y, buttons, 0.f);
			msFn( this );
			handled = msFn.handled();
		}
		else
		{
			GFxKeyEvent gfxevent = translateKeyEvent(event);	
			if (gfxevent.KeyCode != GFxKey::VoidSymbol)
			{
				HandleEventFn kyFn(gfxevent);
				kyFn( this );
				handled = kyFn.handled();
			}

			if (event.utf16Char()[0] != 0)
			{
				GFxCharEvent gfxevent( * (const UInt32*)(event.utf16Char()) );
				handled |= ( pMovieView()->HandleEvent(gfxevent) == GFxMovieView::HE_Completed );
			}
		}
		
		return handled;
	}

	bool PyMovieView::handleMouseButtonEvent( const KeyEvent & event )
	{
		BW_GUARD;
		return this->handleKeyEvent( event );
	}


	PyObject * PyMovieView::pyGetAttribute( const char *attr )
	{
		BW_GUARD;
		PY_GETATTR_STD();

		//Get the variable from the movie
		GFxValue pVal;
		if ( pMovieView_->GetVariable(&pVal, attr) )
		{
			PyObject* pyVal;
			valueToObject( pVal, &pyVal );
			//PY_INCREF here most likely..
			return pyVal;
		}

		return PyObjectPlus::pyGetAttribute(attr);
	}


	int PyMovieView::pySetAttribute( const char *attr, PyObject *value )
	{
		BW_GUARD;
		PY_SETATTR_STD();

		//Set the variable on the movie
		GFxValue pVal;
		objectToValue( value, pVal );
		if ( pMovieView_->SetVariable(attr, pVal, GFxMovie::SV_Normal) )
		{
			return 0;
		}

		return PyObjectPlus::pySetAttribute(attr,value);
	}
}
#endif //#if SCALEFORM_SUPPORT