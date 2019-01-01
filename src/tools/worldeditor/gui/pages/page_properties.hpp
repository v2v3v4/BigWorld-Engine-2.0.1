/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PAGE_PROPERTIES_HPP
#define PAGE_PROPERTIES_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "common/property_table.hpp"
#include "guitabs/guitabs_content.hpp"


class PageProperties : public PropertyTable, public GUITABS::Content
{
	IMPLEMENT_BASIC_CONTENT( Localise(L"WORLDEDITOR/GUI/PAGE_PROPERTIES/SHORT_NAME"),
		Localise(L"WORLDEDITOR/GUI/PAGE_PROPERTIES/LONG_NAME"), 290, 680, NULL )

public:
	PageProperties();
	virtual ~PageProperties();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	// Dialog Data
	enum { IDD = IDD_PAGE_PROPERTIES };

	static PageProperties& instance();

	void adviseSelectedId( std::string id );

	void OnListAddItem();
	void OnListItemRemoveItem();

private:
	bool inited_; 

	void addItems();

	static PageProperties * instance_;

	PropertyItem * rclickItem_;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg LRESULT OnUpdateControls(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDefaultPanels(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSelectPropertyItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChangePropertyItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDblClkPropertyItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRClkPropertyItem(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()
};


IMPLEMENT_BASIC_CONTENT_FACTORY( PageProperties )


#endif // PAGE_PROPERTIES_HPP
