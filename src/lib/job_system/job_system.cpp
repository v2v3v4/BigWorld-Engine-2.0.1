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

#include "cstdmf/singleton.hpp"
#include "cstdmf/bgtask_manager.hpp"
#include "job_system.hpp"

//---------------------------------------------------------------------------

DECLARE_DEBUG_COMPONENT2( "JobSystem", 0 )

PROFILER_DECLARE2( JobSystem_beginFrame, "JobSystem BeginFrame", Profiler::FLAG_WATCH );
PROFILER_DECLARE2( JobSystem_flush, "JobSystem Flush", Profiler::FLAG_WATCH );

BW_SINGLETON_STORAGE( JobSystem );

static float s_dampingRatio = 0.95f;

//---------------------------------------------------------------------------

JobSystem::JobSystem()
{
}

//---------------------------------------------------------------------------

JobSystem::~JobSystem()
{
}

//---------------------------------------------------------------------------

void JobSystem::init()
{
	BW_GUARD;

	// Command buffers
	jobCommands_.init( 1024*1024 );
	consumptionCommands_.init( 64*1024*1024 );

	// Init frame counters
	frameWriteIndex_ = 0;
	grabFrame_ = true;
	numAllocedFrames_ = 0;
	numQueuedFrames_ = 0;

	// Init SyncBlock counters
	writeIndex_ = 0;
	readIndex_ = 0;

	// Output buffer
	outputPos_ = 0;
	outputIndex_ = 0;
	outputBuffer_[0] = (uint8*)malloc( OUTPUT_SIZE );
	outputBuffer_[1] = (uint8*)malloc( OUTPUT_SIZE );

	// Profiling and watchers
	frameTimeStamp_ = timestamp();
	mainWait_ = 0;
	consumeWaitOnBuffer_ = 0;
	consumeWaitOnJobs_ = 0;
	consumeDoJobs_ = 0;

	numJobsWatcher_ = 0;
	numSyncBlocksWatcher_ = 0;

	frameTimeWatcher_ = 0.0f;
	mainUsageTimeWatcher_ = 0.0f;
	mainUsagePercentWatcher_ = 0.0f;
	consumeUsageTimeWatcher_ = 0.0f;
	consumeUsagePercentWatcher_ = 0.0f;
	loadingUsageTimeWatcher_ = 0.0f;
	loadingUsagePercentWatcher_ = 0.0f;
	sumOfCoresTimeWatcher_ = 0.0f;
	sumOfCoresPercentWatcher_ = 0.0f;

	consumeWaitOnBufferWatcher_ = 0.0f;
	consumeWaitOnJobsWatcher_ = 0.0f;

	MF_WATCH( "Job System/Number of Jobs", numJobsWatcher_, Watcher::WT_READ_ONLY,
		"Number of jobs executing per frame" );
	MF_WATCH( "Job System/Number of SyncBlocks", numSyncBlocksWatcher_, Watcher::WT_READ_ONLY,
		"Number of synchronisation blocks (fences) per frame" );

	MF_WATCH( "Job System/Filter Damping Ratio", s_dampingRatio, Watcher::WT_READ_WRITE,
		"Damping ratio for all job system watchers. Set to 0 to disable" );
	MF_WATCH( "Job System/Frame Time", frameTimeWatcher_, Watcher::WT_READ_ONLY,
		"The frame time in milliseconds" );

	MF_WATCH( "Job System/Percent - D3D Core", consumeUsagePercentWatcher_, Watcher::WT_READ_ONLY,
		"Percentage of time that the D3D core spends working" );
	MF_WATCH( "Job System/Percent - Main Core", mainUsagePercentWatcher_, Watcher::WT_READ_ONLY, 
		"Percentage of time that the main core spends working" );
	MF_WATCH( "Job System/Percent - Sum of Cores", sumOfCoresPercentWatcher_, Watcher::WT_READ_ONLY,
		"Percentage of time that the collection of all cores spend working" );

	MF_WATCH( "Job System/Time - D3D Core", consumeUsageTimeWatcher_, Watcher::WT_READ_ONLY,
		"Time that the D3D core spends working" );
	MF_WATCH( "Job System/Time - Main Core", mainUsageTimeWatcher_, Watcher::WT_READ_ONLY,
		"Time that the main core spends working" );
	MF_WATCH( "Job System/Time - Sum of Cores", sumOfCoresTimeWatcher_, Watcher::WT_READ_ONLY,
		"Average time that the collection of all cores spends working" );

	MF_WATCH( "Job System/Wait on Buffer", consumeWaitOnBufferWatcher_, Watcher::WT_READ_ONLY,
		"Time that the D3D core spends waiting for the main core to submit rendering commands" );

	MF_WATCH( "Job System/Wait on Jobs", consumeWaitOnJobsWatcher_, Watcher::WT_READ_ONLY,
		"Time that the D3D core spends waiting for the job cores to produce the data it needs" );

	// Count and assign cores
	DWORD processAffinity;
	DWORD systemAffinity;
	BOOL res = GetProcessAffinityMask( GetCurrentProcess(), &processAffinity, &systemAffinity );
	if ( !res )
	{
		ERROR_MSG( "Job system could not get process affinity mask\n" );
	}

	numCores_ = 0;
	for ( uint i = 0; i < 32; i++ )
	{
		if ( processAffinity & ( 1 << i ) )
		{
			cores_[ numCores_++ ] = 1 << i;
		}
	}

	MF_WATCH( "Job System/Number of Cores", numCores_, Watcher::WT_READ_ONLY,
		"Number of processing cores available to the client" );

	// Start threads
	if ( numCores_ >= 4 )
	{
		// General case, one core for consumption and the rest for jobs
		consumptionThread_.init( &consumptionThreadFunc, NULL );
		Profiler::instance().addThread( consumptionThread_.handle(), "Consumption" );

		// Start the job cores
		uint numJobThreads = numCores_ - 3;
		jobThreads_ = new SimpleThread[numJobThreads];
		for ( uint i = 0; i < numJobThreads; i++ )
		{
			char threadName[20];
			bw_snprintf( threadName, sizeof( threadName ), "Job %d\0", i );
			jobThreads_[i].init( &jobThreadFunc, NULL );
			Profiler::instance().addThread( jobThreads_[i].handle(), threadName );

			char watcherName[40];
			jobWait_[i] = 0;

			jobUsageTimeWatcher_[i] = 0.0f;
			bw_snprintf( watcherName, sizeof( watcherName ), "Job System/Time - Job Core %d", i );
			MF_WATCH( watcherName, jobUsageTimeWatcher_[i], Watcher::WT_READ_ONLY,
				"Time that the job core spends working" );

			jobUsagePercentWatcher_[i] = 0.0f;
			bw_snprintf( watcherName, sizeof( watcherName ), "Job System/Percent - Job Core %d", i );
			MF_WATCH( watcherName, jobUsagePercentWatcher_[i], Watcher::WT_READ_ONLY,
				"Percentage of time that the job core spends working" );
		}

		// Add watchers for loading thread
		MF_WATCH( "Job System/Percent - Loading Core", loadingUsagePercentWatcher_, Watcher::WT_READ_ONLY,
			"Percentage of time that the loading core spends working" );

		MF_WATCH( "Job System/Time - Loading Core", loadingUsageTimeWatcher_, Watcher::WT_READ_ONLY,
			"Time that the loading core spends working" );
	}
	else
	{
		// Special case for two cores. Jobs are executed on the consumption thread.
		consumptionThread_.init( &consumptionThreadFunc2, NULL );
		Profiler::instance().addThread( consumptionThread_.handle(), "Consumption and Jobs" );

		// Add watchers for loading thread
		MF_WATCH( "Job System/Percent - Loading Thread", loadingUsagePercentWatcher_, Watcher::WT_READ_ONLY,
			"Percentage of the time available to a core that the loading thread spends (on whichever core is available)" );

		MF_WATCH( "Job System/Time - Loading Thread", loadingUsageTimeWatcher_, Watcher::WT_READ_ONLY,
			"Time that the loading thread spends working" );
	}
}

//---------------------------------------------------------------------------

void JobSystem::fini()
{
}

//---------------------------------------------------------------------------

void JobSystem::beginFrame()
{
	BW_GUARD;
	PROFILER_SCOPED( JobSystem_beginFrame );

	// Stall until a buffer is available
	uint64 startTime = timestamp();
	while ( numAllocedFrames_ >= CommandBuffer::NUM_BUFFERS )
	{
		SwitchToThread();
	}
	uint64 waitTime = timestamp() - startTime;
	mainWait_ += waitTime;

	InterlockedIncrement( &numAllocedFrames_ );

	// Add an initial curSyncBlock so first back patching isn't a special case
	firstSyncBlock_[frameWriteIndex_].numFrameSyncBlocks_ = 0;
	curSyncBlock_ = &firstSyncBlock_[frameWriteIndex_];
	jobCommands_.write( &firstSyncBlock_[frameWriteIndex_]);
}

//---------------------------------------------------------------------------

void JobSystem::endFrame()
{
	BW_GUARD;

	// Back patch the final sync block
	curSyncBlock_->next_ = (SyncBlock*)consumptionCommands_.getCurrentWrite();

	// Finalise and queue up the buffers
	consumptionCommands_.nextWrite();
	jobCommands_.nextWrite();

	InterlockedIncrement( &numQueuedFrames_ );

	// Advance for next frame
	frameWriteIndex_ = ( frameWriteIndex_ + 1 ) % CommandBuffer::NUM_BUFFERS;
}

//---------------------------------------------------------------------------

static volatile LONG numSyncBlocksInFrame;

void JobSystem::flush()
{
	BW_GUARD;
	PROFILER_SCOPED( JobSystem_flush );

	while ( numQueuedFrames_ )
		SwitchToThread();
}

//---------------------------------------------------------------------------
// The main loop that runs on each thread

void JobSystem::jobLoop()
{
	BW_GUARD;

	static volatile LONG numJobsTotal;
	static volatile LONG numJobsStarted;
	static volatile LONG numJobsFinished;
	static volatile bool haveSyncBlock;

	static volatile LONG numSyncBlocksTotal;
	static volatile LONG numSyncBlocksStarted;
	static volatile bool haveFrame;

	static volatile LONG numCoresWaitingToSync;
	static volatile LONG coreCount;

	LONG coreId = InterlockedIncrement( &coreCount ) - 1;

	SetThreadAffinityMask( GetCurrentThread(), cores_[coreId + 2] );

	if ( coreId == 0 )
	{
		// Wait until consumption finishes and another frame appears
		while ( !grabFrame_ || !numQueuedFrames_ )
			;

		// Grab the first frame
		{
			SimpleMutexHolder smh( jobReadMutex_ );

			if ( !haveFrame )
			{
				FirstSyncBlock* firstSyncBlock = jobCommands_.read<FirstSyncBlock*>();

				numSyncBlocksInFrame = firstSyncBlock->numFrameSyncBlocks_;
				numSyncBlocksTotal = numSyncBlocksInFrame;
				numSyncBlocksStarted = 0;

				haveFrame = true;
				grabFrame_ = false;
			}
		}
	}
	else
	{
		while ( !haveFrame )
			SwitchToThread();
	}

	// Execute the sync blocks until we're done
	while ( 1 )
	{
		// Stall until we have a frame
		MF_ASSERT( haveFrame )

		// Stall until readIndex catches up
		uint64 startTime = timestamp();
		while ( ( readIndex_ + 2 ) <= writeIndex_ )
			;
		uint64 waitTime = timestamp() - startTime;
		jobWait_[coreId] += waitTime;

		Job* job;
		bool doJob;

		{
			SimpleMutexHolder smh( jobReadMutex_ );
			bool grabSyncBlock = !( numSyncBlocksStarted == numSyncBlocksTotal );

			// If we're not working on a sync block, grab one
			if ( !haveSyncBlock && grabSyncBlock )
			{
				SyncBlock* syncBlock = jobCommands_.read<SyncBlock*>();

				numJobsTotal = syncBlock->numJobs_;
				numJobsStarted = 0;
				numJobsFinished = 0;

				haveSyncBlock = true;
				numSyncBlocksStarted++;
			}

			// See if there are unstarted jobs, grab one if there are any
			doJob = !( numJobsStarted == numJobsTotal );

			if ( doJob )
			{
				uint size = jobCommands_.read<uint>();
				jobCommands_.skipPadding();
				job = (Job*)jobCommands_.getCurrent();
				jobCommands_.seek( size );

				numJobsStarted++;
			}
		}

		// If we got a job, do it. Otherwise wait until the sync block is done
		if ( doJob )
		{
			// Execute the job
			job->execute();

			// Increment finished count, move to next sync block on last job
			{
				SimpleMutexHolder smh( jobReadMutex_ );

				// Did we just complete the final job in the sync block?
				if ( ( numJobsFinished + 1 ) == numJobsTotal )
				{
					// The syncBlock is done, wait until all cores stall
					jobReadMutex_.give();
					uint64 startTime = timestamp();
					while ( numCoresWaitingToSync != ( coreCount - 1 ) )
						;
					jobReadMutex_.grab();
					uint64 waitTime = timestamp() - startTime;
					jobWait_[coreId] += waitTime;

					// All cores in sync so let them try to get another sync block
					InterlockedIncrement( &writeIndex_ );
					haveSyncBlock = false;

					// Did we just complete the final sync block in the frame?
					if ( writeIndex_ == numSyncBlocksTotal )
					{
						// Wait until consumption finishes and another frame appears
						uint64 startTime = timestamp();
						while ( !grabFrame_ || !numQueuedFrames_ )
						{
							SwitchToThread();
						}
						uint64 waitTime = timestamp() - startTime;
						jobWait_[coreId] += waitTime;

						// Grab it
						writeIndex_ = 0;
						jobCommands_.nextRead();

						FirstSyncBlock* firstSyncBlock = jobCommands_.read<FirstSyncBlock*>();

						numSyncBlocksInFrame = firstSyncBlock->numFrameSyncBlocks_;
						numSyncBlocksTotal = numSyncBlocksInFrame;
						numSyncBlocksStarted = 0;

						haveFrame = true;
						grabFrame_ = false;
					}
				}

				// Free other cores
				InterlockedIncrement( &numJobsFinished );
			}
		}
		else
		{
			// Wait until the remaining started jobs finish
			uint64 startTime = timestamp();
			InterlockedIncrement( &numCoresWaitingToSync );
			while ( numJobsFinished != numJobsTotal )
				;
			InterlockedDecrement( &numCoresWaitingToSync );
			uint64 waitTime = timestamp() - startTime;
			jobWait_[coreId] += waitTime;
		}
	}
}

//---------------------------------------------------------------------------

void JobSystem::consumptionLoop()
{
	BW_GUARD;

	SetThreadAffinityMask( GetCurrentThread(), cores_[1] );

	while ( 1 )
	{
		// Execute the sync blocks
		do
		{
			// Stall until writeIndex catches up
			uint64 startTime = timestamp();
			while ( writeIndex_ <= readIndex_ )
				;
			uint64 waitTime = timestamp() - startTime;
			consumeWaitOnJobs_ += waitTime;

			// Get the sync block and consume the buffer
			uint size = consumptionCommands_.read<uint>();
			SyncBlock* syncBlock = (SyncBlock*)consumptionCommands_.getCurrent();
			consumptionCommands_.seek( size );

			syncBlock->consume();

			// We've read this buffer so it can be retired
			InterlockedIncrement( &readIndex_ );

		} while ( readIndex_ < numSyncBlocksInFrame );

		// Retire and reset the frame
		InterlockedDecrement( &numAllocedFrames_ );
		InterlockedDecrement( &numQueuedFrames_ );

		readIndex_ = 0;
		consumptionCommands_.nextRead();

		// Tell the cores to grab a frame and wait until they do
		grabFrame_ = true;
		uint64 startTime = timestamp();
		while ( grabFrame_ )
		{
			SwitchToThread();
		}
		uint64 waitTime = timestamp() - startTime;
		consumeWaitOnBuffer_ += waitTime;
	}
}

//---------------------------------------------------------------------------

void JobSystem::consumptionLoop2()
{
	BW_GUARD;

	SetThreadAffinityMask( GetCurrentThread(), cores_[1] );

	double stampsPerMs = stampsPerSecondD() / 1000.0;

	while ( 1 )
	{
		// Stall until writeIndex catches up
		uint64 startTime = timestamp();
		while ( !numQueuedFrames_ )
		{
			SwitchToThread();
		}
		uint64 waitTime = timestamp() - startTime;
		consumeWaitOnBuffer_ += waitTime;

		FirstSyncBlock* firstSyncBlock = jobCommands_.read<FirstSyncBlock*>();

		// Execute jobs and sync blocks
		for ( LONG i = 0; i < firstSyncBlock->numFrameSyncBlocks_; i++ )
		{
			SyncBlock* syncBlock = jobCommands_.read<SyncBlock*>();

			// Execute the jobs
			for ( uint j = 0; j < syncBlock->numJobs_; j++ )
			{
				uint size = jobCommands_.read<uint>();
				jobCommands_.skipPadding();
				Job* job = (Job*)jobCommands_.getCurrent();
				jobCommands_.seek( size );
				job->execute();
			}

			// Jump over the sync block
			uint size = consumptionCommands_.read<uint>();
			consumptionCommands_.seek( size );

			// Consume the buffer
			syncBlock->consume();
		}

		// Retire the frame
		InterlockedDecrement( &numAllocedFrames_ );
		InterlockedDecrement( &numQueuedFrames_ );

		// Next frame
		jobCommands_.nextRead();
		consumptionCommands_.nextRead();
	}
}

//---------------------------------------------------------------------------

void JobSystem::jobThreadFunc( void* arg )
{
	JobSystem::instance().jobLoop();
}

//---------------------------------------------------------------------------

void JobSystem::consumptionThreadFunc( void* arg )
{
	JobSystem::instance().consumptionLoop();
}

//---------------------------------------------------------------------------

void JobSystem::consumptionThreadFunc2( void* arg )
{
	JobSystem::instance().consumptionLoop2();
}

//-----------------------------------------------------------------------------

static inline float ticksToMs( uint64 ticks )
{
	static const double stampsPerMs = stampsPerSecondD() / 1000.0;
	return ( float )( ( double )ticks / stampsPerMs );
}

static inline float filter( float cur, float prev )
{
	return max( 0.0f, prev * s_dampingRatio + cur * ( 1.0f - s_dampingRatio ) );
}

void JobSystem::resetProfiling()
{
	numJobsWatcher_ = numJobs_;
	numSyncBlocksWatcher_ = numSyncBlocks_;

	uint64 prevFrameTimeStamp = frameTimeStamp_;
	frameTimeStamp_ = timestamp();

	s_dampingRatio = min( 1.0f, max( 0.0f, s_dampingRatio ) );

	float frameTime = ticksToMs( frameTimeStamp_ - prevFrameTimeStamp );
	frameTimeWatcher_ = filter( frameTime, frameTimeWatcher_ );

	float consumeUsageTime = frameTime - ticksToMs( consumeWaitOnBuffer_ + consumeWaitOnJobs_ );
	consumeUsageTimeWatcher_ = filter( consumeUsageTime, consumeUsageTimeWatcher_ );
	consumeUsagePercentWatcher_ = filter( ( 100.0f * consumeUsageTime ) / frameTime,
		consumeUsagePercentWatcher_ );

	float mainUsageTime = frameTime - ticksToMs( mainWait_ );
	mainUsageTimeWatcher_ = filter( mainUsageTime, mainUsageTimeWatcher_ );
	mainUsagePercentWatcher_ = filter( ( 100.0f * mainUsageTime ) / frameTime,
		mainUsagePercentWatcher_ );

	float sumOfCoresTime = consumeUsageTime + mainUsageTime;

	consumeWaitOnBufferWatcher_ = filter( ticksToMs( consumeWaitOnBuffer_ ),
		consumeWaitOnBufferWatcher_ );
	consumeWaitOnJobsWatcher_ = filter( ticksToMs( consumeWaitOnJobs_ ),
		consumeWaitOnJobsWatcher_ );

	uint numJobThreads = numCores_ - 2;
	for ( uint i = 0; i < numJobThreads; i++ )
	{
		float jobUsageTime = frameTime - ticksToMs( jobWait_[i] );
		jobUsageTimeWatcher_[i] = filter( jobUsageTime, jobUsageTimeWatcher_[i] );
		jobUsagePercentWatcher_[i] = filter( ( 100.0f * jobUsageTime ) / frameTime,
			jobUsagePercentWatcher_[i] );

		sumOfCoresTime += jobUsageTime;

		jobWait_[i] = 0;
	}

	sumOfCoresTime /= float( numCores_ );
	sumOfCoresTimeWatcher_ = filter( sumOfCoresTime, sumOfCoresTimeWatcher_ );
	sumOfCoresPercentWatcher_ = filter( ( 100.0f * sumOfCoresTime ) / frameTime,
		sumOfCoresPercentWatcher_ );

	numJobs_ = 0;
	numSyncBlocks_ = 0;

	mainWait_ = 0;
	consumeWaitOnBuffer_ = 0;
	consumeWaitOnJobs_ = 0;
	consumeDoJobs_ = 0;

	// Loading thread
	HANDLE loadingThreadHandle;
	const std::vector<HANDLE> & bgTaskThreads = BgTaskManager::instance().getThreadHandles();
	if ( bgTaskThreads.size() > 0 )
	{
		loadingThreadHandle = bgTaskThreads[0];
		if ( bgTaskThreads.size() > 1 )
		{
			WARNING_MSG( "More than one loading thread, profiling will ignore extra threads\n" );
		}
	}
	else
	{
		loadingThreadHandle = 0;
	}

	// Loading thread
	if ( !loadingThreadHandle )
		return;

	FILETIME dummy;
	FILETIME threadKernel;
	FILETIME threadUser;

	GetThreadTimes( loadingThreadHandle, &dummy, &dummy, &threadUser, &threadKernel );

	uint64 threadTime	= ( uint64( threadKernel.dwHighDateTime ) << 32 )
						+ uint64( threadKernel.dwLowDateTime )
						+ ( uint64( threadUser.dwHighDateTime ) << 32 )
						+ uint64( threadUser.dwLowDateTime );

	static float t;
	static uint64 base;

	t += ( float )( double( threadTime - base ) / 10000.0 );
	base = threadTime;

	float dt = min( t, frameTime );
	t -= dt;

	loadingUsageTimeWatcher_ = filter( dt, loadingUsageTimeWatcher_ );
	loadingUsagePercentWatcher_ = filter( ( 100.0f * dt ) / frameTime,
		loadingUsagePercentWatcher_ );
}
