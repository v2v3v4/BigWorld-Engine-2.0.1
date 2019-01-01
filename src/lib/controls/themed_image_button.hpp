/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *	Class that draws an overlay image over the button, allowing image buttons
 *	with XP/Vista themes.
 */
#pragma once

namespace controls
{

	class ThemedImageButton : public CButton
	{
		DECLARE_DYNAMIC(ThemedImageButton)

	public:
		ThemedImageButton();
		~ThemedImageButton();

		HICON SetIcon( HICON hIcon );
		HICON SetCheckedIcon( HICON hIcon );

	protected:
		DECLARE_MESSAGE_MAP()
		afx_msg void OnNotifyCustomDraw ( NMHDR * pNotifyStruct, LRESULT* result );

	private:
		HICON hIcon_;
		HICON hCheckedIcon_;
	};

}