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
#include "gui/propdlgs/psa_empty_properties.hpp"

IMPLEMENT_DYNCREATE(PsaEmptyProperties, PsaProperties)

BEGIN_MESSAGE_MAP(PsaEmptyProperties, PsaProperties)
END_MESSAGE_MAP()

PsaEmptyProperties::PsaEmptyProperties()
: 
PsaProperties(PsaEmptyProperties::IDD)
{
}

PsaEmptyProperties::~PsaEmptyProperties()
{
}

/*virtual*/ void *PsaEmptyProperties::action() 
{ 
    return NULL; 
}

/*virtual*/ void PsaEmptyProperties::SetParameters(SetOperation /*task*/) 
{ 
}
