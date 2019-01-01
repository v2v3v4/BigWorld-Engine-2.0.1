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

// closed_captions.ipp

/**
 *	This method sets the visibility of the closed captions.
 *
 *	@param	state	The new visibility state.
 */
INLINE void ClosedCaptions::visible( bool state )
{
	root_->visible( state );
}


/**
 *	This method returns the visibility of the closed captions.
 *
 *	@return True if the closed captions are visibile.
 */
INLINE bool ClosedCaptions::visible() const
{
	return root_->visible();
}

// closed_captions.ipp
