/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef HOMER_HPP
#define HOMER_HPP

#include "motor.hpp"
#include "pyscript/script.hpp"
#include "pyscript/script_math.hpp"
#include "math/vector3.hpp"

/*~ class BigWorld.Homer
 *
 *	A Homer is a Motor that moves a model towards a target (MatrixProvider).
 *	Additionally an optional proximity callback can be set for the Homer to call
 *	once the model it is moving gets within a specified (proximity) distance of
 *	its target.
 *
 *	A new Homer Motor is created using BigWorld.Homer function.
 */

/**
 *	This class is a Motor that homes in on a target.
 */
class Homer : public Motor
{
	Py_Header( Homer, Motor )

public:
	Homer( PyTypePlus * pType = &Homer::s_type_ );
	~Homer();

	virtual void rev( float dTime );

	PY_FACTORY_DECLARE()

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PY_RW_ATTRIBUTE_DECLARE( target_, target )
	PY_RW_ATTRIBUTE_DECLARE( speed_, speed )
	PY_RW_ATTRIBUTE_DECLARE( turnRate_, turnRate )
	PY_RW_ATTRIBUTE_DECLARE( turnAxis_, turnAxis )

	PY_RW_ATTRIBUTE_DECLARE( tripTime_, tripTime )
	PY_RW_ATTRIBUTE_DECLARE( goneTime_, goneTime )
	PY_RW_ATTRIBUTE_DECLARE( zenithed_, zenithed )

	PY_RW_ATTRIBUTE_DECLARE( proximity_, proximity )
	PY_RW_ATTRIBUTE_DECLARE( proximityCallback_, proximityCallback )

	PY_RW_ATTRIBUTE_DECLARE( arrivalCountLimit_, arrivalCountLimit )

	PY_RW_ATTRIBUTE_DECLARE( scale_, scale )
	PY_RW_ATTRIBUTE_DECLARE( scaleTime_, scaleTime )

private:
	MatrixProviderPtr	target_;

	float			speed_;
	float			turnRate_;	///< In radians per second
	Vector3			turnAxis_;

	float			tripTime_;
	float			goneTime_;
	bool			zenithed_;

	float			proximity_;
	PyObject		* proximityCallback_;

	int				arrivalCount_;
	int				arrivalCountLimit_;

	float			scale_;
	float			scaleTime_;

	void makePCBHappen();
};


#endif // HOMER_HPP
