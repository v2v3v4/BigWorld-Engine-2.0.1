/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// managed_texture.ipp

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif

namespace Moo
{

INLINE
uint32 ManagedTexture::textureMemoryUsed( )
{
	return textureMemoryUsed_;
}

INLINE
bool ManagedTexture::valid() const
{
	return valid_;
}

}

// managed_texture.ipp