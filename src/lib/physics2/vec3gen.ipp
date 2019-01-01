/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *	@file
 */

#ifdef CODE_INLINE
#define INLINE inline
#else
/// INLINE macro
#define INLINE
#endif

/**
 *	Destructor.
 */
INLINE
Vector3Generator::~Vector3Generator()
{
}


/**
 *	This method sets the start of the line.
 */
INLINE
void LineV3Gen::setSource(const Vector3 & source)
{
	start_ = source;
}


/**
 *	This method sets the centre of the source base of the cylinder.
 */
INLINE
void CylinderV3Gen::setSource(const Vector3 & source)
{
	source_ = source;
}

// vec3gen.ipp
