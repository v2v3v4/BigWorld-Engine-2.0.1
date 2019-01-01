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
#include "servo.hpp"
#include "pymodel.hpp"

/*~	class BigWorld.Servo
 *	The Servo class is a Motor that sets the transform 
 *	of a model to the given MatrixProvider.  As the 
 *	MatrixProvider is updated, the model will move.
 */
PY_TYPEOBJECT( Servo )

PY_BEGIN_METHODS( Servo )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( Servo )
	/*~ attribute Servo.signal
	 *
	 *	This attribute is the MatrixProvider that sets the model's transform.
	 *	As this MatrixProvider is updated, the model will be transformed accordingly.
	 *
	 *	@type	Read-Write MatrixProvider
	 */
	PY_ATTRIBUTE( signal )
PY_END_ATTRIBUTES()

/*~	function BigWorld.Servo
 *
 *	Creates and returns a new Servo Motor, which sets the 
 *	transform of a model to the given MatrixProvider.  As 
 *	the MatrixProvider is updated, the model will move.
 *
 *	@param signal	MatrixProvider that model will follow
 */
PY_FACTORY( Servo, BigWorld )


/**
 *	Constructor.
 */
Servo::Servo( MatrixProviderPtr s, PyTypePlus * pType ):
	Motor( pType ),
	signal_( s )
{
}

/**
 *	Destructor.
 */
Servo::~Servo()
{
}


/**
 *	This method runs this motor.
 */
void Servo::rev( float dTime )
{
	BW_GUARD;
	if ( signal_ )
	{
		Matrix newWorld;
		signal_->matrix( newWorld );
		pOwner_->worldTransform( newWorld );
	}
}


/**
 *	Standard get attribute method.
 */
PyObject * Servo::pyGetAttribute( const char * attr )
{
	BW_GUARD;
	PY_GETATTR_STD();

	return Motor::pyGetAttribute( attr );
}


/**
 *	Standard set attribute method.
 */
int Servo::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;
	PY_SETATTR_STD();

	return Motor::pySetAttribute( attr, value );
}


/**
 *	Static python factory method
 */
PyObject * Servo::pyNew( PyObject * args )
{
	BW_GUARD;
	PyObject* pObject;

	if (!PyArg_ParseTuple( args, "O", &pObject ) || !MatrixProvider::Check( pObject ))
	{
		PyErr_SetString(PyExc_TypeError, "Servo takes a matrix provider" );
		return NULL;
	}

	return new Servo( (MatrixProvider*)pObject );
}

// servo.cpp
