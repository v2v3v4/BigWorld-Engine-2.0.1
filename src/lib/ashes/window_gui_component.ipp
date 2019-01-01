/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// window.ipp

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif

INLINE ConstSimpleGUIComponentPtr WindowGUIComponent::nearestRelativeParent(int depth) const
{
	if (depth > 0)
	{
		return ConstSimpleGUIComponentPtr(this);
	}
	
	return SimpleGUIComponent::nearestRelativeParent(depth);
}


// window.ipp