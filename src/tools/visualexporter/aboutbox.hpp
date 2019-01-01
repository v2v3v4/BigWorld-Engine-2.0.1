/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ABOUTBOX_HPP
#define ABOUTBOX_HPP

#pragma warning ( disable : 4530 )

#include <iostream>

#include "mfxexp.hpp"

class AboutBox
{
public:
	AboutBox();
	~AboutBox();
	
	void display( HWND hWnd );

private:
	static BOOL CALLBACK dialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	AboutBox(const AboutBox&);
	AboutBox& operator=(const AboutBox&);

	friend std::ostream& operator<<(std::ostream&, const AboutBox&);
};

#endif
/*aboutbox.hpp*/