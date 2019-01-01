/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#pragma once

#include "controls/defs.hpp"
#include "controls/fwd.hpp"


/////////////////////////////////////////////////////////////////////////////
// Slider

namespace controls
{
	class CONTROLS_DLL Slider : public CSliderCtrl
	{
	
	public:
		Slider();
		virtual ~Slider();
	
		bool doUpdate()
		{
			bool temp = released_;
			released_ = false;
			return temp;
		}
	
	private:
		bool lButtonDown_;
		bool released_;
	
	protected:
		DECLARE_MESSAGE_MAP()
	public:
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	};
}
