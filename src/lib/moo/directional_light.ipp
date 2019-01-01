/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif

namespace Moo {

INLINE
const Colour& DirectionalLight::colour( ) const
{
	return colour_;
}

INLINE
void DirectionalLight::colour( const Colour& colour )
{
	colour_ = colour;
}

INLINE
const Vector3& DirectionalLight::direction( ) const
{
	return direction_;
}

INLINE
void DirectionalLight::direction( const Vector3& direction )
{
	direction_ = direction;
}

INLINE
const Vector3& DirectionalLight::worldDirection( ) const
{
	return worldDirection_;
}

}
/*directional_light.ipp*/
