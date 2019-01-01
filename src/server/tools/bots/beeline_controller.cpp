/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "Python.h"		// See http://docs.python.org/api/includes.html

#include "beeline_controller.hpp"
#include <string>
using namespace std;

#include "cstdmf/debug.hpp"

// #include "resmgr/bwresource.hpp"
// #include "resmgr/datasection.hpp"
// #include "cstdmf/debug.hpp"

#include "Python.h"

DECLARE_DEBUG_COMPONENT2( "Bots", 100 );

namespace Beeline
{

BeelineController::BeelineController( const Vector3 &destinationPos ) :
	destinationPos_( destinationPos )
{
}

bool BeelineController::nextStep (float & speed,
	float dTime,
	Vector3 &pos,
	Direction3D &dir)
{
	// Distance travelled per tick
	float distance = speed * dTime;

	// Vector to dest and zzp respectively
	Vector3 destVec = destinationPos_ - pos;

	// Move closer if we are aren't there yet
	if (destVec.length() > distance)
	{
		pos += destVec * (distance / destVec.length());
		dir.yaw = destVec.yaw();
	}

	return true;
}

// -----------------------------------------------------------------------------
// Section: PatrolFactory
// -----------------------------------------------------------------------------

namespace
{

/**
 *	This class is used to create patrol movement controllers.
 */
class BeelineControllerFactory : public MovementFactory
{
public:
	BeelineControllerFactory() : MovementFactory( "Beeline" )
	{
	}

	/**
	 *	This method returns a patrol movement controller.
	 */
	MovementController *create( const std::string & data, float & speed, Vector3 & position )
	{
		Vector3 destination;

		uint com1 = data.find_first_of( ',' ), com2 = data.find_last_of( ',' );

		destination.x = atof( data.substr( 0, com1 ).c_str() );
		destination.y = atof( data.substr( com1, com2 ).c_str() );
		destination.z = atof( data.substr( com2 ).c_str() );

		return new BeelineController( destination );
	}
};

BeelineControllerFactory s_beelineFactory;

} // anon namespace

}; // namespace Beeline

// BEELINE_CONTROLLER_CPP
