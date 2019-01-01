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


class GeneralEditor;
typedef SmartPointer<GeneralEditor> GeneralEditorPtr;


class GraphicsSettingsTable : public PropertyTable
{
public:
	explicit GraphicsSettingsTable( UINT dlgId );
	virtual ~GraphicsSettingsTable();

	bool needsRestart() const { return needsRestart_; }
	virtual void needsRestart( const std::string& setting );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg LRESULT OnUpdateControls(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSelectPropertyItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChangePropertyItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDblClkPropertyItem(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	bool inited_; 
	GeneralEditorPtr editor_;
	bool needsRestart_;
	std::set<std::string> changedSettings_;

	bool init();
	void tagNameToString( std::string& label );
};