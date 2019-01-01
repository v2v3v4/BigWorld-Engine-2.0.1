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

#include "resource.h"
#include "base_property_table.hpp"
#include "common/property_list.hpp"


class BaseView;


class PropertyTable: public CFormView, public BasePropertyTable
{
public:
	
	PropertyTable( UINT dialogID );

	~PropertyTable();
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	void stretchToRight( CWnd& widget, int pageWidth, int border );
	void OnSize(UINT nType, int cx, int cy);

	BOOL PreTranslateMessage(MSG* pMsg);
};
