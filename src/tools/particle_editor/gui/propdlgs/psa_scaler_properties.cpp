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
#include "particle_editor.hpp"
#include "gui/propdlgs/psa_scaler_properties.hpp"
#include "particle/actions/scaler_psa.hpp"

DECLARE_DEBUG_COMPONENT2( "GUI", 0 )

// PsaScalerProperties

IMPLEMENT_DYNCREATE(PsaScalerProperties, PsaProperties)

PsaScalerProperties::PsaScalerProperties()
	: PsaProperties(PsaScalerProperties::IDD)
{
	BW_GUARD;

	delay_.SetNumericType( controls::EditNumeric::ENT_FLOAT );
	delay_.SetMinimum( 0.0f );
}

PsaScalerProperties::~PsaScalerProperties()
{
}

void PsaScalerProperties::SetParameters(SetOperation task)
{
	BW_GUARD;

	ASSERT(action_);

	SET_FLOAT_PARAMETER(task, size);
	SET_FLOAT_PARAMETER(task, rate);
	SET_FLOAT_PARAMETER(task, delay);

	size_.SetAllowNegative( false );
	rate_.SetAllowNegative( false );
}

void PsaScalerProperties::DoDataExchange(CDataExchange* pDX)
{
	PsaProperties::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PSA_SCALER_SIZE, size_);
	DDX_Control(pDX, IDC_PSA_SCALER_RATE, rate_);
	DDX_Control(pDX, IDC_PSA_DELAY, delay_);
}

BEGIN_MESSAGE_MAP(PsaScalerProperties, PsaProperties)
END_MESSAGE_MAP()


// PsaScalerProperties diagnostics

#ifdef _DEBUG
void PsaScalerProperties::AssertValid() const
{
	PsaProperties::AssertValid();
}

void PsaScalerProperties::Dump(CDumpContext& dc) const
{
	PsaProperties::Dump(dc);
}
#endif //_DEBUG


// PsaScalerProperties message handlers
