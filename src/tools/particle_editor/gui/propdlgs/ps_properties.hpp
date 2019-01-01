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
#include "controls/edit_numeric.hpp"
#include "gui/gui_utilities.hpp"
#include "controls/auto_tooltip.hpp"

class ParticleSystem;

// PsProperties form view

class PsProperties : public CFormView
{
	DECLARE_DYNCREATE(PsProperties)

public:
    enum { IDD = IDD_PS_PROPERTIES };

	PsProperties();  

	/*virtual*/ ~PsProperties();

	virtual void DoDataExchange(CDataExchange* pDX); 

	afx_msg LRESULT OnUpdatePsProperties(WPARAM mParam, LPARAM lParam);

    ParticleSystemPtr action();

	void SetParameters(SetOperation task);

	void OnUpdatePsProperties();

	virtual void OnInitialUpdate();

	afx_msg void OnBnClickedPsButton();    

    DECLARE_MESSAGE_MAP()

    DECLARE_AUTO_TOOLTIP(PsProperties, CFormView)

private:
	bool                initialised_;
    CStatic             nameInvalidMessage_;
	controls::EditNumeric        capacity_;
	controls::EditNumeric        windFactor_;
	controls::EditNumeric        maxLod_;
    CToolTipCtrl        tooltips_;
};
