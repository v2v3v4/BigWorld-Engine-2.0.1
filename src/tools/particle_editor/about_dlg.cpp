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
#include "about_dlg.hpp"
#include "common/compile_time.hpp"
#include "common/tools_common.hpp"

#include "cstdmf/bwversion.hpp"

#include "resmgr/string_provider.hpp"

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

CAboutDlg::CAboutDlg() 
: 
CDialog(CAboutDlg::IDD),
m_background(),
m_font()
{
	BW_GUARD;

    m_background.LoadBitmap( IDB_ABOUT );
    m_font.CreatePointFont( 90, L"Arial", NULL );
}

BOOL CAboutDlg::OnInitDialog()
{
	BW_GUARD;

    CDialog::OnInitDialog();

    BITMAP bitmap;
    m_background.GetBitmap( &bitmap );
    RECT rect = { 0, 0, bitmap.bmWidth, bitmap.bmHeight };
    AdjustWindowRect( &rect, GetWindowLong( m_hWnd, GWL_STYLE ), FALSE );
    MoveWindow( &rect, FALSE );
    CenterWindow();
    SetCapture();

    return TRUE;  // Don't set focus to first control
}

void CAboutDlg::OnPaint()
{
	BW_GUARD;

    CPaintDC dc(this); // device context for painting
    CDC memDC;
    memDC.CreateCompatibleDC( &dc);
    CBitmap* saveBmp = memDC.SelectObject( &m_background );
    CFont* saveFont = memDC.SelectObject( &m_font );

    RECT client;
    GetClientRect( &client );

    dc.SetTextColor(0x00808080);
    dc.BitBlt(0, 0, client.right, client.bottom, &memDC, 0, 0, SRCCOPY);

    memDC.SelectObject( &saveBmp );
    memDC.SelectObject( &saveFont );
    
	std::wstring space = L" ";
	CString builtOn = Localise(L"PARTICLEEDITOR/GUI/ABOUT_BOX/VERSION_BUILT", BWVersion::versionString().c_str(),
		ToolsCommon::isEval() ? space + Localise(L"PARTICLEEDITOR/GUI/ABOUT_BOX/EVAL" ) : L"",
#ifdef _DEBUG
		space + Localise(L"PARTICLEEDITOR/GUI/ABOUT_BOX/DEBUG" ),
#else
		"",
#endif
		aboutCompileTimeString );

    dc.SetBkMode(TRANSPARENT);
    dc.ExtTextOut(70, 310, 0, NULL, builtOn, NULL);
}

void CAboutDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	BW_GUARD;

    CDialog::OnLButtonDown(nFlags, point);
    OnOK();
}

void CAboutDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	BW_GUARD;

    CDialog::OnRButtonDown(nFlags, point);
    OnOK();
}