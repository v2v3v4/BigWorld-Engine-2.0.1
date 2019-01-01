/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GUI_MENU_HPP__
#define GUI_MENU_HPP__

#include "gui_manager.hpp"

BEGIN_GUI_NAMESPACE

class Menu : public Subscriber
{
protected:
	HWND hwnd_;
	HMENU hmenu_;
	virtual void changed( HMENU hmenu, ItemPtr item );
	void insertMenuItem( HMENU hmenu, int index, ItemPtr item );
	virtual void updateSeparator( HMENU hmenu, int& index, ItemPtr item, const MENUITEMINFO& info );
	virtual void updateAction( HMENU hmenu, int& index, ItemPtr item, const MENUITEMINFO& info );
	virtual void updateToggle( HMENU hmenu, int& index, ItemPtr item, const MENUITEMINFO& info );
	virtual void updateExpandedChoice( HMENU hmenu, int& index, ItemPtr item, const MENUITEMINFO& info );
	virtual void updateChoice( HMENU hmenu, int& index, ItemPtr item, const MENUITEMINFO& info );
	virtual void updateGroup( HMENU hmenu, int& index, ItemPtr item, const MENUITEMINFO& info );
	virtual void updateUnknownItem( HMENU hmenu, int& index, ItemPtr item, const MENUITEMINFO& info );
	std::wstring buildMenuText( ItemPtr item );
public:
	Menu( const std::string& root, HWND hwnd = NULL );
	virtual ~Menu();
	virtual void changed( ItemPtr item );
};

END_GUI_NAMESPACE

#endif//GUI_MENU_HPP__
