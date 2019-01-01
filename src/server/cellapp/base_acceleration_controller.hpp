/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ACCELERATING_MOVE_CONTROLLER_HPP
#define ACCELERATING_MOVE_CONTROLLER_HPP


#include "controller.hpp"
#include "network/basictypes.hpp"
#include "updatable.hpp"


typedef SmartPointer< Entity > EntityPtr;


/**
 *	This class forms the base of the accelerating move controllers. It
 *	samples the velocity for the entity and manipulates it by applying
 *	acceleration in order to arrive at a given destination. 
 */
class BaseAccelerationController : public Controller, public Updatable
{
public:
	enum Facing
	{
		FACING_NONE			= 0,
		FACING_VELOCITY		= 1,
		FACING_ACCELERATION	= 2,

		FACING_MAX			= 3
	};

public:
	BaseAccelerationController(	float acceleration,
								float maxSpeed,
								Facing facing = FACING_NONE );

	virtual void startReal( bool isInitialStart );
	virtual void stopReal( bool isFinalStop );

	bool move( const Position3D & destination, bool stopAtDestination );

protected:

	static Vector3	calculateDesiredVelocity(
											const Position3D & currentPosition,
											const Position3D & desiredPosition,
											float acceleration,
											float maxSpeed,
											bool stopAtDestination );

	static Vector3	calculateAccelerationVector(
											const Vector3 & currentVelocity,
											const Vector3 & desiredVelocity );

	virtual void	writeRealToStream( BinaryOStream & stream );
	virtual bool	readRealFromStream( BinaryIStream & stream );

protected:
	float		acceleration_;
	float		maxSpeed_;
	Facing		facing_;	
};



#endif // ACCELERATING_MOVE_CONTROLLER_HPP
