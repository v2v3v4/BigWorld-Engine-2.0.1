/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ABOUT_DLG_HPP
#define ABOUT_DLG_HPP

#include "resource.h"

class CAboutDlg : public CDialog
{
public:
    enum { IDD = IDD_ABOUTBOX };

    CAboutDlg();

protected:
    /*virtual*/ BOOL OnInitDialog();

    afx_msg void OnPaint();

    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

    DECLARE_MESSAGE_MAP()

private:
    CBitmap         m_background;
    CFont           m_font;
};

#endif // ABOUT_DLG_HPP
