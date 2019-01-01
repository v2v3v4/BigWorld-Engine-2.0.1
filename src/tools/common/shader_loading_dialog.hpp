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

// CShaderLoadingDialog dialog
class CShaderLoadingDialog : public CDialog
{
public:
	CShaderLoadingDialog();
	virtual ~CShaderLoadingDialog();

	void setRange( int num );
	void step();

// Dialog Data
	enum { IDD = IDD_SHADER_LOADING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();

	CProgressCtrl bar_;

	DECLARE_MESSAGE_MAP()
};
