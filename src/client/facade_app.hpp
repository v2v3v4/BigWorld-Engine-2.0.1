/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FACADE_APP_HPP
#define FACADE_APP_HPP


#include "cstdmf/main_loop_task.hpp"
#include "romp/time_of_day.hpp"


/**
 *	Facade task
 */
class FacadeApp : public MainLoopTask
{
public:
	FacadeApp();
	~FacadeApp();

	virtual bool init();
	virtual void fini();
	virtual void tick( float dTime );
	virtual void draw();

private:
	static FacadeApp							instance;

	float										dTime_;
	ChunkSpacePtr								lastCameraSpace_;
	SmartPointer<TimeOfDay::UpdateNotifier>		todUpdateNotifier_;
};



#endif // FACADE_APP_HPP


// facade_app.hpp
