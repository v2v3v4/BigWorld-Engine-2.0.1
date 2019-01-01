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

#include "afxwin.h"
#include "common/property_table.hpp"

class ParticleSystem;

// MpsProperties form view

class MpsProperties : public PropertyTable
{
	DECLARE_DYNCREATE(MpsProperties)

public:
    enum { IDD = IDD_MPS_PROPERTIES };

	MpsProperties();  

	/*virtual*/ ~MpsProperties();

	virtual void DoDataExchange(CDataExchange* pDX); 

    MetaParticleSystemPtr metaPS();

	virtual void OnInitialUpdate();

	afx_msg LRESULT OnChangePropertyItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()

private:
	GeneralEditorPtr editor_;
	bool elected_;
};
