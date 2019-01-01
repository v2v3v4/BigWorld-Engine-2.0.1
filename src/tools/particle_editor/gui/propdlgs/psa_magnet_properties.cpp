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
#include "gui/propdlgs/psa_magnet_properties.hpp"
#include "particle/actions/magnet_psa.hpp"

DECLARE_DEBUG_COMPONENT2( "GUI", 0 )

IMPLEMENT_DYNCREATE(PsaMagnetProperties, PsaProperties)

PsaMagnetProperties::PsaMagnetProperties()
: 
PsaProperties(PsaMagnetProperties::IDD)
{
	BW_GUARD;

	minDist_.SetMinimum(0.f, false);
	minDist_.SetAllowNegative(false);
	delay_.SetNumericType( controls::EditNumeric::ENT_FLOAT );
	delay_.SetMinimum( 0.0f );
}

PsaMagnetProperties::~PsaMagnetProperties()
{
}

void PsaMagnetProperties::SetParameters(SetOperation task)
{
	BW_GUARD;

	ASSERT(action_);

	SET_FLOAT_PARAMETER(task, strength);
	SET_FLOAT_PARAMETER(task, minDist);
	SET_FLOAT_PARAMETER(task, delay);
}


void PsaMagnetProperties::DoDataExchange(CDataExchange* pDX)
{
	PsaProperties::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PSA_MAGNET_STRENGTH, strength_);
	DDX_Control(pDX, IDC_PSA_MAGNET_MINDIST, minDist_);
	DDX_Control(pDX, IDC_PSA_DELAY, delay_);
}

BEGIN_MESSAGE_MAP(PsaMagnetProperties, PsaProperties)
END_MESSAGE_MAP()


// PsaMagnetProperties diagnostics

#ifdef _DEBUG
void PsaMagnetProperties::AssertValid() const
{
	PsaProperties::AssertValid();
}

void PsaMagnetProperties::Dump(CDumpContext& dc) const
{
	PsaProperties::Dump(dc);
}
#endif //_DEBUG


// PsaMagnetProperties message handlers
