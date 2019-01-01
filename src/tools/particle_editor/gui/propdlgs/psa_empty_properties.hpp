/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PSA_EMPTY_PROPERTIES_HPP
#define PSA_EMPTY_PROPERTIES_HPP

#include "gui/propdlgs/psa_properties.hpp"

class PsaEmptyProperties : public PsaProperties
{
    DECLARE_DYNCREATE(PsaEmptyProperties)

public:
    enum { IDD = IDD_PSA_EMPTY };

    PsaEmptyProperties(); 

    /*virtual*/ ~PsaEmptyProperties();

    /*virtual*/ void *action();

    /*virtual*/ void SetParameters(SetOperation);

protected:
    DECLARE_MESSAGE_MAP()
};


#endif // PSA_EMPTY_PROPERTIES_HPP
