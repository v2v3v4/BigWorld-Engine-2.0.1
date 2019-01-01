/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONSOLE_MANAGER_HPP
#define CONSOLE_MANAGER_HPP

#include <map>
#include <set>

#include "input/input.hpp"

class XConsole;

/**
 *	This class is the base class for all consoles.
 */
class ConsoleManager : public InputHandler
{
public:
	///	@name Constructors and Destructors.
	//@{
	ConsoleManager();
	~ConsoleManager();
	//@}

	void add( XConsole * pConsole, const char * label,
		KeyCode::Key key = KeyCode::KEY_NONE, uint32 modifiers = 0 );
	void draw( float dTime );

	static ConsoleManager & instance();
	
	static void createInstance();
	static void deleteInstance();

	void fini();

	/// @name Accessors
	//@{
	XConsole * pActiveConsole() const;
	XConsole * find( const char * name ) const;
	const std::string & consoleName( XConsole * console ) const;
	//@}

	bool handleKeyEvent( const KeyEvent & event );
	bool handleAxisEvent( const AxisEvent & event );

	void activate( XConsole * pConsole );
	void activate( const char * label );
	void toggle( const char * label );
	void deactivate();
	
	bool darkenBackground() const;
	void setDarkenBackground(bool enable);

private:
	ConsoleManager(const ConsoleManager&);
	ConsoleManager& operator=(const ConsoleManager&);

	typedef std::map< uint32, XConsole *> KeyedConsoles;
	typedef StringHashMap< XConsole *> LabelledConsoles;
	typedef std::set< XConsole * > ConsoleSet;

	XConsole * pActiveConsole_;
	KeyedConsoles keyedConsoles_;
	LabelledConsoles labelledConsoles_;
	ConsoleSet consoleSet_;
	
	bool darkenBackground_;

	static ConsoleManager* s_instance_;
};


#ifdef CODE_INLINE
	#include "console_manager.ipp"
#endif


#endif // CONSOLE_MANAGER_HPP
