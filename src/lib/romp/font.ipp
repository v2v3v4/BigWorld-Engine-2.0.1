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

INLINE FontMetrics& Font::metrics()
{
	return metrics_;
}


INLINE const FontMetrics& Font::metrics() const
{
	return metrics_;
}


INLINE void Font::fitToScreen( bool state, const Vector2& numCharsXY )
{
	fitToScreen_ = state;
	numCharsXY_ = numCharsXY;
}


INLINE bool Font::fitToScreen() const
{
	return fitToScreen_;
}
