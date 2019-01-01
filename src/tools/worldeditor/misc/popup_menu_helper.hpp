/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef POPUP_MENU_HELPER_HPP
#define POPUP_MENU_HELPER_HPP


#include "chunk/chunk_item.hpp"
#include "common/popup_menu.hpp"


/**
 *	This class has utility methods for creating some popup menus in WE.
 */
class PopupMenuHelper
{
public:
	static int buildCommandMenu(
					ChunkItemPtr pItem, int baseIdx, PopupMenu & destMenu );
};


#endif // POPUP_MENU_HELPER_HPP
