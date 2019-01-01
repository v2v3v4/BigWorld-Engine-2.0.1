/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// slider.cpp : implementation file
//

#include "pch.hpp"
#include "slider.hpp"

using namespace controls;

// Slider

Slider::Slider()
: lButtonDown_( false )
{
}

Slider::~Slider()
{
}


BEGIN_MESSAGE_MAP(Slider, CSliderCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// Slider message handlers

void Slider::OnLButtonDown(UINT nFlags, CPoint point)
{
	BW_GUARD;

	CRect rect;
	GetChannelRect(&rect);
	CRect tRect;
	GetThumbRect(&tRect);
	int left = rect.TopLeft().x;
	int right = rect.BottomRight().x;
	int offset = (point.x - left);
	int rOffset = (right - point.x);

	float tHalfWidth = ( tRect.right - tRect.left ) / 2.f;

	int pos;
	if( offset < tHalfWidth )
		pos = GetRangeMin();
	else if( right - offset < tHalfWidth )
		pos = GetRangeMax();
	else
	{
		float portion = ( offset - tHalfWidth ) / (float)(right - tHalfWidth * 2 - left);
		pos = GetRangeMin() + (int)((GetRangeMax() - GetRangeMin()) * portion + 0.5 );
	}
	SetPos( pos );
	GetParent()->SendMessage(WM_HSCROLL, MAKEWPARAM(TB_ENDTRACK, (short)pos), (LPARAM)GetSafeHwnd());

	lButtonDown_ = true;

	CSliderCtrl::OnLButtonDown(nFlags, point);
}

void Slider::OnLButtonUp(UINT nFlags, CPoint point)
{
	BW_GUARD;

	if (lButtonDown_)
		released_ = true;
		
	lButtonDown_ = false;

	CSliderCtrl::OnLButtonUp(nFlags, point);
}
