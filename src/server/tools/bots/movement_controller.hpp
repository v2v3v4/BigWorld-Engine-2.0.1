/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MOVEMENT_CONTROLLER_HPP
#define MOVEMENT_CONTROLLER_HPP

#include "math/vector3.hpp"
#include "network/basictypes.hpp"

/**
 *	This class is the interface for movement controllers.
 */
class MovementController
{
public:
	virtual bool nextStep( float & speed, float dTime,
		Vector3 & pos, Direction3D & dir ) = 0;
};


/**
 *	This class is the base class for all movement factories. They are
 *	responsible for creating the movement controllers.
 */
class MovementFactory
{
public:
	MovementFactory( const char * name );
	virtual MovementController * create( const std::string & data,
		float & speed, Vector3 & startPos ) = 0;
};

#endif // MOVEMENT_CONTROLLER_HPP
