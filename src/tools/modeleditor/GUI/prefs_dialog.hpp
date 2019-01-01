/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "resource.h"

#include "controls/auto_tooltip.hpp"

class CPrefsDlg : public CDialog
{
	DECLARE_AUTO_TOOLTIP( CPrefsDlg, CDialog )

public:
	CPrefsDlg();

	enum { IDD = IDD_PREFS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	
public:
	virtual BOOL OnInitDialog();

	void OnOK();

private:
	afx_msg LRESULT OnShowTooltip(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHideTooltip(WPARAM wParam, LPARAM lParam);

	CButton showSplashScreen_;
	CButton loadLastModel_;
	CButton loadLastLights_;
	
	CButton zoomOnLoad_;
	CButton regenBBOnLoad_;

	CButton animateZoom_;
	CButton lockLOD_;
	CButton invertMouse_;
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedPrefsInvertMouse();
};