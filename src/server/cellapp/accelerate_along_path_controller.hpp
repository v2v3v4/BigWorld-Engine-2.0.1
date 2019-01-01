/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ACCELERATE_ALONG_PATH_CONTROLLER_HPP
#define ACCELERATE_ALONG_PATH_CONTROLLER_HPP

#include "base_acceleration_controller.hpp"


/**
 *	This class forms a controller that moves an entity along a list of
 *	waypoints. The controller accelerates the entity though
 *	each waypoint, finally coming to a halt at the last one.
 */
class AccelerateAlongPathController : public BaseAccelerationController
{
	DECLARE_CONTROLLER_TYPE( AccelerateAlongPathController )

public:
	AccelerateAlongPathController();
	AccelerateAlongPathController(	std::vector< Position3D > & waypoints,
									float acceleration,
									float maxSpeed,
									Facing facing );

	virtual void		writeRealToStream( BinaryOStream & stream );
	virtual bool		readRealFromStream( BinaryIStream & stream );
	void				update();

private:
	std::vector< Position3D >	waypoints_;
	uint						progress_;
};


#endif // ACCELERATE_ALONG_PATH_CONTROLLER_HPP
