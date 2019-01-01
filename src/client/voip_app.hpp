/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef VOIP_APP_HPP
#define VOIP_APP_HPP


#include "cstdmf/main_loop_task.hpp"


class VOIPClient;


/**
 *	This class is a singleton main loop task for managing and ticking the VoIP
 *	client.
 *	This task must be initialised before any script code that could call
 *	BigWorld.VOIP.* is executed.
 *
 *	@see	VOIPClient
 *	@see	PyVOIP
 */
class VOIPApp : public MainLoopTask
{
public:
	VOIPApp();
	~VOIPApp();

	virtual bool init();
	virtual void fini();
	virtual void tick( float dTime );
	virtual void draw();

	static VOIPClient & getVOIPClient();

private:
	VOIPClient *	voipClient_;

	static VOIPApp	s_instance_;
};



#endif // VOIP_APP_HPP

// voip_app.hpp