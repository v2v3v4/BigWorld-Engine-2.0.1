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
#define INLINE    inline
#else
/// INLINE macro.
#define INLINE
#endif

// -----------------------------------------------------------------------------
// Section: Accessors
// -----------------------------------------------------------------------------



/**
 *	This method returns the name of this description.
 */
INLINE const std::string& BaseUserDataObjectDescription::name() const
{
	return name_;
}

// base_user_data_object_description.ipp
