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


#include "worldeditor/resource.h"


/**
 *	This dialog warns the user that Process Data might clear the Undo/Redo
 *	stack.
 */
class UndoWarnDlg : public CDialog
{
	DECLARE_DYNAMIC( UndoWarnDlg )

public:
	enum { IDD = IDD_UNDOWARN };

	UndoWarnDlg( CWnd * pParent = NULL );
	virtual ~UndoWarnDlg();

	bool dontRepeat() const { return dontRepeat_; }

protected:
	virtual void DoDataExchange( CDataExchange * pDX );

	BOOL OnInitDialog();
	void OnOK();

	DECLARE_MESSAGE_MAP()

	bool dontRepeat_;
	CButton dontRepeatBtn_;
};
