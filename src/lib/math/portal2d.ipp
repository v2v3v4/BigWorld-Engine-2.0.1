/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// portal2d.ipp

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif

/**
 *	Add the given point to the portal
 */
INLINE void Portal2D::addPoint( const Vector2 &v )
{
	points_.push_back( v );
}


// portal2D.ipp
