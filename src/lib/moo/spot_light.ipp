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
const Vector3& SpotLight::position( ) const
{
	return position_;
}

INLINE
void SpotLight::position( const Vector3& position )
{
	dirty_ = true;
	position_ = position;
}

INLINE
const Vector3& SpotLight::direction( ) const
{
	return direction_;
}

INLINE
void SpotLight::direction( const Vector3& direction )
{
	dirty_ = true;
	direction_ = direction;
}

INLINE
float SpotLight::innerRadius( ) const
{
	return innerRadius_;
}

INLINE
void SpotLight::innerRadius( float innerRadius )
{
	dirty_ = true;
	innerRadius_ = innerRadius;
	if( innerRadius_ > outerRadius_ )
		outerRadius( innerRadius_ );
}

INLINE
float SpotLight::outerRadius( ) const
{
	return outerRadius_;
}

INLINE
void SpotLight::outerRadius( float outerRadius )
{
	dirty_ = true;
	outerRadius_ = outerRadius;
	if( innerRadius_ > outerRadius_ )
		innerRadius( outerRadius_ );
}

INLINE
float SpotLight::cosConeAngle( ) const
{
	return cosConeAngle_;
}

INLINE
void SpotLight::cosConeAngle( float cosConeAngle )
{
	dirty_ = true;
	cosConeAngle_ = cosConeAngle;
}

INLINE
const Colour& SpotLight::colour( ) const
{
	return colour_;
}

INLINE
void SpotLight::colour( const Colour& colour )
{
	colour_ = colour;
}

INLINE
const Vector3& SpotLight::worldPosition( ) const
{
	return worldPosition_;
}

INLINE
const Vector3& SpotLight::worldDirection( ) const
{
	return worldDirection_;
}

INLINE
float SpotLight::worldInnerRadius( ) const
{
	return worldInnerRadius_;
}

INLINE
float SpotLight::worldOuterRadius( ) const
{
	return worldOuterRadius_;
}

INLINE
bool SpotLight::intersects( const BoundingBox& worldSpaceBB ) const
{
	if (!worldSpaceBB.intersects( worldPosition_, worldOuterRadius_ ))
	{
		return false;
	}
	
	const_cast<SpotLight*>(this)->updateInternalBounds();
	if (!worldSpaceBB.intersects( lightBounds_ ))
	{
		return false;
	}
	
	worldSpaceBB.calculateOutcode( lightView_ );
	if (worldSpaceBB.combinedOutcode() != 0)
	{
		return false;
	}	
	
	return true;
}

INLINE
Vector4* SpotLight::getTerrainLight( uint32 timestamp, float lightScale )
{
	if(  terrainTimestamp_ != timestamp )
	{
		createTerrainLight( lightScale );
		terrainTimestamp_ = timestamp;
	}
	return terrainLight_;
}


}
/*spot_light.ipp*/
