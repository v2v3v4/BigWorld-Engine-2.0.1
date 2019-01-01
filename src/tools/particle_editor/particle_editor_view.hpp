/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PARTICLE_EDITOR_VIEW_HPP
#define PARTICLE_EDITOR_VIEW_HPP

#include "fwd.hpp"

class ParticleEditorView : public CView
{
protected:
	ParticleEditorView();

	BOOL PreCreateWindow(CREATESTRUCT& cs);

	DECLARE_DYNCREATE(ParticleEditorView)

	CRect lastRect_;

public:
    /*virtual*/ ~ParticleEditorView();

    static ParticleEditorView *instance();

	ParticleEditorDoc* GetDocument() const;

    /*virtual*/ void OnDraw(CDC *dc);

	/*virtual*/ void 
    OnActivateView
    (
        BOOL            bActivate, 
        CView           *pActivateView, 
        CView           *
    );

protected:
    afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnKillFocus( CWnd *pNewWnd );
	DECLARE_MESSAGE_MAP()

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

    static ParticleEditorView   *s_instance_;
};

#endif // PARTICLE_EDITOR_VIEW_HPP
