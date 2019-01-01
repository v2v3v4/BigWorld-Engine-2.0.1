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

#include "editor_views.hpp"
#include "property_table.hpp"
#include "gizmo/general_editor.hpp"


DECLARE_DEBUG_COMPONENT( 0 )


PropertyTable::PropertyTable( UINT dialogID ):
	CFormView( dialogID )
{
}


PropertyTable::~PropertyTable()
{
}


void PropertyTable::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PROPERTIES_LIST, *propertyList());
}


BEGIN_MESSAGE_MAP(PropertyTable, CFormView)
	ON_WM_SIZE()
END_MESSAGE_MAP()


BOOL PropertyTable::PreTranslateMessage(MSG* pMsg)
{
	BW_GUARD;

    if(pMsg->message == WM_KEYDOWN)
    {
        if (pMsg->wParam == VK_TAB)
		{
			if (GetAsyncKeyState(VK_SHIFT) < 0)
			{
				if (propertyList()->changeSelectItem( -1 )) return true;
			}
			else
			{
				if (propertyList()->changeSelectItem( +1 )) return true;
			}
		}
        if (pMsg->wParam == VK_RETURN)
		{
			propertyList()->deselectCurrentItem();
			//return true;
		}
    }

	return CFormView::PreTranslateMessage(pMsg);
}


void PropertyTable::stretchToRight( CWnd& widget, int pageWidth, int border )
{
	BW_GUARD;

	CRect rect;
	widget.GetWindowRect( &rect );
    ScreenToClient( &rect );
	widget.SetWindowPos( 0, 0, 0, pageWidth - rect.left - border, rect.Height(), SWP_NOMOVE | SWP_NOZORDER );
}


void PropertyTable::OnSize( UINT nType, int cx, int cy )
{
	BW_GUARD;

	stretchToRight( *propertyList(), cx, 12 );
	propertyList()->setDividerPos( (cx - 12) / 2 );
	propertyList()->RedrawWindow();
	RedrawWindow();
	CFormView::OnSize( nType, cx, cy );
}
