/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VIEW_DRAW_UTILS_HPP
#define VIEW_DRAW_UTILS_HPP


/**
 *	This class contains a set of utility functions useful for drawing in the
 *	post-processing panel.
 */
class ViewDrawUtils
{
public:
	static void drawBoxConection( CDC & dc, const CRect & rectStart,
					const CRect & rectEnd, CRect & retRect, COLORREF colour );

	static void drawBitmap( CDC & dc, CBitmap & bmp, int x, int y, int w, int h, bool forceTransparent = false );

	static void drawRect( CDC & dc, const CRect & rect, CBrush & brush, CPen & pen, int roundSize );
};


#endif // VIEW_DRAW_UTILS_HPP
