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
#include "gui/propdlgs/psa_properties.hpp"
#include "controls/edit_numeric.hpp"

class ScalerPSA;

// PsaScalerProperties form view

class PsaScalerProperties : public PsaProperties
{
	DECLARE_DYNCREATE(PsaScalerProperties)

public:
	PsaScalerProperties();           // protected constructor used by dynamic creation
	virtual ~PsaScalerProperties();

	enum { IDD = IDD_PSA_SCALER_PROPERTIES };
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()


public:
	ScalerPSA *	action() { return reinterpret_cast<ScalerPSA *>(&*action_); }
	void		SetParameters(SetOperation task);

	controls::EditNumeric size_;
	controls::EditNumeric rate_;
	controls::EditNumeric	delay_;
};
