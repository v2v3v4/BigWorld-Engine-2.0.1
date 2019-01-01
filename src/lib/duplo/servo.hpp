/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SERVO_HPP
#define SERVO_HPP

#include "motor.hpp"
#include "pyscript/script_math.hpp"

/**
 *	This class is a motor for a model that sets the position
 *	of the model based on a signal.
 *
 *	This class actually sets the transform of a model to the
 *	given MatrixProvider.
 */
class Servo : public Motor
{
	Py_Header( Servo, Motor )

public:
	Servo( MatrixProviderPtr s, PyTypePlus * pType = &Servo::s_type_ );
	~Servo();

	PY_FACTORY_DECLARE()
	PY_RW_ATTRIBUTE_DECLARE( signal_, signal )

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	virtual void rev( float dTime );
private:
	MatrixProviderPtr	signal_;
};

#endif