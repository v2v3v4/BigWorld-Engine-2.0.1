/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ORBITOR_HPP
#define ORBITOR_HPP

#include "motor.hpp"
#include "pyscript/script.hpp"
#include "pyscript/script_math.hpp"
#include "math/vector3.hpp"

/*~ class BigWorld.Orbitor
 *
 *	An Orbitor is a Motor that moves a model towards a target (MatrixProvider) and
 *	when within a certain distance starts to move towards an orbiting path around the
 *	target. Additionally an optional proximity callback can be set for the Orbitor to call
 *	once the model it is moving gets within a specified (proximity) distance of
 *	its target.
 *
 *	A new Orbitor Motor is created using BigWorld.Orbitor function.
 */

/**
 *	This class is a Motor that homes in on a target and then orbits the target.
 */
class Orbitor : public Motor
{
	Py_Header( Orbitor, Motor )

public:
	Orbitor( PyTypePlus * pType = &Orbitor::s_type_ );
	~Orbitor();

	virtual void rev( float dTime );

	PY_FACTORY_DECLARE()

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PY_RW_ATTRIBUTE_DECLARE( target_, target )
	PY_RW_ATTRIBUTE_DECLARE( speed_, speed )
	PY_RW_ATTRIBUTE_DECLARE( spinRadius_, spinRadius )
	PY_RW_ATTRIBUTE_DECLARE( startSpin_, startSpin )

	PY_RW_ATTRIBUTE_DECLARE( maxSpeed_, maxSpeed)
	PY_RW_ATTRIBUTE_DECLARE( endSpeed_, endSpeed)
	PY_RW_ATTRIBUTE_DECLARE( slowHeigth_, slowHeigth)
	PY_RW_ATTRIBUTE_DECLARE( maxAccel_, maxAccel )

	PY_RW_ATTRIBUTE_DECLARE( weightingFunction_, weightingFunction )
	PY_RW_ATTRIBUTE_DECLARE( distanceScaler_, distanceScaler )


	PY_RW_ATTRIBUTE_DECLARE( wobbleFreq_, wobbleFreq)
	PY_RW_ATTRIBUTE_DECLARE( wobbleMax_, wobbleMax)
	PY_RW_ATTRIBUTE_DECLARE( wobble_, wobble )

	PY_RW_ATTRIBUTE_DECLARE( proximity_, proximity )
	PY_RW_ATTRIBUTE_DECLARE( proximityCallback_, proximityCallback )


private:
	MatrixProviderPtr	target_;

	//Kinematics values
	float			speed_;
	float			startSpin_;
	float			spinRadius_;
	float			maxSpeed_;
	float			slowHeigth_;
	float			maxAccel_;
	float			endSpeed_;

	//Transition function
	int				weightingFunction_;
	float			distanceScaler_;

	//Oscillation values
	bool			wobble_;
	bool			spinning_;	
	float			spinTime_;
	float			wobbleFreq_;
	float			wobbleMax_;

	//Proximity callback
	float			proximity_;
	PyObject		* proximityCallback_;

	void makePCBHappen();
};


#endif // ORBITOR_HPP
