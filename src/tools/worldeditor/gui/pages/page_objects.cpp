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
#include "worldeditor/framework/world_editor_app.hpp"
#include "worldeditor/world/world_manager.hpp"
#include "worldeditor/framework/mainframe.hpp"
#include "worldeditor/gui/pages/page_objects.hpp"
#include "worldeditor/scripting/we_python_adapter.hpp"
#include "worldeditor/misc/placement_presets.hpp"
#include "appmgr/options.hpp"
#include "common/user_messages.hpp"
#include "guimanager/gui_manager.hpp"


DECLARE_DEBUG_COMPONENT( 0 )


// PageObjects


// GUITABS content ID ( declared by the IMPLEMENT_BASIC_CONTENT macro )
const std::wstring PageObjects::contentID = L"PageObjects";


PageObjects::PageObjects()
	: CFormView(PageObjects::IDD),
	resizeReady_( false ),
	pageReady_( false ),
	lastCoordType_(""),
	lastSnapType_( -1 ),
	lastDragOnSelect_( -1 ),
	lastGridSnap_( -1 )
{
	BW_GUARD;

	snapsX_.SetMinimum(0, false);
	snapsY_.SetMinimum(0, false);
	snapsZ_.SetMinimum(0, false);
	snapsAngle_.SetMinimum(0, false);
}

PageObjects::~PageObjects()
{
	BW_GUARD;

	PlacementPresets::instance()->removeComboBox( &placementMethod_ );
}

void PageObjects::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_OBJECTS_SELECTION_FILTER, selectionFilter_);

	DDX_Control(pDX, IDC_OBJECTS_COORDS_WORLD, worldCoords_);
	DDX_Control(pDX, IDC_OBJECTS_COORDS_LOCAL, localCoords_);
	DDX_Control(pDX, IDC_OBJECTS_COORDS_VIEW, viewCoords_);

	DDX_Control(pDX, IDC_OBJECTS_LOCK_FREE, freeSnap_);
	DDX_Control(pDX, IDC_OBJECTS_LOCK_TERRAIN, terrainSnap_);
	DDX_Control(pDX, IDC_OBJECTS_LOCK_OBSTACLE, obstacleSnap_);

	DDX_Control(pDX, IDC_OBJECTS_LOCK_TO_GRID, gridSnap_);
	DDX_Control(pDX, IDC_OPTIONS_SNAPS_X, snapsX_);
	DDX_Control(pDX, IDC_OPTIONS_SNAPS_Y, snapsY_);
	DDX_Control(pDX, IDC_OPTIONS_SNAPS_Z, snapsZ_);
	DDX_Control(pDX, IDC_OPTIONS_SNAPS_ANGLE, snapsAngle_);

	DDX_Control(pDX, IDC_OBJECTS_DRAG_ON_SELECT, dragOnSelect_);

	resizeReady_ = true;
	DDX_Control(pDX, IDC_OBJECTS_SELECT_PLACEMENT_SETTING, placementMethod_);
}

void PageObjects::InitPage()
{
	BW_GUARD;

	//Handle page initialisation here...

	if (WorldEditorApp::instance().pythonAdapter())
	{
		WorldEditorApp::instance().pythonAdapter()->selectFilterUpdate( &selectionFilter_ );
	}

	Vector3 movementSnap(1.f, 1.f, 1.f);
	movementSnap = Options::getOptionVector3("snaps/movement", movementSnap);
	snapsX_.SetValue(movementSnap.x);
	snapsY_.SetValue(movementSnap.y);
	snapsZ_.SetValue(movementSnap.z);

	float snapAngle(1.f);
	snapAngle = Options::getOptionFloat("snaps/angle", snapAngle);
	snapsAngle_.SetValue(snapAngle);

	PlacementPresets::instance()->addComboBox( &placementMethod_ );
	PlacementPresets::instance()->readPresets();

	INIT_AUTO_TOOLTIP();
	
	pageReady_ = true;
}


BEGIN_MESSAGE_MAP(PageObjects, CFormView)
	ON_WM_SIZE()
	ON_MESSAGE (WM_ACTIVATE_TOOL, OnActivateTool)
	ON_MESSAGE(WM_UPDATE_CONTROLS, OnUpdateControls)
	ON_WM_HSCROLL()

	ON_MESSAGE(WM_SHOW_TOOLTIP, OnShowTooltip)
	ON_MESSAGE(WM_HIDE_TOOLTIP, OnHideTooltip)

	ON_CBN_SELCHANGE(IDC_OBJECTS_SELECTION_FILTER, OnCbnSelchangeObjectsSelectionFilter)

	ON_BN_CLICKED(IDC_OBJECTS_COORDS_WORLD, OnBnClickedObjectsCoordsWorld)
	ON_BN_CLICKED(IDC_OBJECTS_COORDS_LOCAL, OnBnClickedObjectsCoordsLocal)
	ON_BN_CLICKED(IDC_OBJECTS_COORDS_VIEW, OnBnClickedObjectsCoordsView)

	ON_BN_CLICKED(IDC_OBJECTS_LOCK_FREE, OnBnClickedObjectsLockFree)
	ON_BN_CLICKED(IDC_OBJECTS_LOCK_TERRAIN, OnBnClickedObjectsLockTerrain)
	ON_BN_CLICKED(IDC_OBJECTS_LOCK_OBSTACLE, OnBnClickedObjectsLockObstacle)

	ON_BN_CLICKED(IDC_OBJECTS_LOCK_TO_GRID, OnBnClickedObjectsLockToGrid)
	ON_BN_CLICKED(IDC_OPTIONS_SHELLSNAPS, OnBnClickedOptionsShellsnaps)
	ON_BN_CLICKED(IDC_OPTIONS_UNITSNAPS, OnBnClickedOptionsUnitsnaps)
	ON_BN_CLICKED(IDC_OPTIONS_POINTONESNAPS, OnBnClickedOptionsPointonesnaps)

	ON_MESSAGE(WM_EDITNUMERIC_CHANGE, OnChangeEditNumeric)

	ON_BN_CLICKED(IDC_OBJECTS_DRAG_ON_SELECT, OnBnClickedObjectsDragOnSelect)

	ON_CBN_SELCHANGE(IDC_OBJECTS_SELECT_PLACEMENT_SETTING, OnCbnSelchangeObjectsSelectPlacementSetting)
	ON_BN_CLICKED(IDC_OBJECTS_EDIT_PLACEMENT_SETTINGS, OnBnClickedObjectsEditPlacementSettings)
END_MESSAGE_MAP()


LRESULT PageObjects::OnActivateTool(WPARAM wParam, LPARAM lParam)
{
	BW_GUARD;

	const wchar_t *activePageId = (const wchar_t*)(wParam);
	if (getContentID() == activePageId)
	{
		if (WorldEditorApp::instance().pythonAdapter())
		{
			WorldEditorApp::instance().pythonAdapter()->onPageControlTabSelect("pgc", "Object");
		}
	}
	return 0;
}

void PageObjects::stretchToRight( CWnd& widget, int pageWidth, int border )
{
	BW_GUARD;

	CRect rect;
	widget.GetWindowRect( &rect );
    ScreenToClient( &rect );
	widget.SetWindowPos( 0, rect.left, rect.top, pageWidth - rect.left - border, rect.Height(), SWP_NOZORDER );
	widget.RedrawWindow();
}

afx_msg void PageObjects::OnSize( UINT nType, int cx, int cy )
{
	BW_GUARD;

	CFormView::OnSize( nType, cx, cy );

	if ( !resizeReady_ )
		return;
		
	//stretchToRight( selectionFilter_, cx, 12 );
}

void PageObjects::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	BW_GUARD;

	// this captures all the scroll events for the page
	if ( !pageReady_ )
		InitPage();

	//Handle h-scroll events here...

	CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
}

afx_msg LRESULT PageObjects::OnShowTooltip(WPARAM wParam, LPARAM lParam)
{
	char* msg = (char*)lParam;
	//MainFrame::instance().SetMessageText( msg );
	return 0;
}

afx_msg LRESULT PageObjects::OnHideTooltip(WPARAM wParam, LPARAM lParam)
{
	//MainFrame::instance().SetMessageText( "" );
	return 0;
}


afx_msg LRESULT PageObjects::OnUpdateControls(WPARAM wParam, LPARAM lParam)
{
	BW_GUARD;

	if ( !pageReady_ )
		InitPage();

	//Ensure the correct coordinate system is selected
	std::string coordType = Options::getOptionString( "tools/coordFilter" );
	if ( coordType != lastCoordType_)
	{
		worldCoords_.SetCheck( coordType == "World" ? BST_CHECKED : BST_UNCHECKED );
		localCoords_.SetCheck( coordType == "Local" ? BST_CHECKED : BST_UNCHECKED );
		viewCoords_.SetCheck( coordType == "View" ? BST_CHECKED : BST_UNCHECKED );
		lastCoordType_ = coordType;
	}

	//Ensure the correct snap type is selected
	int snapType = Options::getOptionInt( "snaps/itemSnapMode");
	if ( snapType != lastSnapType_)
	{
		freeSnap_.SetCheck( snapType == 0 ? BST_CHECKED : BST_UNCHECKED );
		terrainSnap_.SetCheck( snapType == 1 ? BST_CHECKED : BST_UNCHECKED );
		obstacleSnap_.SetCheck( snapType == 2 ? BST_CHECKED : BST_UNCHECKED );
		lastSnapType_ = snapType;
	}

	//Update the drag on select checkbox
	int dragOnSelect = Options::getOptionInt( "dragOnSelect" );
	if ( dragOnSelect != lastDragOnSelect_)
	{
		dragOnSelect_.SetCheck( dragOnSelect == 1 ? BST_CHECKED : BST_UNCHECKED );
		lastDragOnSelect_ = dragOnSelect;
	}

	//Update the grid snap checkbox
	int gridSnap = Options::getOptionInt( "snaps/xyzEnabled");
	if ( gridSnap != lastGridSnap_)
	{
		gridSnap_.SetCheck( gridSnap == 1 ? BST_CHECKED : BST_UNCHECKED );
		lastGridSnap_ = gridSnap;
	}

	if (!selectionFilter_.GetDroppedState() && 
		WorldEditorApp::instance().pythonAdapter())
	{
		WorldEditorApp::instance().pythonAdapter()->selectFilterUpdate( &selectionFilter_ );
	}
	return 0;
}

void PageObjects::OnCbnSelchangeObjectsSelectionFilter()
{
	BW_GUARD;

	CString sel;
	int index = selectionFilter_.GetCurSel();
	selectionFilter_.GetLBText( index, sel );

	if (WorldEditorApp::instance().pythonAdapter())
	{
		WorldEditorApp::instance().pythonAdapter()->selectFilterChange( bw_wtoutf8( sel.GetString() ));
	}
}

void PageObjects::OnBnClickedObjectsCoordsWorld()
{
	BW_GUARD;

	if (WorldEditorApp::instance().pythonAdapter())
	{
		WorldEditorApp::instance().pythonAdapter()->coordFilterChange( "World" );
	}
}

void PageObjects::OnBnClickedObjectsCoordsLocal()
{
	BW_GUARD;

	if (WorldEditorApp::instance().pythonAdapter())
	{
		WorldEditorApp::instance().pythonAdapter()->coordFilterChange( "Local" );
	}
}

void PageObjects::OnBnClickedObjectsCoordsView()
{
	BW_GUARD;

	if (WorldEditorApp::instance().pythonAdapter())
	{
		WorldEditorApp::instance().pythonAdapter()->coordFilterChange( "View" );
	}
}
	
void PageObjects::OnBnClickedObjectsLockFree()
{
	BW_GUARD;

	if (WorldEditorApp::instance().pythonAdapter())
	{
		WorldEditorApp::instance().pythonAdapter()->ActionScriptExecute("actXZSnap");
	}
}

void PageObjects::OnBnClickedObjectsLockTerrain()
{
	BW_GUARD;

	if (WorldEditorApp::instance().pythonAdapter())
	{
		WorldEditorApp::instance().pythonAdapter()->ActionScriptExecute("actTerrainSnaps");
	}
}

void PageObjects::OnBnClickedObjectsLockObstacle()
{
	BW_GUARD;

	if (WorldEditorApp::instance().pythonAdapter())
	{
		WorldEditorApp::instance().pythonAdapter()->ActionScriptExecute("actObstacleSnap");
	}
}

void PageObjects::OnBnClickedObjectsDragOnSelect()
{
	BW_GUARD;

	if (WorldEditorApp::instance().pythonAdapter())
	{
		WorldEditorApp::instance().pythonAdapter()->ActionScriptExecute("actDragOnSelect");
	}
}

void PageObjects::OnBnClickedObjectsLockToGrid()
{
	BW_GUARD;

	if (WorldEditorApp::instance().pythonAdapter())
	{
		WorldEditorApp::instance().pythonAdapter()->ActionScriptExecute("actToggleSnaps");
	}
}

void PageObjects::OnBnClickedOptionsShellsnaps()
{
	BW_GUARD;

	Vector3 shellSnap(4.f, 1.f, 4.f);
	shellSnap = Options::getOptionVector3("shellSnaps/movement", shellSnap);

	snapsX_.SetValue(shellSnap.x);
	snapsY_.SetValue(shellSnap.y);
	snapsZ_.SetValue(shellSnap.z);

	float shellSnapAngle(90.f);
	shellSnapAngle = Options::getOptionFloat("shellSnaps/angle", shellSnapAngle);
	snapsAngle_.SetValue(shellSnapAngle);

	OnChangeEditNumeric(0, 0);
}

void PageObjects::OnBnClickedOptionsUnitsnaps()
{
	BW_GUARD;

	snapsX_.SetValue(1.f);
	snapsY_.SetValue(1.f);
	snapsZ_.SetValue(1.f);
	snapsAngle_.SetValue(1.f);

	OnChangeEditNumeric(0, 0);
}

void PageObjects::OnBnClickedOptionsPointonesnaps()
{
	BW_GUARD;

	snapsX_.SetValue(0.1f);
	snapsY_.SetValue(0.1f);
	snapsZ_.SetValue(0.1f);
	snapsAngle_.SetValue(1.f);

	OnChangeEditNumeric(0, 0);
}

LRESULT PageObjects::OnChangeEditNumeric(WPARAM mParam, LPARAM lParam)
{
	BW_GUARD;

	Vector3 movemenSnap(snapsX_.GetValue(), snapsY_.GetValue(), snapsZ_.GetValue());
	Options::setOptionVector3("snaps/movement", movemenSnap);
	Options::setOptionFloat("snaps/angle", snapsAngle_.GetValue());
	return 0;
}

void PageObjects::OnCbnSelchangeObjectsSelectPlacementSetting()
{
	BW_GUARD;

	PlacementPresets::instance()->currentPreset( &placementMethod_ );
}

void PageObjects::OnBnClickedObjectsEditPlacementSettings()
{
	BW_GUARD;

	PlacementPresets::instance()->showDialog();
}
