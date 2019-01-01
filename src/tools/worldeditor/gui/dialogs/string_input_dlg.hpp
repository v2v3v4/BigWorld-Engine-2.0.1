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
 *	This class implements a genering single-string input dialog.
 */
class StringInputDlg : public CDialog
{
public:
	StringInputDlg( CWnd* pParent = NULL );   // standard constructor
	virtual ~StringInputDlg();

// Dialog Data
	enum { IDD = IDD_STRING_INPUT };

	void init( const std::wstring & caption, const std::wstring & label,
										int maxLen, const std::string & str );

	const std::string result() const { return bw_wtoutf8( (LPCTSTR)str_ ); }

protected:
	CString caption_;
	CString label_;
	CString str_;
	int maxLen_;

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
