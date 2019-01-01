/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DRAGLISTBOX_HPP
#define DRAGLISTBOX_HPP

/**
 *  This is a CListBox that drags selected items as text and has a callback
 *  mechanism for tooltips.
 */
class DragListBox : public CListBox
{
public:
    typedef void (*ToolTipCB)
    (
        unsigned int    item, 
        std::string     &tooltip,
        void            *data
    );

    DragListBox();

    void setTooltipCallback(ToolTipCB cb, void *data = NULL);

protected:
    /*virtual*/ void PreSubclassWindow();

    /*virtual*/ int OnToolHitTest(CPoint point, TOOLINFO *ti) const;

    BOOL OnToolTipText(UINT id, NMHDR *pNMHDR, LRESULT *result);

    afx_msg void OnLButtonDown(UINT flags, CPoint point);

    afx_msg void OnMouseMove(UINT flags, CPoint point);

    DECLARE_MESSAGE_MAP()

private:
    UINT                        dragIndex_;
    ToolTipCB                   tooltipCB_;
    void                        *data_;
};

#endif // DRAGLISTBOX_HPP
