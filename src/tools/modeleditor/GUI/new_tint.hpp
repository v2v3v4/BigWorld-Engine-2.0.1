/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "resource.h"       // main symbols

class CNewTint : public CDialog
{
public:
	CNewTint( std::vector< std::string >& otherTintNames );
	virtual BOOL OnInitDialog();

	const std::string& tintName() { return tintName_; };
	const std::string& fxFile() { return fxFile_; }
	const std::string& mfmFile() { return mfmFile_; }

// Dialog Data
	enum { IDD = IDD_NEW_TINT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
private:
	std::vector< std::string >& tintNames_;
	
	CEdit name_;

	CButton fxCheck_;
	CButton mfmCheck_;

	CComboBox fxList_;
	CComboBox mfmList_;

	CButton fxSel_;
	CButton mfmSel_;

	std::string tintName_;
	std::string fxFile_;
	std::string mfmFile_;

	CButton ok_;

	void checkComplete();
	void redrawList( CComboBox& list, const std::string& name );

	afx_msg void OnEnChangeNewTintName();
	afx_msg void OnBnClickedNewTintMfmCheck();
	afx_msg void OnBnClickedNewTintFxCheck();
	afx_msg void OnCbnSelchangeNewTintFxList();
	afx_msg void OnCbnSelchangeNewTintMfmList();
	afx_msg void OnBnClickedNewTintFxSel();
	afx_msg void OnBnClickedNewTintMfmSel();

	void OnOK();
};
