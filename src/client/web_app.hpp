/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WEB_APP_HPP
#define WEB_APP_HPP

#include "cstdmf/main_loop_task.hpp"

/**
 *	WebApp task
 */
class WebApp : public MainLoopTask
{
public:
	WebApp();
	~WebApp();

	virtual bool init();
	virtual void fini();
	virtual void tick( float dTime );

	static WebApp instance;
};

#endif // WEB_APP_HPP
