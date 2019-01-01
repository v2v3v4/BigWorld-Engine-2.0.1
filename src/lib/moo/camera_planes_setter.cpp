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
#include "camera_planes_setter.hpp"

namespace Moo
{

/**
 *	This constructor saves the current values for the near and far planes
 *  of Moo's current camera and sets the new values
 *
 *	@param	nearPlane		new near plane
 *	@param	farPlane		new far plane
 */
CameraPlanesSetter::CameraPlanesSetter( float nearPlane, float farPlane )
{
	BW_GUARD;
	Moo::Camera camera = Moo::rc().camera();
	savedNearPlane_ = camera.nearPlane();
	savedFarPlane_ = camera.farPlane();
	camera.nearPlane( nearPlane );
	camera.farPlane( farPlane );
	Moo::rc().camera( camera );
	Moo::rc().updateViewTransforms();
	Moo::rc().updateProjectionMatrix();
}


/**
 *	This destructor automatically resets the saved near and far plane values.
 */
CameraPlanesSetter::~CameraPlanesSetter()
{
	BW_GUARD;
	Moo::rc().camera().nearPlane( savedNearPlane_ );
	Moo::rc().camera().farPlane( savedFarPlane_ );
	Moo::rc().camera( Moo::rc().camera() );
	Moo::rc().updateViewTransforms();
	Moo::rc().updateProjectionMatrix();
}


}	//namespace Moo