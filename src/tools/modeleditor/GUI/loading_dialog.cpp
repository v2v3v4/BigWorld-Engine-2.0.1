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

#include "loading_dialog.hpp"

CLoadingDialog::CLoadingDialog( const std::wstring& fileName ) :
CDialog(CLoadingDialog::IDD),
fileName_( fileName )
{
	BW_GUARD;

	Create( IDD_LOADING );
}

CLoadingDialog::~CLoadingDialog()
{
	BW_GUARD;

	DestroyWindow();
}

void CLoadingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_BAR, bar_);

}

BOOL CLoadingDialog::OnInitDialog() 
{
	BW_GUARD;

   CDialog::OnInitDialog();
   
   SetWindowText( fileName_.c_str() );

   return TRUE;
}

void CLoadingDialog::setRange( int num )
{
	BW_GUARD;

	bar_.SetRange( 0, num );
	bar_.SetStep( 1 );
}

void CLoadingDialog::step()
{
	BW_GUARD;

	bar_.StepIt();
}

BEGIN_MESSAGE_MAP(CLoadingDialog, CDialog)
END_MESSAGE_MAP()