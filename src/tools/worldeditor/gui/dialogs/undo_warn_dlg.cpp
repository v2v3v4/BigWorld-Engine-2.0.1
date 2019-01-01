/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#include "pch.hpp"
#include "undo_warn_dlg.hpp"


IMPLEMENT_DYNAMIC( UndoWarnDlg, CDialog )


UndoWarnDlg::UndoWarnDlg( CWnd * pParent ) :
	CDialog( UndoWarnDlg::IDD, pParent ),
	dontRepeat_( false )
{
}


UndoWarnDlg::~UndoWarnDlg()
{
}


void UndoWarnDlg::DoDataExchange( CDataExchange * pDX )
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_UNDOWARN_DONT_REPEAT, dontRepeatBtn_ );
}


BOOL UndoWarnDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;
}


void UndoWarnDlg::OnOK()
{
	BW_GUARD;

	if (dontRepeatBtn_.GetCheck() == BST_CHECKED)
	{
		dontRepeat_ = true;
	}
	CDialog::OnOK();
}


BEGIN_MESSAGE_MAP( UndoWarnDlg, CDialog )
END_MESSAGE_MAP()