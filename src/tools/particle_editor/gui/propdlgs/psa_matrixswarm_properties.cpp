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
#include "psa_matrixswarm_properties.hpp"
#include "particle/actions/matrix_swarm_psa.hpp"

DECLARE_DEBUG_COMPONENT2("GUI", 0)

IMPLEMENT_DYNCREATE(PsaMatrixSwarmProperties, PsaProperties)

PsaMatrixSwarmProperties::PsaMatrixSwarmProperties()
:
PsaProperties(IDD)
{
	BW_GUARD;

	delay_.SetNumericType( controls::EditNumeric::ENT_FLOAT );
	delay_.SetMinimum( 0.0f );
}

/*virtual*/ void PsaMatrixSwarmProperties::SetParameters(SetOperation task)
{
	BW_GUARD;

	SET_FLOAT_PARAMETER(task, delay);
}


void PsaMatrixSwarmProperties::DoDataExchange(CDataExchange* pDX)
{
	BW_GUARD;

	PsaProperties::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PSA_DELAY, delay_);
}


BEGIN_MESSAGE_MAP(PsaMatrixSwarmProperties, PsaProperties)
END_MESSAGE_MAP()
