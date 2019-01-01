/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// base_camera.ipp

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif


/**
 *	This method returns the view matrix
 */
INLINE const Matrix & BaseCamera::view() const
{
	return view_;
}

/**
 *	This method returns the inverse view matrix
 */
INLINE const Matrix & BaseCamera::invView() const
{
	return invView_;
}


/**
 *	This method is a base class stub for handling key events
 */
INLINE bool BaseCamera::handleKeyEvent( const KeyEvent & )
{
	return false;
}


/**
 *	This method is a base class stub for handling mouse events
 */
INLINE bool BaseCamera::handleMouseEvent( const MouseEvent & )
{
	return false;
}


/**
 *	This method is a base class stub for handling axis events
 */
INLINE bool BaseCamera::handleAxisEvent( const AxisEvent & )
{
	return false;
}


// base_camera.ipp
