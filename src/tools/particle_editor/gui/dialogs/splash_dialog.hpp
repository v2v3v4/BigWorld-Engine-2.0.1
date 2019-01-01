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

#include "resource.h"
#include "resource_loader.hpp"

// CSplashDlg dialog

class CSplashDlg : public CDialog, public ISplashVisibilityControl
{
	DECLARE_DYNAMIC(CSplashDlg)

public:
	CSplashDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSplashDlg();

// Dialog Data
	enum { IDD = IDD_SPLASH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	static ISplashVisibilityControl* getSVC();
	static void ShowSplashScreen(CWnd* pParentWnd);
	void setSplashVisible( bool visible );
	void HideSplashScreen(void);
	static BOOL PreTranslateAppMessage(MSG* pMsg);
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
};

extern CSplashDlg* s_SplashDlg;
