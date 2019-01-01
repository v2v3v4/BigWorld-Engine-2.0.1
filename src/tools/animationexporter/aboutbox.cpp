/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#include "aboutbox.hpp"

#ifndef CODE_INLINE
#include "aboutbox.ipp"
#endif

AboutBox::AboutBox()
{
}

AboutBox::~AboutBox()
{
}

void AboutBox::display( HWND hWnd )
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, dialogProc, 0);
}

BOOL CALLBACK AboutBox::dialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		CenterWindow(hWnd, GetParent(hWnd)); 
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(hWnd, 1);
			break;
		}
		break;
		default:
			return FALSE;
	}
	return TRUE;
}

std::ostream& operator<<(std::ostream& o, const AboutBox& t)
{
	o << "AboutBox\n";
	return o;
}


/*aboutbox.cpp*/
