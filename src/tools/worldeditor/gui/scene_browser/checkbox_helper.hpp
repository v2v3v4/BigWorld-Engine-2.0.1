/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHECKBOX_HELPER_HPP
#define CHECKBOX_HELPER_HPP


class CheckboxHelper
{
public:
	void init( int checkedResId, int uncheckedResId,
		COLORREF fgSel, COLORREF bgSel, COLORREF fgEven, COLORREF bgEven,
		COLORREF fgOdd, COLORREF bgOdd );

	void draw( CDC & dc, const CRect & rect, const CPoint & pt,
									bool selected, bool odd, bool checked );

	void rect( CRect & retRect ) const;

private:
	CBitmap checkOnOdd_;
	CBitmap checkOnEven_;
	CBitmap checkOnSel_;
	CBitmap checkOffOdd_;
	CBitmap checkOffEven_;
	CBitmap checkOffSel_;
};

#endif // CHECKBOX_HELPER_HPP
