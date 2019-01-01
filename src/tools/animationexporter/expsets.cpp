/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#pragma warning ( disable: 4503 )
#pragma warning ( disable: 4786 )


#include "expsets.hpp"
#include <string>

// max includes
#include "max.h"

// exporter resources
#include "resource.h"

//the instance
extern HINSTANCE hInstance;


ExportSettings::ExportSettings()
: staticFrame_( 0 ),
  frameFirst_( 0 ),
  frameLast_( 100 ),
  nodeFilter_( VISIBLE ),
  allowScale_( false ),
  exportMorphAnimation_( true ),
  exportNodeAnimation_( true ),
  exportCueTrack_( false ),
  useLegacyOrientation_( false )
{
}

ExportSettings::~ExportSettings()
{
}

bool ExportSettings::displayDialog( HWND hWnd )
{
	if (!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MFEXPORT_DLG), hWnd, dialogProc, (LPARAM)this)) 
	{
		return false;
	}
	return true;
}

void ExportSettings::displayReferenceHierarchyFile( HWND hWnd ) const
{
	std::string displayName("");
	if (!this->referenceNodesFile_.empty())
	{
		displayName = "( " + this->referenceNodesFile_ + " )";
	}
	SetDlgItemText( hWnd, IDC_REFERENCEHIERARCHYFILE,
		this->referenceNodesFile_.c_str() );
	SetDlgItemText( hWnd, IDC_REFERENCEHIERARCHY_FILENAME, displayName.c_str() );
}

BOOL CALLBACK ExportSettings::dialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
//	ISpinnerControl  *spin;

	ExportSettings *settings = (ExportSettings*)GetWindowLong(hWnd,GWL_USERDATA); 

	switch (msg)
	{
	case WM_INITDIALOG:
		settings = (ExportSettings*)lParam;
		SetWindowLong(hWnd,GWL_USERDATA,lParam); 
		CenterWindow(hWnd, GetParent(hWnd)); 

		CheckDlgButton( hWnd, IDC_ALLOWSCALE, settings->allowScale_ );
		CheckDlgButton( hWnd, IDC_EXPORTMORPHANIMATION, settings->exportMorphAnimation_ );
		CheckDlgButton( hWnd, IDC_EXPORTNODEANIMATION, settings->exportNodeAnimation_ );

		CheckDlgButton( hWnd, IDC_REFERENCEHIERARCHY,
            settings->referenceNodesFile_ != "");
		settings->displayReferenceHierarchyFile( hWnd );

		CheckDlgButton( hWnd, IDC_CUETRACK, settings->exportCueTrack_ );
		CheckDlgButton( hWnd, IDC_LEGACY_ORIENTATION, settings->useLegacyOrientation_ );
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{
		case IDC_REFERENCEHIERARCHY:
			if (IsDlgButtonChecked(hWnd, IDC_REFERENCEHIERARCHY) != TRUE)
			{
				settings->referenceNodesFile_ = "";
			}
			else
			{
				settings->referenceNodesFile_ = settings->getReferenceFile( hWnd );
			}

			CheckDlgButton( hWnd, IDC_REFERENCEHIERARCHY,
				settings->referenceNodesFile_ != "");

			settings->displayReferenceHierarchyFile( hWnd );

			break;

		case IDOK:
			settings->allowScale_ = IsDlgButtonChecked( hWnd, IDC_ALLOWSCALE ) == TRUE;
			settings->exportMorphAnimation_ = IsDlgButtonChecked( hWnd, IDC_EXPORTMORPHANIMATION ) == TRUE;
			settings->exportNodeAnimation_ = IsDlgButtonChecked( hWnd, IDC_EXPORTNODEANIMATION ) == TRUE;

			// store whether we should export cue tracks or not
			settings->exportCueTrack_ = IsDlgButtonChecked( hWnd, IDC_CUETRACK ) == BST_CHECKED;

			settings->useLegacyOrientation_ = IsDlgButtonChecked( hWnd, IDC_LEGACY_ORIENTATION ) == TRUE;

			EndDialog(hWnd, 1);
			break;

		case IDSETTINGS:
			DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SETTINGSDIALOG), hWnd, settingsDialogProc, (LPARAM)settings);
			break;

		case IDCANCEL:
			EndDialog(hWnd, 0);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

BOOL CALLBACK ExportSettings::settingsDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ExportSettings *settings = (ExportSettings*)GetWindowLong(hWnd,GWL_USERDATA); 

	std::string typeDesc;

	switch (msg)
	{
	case WM_INITDIALOG:
		{
		settings = (ExportSettings*)lParam;
		SetWindowLong(hWnd,GWL_USERDATA,lParam); 
		CenterWindow(hWnd, GetParent(hWnd)); 		
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{
		case IDOK:
			{
			EndDialog(hWnd, 1);
			}
			break;
		case IDCANCEL:
			EndDialog(hWnd, 0);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

std::string ExportSettings::getReferenceFile( HWND hWnd )
{
	OPENFILENAME	ofn;
	memset( &ofn, 0, sizeof(ofn) );

	char * filters = "Visual Files\0*.visual\0\0";

	char	filename[512] = "";

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filters;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrTitle = "Select reference hierarchy file";
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |
		OFN_NOCHANGEDIR | OFN_READONLY | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "MFX";


	if (!GetOpenFileName( &ofn )) return "";

	return filename;
}

ExportSettings& ExportSettings::instance()
{
	static ExportSettings expsets;
	return expsets;
}


std::ostream& operator<<(std::ostream& o, const ExportSettings& t)
{
	o << "ExportSettings\n";
	return o;
}

float ExportSettings::unitScale( ) const
{
	return (float)GetMasterScale( UNITS_CENTIMETERS ) / 100;
}
