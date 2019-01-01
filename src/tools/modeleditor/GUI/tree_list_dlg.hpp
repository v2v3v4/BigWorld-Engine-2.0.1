/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#pragma once
#include "resource.h"

#include "mutant.hpp"

typedef std::pair < std::string , std::string > StringPair;

// TreeListDlg

class TreeListDlg: public CDialog
{
	DECLARE_DYNCREATE(TreeListDlg)

public:
	//This is the default constructor required for DYNCREATE
	TreeListDlg(): CDialog(IDD_EMPTY) {}
	
	TreeListDlg( UINT dialogID, TreeRoot* tree_root_, const std::string& what, const std::string& currentModel = "" );
	virtual ~TreeListDlg();

private:
	HTREEITEM selItem_ ;
	StringPair selID_;
	std::string search_str_;

	TreeRoot* treeRoot_;
	std::string what_;
	std::string currentModel_;

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:

	void OnUpdateTreeList();
	
	afx_msg void OnEnSetFocusSearch();
	afx_msg void OnEnChangeSearch();
	afx_msg void OnStnClickedCancelSearch();
	afx_msg void OnTvnSelChangedTree(NMHDR *pNMHDR, LRESULT *pResult);

	virtual void selChange( const StringPair& itemID ) {};

	StringPair& selID() { return selID_; };
	HTREEITEM& selItem() { return selItem_; };
	CTreeCtrl& tree() { return tree_; };

private:

	bool ignoreSelChange_;

	CStatic search_bkg_;
	CEdit search_;
	CWnd search_button_;
	CWnd search_cancel_;
	CTreeCtrl tree_;

	std::vector<std::string*> pathData_;
};
