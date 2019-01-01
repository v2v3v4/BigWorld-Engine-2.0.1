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

#include "directional_light.hpp"

#ifndef CODE_INLINE
#include "directional_light.ipp"
#endif

namespace Moo {

/**
 *	Constructor
 */
DirectionalLight::DirectionalLight( )
: colour_( 0, 0, 0, 0 ),
  direction_( 0, 0, 1 )
#ifdef EDITOR_ENABLED
  , multiplier_( 1.f )
#endif
{
	// give the worldTransformed attributes default values.
	worldDirection_ = direction_;
}

/**
 *	Constructor
 */
DirectionalLight::DirectionalLight( const Colour& colour, const Vector3& direction )
: colour_( colour ),
  direction_( direction )
#ifdef EDITOR_ENABLED
  , multiplier_( 1.f )
#endif
{
	// give the worldTransformed attributes default values.
	worldDirection_ = direction;
}

/**
 *	Destructor
 */
DirectionalLight::~DirectionalLight()
{
}

/**
 *  Transforms the light to world space. Should behave properly with all non-zero matrices.
 */
void DirectionalLight::worldTransform( const Matrix& transform )
{

	// Get the transpose of the inverse matrix, so that the direction of the 
	// light gets affected properly by non uniform scale.
	Matrix t;
	if( XPMatrixInverse( &t, NULL, &transform ) )
	{
		XPMatrixTranspose( &t, &t );
	}
	else
	{
		t = transform;
	}

	// transform the light direction. (uses transformnormal as it is essentially the same thing)
	XPVec3TransformNormal( &worldDirection_, &direction_, &t );
	XPVec3Normalize( &worldDirection_, &worldDirection_ );

}

}

// directional_light.cpp
