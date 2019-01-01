// MainFrm.h : interface of the CMainFrame class
//
#pragma once


#include "common/base_mainframe.hpp"
#include "guimanager/gui_action_maker.hpp"
#include "guimanager/gui_updater_maker.hpp"


class CMainFrame : public BaseMainFrame,
	GUI::ActionMaker<CMainFrame>,
	GUI::UpdaterMaker<CMainFrame>
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

private:
	CCreateContext	m_context;

// Attributes
public:

// Operations
public:
	static CMainFrame & instance()	{ return *s_instance; }

	bool resizing() { return resizing_; }

	afx_msg void OnClose();

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCreateClient(LPCREATESTRUCT, CCreateContext* pContext);

	bool handleGUIAction( GUI::ItemPtr item );
	bool showToolbar( GUI::ItemPtr item );
	bool hideToolbar( GUI::ItemPtr item );
	unsigned int updateToolbar( GUI::ItemPtr item );

	POINT currentCursorPosition() const;
	Vector3 getWorldRay(int x, int y) const;
	bool cursorOverGraphicsWnd() const;

	void OnGUIManagerCommand(UINT nID);
	void OnGUIManagerCommandUpdate(CCmdUI * cmdUI);
	void updateGUI( bool force = false );

	void setStatusText( UINT id, const wchar_t* text )
	{
		m_wndStatusBar.SetPaneText(
			m_wndStatusBar.CommandToIndex(id),
			text,
			TRUE);
	}

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:  // control bar embedded members
	CStatusBar			m_wndStatusBar;

private:
	static CMainFrame * s_instance;

	CWnd* view_;

	bool resizing_;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()

	afx_msg void OnMenuSelect( UINT nItemID, UINT nFlags, HMENU hSysMenu );
	afx_msg void OnExitMenuLoop( BOOL bIsTrackPopupMenu );
	afx_msg void OnToolbarHotItemChange( UINT id, NMHDR* pNotifyStruct, LRESULT* result );
	afx_msg LRESULT OnEnterSizeMove (WPARAM, LPARAM);
	afx_msg LRESULT OnExitSizeMove (WPARAM, LPARAM);
};
