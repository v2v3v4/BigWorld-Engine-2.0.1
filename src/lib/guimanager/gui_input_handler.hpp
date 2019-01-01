/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GUI_INPUT_HANDLER_HPP__
#define GUI_INPUT_HANDLER_HPP__

#include "gui_item.hpp"
#include "input/input.hpp"
#include <map>
#include <string>

BEGIN_GUI_NAMESPACE

class BigworldInputDevice : public GUI::InputDevice
{
	const bool* keyDownTable_;
	static std::map<char, int> keyMap_;
	static std::map<std::string, int> nameMap_;
	static bool lastKeyDown[ KeyCode::NUM_KEYS ];
	static bool down( const bool* keyDownTable, char ch );
	static bool down( const bool* keyDownTable, const std::string& key );
public:
	BigworldInputDevice( const bool* keyDownTable );
	virtual bool isKeyDown( const std::string& key );
	static void refreshKeyDownState( const bool* keyDown );
};

class Win32InputDevice : public GUI::InputDevice
{
	static std::map<char, int> keyMap_;
	static std::map<std::string, int> nameMap_;
	static bool down( char ch );
	static bool down( const std::string& key );
	static char ch_;
public:
	Win32InputDevice( char ch );
	virtual bool isKeyDown( const std::string& key );

	static void install();

	static void fini();
};

END_GUI_NAMESPACE

#endif//GUI_INPUT_HANDLER_HPP__
