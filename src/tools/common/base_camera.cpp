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
#include "base_camera.hpp"

#ifndef CODE_INLINE
#include "base_camera.ipp"
#endif

#include "moo/render_context.hpp"
#include "input/py_input.hpp"

DECLARE_DEBUG_COMPONENT2( "Editor", 0 )


/**
 *	Constructor.
 */
BaseCamera::BaseCamera( PyTypePlus * pType ) :
	PyObjectPlus( pType ),
    invert_( false ),
	windowHandle_( 0 )
{
	BW_GUARD;

	view_.setIdentity();

	speed_[0] = 4.f;
	speed_[1] = 40.f;
}


/**
 *	Destructor.
 */
BaseCamera::~BaseCamera()
{
}


/**
 *	Render method. Simply sets the view transform to our one.
 */
void BaseCamera::render( float dTime )
{
	BW_GUARD;

	Moo::rc().view( this->view() );
}

void BaseCamera::windowHandle( const HWND handle )
{
    windowHandle_ = handle;
}


// -----------------------------------------------------------------------------
// Section: Python stuff
// -----------------------------------------------------------------------------

PY_TYPEOBJECT( BaseCamera )

PY_BEGIN_METHODS( BaseCamera )
	/*~ function BaseCamera.update
	 *	@components{ tools }
	 *
	 *	This method implements camera-type dependent handling, usually done
	 *	before rendering each frame.
	 *
	 *	@param dTime	The change in time since last frame
	 *	@param activeInputHandler Whether or not the tool has the input focus.
	 */
	PY_METHOD( update )
	/*~ function BaseCamera.render
	 *	@components{ tools }
	 *
	 *	This method simply sets the rendering view transform to the view
	 *	transform of this camera.
	 *
	 *	@param dTime	The change in time since last frame
	 */
	PY_METHOD( render )
	/*~ function BaseCamera.handleKeyEvent
	 *	@components{ tools }
	 *
	 *	This method handles key input events that can be handled by the camera.
	 *
	 *	@param event The KeyEvent object containing the information about the key event.
	 *	@return		True if the event was handled by the camera, False otherwise.
	 */
	PY_METHOD( handleKeyEvent )
	/*~ function BaseCamera.handleMouseEvent
	 *	@components{ tools }
	 *
	 *	This method handles mouse input events that can be handled by the camera.
	 *
	 *	@param event The MouseEvent object containing the information about the event.
	 *	@return		True if the event was handled by the camera, False otherwise.
	 */
	PY_METHOD( handleMouseEvent )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( BaseCamera )
	/*~ attribute BaseCamera.speed
	 *	@components{ tools }
	 *
	 *	The speed attribute determines how quick the camera will move when the
	 *	user is interacting with it (via the WASD keys for example). 
	 *
	 *	@type	Float
	 */
	PY_ATTRIBUTE( speed )

	/*~ attribute BaseCamera.turboSpeed
	 *	@components{ tools }
	 *
	 *	The turboSpeed attribute determines how quick the camera will move when the
	 *	user is interacting with it in "turbo" mode (via the WASD keys for example). 
	 *	For information on how to enable turbo mode in the tool, please refer to 
	 *	the tool's help menu items and documentation.
	 *
	 *	@type	Float
	 */
	PY_ATTRIBUTE( turboSpeed )

	/*~ attribute BaseCamera.invert
	 *	@components{ tools }
	 *
	 *	The invert attribute allows for inverting the direction of movement of
	 *	the camera when the user is interacting with it.
	 *
	 *	@type	Boolean
	 */
	PY_ATTRIBUTE( invert )

	/*~ attribute BaseCamera.view
	 *	@components{ tools }
	 *
	 *	The view attribute determines the view matrix used for the camera.
	 *
	 *	@type	Matrix
	 */
	PY_ATTRIBUTE( view )

	/*~ attribute BaseCamera.position
	 *	@components{ tools }
	 *
	 *	The position attribute determines the translation component of the
	 *	camera's view matrix.
	 *
	 *	@type	Vector3
	 */
	PY_ATTRIBUTE( position )
PY_END_ATTRIBUTES()


/**
 *	Get an attribute for python
 */
PyObject * BaseCamera::pyGetAttribute( const char * attr )
{
	BW_GUARD;

	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}

/**
 *	Set an attribute for python
 */
int BaseCamera::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;

	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}

// ----------------------------------------------------------------------------
// Section: InvViewMatrixProvider
// ----------------------------------------------------------------------------
class InvViewMatrixProvider : public MatrixProvider
{
	Py_Header( InvViewMatrixProvider, MatrixProvider )
public:
	InvViewMatrixProvider() : MatrixProvider( false, &s_type_ ) {}

	virtual void matrix( Matrix & m ) const
	{
		m = Moo::rc().invView();
	}
	PY_DEFAULT_CONSTRUCTOR_FACTORY_DECLARE()
};


PY_TYPEOBJECT( InvViewMatrixProvider )

PY_BEGIN_METHODS( InvViewMatrixProvider )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( InvViewMatrixProvider )
PY_END_ATTRIBUTES()

/*~ function BigWorld.InvViewMatrix
 *	@components{ tools }
 *
 *	This function returns a new InvViewMatrixProvider which is a MatrixProvider
 *	that can be used to access the rendering engine's inverse view matrix. 
 */
PY_FACTORY_NAMED( InvViewMatrixProvider, "InvViewMatrix", BigWorld )


// base_camera.cpp
