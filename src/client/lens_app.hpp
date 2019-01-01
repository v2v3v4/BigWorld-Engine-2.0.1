/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LENS_APP_HPP
#define LENS_APP_HPP


#include "cstdmf/main_loop_task.hpp"

/**
 *	Lens task
 */
class LensApp : public MainLoopTask
{
public:
	LensApp();
	~LensApp();

	virtual bool init();
	virtual void fini();
	virtual void tick( float dTime );
	virtual void draw();

private:
	static LensApp instance;

	float dTime_;
};



#endif // LENS_APP_HPP

// lens_app.hpp
