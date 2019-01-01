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

#include "cstdmf/time_queue.hpp"

class Handler : public TimerHandler
{
protected:
	virtual void handleTimeout( TimerHandle handle, void * pUser )
	{
	}

	virtual void onRelease( TimerHandle handle, void * pUser )
	{
		delete this;
	}
};

TEST( Purge )
{
	std::vector< TimerHandle > handles;

	TimeQueue timeQueue;

	const uint NUM_TIMERS = 50;

	for (uint i = 0; i < NUM_TIMERS; ++i)
	{
		handles.push_back( timeQueue.add( i, 0, new Handler, NULL ) );
	}

	CHECK( timeQueue.size() == NUM_TIMERS );

	// Cancel more than half
	uint numToCancel = 2 * NUM_TIMERS / 3;

	for (uint i = 0; i < numToCancel; ++i)
	{
		handles[i].cancel();
	}

	// If more than half of the timers are cancelled, they should be purged
	// immediately.
	CHECK( timeQueue.size() < NUM_TIMERS/2 );
}

TEST( BadPurge )
{
	// Check the case where purge is called while a once-off timer is being
	// cancelled.
	TimeQueue timeQueue;
	timeQueue.add( 1, 0, new Handler, NULL );
	timeQueue.process( 2 );
}

// test_time_queue.cpp
