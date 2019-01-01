/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GUI_FORWARD_HPP__
#define GUI_FORWARD_HPP__

#ifndef GUI_COMMAND_START
#define GUI_COMMAND_START	( 40000 )
#endif// GUI_COMMAND_START

#ifndef GUI_COMMAND_END
#define GUI_COMMAND_END	( 50000 )
#endif// GUI_COMMAND_END

#define BEGIN_GUI_NAMESPACE	namespace GUI	{
#define END_GUI_NAMESPACE	}
#define USING_GUI_NAMEAPCE using namespace GUI;

#include "cstdmf/smartpointer.hpp"

BEGIN_GUI_NAMESPACE

class Item;
typedef SmartPointer<Item> ItemPtr;


class ItemType;
typedef SmartPointer<ItemType> ItemTypePtr;


class Manager;
class Menu;
class Toolbar;


END_GUI_NAMESPACE

#endif//GUI_FORWARD_HPP__
