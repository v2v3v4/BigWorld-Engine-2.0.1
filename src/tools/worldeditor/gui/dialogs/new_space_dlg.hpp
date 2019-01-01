/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef NEW_SPACE_DLG_HPP
#define NEW_SPACE_DLG_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/resource.h"
#include "controls/auto_tooltip.hpp"
#include "controls/image_control.hpp"
#include "controls/edit_numeric.hpp"
#include <afxwin.h>


class NewSpaceDlg : public CDialog
{
	DECLARE_DYNAMIC(NewSpaceDlg)

	DECLARE_AUTO_TOOLTIP( NewSpaceDlg, CDialog )

public:
	NewSpaceDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~NewSpaceDlg();

// Dialog Data
	enum { IDD = IDD_NEWSPACE };

private:
	CString createdSpace_;

	void initIntEdit(
		controls::EditNumeric& edit, int minVal, int maxVal, int val );

	void validateFile( CEdit& ctrl, bool isPath );
	bool validateDefaultTexture();
	void formatChunkToKms( CString& str );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit space_;
	controls::EditNumeric width_;
	controls::EditNumeric height_;
	CComboBox			  heightMapSize_;
	CComboBox			  normalMapSize_;
	controls::EditNumeric holeMapSize_;
	CComboBox			  shadowMapSize_;
	controls::EditNumeric blendMapSize_;
	CEdit defaultTexture_;
	controls::ImageControl32 textureImage_;
	CProgressCtrl progress_;
	CString defaultSpace_;
	std::string defaultSpacePath_;
	CStatic widthKms_;
	CStatic heightKms_;

	CString createdSpace() { return createdSpace_; }

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeSpace();
	afx_msg void OnEnChangeSpacePath();
	afx_msg void OnBnClickedNewspaceBrowse();
	afx_msg void OnEnTextureChange();
	afx_msg void OnBnTextureBrowse();
	afx_msg void OnEnChangeWidth();
	afx_msg void OnEnChangeHeight();
	afx_msg void OnBnClickedLegacy();
	CEdit spacePath_;
	CButton buttonCancel_;
	CButton buttonCreate_;
};


#endif // NEW_SPACE_DLG_HPP
