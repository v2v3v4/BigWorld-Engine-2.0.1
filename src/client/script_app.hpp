/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SCRIPT_APP_HPP
#define SCRIPT_APP_HPP


#include "cstdmf/main_loop_task.hpp"


/**
 *	Script task
 */
class ScriptApp : public MainLoopTask
{
	ScriptApp();
	~ScriptApp();

	virtual bool init();
	virtual void fini();
	virtual void tick( float dTime );
	virtual void inactiveTick( float dTime );
	virtual void draw();

private:
	static ScriptApp instance;
};



#endif // SCRIPT_APP_HPP


// script_app.hpp