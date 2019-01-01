/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WORKER_THREAD_HPP
#define WORKER_THREAD_HPP

#include "cstdmf/concurrency.hpp"

#include <map>

class WorkerJob;

/**
 *	This class is a worker thread.
 */
class WorkerThread : public SimpleThread	// hello java!
{
public:
	WorkerThread();
	~WorkerThread();

private:
	WorkerThread( const WorkerThread& );
	WorkerThread& operator=( const WorkerThread& );

	static void s_run( void * arg ) { ((WorkerThread*)arg)->run(); }
	void run();

	void add( WorkerJob * pJob, uint64 delay = 0 );
	void del( WorkerJob * pJob );

	typedef std::map<uint64,WorkerJob*> DoleQueue;
	DoleQueue		queue_;
	SimpleMutex		queueLock_;

	bool			ready_;
	bool			done_;

	friend class WorkerJob;
};

/**
 *	This class is a job that executes in a worker thread.
 *
 *	Jobs are in one of three states: in their worker thread's queue,
 *	currently running, or disowned. State transitions are guarded in such
 *	a way that it is safe to delete a job while it is running in another
 *	thread. It blocks until the current run of the job has completed.
 *
 *	Jobs present when the WorkerThread is deleted are disowned.
 */
class WorkerJob
{
public:
	WorkerJob();
	virtual ~WorkerJob();

	void submit( WorkerThread * pThread );
	void withdraw();

	/**
	 *	This is the method that is called to do stuff in the thread.
	 *	It should return the desired time in seconds that the thread
	 *	wants to wait before being called again. Zero is fine if you
	 *	want to be called asap (although other jobs might be in front).
	 *	If you return < 0 you will not be rescheduled (you will be
	 *	disowned) and if < -1 you will also be deleted.
	 */
	virtual float operator()() = 0;

	static const float DONT_RESCHEDULE = -0.5f;
	static const float DONT_RESCHEDULE_AND_DESTROY = -1.5f;

	bool isDisowned() const		{ return pThread_ == NULL; }

private:
	WorkerThread * pThread_;
	uint64			nextTime_;
	bool			running_;	// only changed under queueLock_!
	bool			deleting_;	// set when deletion desired

	void disowned();

	friend class WorkerThread;
};


#endif // WORKER_THREAD_HPP
