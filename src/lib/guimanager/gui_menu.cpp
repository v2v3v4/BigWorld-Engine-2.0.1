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
#include "gui_menu.hpp"
#include "cstdmf/string_utils.hpp"
#include <stack>

BEGIN_GUI_NAMESPACE

Menu::Menu( const std::string& root, HWND hwnd /*= NULL*/ )
	: Subscriber( root ), hwnd_( hwnd )
{
	BW_GUARD;

	if( hwnd_ )
	{
		hmenu_ = GetMenu( hwnd_ );
		if( hmenu_ )
		{
			while( GetMenuItemCount( hmenu_ ) != 0 )
				DeleteMenu( hmenu_, 0, MF_BYPOSITION );
		}
	}
	else
		hmenu_ = CreatePopupMenu();
	MENUINFO info = { sizeof( info ), MIM_BACKGROUND };
	info.hbrBack = GetSysColorBrush( COLOR_MENUBAR );
	SetMenuInfo( hmenu_, &info );
}

Menu::~Menu()
{
	BW_GUARD;

	if( ! hwnd_ )
		DestroyMenu( hmenu_ );
}

void Menu::changed( ItemPtr item )
{
	BW_GUARD;

	std::stack<HMENU> menus;
	menus.push( hmenu_ );
	bool found = false;
	while( !menus.empty() )
	{
		HMENU hmenu = menus.top();
		menus.pop();
		MENUITEMINFO info = { sizeof( info ), MIIM_ID | MIIM_DATA };
		for( int i = 0; i < GetMenuItemCount( hmenu ); ++i )
			if( GetMenuItemInfo( hmenu, i, TRUE, &info ) &&
				info.wID == item->commandID() &&
				( Item* )info.dwItemData == item )
				changed( hmenu, item );
	}
	if( !found )// we'll do a full refresh
		changed( hmenu_, rootItem() );
}

void Menu::changed( HMENU hmenu, ItemPtr item )
{
	BW_GUARD;

	static const unsigned int MAX_MENU_TEXT = 1024;
	wchar_t txtBuf[ MAX_MENU_TEXT + 1 ];
	int i = 0;
	unsigned int j = 0;
	while( i < GetMenuItemCount( hmenu ) )
	{
		MENUITEMINFO info = { sizeof( info ),
			MIIM_BITMAP | MIIM_CHECKMARKS | MIIM_DATA | MIIM_FTYPE | MIIM_ID |
			MIIM_STATE | MIIM_STRING | MIIM_SUBMENU };
		info.cch = MAX_MENU_TEXT;
		info.dwTypeData = txtBuf;
		GetMenuItemInfo( hmenu, i, TRUE, &info );

		if( j < item->num() )
		{
			ItemPtr sub = ( *item )[ j ];
			if( ( Item* )info.dwItemData != sub )
			{
				insertMenuItem( hmenu, i, sub );
				ZeroMemory( &info, sizeof( info ) );
				info.cbSize = sizeof( info );
				info.fMask = MIIM_BITMAP | MIIM_CHECKMARKS | MIIM_DATA | MIIM_FTYPE | MIIM_ID |
					MIIM_STATE | MIIM_STRING | MIIM_SUBMENU;
				info.cch = MAX_MENU_TEXT;
				info.dwTypeData = txtBuf;
				GetMenuItemInfo( hmenu, i, TRUE, &info );
			}
			if( sub->type() == "SEPARATOR" )
				updateSeparator( hmenu, i, sub, info );
			else if( sub->type() == "GROUP" )
				updateGroup( hmenu, i, sub, info );
			else if( sub->type() == "ACTION" )
				updateAction( hmenu, i, sub, info );
			else if( sub->type() == "TOGGLE" )
				updateToggle( hmenu, i, sub, info );
			else if( sub->type() == "CHOICE" )
				updateChoice( hmenu, i, sub, info );
			else if( sub->type() == "EXPANDED_CHOICE" )
				updateExpandedChoice( hmenu, i, sub, info );
			else
				updateUnknownItem( hmenu, i, sub, info );
			GetMenuItemInfo( hmenu, i, TRUE, &info );
			++i;
			++j;
		}
		else
			DeleteMenu( hmenu, i, MF_BYPOSITION );
	}
	for(; j< item->num(); ++j )
	{
		ItemPtr sub = ( *item )[ j ];
		insertMenuItem( hmenu, i, sub );
		MENUITEMINFO info = { sizeof( info ),
			MIIM_BITMAP | MIIM_CHECKMARKS | MIIM_DATA | MIIM_FTYPE | MIIM_ID |
			MIIM_STATE | MIIM_STRING | MIIM_SUBMENU };
		info.cch = MAX_MENU_TEXT;
		info.dwTypeData = txtBuf;
		GetMenuItemInfo( hmenu, i, TRUE, &info );
		if( sub->type() == "SEPARATOR" )
			updateSeparator( hmenu, i, sub, info );
		else if( sub->type() == "GROUP" )
			updateGroup( hmenu, i, sub, info );
		else if( sub->type() == "ACTION" )
			updateAction( hmenu, i, sub, info );
		else if( sub->type() == "TOGGLE" )
			updateToggle( hmenu, i, sub, info );
		else if( sub->type() == "CHOICE" )
			updateChoice( hmenu, i, sub, info );
		else if( sub->type() == "EXPANDED_CHOICE" )
			updateExpandedChoice( hmenu, i, sub, info );
		else
			updateUnknownItem( hmenu, i, sub, info );
		GetMenuItemInfo( hmenu, i, TRUE, &info );
		++i;
	}
}

void Menu::insertMenuItem( HMENU hmenu, int index, ItemPtr item )
{
	BW_GUARD;

	MENUITEMINFO info = { sizeof( info ), MIIM_STRING | MIIM_ID | MIIM_DATA, 0, 0,
		item->commandID(), NULL, NULL, NULL, (DWORD)item.getObject(), L"menu" };
	InsertMenuItem( hmenu, index, MF_BYPOSITION, &info );
}

void Menu::updateSeparator( HMENU hmenu, int& index, ItemPtr item, const MENUITEMINFO& info )
{
	BW_GUARD;

	if( ( info.fType & MFT_SEPARATOR ) == 0 )
	{
		MENUITEMINFO info = { sizeof( info ), MIIM_FTYPE, MFT_SEPARATOR };
		SetMenuItemInfo( hmenu, index, TRUE, &info );
	}
}

void Menu::updateAction( HMENU hmenu, int& index, ItemPtr item, const MENUITEMINFO& info )
{
	BW_GUARD;

	if( !info.dwTypeData || buildMenuText( item ) != (LPCTSTR)info.dwTypeData )
		ModifyMenu( hmenu, index, MF_BYPOSITION | MF_STRING, item->commandID(),
			buildMenuText( item ).c_str() );
	EnableMenuItem( hmenu, index, MF_BYPOSITION | ( item->update() ? MF_ENABLED : MF_GRAYED ) );
}

void Menu::updateToggle( HMENU hmenu, int& index, ItemPtr item, const MENUITEMINFO& info )
{
	BW_GUARD;

	if( !info.dwTypeData || buildMenuText( item ) != (LPCTSTR)info.dwTypeData )
		ModifyMenu( hmenu, index, MF_BYPOSITION | MF_STRING, item->commandID(),
			buildMenuText( item ).c_str() );
	if( item->num() != 2 )// exactly 2
		throw 1;
	EnableMenuItem( hmenu, index, MF_BYPOSITION | ( item->update() ? MF_ENABLED : MF_GRAYED ) );
	CheckMenuItem( hmenu, index, MF_BYPOSITION | ( ( *item )[ 0 ]->update() ? MF_UNCHECKED : MF_CHECKED ) );
}

void Menu::updateExpandedChoice( HMENU hmenu, int& index, ItemPtr item, const MENUITEMINFO& info )
{
	BW_GUARD;

	HMENU sub = info.hSubMenu;
	if( !info.dwTypeData || buildMenuText( item ) != (LPCTSTR)info.dwTypeData )
		ModifyMenu( hmenu, index, MF_BYPOSITION | MF_STRING, item->commandID(),
			buildMenuText( item ).c_str() );
	if( sub == NULL )
	{
		MENUITEMINFO info = { sizeof( info ), MIIM_SUBMENU };
		info.hSubMenu = ( sub = CreateMenu() );
		SetMenuItemInfo( hmenu, index, TRUE, &info );
	}
	EnableMenuItem( hmenu, index, MF_BYPOSITION |
		( item->num() && item->update() ? MF_ENABLED : MF_GRAYED ) );
	int subIndex = 0;
	updateChoice( sub, subIndex, item, info );
}

void Menu::updateChoice( HMENU hmenu, int& index, ItemPtr item, const MENUITEMINFO& info )
{
	BW_GUARD;

	static const unsigned int MAX_MENU_TEXT = 1024;
	wchar_t txtBuf[ MAX_MENU_TEXT + 1 ];
	unsigned int subItemIndex = 0;
	while( index < GetMenuItemCount( hmenu ) )
	{
		MENUITEMINFO info = { sizeof( info ),
			MIIM_BITMAP | MIIM_CHECKMARKS | MIIM_DATA | MIIM_FTYPE | MIIM_ID |
			MIIM_STATE | MIIM_STRING | MIIM_SUBMENU };
		info.cch = MAX_MENU_TEXT;
		info.dwTypeData = txtBuf;
		GetMenuItemInfo( hmenu, index, TRUE, &info );

		if( info.dwItemData != (DWORD)item.getObject() )
			break;

		if( subItemIndex < item->num() )
		{
			ItemPtr subItem = ( *item )[ subItemIndex ];
			if( !info.dwTypeData || buildMenuText( subItem ) != (LPCTSTR)info.dwTypeData )
				insertMenuItem( hmenu, index, item );
			ModifyMenu( hmenu, index, MF_BYPOSITION | MF_STRING, subItem->commandID(),
				buildMenuText( subItem ).c_str() );
			EnableMenuItem( hmenu, index, MF_BYPOSITION | ( item->update() ? MF_ENABLED : MF_GRAYED ) );
			CheckMenuItem( hmenu, index, MF_BYPOSITION | ( subItem->update() ? MF_CHECKED : MF_UNCHECKED ) );
			++index;
			++subItemIndex;
		}
		else
			DeleteMenu( hmenu, index, MF_BYPOSITION );
	}
	while( subItemIndex < item->num() )
	{
		ItemPtr subItem = ( *item )[ subItemIndex ];
		insertMenuItem( hmenu, index, item );
		ModifyMenu( hmenu, index, MF_BYPOSITION | MF_STRING, subItem->commandID(),
			buildMenuText( subItem ).c_str() );
		EnableMenuItem( hmenu, index, MF_BYPOSITION | ( item->update() ? MF_ENABLED : MF_GRAYED ) );
		CheckMenuItem( hmenu, index, MF_BYPOSITION | ( subItem->update() ? MF_CHECKED : MF_UNCHECKED ) );
		++subItemIndex;
	}
	--index;
}

void Menu::updateGroup( HMENU hmenu, int& index, ItemPtr item, const MENUITEMINFO& info )
{
	BW_GUARD;

	HMENU sub = info.hSubMenu;
	if( !info.dwTypeData || buildMenuText( item ) != (LPCTSTR)info.dwTypeData )
		ModifyMenu( hmenu, index, MF_BYPOSITION | MF_STRING, item->commandID(),
			buildMenuText( item ).c_str() );
	if( sub == NULL )
	{
		MENUITEMINFO info = { sizeof( info ), MIIM_SUBMENU };
		info.hSubMenu = ( sub = CreateMenu() );
		SetMenuItemInfo( hmenu, index, TRUE, &info );
	}
	EnableMenuItem( hmenu, index, MF_BYPOSITION | ( item->update() ? MF_ENABLED : MF_GRAYED ) );
	changed( sub, item );
}

void Menu::updateUnknownItem( HMENU hmenu, int& index, ItemPtr item, const MENUITEMINFO& info )
{}

std::wstring Menu::buildMenuText( ItemPtr item )
{
	BW_GUARD;

	std::wstring text;
	bw_utf8tow( item->displayName(), text );
	if ( !item->shortcutKey().empty() )
	{
		std::wstring wshortcutkey;
		bw_utf8tow( item->shortcutKey(), wshortcutkey );
		text += L"\t" + wshortcutkey;
	}
	return text;
}

END_GUI_NAMESPACE
