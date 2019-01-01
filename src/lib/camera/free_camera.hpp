/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FREE_CAMERA_HPP
#define FREE_CAMERA_HPP


#include "base_camera.hpp"
#include "pyscript/script_math.hpp"

/*~ class BigWorld.FreeCamera
 *
 *	A FreeCamera is a camera that can be moved around using the mouse and arrow
 *	keys.  It doens't have any ties to entities, and doesn't do any physics or
 *  geometry checks, so it can be used to easily explore the world.
 *
 *	It processes both mouse and keyboard events, and consumes the events it
 *  uses.  It consumes: all mouse events, and all key events involving the
 *	arrow keys.
 *
 *
 *	It inherits from BaseCamera.
 *
 *	A FreeCamera can be created using the BigWorld.FreeCamera function.
 */
/**
 *	This class is a camera that can be moved around freely by the mouse
 */
class FreeCamera : public BaseCamera
{
	Py_Header( FreeCamera, BaseCamera )

public:
	FreeCamera( PyTypePlus * pType = &s_type_ );
	~FreeCamera();

	virtual bool handleKeyEvent( const KeyEvent & ev );
	virtual bool handleMouseEvent( const MouseEvent & event );
	virtual bool handleAxisEvent( const AxisEvent & event );

	virtual void set( const Matrix & viewMatrix );
	virtual void update( float dTime );

	PyObject *	pyGetAttribute( const char * attr );
	int			pySetAttribute( const char * attr, PyObject * value );

	PY_RW_ATTRIBUTE_DECLARE( fixed_, fixed )
	PY_RW_ATTRIBUTE_DECLARE( invViewProvider_, invViewProvider )

	PY_FACTORY_DECLARE()

private:
	FreeCamera( const FreeCamera& );
	FreeCamera& operator=( const FreeCamera& );

	MatrixProviderPtr	invViewProvider_;
	bool		fixed_;
};



#endif // FREE_CAMERA_HPP
