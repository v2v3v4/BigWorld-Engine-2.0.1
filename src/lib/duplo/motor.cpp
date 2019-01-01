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

#include "motor.hpp"
#include "pymodel.hpp"

#include "pyscript/script.hpp"

DECLARE_DEBUG_COMPONENT2( "Motor", 0 )


PY_TYPEOBJECT( Motor )

PY_BEGIN_METHODS( Motor )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( Motor )
	/*~ attribute Motor.owner
	 *
	 *	This read only attribute is the PyModel which the motor is controlling.
	 *
	 *	@type	Read-Only PyModel
	 */
	PY_ATTRIBUTE( owner )
PY_END_ATTRIBUTES()


/**
 *	Standard get attribute method.
 */
PyObject * Motor::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return PyObjectPlus::pyGetAttribute( attr );
}

/**
 *	Standard set attribute method.
 */
int Motor::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return PyObjectPlus::pySetAttribute( attr, value );
}


/**
 *	Get this model's owner.
 *
 *	Only in the source file so we don't have to include pymodel and script
 *	in the header file.
 */
PyObject * Motor::pyGet_owner()
{
	BW_GUARD;
	return Script::getData( pOwner_ );
}

// motor.cpp
