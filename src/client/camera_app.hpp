/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CAMERA_APP_HPP
#define CAMERA_APP_HPP


#include "cstdmf/main_loop_task.hpp"


/**
 *	Camera task
 */
class CameraApp : public MainLoopTask
{
public:
	CameraApp();
	~CameraApp();

	virtual bool init();
	virtual void fini();
	virtual void tick( float dTime );
	virtual void draw();
	virtual void inactiveTick( float dTime );

private:
	static CameraApp instance;
};



#endif // CAMERA_APP_HPP

// camera_app.hpp