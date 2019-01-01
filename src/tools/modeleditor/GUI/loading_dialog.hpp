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

// CModelessInfoDialog

class CLoadingDialog : public CDialog
{
public:
	CLoadingDialog( const std::wstring& fileName );
	~CLoadingDialog();

	void setRange( int num );

	void step();

// Dialog Data
	enum { IDD = IDD_LOADING };

protected:
	void DoDataExchange( CDataExchange* pDX );
	BOOL OnInitDialog();

	 std::wstring fileName_;

	 CProgressCtrl bar_;

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};
