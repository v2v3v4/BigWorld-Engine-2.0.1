/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/bgtask_manager.hpp"

#include <stdio.h>

class MyTask : public BackgroundTask
{
public:
	MyTask( int & bgCount, int & fgCount, int & delCount ) :
		bgCount_( bgCount ),
		fgCount_( fgCount ),
		delCount_( delCount )
	{}

	virtual ~MyTask()
	{
		++delCount_;
	}

	virtual void doBackgroundTask( BgTaskManager & mgr )
	{
		mgr.addMainThreadTask( this );

		SimpleMutexHolder lock( s_mutex_ );
		++bgCount_;
	}

	virtual void doMainThreadTask( BgTaskManager & mgr )
	{
		++fgCount_;
	}

	int bgCount() const	{ return bgCount_; }
	int fgCount() const	{ return fgCount_; }

private:
	int & bgCount_;
	int & fgCount_;
	int & delCount_;

	static SimpleMutex s_mutex_;
};

SimpleMutex MyTask::s_mutex_;


TEST( BgTaskMgr )
{
	int bgCount = 0;
	int fgCount = 0;
	int delCount = 0;
	BgTaskManager mgr;

	mgr.startThreads( 10 );

	const int COUNT = 1000;

	for (int i = 0; i < COUNT; ++i)
	{
		mgr.addBackgroundTask( new MyTask( bgCount, fgCount, delCount ) );
	}

	mgr.stopAll( /* discardPendingTasks: */false, /* waitForThreads: */true );

	CHECK_EQUAL( COUNT, bgCount );
	CHECK_EQUAL( COUNT, fgCount );
	CHECK_EQUAL( COUNT, delCount );
}

// test_bgtasks.cpp
