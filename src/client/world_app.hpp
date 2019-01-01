/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WORLD_APP_HPP
#define WORLD_APP_HPP


#include "cstdmf/main_loop_task.hpp"


/**
 *	World task
 */
class WorldApp : public MainLoopTask
{
public:
	WorldApp();
	~WorldApp();

	virtual bool init();
	virtual void fini();
	virtual void tick( float dTime );
	virtual void draw();
	virtual void inactiveTick( float dTime );

	static WorldApp instance;

	bool canSeeTerrain() const					{ return canSeeTerrain_; }

private:
	bool canSeeTerrain_;
	float dTime_;

public:
	uint32				wireFrameStatus_;
	uint32				debugSortedTriangles_;
};



#endif // WORLD_APP_HPP


// world_app.hpp