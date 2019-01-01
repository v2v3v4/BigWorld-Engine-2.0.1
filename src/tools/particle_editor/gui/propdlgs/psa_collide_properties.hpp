/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PSA_COLLIDE_PROPERTIES_HPP
#define PSA_COLLIDE_PROPERTIES_HPP

#include "afxwin.h"
#include "fwd.hpp"
#include "gui/propdlgs/psa_properties.hpp"
#include "controls/edit_numeric.hpp"
#include "controls/image_button.hpp"

class PsaCollideProperties : public PsaProperties
{
    DECLARE_DYNCREATE(PsaCollideProperties)

public:
    enum { IDD = IDD_PSA_COLLIDE_PROPERTIES };

    PsaCollideProperties(); 

    /*virtual*/ ~PsaCollideProperties();  

    /*virtual*/ void OnInitialUpdate();

    /*virtual*/ void DoDataExchange(CDataExchange* pDX); 

     CollidePSA *action() { return reinterpret_cast<CollidePSA *>(&*action_); }

    void SetParameters(SetOperation task);

private:  
    void UpdateState();

	void updateSoundLists();

    afx_msg void OnBnClickedPsaCollideSoundEnabled();
	afx_msg void OnCbnSelchangePsaCollideSoundProjectList();
	afx_msg void OnCbnSelchangePsaCollideSoundGroupList();
	afx_msg void OnCbnSelchangePsaCollideSoundNameList();

    DECLARE_MESSAGE_MAP()

private:
    controls::EditNumeric	elasticity_;
	controls::EditNumeric	delay_;
    CButton                 soundEnabled_;
    CComboBox               soundProject_;
    CComboBox               soundGroup_;
    CComboBox               soundName_;
};

#endif // PSA_COLLIDE_PROPERTIES_HPP
