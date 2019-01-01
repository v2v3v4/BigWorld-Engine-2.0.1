/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ACCELERATE_TO_POINT_CONTROLLER_HPP
#define ACCELERATE_TO_POINT_CONTROLLER_HPP


#include "base_acceleration_controller.hpp"


/**
 *	This movement controller accelerates the entity toward a point in space.
 *	It can be configured to rotate the entity to match its current movement
 *	and to come gently to a halt at its final destination.
 */
class AccelerateToPointController : public BaseAccelerationController
{
	DECLARE_CONTROLLER_TYPE( AccelerateToPointController )

public:
	AccelerateToPointController( Vector3 destination = Vector3(0,0,0),
								float acceleration = 0.0f,
								float maxSpeed = 0.0f,
								Facing facing = FACING_NONE,
								bool stopAtDestination = true );

	virtual void		writeRealToStream( BinaryOStream & stream );
	virtual bool		readRealFromStream( BinaryIStream & stream );
	void				update();

private:
	Vector3				destination_;
	bool				stopAtDestination_;
};

#endif // ACCELERATE_TO_POINT_CONTROLLER_HPP
