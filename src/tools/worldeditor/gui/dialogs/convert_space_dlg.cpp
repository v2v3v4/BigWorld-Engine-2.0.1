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
#include "worldeditor/gui/dialogs/convert_space_dlg.hpp"
#include "appmgr/options.hpp"
#include "terrain/terrain_settings.hpp"


IMPLEMENT_DYNAMIC(ConvertSpaceDlg, CDialog)

ConvertSpaceDlg::ConvertSpaceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(ConvertSpaceDlg::IDD, pParent)
{
}


ConvertSpaceDlg::~ConvertSpaceDlg()
{
}


void ConvertSpaceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONV_HEIGHTMAP_SIZE, heightMapSize_);
	DDX_Control(pDX, IDC_CONV_NORMALMAP_SIZE, normalMapSize_);
	DDX_Control(pDX, IDC_CONV_HOLEMAP_SIZE, holeMapSize_);
	DDX_Control(pDX, IDC_CONV_SHADOWMAP_SIZE, shadowMapSize_);
	DDX_Control(pDX, IDC_CONV_BLENDMAP_SIZE, blendMapSize_);
	DDX_Control(pDX, IDC_ICON_WARNING, iconWarning_);
	DDX_Control(pDX, IDCANCEL, buttonCancel_);
	DDX_Control(pDX, IDOK, buttonCreate_);
}


BEGIN_MESSAGE_MAP(ConvertSpaceDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// ConvertSpace message handlers

BOOL ConvertSpaceDlg::OnInitDialog()
{
	BW_GUARD;

	CDialog::OnInitDialog();

	INIT_AUTO_TOOLTIP();

	wchar_t buffer[64];
	int size = MIN_MAP_SIZE;
	while ( size <= MAX_MAP_SIZE )
	{
		bw_snwprintf(buffer, ARRAY_SIZE(buffer), L"%d", size );

		heightMapSize_.AddString( buffer );
		normalMapSize_.AddString( buffer );
		shadowMapSize_.AddString( buffer );

		size *= 2;
	}

	_itow( Options::getOptionInt( "terrain2/defaults/heightMapSize", 
		Terrain::TerrainSettings::defaults()->heightMapSize() ), buffer, 10 );
	heightMapSize_.SelectString( -1, buffer );
	_itow( Options::getOptionInt( "terrain2/defaults/normalMapSize", 
		Terrain::TerrainSettings::defaults()->normalMapSize() ), buffer, 10 );
	normalMapSize_.SelectString( -1, buffer );
	holeMapSize_.initInt( MIN_HOLE_MAP_SIZE, MAX_MAP_SIZE,
		Options::getOptionInt( "terrain2/defaults/holeMapSize", 
		Terrain::TerrainSettings::defaults()->holeMapSize() ) );
	holeMapSize_.setSilent( false );
	_itow( Options::getOptionInt( "terrain2/defaults/shadowMapSize", 
		Terrain::TerrainSettings::defaults()->shadowMapSize() ), buffer, 10 );
	shadowMapSize_.SelectString( -1, buffer );
	blendMapSize_.initInt( MIN_HOLE_MAP_SIZE, MAX_MAP_SIZE,
		Options::getOptionInt( "terrain2/defaults/blendMapSize", 
		Terrain::TerrainSettings::defaults()->blendMapSize() ) );
	blendMapSize_.setSilent( false );

	iconWarning_.SetIcon( LoadIcon( NULL, IDI_WARNING ) );

	UpdateData( FALSE );

	return TRUE;  // return TRUE unless you set the focus to a control
}


void ConvertSpaceDlg::OnBnClickedOk()
{
	BW_GUARD;

	// store the defaults
	if (GetFocus() != &buttonCreate_)
	{
		if (!holeMapSize_.isValidValue())
		{
			holeMapSize_.setSilent( true );
			holeMapSize_.SetFocus();
			holeMapSize_.setSilent( false );
			return;
		}

		if (!blendMapSize_.isValidValue())
		{
			blendMapSize_.setSilent( true );
			blendMapSize_.SetFocus();
			blendMapSize_.setSilent( false );
			return;
		}
	}

	CString buffer;
	heightMapSize_.GetLBText( heightMapSize_.GetCurSel(), buffer );
	Options::setOptionInt( "terrain2/defaults/heightMapSize", _wtoi( buffer.GetString() ) );
	normalMapSize_.GetLBText( normalMapSize_.GetCurSel(), buffer );
	Options::setOptionInt( "terrain2/defaults/normalMapSize", _wtoi( buffer.GetString() ) );
	Options::setOptionInt( "terrain2/defaults/holeMapSize", holeMapSize_.GetIntegerValue() );
	shadowMapSize_.GetLBText( shadowMapSize_.GetCurSel(), buffer );
	Options::setOptionInt( "terrain2/defaults/shadowMapSize", _wtoi( buffer.GetString() ) );
	Options::setOptionInt( "terrain2/defaults/blendMapSize", blendMapSize_.GetIntegerValue() );

	OnOK();
}
