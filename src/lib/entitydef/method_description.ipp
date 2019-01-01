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
#define INLINE	inline
#else
/// INLINE macro.
#define INLINE
#endif

// -----------------------------------------------------------------------------
// Section: Accessors
// -----------------------------------------------------------------------------

/**
 *	This method returns the priority associated with this method. It is used for
 *	deciding whether or not to send to the client.
 */
INLINE float MethodDescription::priority() const
{
	return priority_;
}

// method_description.ipp
