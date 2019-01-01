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

#include "camera.hpp"

#ifndef CODE_INLINE
    #include "camera.ipp"
#endif

namespace Moo
{

/**
 *	Finds the point on the near plane in camera space,
 *	given x and y in clip space.
 */
Vector3 Camera::nearPlanePoint( float xClip, float yClip ) const
{
	const float yLength = nearPlane_ * tanf( fov_ * 0.5f );
	const float xLength = yLength * aspectRatio_;

	return Vector3( xLength * xClip, yLength * yClip, nearPlane_ );
}

}

// camera.cpp
