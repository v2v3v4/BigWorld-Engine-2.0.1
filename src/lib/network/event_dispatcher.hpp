/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EVENT_DISPATCHER_HPP
#define EVENT_DISPATCHER_HPP

#ifdef _WIN32
	#ifdef _XBOX360
		#include <winsockx.h>
	#else
		#include <Winsock.h> // for fdset
	#endif
#endif

#include "interfaces.hpp"

#include "cstdmf/timestamp.hpp"
#include "cstdmf/watcher.hpp"

// #include "cstdmf/time_queue.hpp"
template <class TYPE> class TimeQueueT;
typedef TimeQueueT< uint64 > TimeQueue64;


#include <set>

namespace Mercury
{

class DispatcherCoupling;
class ErrorReporter;
class EventPoller;
class FrequentTask;
class FrequentTasks;


/**
 *	This class is responsible for dispatching events. These include timers,
 *	network activity and file activity events.
 */
class EventDispatcher
{
public:
	EventDispatcher();
	~EventDispatcher();

	void processContinuously();
	int  processOnce( bool shouldIdle = false );
	void processUntilBreak();

	void breakProcessing( bool breakState = true );
	bool processingBroken() const;

	void attach( EventDispatcher & childDispatcher );
	void detach( EventDispatcher & childDispatcher );

	bool registerFileDescriptor( int fd, InputNotificationHandler * handler );
	bool deregisterFileDescriptor( int fd );
	bool registerWriteFileDescriptor( int fd, InputNotificationHandler * handler );
	bool deregisterWriteFileDescriptor( int fd );

	INLINE TimerHandle addTimer( int64 microseconds,
					TimerHandler * handler, void* arg = NULL );
	INLINE TimerHandle addOnceOffTimer( int64 microseconds,
					TimerHandler * handler, void * arg = NULL );

	void addFrequentTask( FrequentTask * pTask );
	bool cancelFrequentTask( FrequentTask * pTask );

	uint64 timerDeliveryTime( TimerHandle handle ) const;
	uint64 timerIntervalTime( TimerHandle handle ) const;
	uint64 & timerIntervalTime( TimerHandle handle );

	uint64 getSpareTime() const;
	void clearSpareTime();
	double proportionalSpareTime() const;

	INLINE double maxWait() const;
	INLINE void maxWait( double seconds );

	ErrorReporter & errorReporter()	{ return *pErrorReporter_; }

#if ENABLE_WATCHERS
	static WatcherPtr pWatcher();
#endif

private:
	TimerHandle addTimerCommon( int64 microseconds,
		TimerHandler * handler,
		void * arg,
		bool recurrent );

	void processFrequentTasks();
	void processTimers();
	void processStats();
	int processNetwork( bool shouldIdle );

	double calculateWait() const;

	void attachTo( EventDispatcher & parentDispatcher );
	void detachFrom( EventDispatcher & parentDispatcher );

	bool breakProcessing_;

	EventPoller * pPoller_;

	TimeQueue64 * pTimeQueue_;

	FrequentTasks * pFrequentTasks_;

	ErrorReporter * pErrorReporter_;

	// Statistics
	TimeStamp		accSpareTime_;
	TimeStamp		oldSpareTime_;
	TimeStamp		totSpareTime_;
	TimeStamp		lastStatisticsGathered_;

	uint32 numTimerCalls_;

	double maxWait_;

	DispatcherCoupling * pCouplingToParent_;

	typedef std::vector< EventDispatcher * > ChildDispatchers;
	ChildDispatchers childDispatchers_;
};

} // namespace Mercury

#ifdef CODE_INLINE
#include "event_dispatcher.ipp"
#endif

#endif // EVENT_DISPATCHER_HPP
