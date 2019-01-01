/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONTROLS_UTILS_HPP
#define CONTROLS_UTILS_HPP

#include "controls/defs.hpp"
#include "controls/fwd.hpp"

namespace controls
{
	enum HALIGNMENT
	{
		LEFT,
		CENTRE_HORZ,
		RIGHT
	};

	enum VALIGNMENT
	{
		TOP,
		CENTRE_VERT,
		BOTTOM
	};

    void 
    replaceColour
    (
        HBITMAP             hbitmap,
        COLORREF            srcColour,
        COLORREF            dstColour
    );

    void
    replaceColour
    (
        HBITMAP             hbitmap,
        COLORREF            dstColour        
    );

    COLORREF
    getPixel
    (
        HBITMAP             hbitmap,
        unsigned int        x,
        unsigned int        y
    );

    static int NO_RESIZE = std::numeric_limits<int>::max();

    void childResize(CWnd &wnd, int left, int top, int right, int bottom);
    void childResize(CWnd &wnd, CRect const &extents);

    void childResize(CWnd &parent, UINT id, int left, int top, int right, int bottom);
    void childResize(CWnd &parent, UINT id, CRect const &extents);

    CRect childExtents(CWnd const &child);
    CRect childExtents(CWnd const &parent, UINT id);

    void enableWindow(CWnd &parent, UINT id, bool enable);
	void showWindow(CWnd &parent, UINT id, UINT show);

    bool isButtonChecked(CWnd const &parent, UINT id);
    void checkButton(CWnd &parent, UINT id, bool check);

	void setWindowText(CWnd &parent, UINT id, std::string const &text);
	void setWindowText(CWnd &parent, UINT id, std::wstring const &text);
    std::string getWindowText(CWnd const &parent, UINT id);

	CPoint validateDlgPos( const CSize& size, const CPoint& pt,
		HALIGNMENT hAlignment, VALIGNMENT vAlignment );

	std::string loadString(UINT id);
	std::wstring loadStringW(UINT id);
}


#endif // CONTROLS_UTILS_HPP
