/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GUI_APP_HPP
#define GUI_APP_HPP


#include "cstdmf/main_loop_task.hpp"


/**
 *	GUI task
 */
class GUIApp : public MainLoopTask
{
public:
	GUIApp();
	~GUIApp();

	virtual bool init();
	virtual void fini();
	virtual void tick( float dTime );
	virtual void draw();

public:
	static GUIApp instance;

private:
};


#endif // GUI_APP_HPP


// gui_app.hpp
