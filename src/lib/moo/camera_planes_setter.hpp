/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CAMERA_PLANES_SETTER_HPP
#define CAMERA_PLANES_SETTER_HPP

namespace Moo
{
/**
 *	This class is a scoped camera near/far planes setter, allowing you to
 *	easily set these values on the device.
 *
 *	The previous settings are restored automatically when the
 *	class goes out of scope.
 */
	class CameraPlanesSetter
	{
	public:		
		CameraPlanesSetter( float nearPlane, float farPlane );
		~CameraPlanesSetter();
	private:		
		float savedNearPlane_;
		float savedFarPlane_;
	};
};	//namespace Moo

#endif	//CAMERA_PLANES_SETTER_HPP