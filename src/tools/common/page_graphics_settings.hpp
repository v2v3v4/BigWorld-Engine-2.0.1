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
#include "common/graphics_settings_table.hpp"
#include "guitabs/guitabs_content.hpp"


class GeneralEditor;
typedef SmartPointer<GeneralEditor> GeneralEditorPtr;


class PageGraphicsSettings : public GraphicsSettingsTable, public GUITABS::Content
{
	IMPLEMENT_BASIC_CONTENT( L("COMMON/PAGE_GRAPHICS_SETTINGS/SHORT_NAME"),
		L("COMMON/PAGE_GRAPHICS_SETTINGS/LONG_NAME"), 290, 680, NULL )

public:
	PageGraphicsSettings();
	virtual ~PageGraphicsSettings();

	/*virtual*/ void needsRestart( const std::string& setting );

	// Dialog Data
	enum { IDD = IDD_PAGE_GRAPHICS_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnSize( UINT nType, int cx, int cy );
	DECLARE_MESSAGE_MAP()

private:
	CStatic messageText_;

	bool init();
	void onSizeInternal();
};

IMPLEMENT_BASIC_CONTENT_FACTORY( PageGraphicsSettings )
