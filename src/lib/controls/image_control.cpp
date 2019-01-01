/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "controls/image_control.hpp"


using namespace controls;


BEGIN_MESSAGE_MAP(ImageControl8, CWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


/*afx_msg*/ void ImageControl8::OnPaint()
{
	Base::OnPaint();
}


/*afx_msg*/ BOOL ImageControl8::OnEraseBkgnd(CDC *dc)
{
	BW_GUARD;

	return Base::OnEraseBkgnd(dc);
}


BEGIN_MESSAGE_MAP(ImageControl32, CWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


/*afx_msg*/ void ImageControl32::OnPaint()
{
	BW_GUARD;

	Base::OnPaint();
}


/*afx_msg*/ BOOL ImageControl32::OnEraseBkgnd(CDC *dc)
{
	BW_GUARD;

	return Base::OnEraseBkgnd(dc);
}
