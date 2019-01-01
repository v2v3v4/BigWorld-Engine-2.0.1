/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONTROLS_AUTO_TOOLTIP_HPP
#define CONTROLS_AUTO_TOOLTIP_HPP

#include "controls/defs.hpp"
#include "controls/fwd.hpp"
#include "controls/user_messages.hpp"
#include "resmgr/string_provider.hpp"

//
// These macros make writing tooltips for dialogs and other windows easy.
// To use it you do something similar to the following in the header:
//
//      CMyWindow : public CDialog
//      {
//          ...
//          DECLARE_AUTO_TOOLTIP(CMyWindow, CDialog)
//      };
//
// In the OnInitDialog/InitialUpdate function do something like:
//
//      BOOL CMyWinodw::OnInitDialog()
//      {
//          BOOL result = CDialog::OnInitDialog();
//          ...
//          INIT_AUTO_TOOLTIP();
//      }
//
// The tooltips use strings whose ID is the same as the control they represent.
//
// If your window class uses PreTranslateMessage then use 
//    DECLARE_AUTO_TOOLTIP_EX() instead of DECLARE_AUTO_TOOLTIP() and
// call CALL_TOOLTIPS(msg); inside PreTranslateMessage.
//
// The messages WM_SHOW_TOOLTIP and WM_HIDE_TOOLTIP are sent to your window
// when a tooltip is displayed/hidden.  The LPARAM is always NULL. WPARAM
// contains a pointer to the tooltip text if a WM_SHOW_TOOLTIP is being
// generated NULL otherwise.
//

#define DECLARE_AUTO_TOOLTIP(CLASSNAME, PARENT)                                \
    private:                                                                   \
        CToolTipCtrl __tooltips__;                                             \
		wchar_t __tipText__[512];                                              \
                                                                               \
        static BOOL CALLBACK __ttcallback__(HWND hchild, LPARAM myself)        \
        {                                                                      \
			BW_GUARD;                                                          \
                                                                               \
            CLASSNAME *me    = (CLASSNAME *)myself;                            \
            CWnd      *child = CWnd::FromHandle(hchild);                       \
            UINT      id     = child->GetDlgCtrlID();                          \
            CString text;                                                      \
			if (text.LoadString(id) != FALSE)                                  \
                me->__tooltips__.AddTool(child, text);                         \
            return TRUE;                                                       \
        }                                                                      \
                                                                               \
    public:                                                                    \
        CToolTipCtrl & toolTipCtrl() { return __tooltips__; }                  \
        CToolTipCtrl const & toolTipCtrl() const { return __tooltips__ ; }     \
                                                                               \
    protected:                                                                 \
                                                                               \
        /*virtual*/ BOOL PreTranslateMessage(MSG *msg)                         \
        {                                                                      \
		    BW_GUARD;                                                          \
                                                                               \
            if (::IsWindow(__tooltips__.GetSafeHwnd()))                        \
            {                                                                  \
                __tooltips__.RelayEvent(msg);                                  \
                static bool __lastShown__ = false;                             \
                bool        __thisShown__ = false;                             \
				TOOLINFO ti;                                                   \
				ti.lpszText = (LPTSTR)__tipText__;                             \
				ti.cbSize = sizeof(TOOLINFO);                                  \
                if                                                             \
                (                                                              \
                    __tooltips__.SendMessage(TTM_GETCURRENTTOOL, 0,            \
						(LPARAM)(LPTOOLINFO)&ti)                               \
                    !=                                                         \
                    FALSE                                                      \
                )                                                              \
                {                                                              \
					CWnd* wnd = CWnd::FromHandle( (HWND)ti.uId );              \
					CString s;                                                 \
					if (s.LoadString(wnd->GetDlgCtrlID()) != FALSE &&          \
						!__lastShown__&& isLocaliseToken( s ) )                \
					{                                                          \
						LPCTSTR text = Localise( (LPCTSTR)s + 1,               \
							StringProvider::RETURN_NULL_IF_NOT_EXISTING	);     \
						if( text )                                             \
							__tooltips__.UpdateTipText( text, this, ti.uId );  \
					}                                                          \
                    __thisShown__ = true;                                      \
                }                                                              \
                else                                                           \
                {                                                              \
                    __thisShown__ = false;                                     \
                }                                                              \
                if (__thisShown__ != __lastShown__)                            \
                {                                                              \
				SendMessage                                                    \
                    (                                                          \
                        __thisShown__ ? WM_SHOW_TOOLTIP : WM_HIDE_TOOLTIP,     \
						__thisShown__ ? (WPARAM)(&(ti.lpszText)) : 0,          \
                        0                                                      \
                    );                                                         \
                    __lastShown__ = __thisShown__;                             \
                }                                                              \
            }                                                                  \
            return PARENT::PreTranslateMessage(msg);                           \
        }

#define DECLARE_AUTO_TOOLTIP_EX(CLASSNAME)                                     \
    private:                                                                   \
        CToolTipCtrl __tooltips__;                                             \
		wchar_t __tipText__[512];                                              \
                                                                               \
        static BOOL CALLBACK __ttcallback__(HWND hchild, LPARAM myself)        \
        {                                                                      \
		    BW_GUARD;                                                          \
                                                                               \
            CLASSNAME *me    = (CLASSNAME *)myself;                            \
            CWnd      *child = CWnd::FromHandle(hchild);                       \
            UINT      id     = child->GetDlgCtrlID();                          \
            CString text;                                                      \
			if (text.LoadString(id) != FALSE)                                  \
                me->__tooltips__.AddTool(child, text);                         \
            return TRUE;                                                       \
        }                                                                      \
                                                                               \
    public:                                                                    \
        CToolTipCtrl & toolTipCtrl() { return __tooltips__; }                  \
        CToolTipCtrl const & toolTipCtrl() const { return __tooltips__ ; }     \
                                                                               \
    private:

#define INIT_AUTO_TOOLTIP()                                                    \
    __tooltips__.CreateEx(this, TTS_ALWAYSTIP, WS_EX_TOPMOST);                 \
    EnumChildWindows(*this, __ttcallback__, (LPARAM)this);                     \
    __tooltips__.Activate(TRUE);                                               \
    __tooltips__.SetWindowPos                                                  \
    (                                                                          \
            &CWnd::wndTopMost,                                                 \
            0, 0, 0, 0,                                                        \
            SWP_NOMOVE | SWP_NOSIZE                                            \
    );                                                                         \
    __tooltips__.SetMaxTipWidth(SHRT_MAX)

#define CALL_TOOLTIPS(MESSAGE)                                                 \
    if (::IsWindow(__tooltips__.GetSafeHwnd()))                                \
    {                                                                          \
        __tooltips__.RelayEvent(MESSAGE);                                      \
        static bool __lastShown__ = false;                                     \
        bool        __thisShown__ = false;                                     \
		TOOLINFO ti;                                                           \
		ti.lpszText = (LPTSTR)__tipText__;                                     \
		ti.cbSize = sizeof(TOOLINFO);                                          \
        if (__tooltips__.SendMessage(TTM_GETCURRENTTOOL, 0,                    \
			(LPARAM)(LPTOOLINFO)&ti)                                           \
           !=                                                                  \
           FALSE                                                               \
		)                                                                      \
		{                                                                      \
            __thisShown__ = true;                                              \
		}                                                                      \
        else                                                                   \
		{                                                                      \
            __thisShown__ = false;                                             \
		}                                                                      \
        if (__thisShown__ != __lastShown__)                                    \
        {                                                                      \
            SendMessage                                                        \
            (                                                                  \
                __thisShown__ ? WM_SHOW_TOOLTIP : WM_HIDE_TOOLTIP,             \
				__thisShown__ ? (WPARAM)(&(ti.lpszText)) : 0,                  \
                0                                                              \
            );                                                                 \
            __lastShown__ = __thisShown__;                                     \
        }                                                                      \
    }                                                                          \
    /*The next line is to force a required semi-colon*/                        \
    while (false) 

#define CALL_TOOLTIPS_NO_UPDATE(MESSAGE)                                       \
    __tooltips__.RelayEvent(MESSAGE) 

#endif // CONTROLS_AUTO_TOOLTIP_HPP
