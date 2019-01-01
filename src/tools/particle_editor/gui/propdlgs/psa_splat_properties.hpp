/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PSA_SPLAT_PROPERTIES_HPP
#define PSA_SPLAT_PROPERTIES_HPP

#include "resource.h"
#include "controls/edit_numeric.hpp"
#include "psa_properties.hpp"

class SplatPSA;

class PsaSplatProperties : public PsaProperties
{
public:
    DECLARE_DYNCREATE(PsaSplatProperties)

    enum { IDD = IDD_PSA_SPLAT_PROPERTIES };

    //
    // Constructor.
    //
    PsaSplatProperties();

	SplatPSA *	action() { return reinterpret_cast<SplatPSA *>(&*action_); }

    //
    // Set the parameters.
    //
    // @param task      The parameters to set.
    //
    /*virtual*/ void SetParameters(SetOperation /*task*/);


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()


private:
	controls::EditNumeric	delay_;
};

#endif // PSA_SPLAT_PROPERTIES_HPP
