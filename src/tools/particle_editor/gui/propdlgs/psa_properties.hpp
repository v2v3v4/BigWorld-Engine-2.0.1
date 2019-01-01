/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PSA_PROPERTIES_HPP
#define PSA_PROPERTIES_HPP

#include "gui/gui_utilities.hpp"
#include "controls/auto_tooltip.hpp"
#include "particle/actions/particle_system_action.hpp"

class PsaProperties : public CFormView
{
public:
    PsaProperties(UINT nIDTemplate);

    void    SetPSA(ParticleSystemActionPtr action);

    void    CopyDataToControls();
    void    CopyDataToPSA();

    virtual void SetParameters(SetOperation task) = 0;

    // windows member overrides
    virtual void PostNcDestroy();
    virtual void OnInitialUpdate();
    virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
    afx_msg LRESULT OnUpdatePsaProperties(WPARAM mParam, LPARAM lParam);
    DECLARE_AUTO_TOOLTIP_EX(PsaProperties)
    DECLARE_MESSAGE_MAP()

protected:
    ParticleSystemActionPtr	action_;
    bool                    initialised_;
};

#endif // PSA_PROPERTIES_HPP
