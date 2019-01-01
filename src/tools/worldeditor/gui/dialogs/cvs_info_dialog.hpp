/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CVS_INFO_DIALOG_HPP
#define CVS_INFO_DIALOG_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/resource.h"
#include "worldeditor/misc/cvswrapper.hpp"


class CVSInfoDialog : public CDialog, public CVSLog
{
public:
	CVSInfoDialog( const std::wstring& title );
	~CVSInfoDialog();

	virtual void add( const std::wstring& msg );

// Dialog Data
	enum { IDD = IDD_MODELESS_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	std::wstring title_;

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};


#endif // CVS_INFO_DIALOG_HPP
