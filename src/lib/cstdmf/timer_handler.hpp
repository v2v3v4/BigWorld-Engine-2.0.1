/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TIMER_HANDLER_HPP
#define TIMER_HANDLER_HPP

#include "cstdmf/debug.hpp"

/**
 *	This type is a handle to a timer event.
 */
class TimeQueueNode;

/**
 *	This class is a handle to a timer added to TimeQueue.
 */
class TimerHandle
{
public:
	explicit TimerHandle( TimeQueueNode * pNode = NULL ) : pNode_( pNode ) {}

	void cancel();
	void clearWithoutCancel()	{ pNode_ = NULL; }

	bool isSet() const		{ return pNode_ != NULL; }

	friend bool operator==( TimerHandle h1, TimerHandle h2 );

	// This should only be used by TimeQueueT
	TimeQueueNode * pNode() const	{ return pNode_; }

private:
	TimeQueueNode * pNode_;
};

inline bool operator==( TimerHandle h1, TimerHandle h2 )
{
	return h1.pNode_ == h2.pNode_;
}


/**
 *	This is an interface which must be derived from in order to
 *	receive time queue events.
 */
class TimerHandler
{
public:
	TimerHandler() : numTimesRegistered_( 0 ) {}
	virtual ~TimerHandler()
	{
		MF_ASSERT( numTimesRegistered_ == 0 );
	};

	/**
	 * 	This method is called when a timeout expires.
	 *
	 * 	@param	handle	The handle returned when the event was added.
	 * 	@param	pUser	The user data passed in when the event was added.
	 */
	virtual void handleTimeout( TimerHandle handle, void * pUser ) = 0;

protected:
	virtual void onRelease( TimerHandle handle, void * pUser ) {}

private:
	friend class TimeQueueNode;
	void incTimerRegisterCount() { ++numTimesRegistered_; }
	void decTimerRegisterCount() { --numTimesRegistered_; }
	void release( TimerHandle handle, void * pUser )
	{
		this->decTimerRegisterCount();
		this->onRelease( handle, pUser );
	}

	int numTimesRegistered_;
};

#endif // TIMER_HANDLER_HPP
