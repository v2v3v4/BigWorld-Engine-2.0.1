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


/**
 *	This class implements a simple dialog used to change the name of an
 *	Asset Browser dialog.
 */
class UalNameDlg : public CDialog
{
public:
	UalNameDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~UalNameDlg();

// Dialog Data
	enum { IDD = IDD_UALNAME };

	void getNames( std::wstring& shortName, std::wstring& longName );
	void setNames( const std::wstring& shortName, const std::wstring& longName );

protected:
	CString longName_;
	CString shortName_;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void OnOK();

	DECLARE_MESSAGE_MAP()
};
