/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WORLD_EDITOR_VIEW_HPP
#define WORLD_EDITOR_VIEW_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"


class WorldEditorView : public CView
{
protected: 
	DECLARE_DYNCREATE(WorldEditorView)

	WorldEditorView();

public:
	virtual ~WorldEditorView();

	WorldEditorDoc* GetDocument() const;

	virtual void OnDraw(CDC* pDC); 
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:
	afx_msg void OnPaint();	
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	afx_msg BOOL OnSetCursor( CWnd*, UINT, UINT );
	afx_msg void OnKillFocus( CWnd *pNewWnd );
	DECLARE_MESSAGE_MAP()

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

private:
	CRect		lastRect_;
};


#endif // WORLD_EDITOR_VIEW_HPP
