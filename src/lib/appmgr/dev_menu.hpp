/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DEV_MENU_HPP
#define DEV_MENU_HPP

#include <iostream>
#include "module.hpp"
#include "romp/font.hpp"
class SimpleGUIComponent;
class TextGUIComponent;
class Button;

class DevMenu : public FrameworkModule
{
public:
	DevMenu();
	~DevMenu();

	virtual void onStart();
	virtual int	 onStop();

	virtual void onPause();
	virtual void onResume( int exitCode );

	virtual void render( float dTime );

	// Input handlers
	virtual bool handleKeyEvent( const KeyEvent & /*event*/ );
	virtual bool handleMouseEvent( const MouseEvent & /*event*/ );

private:
	DevMenu(const DevMenu&);
	DevMenu& operator=(const DevMenu&);

	typedef std::vector< std::string > Modules;
	Modules	modules_;

	SimpleGUIComponent* watermark_;

	typedef std::vector< Button* > Buttons;
	Buttons	buttons_;

	typedef std::vector< TextGUIComponent* > MenuItems;
	MenuItems menuItems_;

	friend std::ostream& operator<<(std::ostream&, const DevMenu&);
};

#ifdef CODE_INLINE
#include "dev_menu.ipp"
#endif




#endif
/*dev_menu.hpp*/
