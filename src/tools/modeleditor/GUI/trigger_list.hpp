/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "resource.h"       // main symbols

#include <set>

/*
 *	This class is used to manage the currently set action caps
 */
class CheckList: public CTreeCtrl
{
protected:
	DECLARE_MESSAGE_MAP()

private:
	std::set< int > capsSet_;

public:
	void capsStr( const std::string& caps );
	std::string caps();
	void updateList();
	void redrawList();

};

/*
 * This class is used for managing the action cap window
 */
class CTriggerList : public CDialog
{
public:
	CTriggerList( const std::string& capsName, std::vector< DataSectionPtr >& capsList, const std::string& capsString = "" );
	~CTriggerList();

	virtual BOOL OnInitDialog();

	std::string caps();

// Dialog Data
	enum { IDD = IDD_ACT_TRIGGER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
	
private:
	std::vector< DataSectionPtr > capsList_;
	std::vector< int* > capsData_;

	CheckList checkList_;
	std::string capsName_;
	std::string capsStr_;

public:
	afx_msg void OnPaint();
	void OnOK();
};
