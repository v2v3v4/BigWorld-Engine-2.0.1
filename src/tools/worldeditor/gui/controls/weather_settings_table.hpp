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

#include "common/property_table.hpp"
#include "gizmo/general_properties.hpp"
#include "math/colour.hpp"
#include "data_section_proxy.hpp"
#include "worldeditor/framework/world_editor_app.hpp"
#include "worldeditor/scripting/we_python_adapter.hpp"


/**
 *	This class sub-classes the PropertyTable and provides specific
 *	property instantiation for weather systems.
 */
class WeatherSettingsTable : public PropertyTable
{
public:
	explicit WeatherSettingsTable( UINT dlgId );
	virtual ~WeatherSettingsTable();

protected:
	bool init( DataSectionPtr pSection = NULL );
	void initDragDrop();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg LRESULT OnUpdateControls(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSelectPropertyItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChangePropertyItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDblClkPropertyItem(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:	
	DataSectionPtr pSystem_;
	GeneralEditorPtr editor_;
	void onSettingChanged();
	CRect	dropTest( UalItemInfo* ii );
	bool	doDrop( UalItemInfo* ii );
	void tagNameToString( std::string& label );
};
