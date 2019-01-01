/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "tree_list_dlg.hpp"

typedef std::pair < std::string , std::string > StringPair;

class CChooseAnim : public TreeListDlg
{
public:
	CChooseAnim( int dialogID, bool withName, const std::string& currentModel = "" );
	virtual BOOL OnInitDialog();

	virtual void selChange( const StringPair& animId );
	afx_msg void OnEnChangeActName();

	std::string& actName() { return actName_; };
	std::string& animName() { return animName_; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
private:
	CEdit name_;
	CButton ok_;

	std::string actName_;
	std::string animName_;

	bool withName_;

	bool defaultName_;
	bool defaultNameChange_;
	
public:
	afx_msg void OnNMDblclkSearchTree(NMHDR *pNMHDR, LRESULT *pResult);
};
