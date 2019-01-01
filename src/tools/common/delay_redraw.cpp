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
#include "delay_redraw.hpp"

std::map< CWnd*, int > DelayRedraw::s_counter_;

DelayRedraw::DelayRedraw( CWnd* wnd ):
	wnd_(wnd)
{
	BW_GUARD;

	if (s_counter_[wnd_]++ == 0)
	{
		wnd_->SetRedraw( FALSE );
	}
}

DelayRedraw::~DelayRedraw()
{
	BW_GUARD;

	if (--s_counter_[wnd_] <= 0)
	{
		wnd_->SetRedraw( TRUE );
		wnd_->Invalidate(); 
	}
}
