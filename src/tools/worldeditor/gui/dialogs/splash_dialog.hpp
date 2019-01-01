/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPLASH_DIALOG_HPP
#define SPLASH_DIALOG_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/resource.h"
#include "common/resource_loader.hpp"


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
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	void setSplashVisible( bool visible );

	static ISplashVisibilityControl* getSVC();
	static void ShowSplashScreen(CWnd* pParentWnd);
	static void HideSplashScreen(void);
	static bool visibleInfo( CRect* rect );
	static BOOL PreTranslateAppMessage(MSG* pMsg);
};

extern CSplashDlg* s_SplashDlg;


#endif // SPLASH_DIALOG_HPP
