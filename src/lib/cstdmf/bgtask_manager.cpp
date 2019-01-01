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

#include "bgtask_manager.hpp"
#include "diary.hpp"
#include "profiler.hpp"

DECLARE_DEBUG_COMPONENT2( "CStdMF", 0 )

/*static*/ BgTaskManager* BgTaskManager::s_instance_ = NULL;


// -----------------------------------------------------------------------------
// Section: BackgroundTask
// -----------------------------------------------------------------------------

//#define FORCE_MAIN_THREAD

/**
 *	Constructor.
 */
CStyleBackgroundTask::CStyleBackgroundTask( FUNC_PTR bgFunc, void * bgArg,
		FUNC_PTR fgFunc, void * fgArg ) :
	bgFunc_( bgFunc ),
	bgArg_( bgArg ),
	fgFunc_( fgFunc ),
	fgArg_( fgArg )
{
}


/**
 *	This method is called in a background thread to perform this task. The task
 *	manager is passed in. At the end of the background task, this object is
 *	added back as a main thread task, if necessary.
 */
void CStyleBackgroundTask::doBackgroundTask( BgTaskManager & mgr )
{
	if (bgFunc_)
	{
		(*bgFunc_)( bgArg_ );
	}

	if (fgFunc_)
	{
		mgr.addMainThreadTask( this );
	}
}


/**
 *	This method override BackgroundTask and is called in the main thread
 *	once the background task has finished.
 */
void CStyleBackgroundTask::doMainThreadTask( BgTaskManager & mgr )
{
	if (fgFunc_)
	{
		(*fgFunc_)( fgArg_ );
	}
}


// -----------------------------------------------------------------------------
// Section: BackgroundTaskThread
// -----------------------------------------------------------------------------

//Note about ENABLE_THREAD_TIMING_SYNCHRONISATION.  This can be turned on if you
//need accurate results in your Diary profiling.  It does two things:
//1. changes the threading organisation so that each thread runs on exactly one
//cpu.  This allows the use of fast accurate timestamps and avoids problems
//where a particular task is switched between cpus.
//2. performs a very quick synchronisation task that outputs special diary
//entries from the separate threads as close together as possible, diary spy
//uses this to line up the two sets of timing data.
#ifdef _WIN32
#if (ENABLE_DIARIES)
#define ENABLE_THREAD_TIMING_SYNCHRONISATION 0
#endif	//ENABLE_DIARIES
#endif	//_WIN32

#if ENABLE_THREAD_TIMING_SYNCHRONISATION
volatile int g_threadSync = 0;
#endif

/**
 * BackgroundTaskThread
 */
BackgroundTaskThread::BackgroundTaskThread( BgTaskManager & mgr,
		BackgroundThreadDataPtr pData ) :
	SimpleThread(),
	mgr_( mgr ),
	pData_( pData )
{
	this->SimpleThread::init( BackgroundTaskThread::s_start, this );

#if ENABLE_THREAD_TIMING_SYNCHRONISATION
	while ( !g_threadSync );	//poll variable until set
	DiaryEntryPtr de = Diary::instance().add( "Global thread start 1" );
	de->stop();
#endif
}


/**
 *	This static method is the entry-point for the background thread.
 */
void BackgroundTaskThread::s_start( void * arg )
{
	BackgroundTaskThread * pTaskThread = (BackgroundTaskThread*)arg;

#if ENABLE_THREAD_TIMING_SYNCHRONISATION
	SetThreadAffinityMask(GetCurrentThread(), 2);
	g_threadSync = 1;
	DiaryEntryPtr de = Diary::instance().add( "Global thread start 2" );
	de->stop();
#elif defined( _WIN32 )
	// If we have a quad core or better put the background tasks on a
	// dedicated core (the last one)
	uint numCores = getNumCores();
	if ( numCores > 2 )
	{
		SetThreadAffinityMask(GetCurrentThread(), 1 << ( numCores - 1 ) );
	}
#endif

	pTaskThread->run();
}


/**
 *	This method is the main function of the background thread.
 */
void BackgroundTaskThread::run()
{
	if (pData_)
	{
		pData_->onStart( *this );
	}

	while (true)
	{
		BackgroundTaskPtr pTask = mgr_.pullBackgroundTask();

		// A NULL task indicates that the thread should terminate.
		if (pTask)
		{
			#ifdef FORCE_MAIN_THREAD
			MF_ASSERT( 0 );
			#endif
			#ifdef _WIN32
			InterlockedIncrement( &mgr_.workingCount_ );
			#endif
			pTask->doBackgroundTask( mgr_, this );
			#ifdef _WIN32
			InterlockedDecrement( &mgr_.workingCount_ );
			#endif
		}
		else
		{
			break;
		}
	}

	if (pData_)
	{
		pData_->onEnd( *this );
	}

	// Inform the main thread that this thread has terminated.
	mgr_.addMainThreadTask( new ThreadFinisher( this ) );
}

/**
 *	This method tells the caller if any background tasks are either being
 *  processed or queued.
 */

#ifdef _WIN32
bool BgTaskManager::isWorking()
{
	// If any threads are working on a task, we are working
	if ( workingCount_ )
	{
		return true;
	}

	// If there is anything in the list, we are working
	if ( !bgTaskList_.isEmpty() )
	{
		return true;
	}

	// Not working
	return false;
}
#endif


// -----------------------------------------------------------------------------
// Section: ThreadFinisher
// -----------------------------------------------------------------------------

/**
 *	This method informs the thread manager that a thread has finished.
 */
void ThreadFinisher::doMainThreadTask( BgTaskManager & mgr )
{
	mgr.onThreadFinished( pThread_ );
}


// -----------------------------------------------------------------------------
// Section: BackgroundTaskList
// -----------------------------------------------------------------------------

/**
 *	This method adds a task to the list of tasks in a thread-safe way.
 *
 *	@param pTask	The task to add
 *	@param priority	The priority to add this task at. A higher value means that
 *		the task is done earlier.
 */
void BgTaskManager::BackgroundTaskList::push( BackgroundTaskPtr pTask,
												int priority )
{
	{
		SimpleMutexHolder holder( mutex_ );

		// Insert with highest priority value tasks first
		List::reverse_iterator iter = list_.rbegin();

		while ((iter != list_.rend()) && (priority > iter->first))
		{
			++iter;
		}

		list_.insert( iter.base(), std::make_pair( priority, pTask ) );
	}

	semaphore_.push();
}


/**
 *	This method pulls a task from the list of tasks in a thread-safe way. It
 *	waits on a semaphore if no tasks exist.
 */
BackgroundTaskPtr BgTaskManager::BackgroundTaskList::pull()
{
	semaphore_.pull();
	mutex_.grab();
	BackgroundTaskPtr pTask = list_.front().second;
	list_.pop_front();
	mutex_.give();

	return pTask;
}

/**
 *	This method tells the caller if the list is empty.
 */
bool BgTaskManager::BackgroundTaskList::isEmpty()
{
	SimpleMutexHolder holder( mutex_ );
	return list_.empty();
}

/**
 *	This method clears the task list.
 */
void BgTaskManager::BackgroundTaskList::clear()
{
	mutex_.grab();

	while (semaphore_.pullTry())
	{
		list_.pop_front();
	}

	mutex_.give();
}


// -----------------------------------------------------------------------------
// Section: BgTaskManager
// -----------------------------------------------------------------------------

/**
 *	The method returns the singleton instance of this class.
 *
 *	Note: This class does not need to be run as a singleton, other instances can
 *	be created. If this method is not called, no singleton is created.
 */
BgTaskManager & BgTaskManager::instance()
{
	if (s_instance_ == NULL)
	{
		s_instance_ = new BgTaskManager();
	}
	return *s_instance_;

}


/*
 * Explicit clean up
 */
void BgTaskManager::fini()
{
	if (s_instance_)
	{
		delete s_instance_;
		s_instance_ = NULL;
	}
}


/**
 *	Constructor.
 */
BgTaskManager::BgTaskManager() :
	numRunningThreads_( 0 ),
	numUnstoppedThreads_( 0 ),
	workingCount_( 0 )
{
}


/**
 *	Destructor.
 */
BgTaskManager::~BgTaskManager()
{
	this->stopAll();
}


/**
 *	This method starts the background threads.
 *
 *	@param numThreads The number of threads to add to the thread pool.
 */
void BgTaskManager::startThreads( int numThreads,
			BackgroundThreadDataPtr pData )
{
	numUnstoppedThreads_ += numThreads;
	numRunningThreads_ += numThreads;

	for (int i = 0; i < numThreads; ++i)
	{
		// This object deletes itself
		BackgroundTaskThread * pThread =
			new BackgroundTaskThread( *this, pData );

#ifdef _WIN32
		Profiler::instance().addThread( pThread->handle(), "Background Task" );
		threadHandles_.push_back( pThread->handle() );
#else
		// Stop compiler warning of unused variable
		(void)pThread;
#endif
	}
}


/**
 *	This method stops all threads. This method should only be called by the main
 *	thread.
 *
 *	@param discardPendingTasks If true, tasks that have yet to be added to a
 *		background thread will be deleted.
 *  @param waitForThreads	If true, this method blocks until all
 *                          the owned threads have terminated.
 */
void BgTaskManager::stopAll( bool discardPendingTasks, bool waitForThreads )
{
	if (discardPendingTasks)
	{
		bgTaskList_.clear();
	}

	for (int i = 0; i < numUnstoppedThreads_; ++i)
	{
		// NULL indicates that a thread should shut down.
		bgTaskList_.push( NULL );
	}

	numUnstoppedThreads_ = 0;

	if (waitForThreads && numRunningThreads_ > 0)
	{
		TRACE_MSG( "BgTaskManager::stopAll: Waiting for threads: %d\n",
			numRunningThreads_ );

		while (true)
		{
			this->tick();

			if (numRunningThreads_)
			{
#ifdef _WIN32
				Sleep( 100 );
#else
				usleep( 100000 );
#endif
			}
			else
			{
				break;
			}
		}
	}
}


/**
 *	This method adds a task that should be processed by a background thread.
 *
 *	@param pTask	The task to add.
 *	@param priority	The priority to add this task at. A higher value means that
 *		the task is done earlier.
 */
void BgTaskManager::addBackgroundTask( BackgroundTaskPtr pTask, int priority )
{
#ifdef FORCE_MAIN_THREAD
	pTask->doBackgroundTask( *this, NULL );
#else
	bgTaskList_.push( pTask, priority );
#endif
}


/**
 *	This method adds a task that should be processed by a background thread.
 */
void BgTaskManager::addMainThreadTask( BackgroundTaskPtr pTask )
{
	fgTaskListMutex_.grab();
	fgTaskList_.push_back( pTask );
	fgTaskListMutex_.give();
}


/**
 *	This method should be called periodically in the main thread. It processes
 *	all foreground tasks.
 */
void BgTaskManager::tick()
{
	fgTaskListMutex_.grab();
	fgTaskList_.swap( newTasks_ );
	fgTaskListMutex_.give();

	ForegroundTaskList::iterator iter = newTasks_.begin();

	while (iter != newTasks_.end())
	{
		(*iter)->doMainThreadTask( *this );

		++iter;
	}

	newTasks_.clear();
}


/**
 *	This method is called to inform the manager that a thread has finished.
 */
void BgTaskManager::onThreadFinished( BackgroundTaskThread * pThread )
{
	--numRunningThreads_;
	delete pThread;
	TRACE_MSG( "BgTaskManager::onThreadFinished: "
		"Thread finished. %d remaining\n", numRunningThreads_ );
}


/**
 *	This method pulls a task from the list of background tasks in a thread-safe
 *	way. It waits on a semaphore if no tasks exist.
 */
BackgroundTaskPtr BgTaskManager::pullBackgroundTask()
{
	return bgTaskList_.pull();
}

// bgtask_manager.cpp
