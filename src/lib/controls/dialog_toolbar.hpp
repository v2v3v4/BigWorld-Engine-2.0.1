/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONTROLS_DIALOG_TOOLBAR_HPP
#define CONTROLS_DIALOG_TOOLBAR_HPP


#include "controls/defs.hpp"
#include "controls/fwd.hpp"
#include "controls/auto_tooltip.hpp"


namespace controls
{
    /**
     *  This is a CToolbar suitable for embedding into CDialog derived classes.
     */
    class CONTROLS_DLL DialogToolbar : public CToolBar
    {
    public:
        DialogToolbar();

        void Subclass(UINT id);

        BOOL LoadToolBarEx(UINT id, UINT disabledId = 0);

    protected:
        afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wparam, LPARAM lparam);

        DECLARE_MESSAGE_MAP()

        DECLARE_AUTO_TOOLTIP(DialogToolbar, CToolBar)

    private:
        CBitmap             disabledBmp_;
        CImageList          disabledImgList_;
    };
}

#endif // CONTROLS_DIALOG_TOOLBAR_HPP
