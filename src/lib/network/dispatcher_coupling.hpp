/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DISPATCHER_COUPLING_HPP
#define DISPATCHER_COUPLING_HPP

#include "event_dispatcher.hpp"
#include "frequent_tasks.hpp"

namespace Mercury
{

/**
 *	This class is used to join to Dispatchers. Generally, there is only one
 *	Dispatcher per thread. To support more than one, this class can be used to
 *	attach one to another.
 */
class DispatcherCoupling : public FrequentTask
{
public:
	DispatcherCoupling( EventDispatcher & mainDispatcher,
			EventDispatcher & childDispatcher ) :
		mainDispatcher_( mainDispatcher ),
		childDispatcher_( childDispatcher )
	{
		mainDispatcher.addFrequentTask( this );
	}

	~DispatcherCoupling()
	{
		mainDispatcher_.cancelFrequentTask( this );
	}

private:
	void doTask()
	{
		childDispatcher_.processOnce();
	}

	EventDispatcher & mainDispatcher_;
	EventDispatcher & childDispatcher_;
};

} // namespace Mercury

#endif // DISPATCHER_COUPLING_HPP
