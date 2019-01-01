/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BEELINE_CONTROLLER_HPP
#define BEELINE_CONTROLLER_HPP

#include "movement_controller.hpp"

namespace Beeline
{
	// This could probably derive from GraphTraverser itself, but I want to do
	// it as a standalone class as a learning experience
	class BeelineController : public MovementController
	{
	public:
		BeelineController( const Vector3 &destinationPos );
		virtual bool nextStep (float &speed,
							   float dTime,
							   Vector3 &pos,
							   Direction3D &dir);

	protected:
		Vector3 destinationPos_;
	};
};

#endif // BEELINE_CONTROLLER_HPP
