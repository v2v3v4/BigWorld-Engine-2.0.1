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

class CTextureFeed : public CDialog
{
public:
	enum { IDD = IDD_TEXTURE_FEED };
	
	CTextureFeed( const std::wstring& feedName );

	virtual BOOL OnInitDialog();

	std::wstring feedName() { return feedName_; }


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	
private:
	void checkComplete();

	std::wstring feedName_;
	
	CEdit name_;
	
	CButton ok_;


public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnEnChangeTextureFeedName();
	afx_msg void OnBnClickedRemoveTextureFeed();
};