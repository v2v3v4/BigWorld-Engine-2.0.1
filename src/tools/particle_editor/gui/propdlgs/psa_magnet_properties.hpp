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

class MagnetPSA;


// PsaMagnetProperties form view

class PsaMagnetProperties : public PsaProperties
{
	DECLARE_DYNCREATE(PsaMagnetProperties)

public:
	PsaMagnetProperties(); 
	virtual ~PsaMagnetProperties();

	enum { IDD = IDD_PSA_MAGNET_PROPERTIES };
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()


public:
	MagnetPSA *		action() { return reinterpret_cast<MagnetPSA *>(&*action_); }
	void			SetParameters(SetOperation task);

	controls::EditNumeric strength_;
	controls::EditNumeric minDist_;
	controls::EditNumeric	delay_;
};
