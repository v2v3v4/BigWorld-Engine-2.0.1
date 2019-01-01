/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef JOB_SYSTEM_HPP
#define JOB_SYSTEM_HPP

#include <cstdmf/stdmf.hpp>
#include <cstdmf/singleton.hpp>
#include "command_buffer.hpp"

//-----------------------------------------------------------------------------

class Job;
class SyncBlock;

//---------------------------------------------------------------------------

class __declspec( align( 16 ) ) Job
{
	friend class JobSystem;

private:
	SyncBlock* syncBlock_;		// The SyncBlock that the job is part of

private:
	virtual void execute() = 0;
};

//---------------------------------------------------------------------------

class SyncBlock
{
	friend class JobSystem;

private:
	uint numJobs_;

protected:
	void* next_;				// Back patched (to know how much we can consume)

private:
	virtual void consume() = 0;
};

//---------------------------------------------------------------------------

class JobSystem : public Singleton<JobSystem>
{
private:
	class FirstSyncBlock : public SyncBlock
	{
	public:
		virtual void consume() { }

	public:
		LONG numFrameSyncBlocks_;		// Not including this one
	};

public:
	JobSystem();
	~JobSystem();

	void init();
	void fini();

	void beginFrame();
	void endFrame();
	void flush();

	CommandBuffer* getConsumptionCommands();

	template<class C> C* allocSyncBlock();
	template<class C> C* allocJob( uint extraDataSize = 0 );
	template<class C> C* allocOutput( uint num = 1 );

	bool canBegin() const;

	void resetProfiling();

private:
	void jobLoop();									// Job cores
	void consumptionLoop();							// Consumption core
	void consumptionLoop2();						// Consumption core

	static void jobThreadFunc( void* arg );
	static void consumptionThreadFunc( void* arg );
	static void consumptionThreadFunc2( void* arg );

private:
	static const uint OUTPUT_SIZE = 4*1024*1024;

	// Cores and threads
	SimpleThread* jobThreads_;
	SimpleThread consumptionThread_;

	uint numCores_;
	uint cores_[32];

	// Command buffers
	CommandBuffer jobCommands_;
	CommandBuffer consumptionCommands_;

	// Frame counters and book keeping
	uint frameWriteIndex_;
	volatile bool grabFrame_;			// Its OK to grab a frame
	volatile LONG numAllocedFrames_;
	volatile LONG numQueuedFrames_;

	// Sync block counters
	volatile LONG writeIndex_;			// Written by job, read by consumption
	volatile LONG readIndex_;			// Written by consumption, read by job

	// Output buffer
	uint outputPos_;					// Main thread
	uint outputIndex_;					// Main thread
	uint8* outputBuffer_[2];

	// Book keeping for creating a chain of sync blocks (written by main thread)
	FirstSyncBlock firstSyncBlock_[CommandBuffer::NUM_BUFFERS];	// The first curSyncBlock
	SyncBlock* curSyncBlock_;			// Needed so we can back patch it later

	SimpleMutex jobReadMutex_;

	// Profiling
	volatile uint64 frameTimeStamp_;
	volatile uint64 mainWait_;
	volatile uint64 consumeWaitOnBuffer_;
	volatile uint64 consumeWaitOnJobs_;
	volatile uint64 consumeDoJobs_;
	volatile uint64 jobWait_[30];

	uint numJobs_;
	uint numSyncBlocks_;

	uint numJobsWatcher_;
	uint numSyncBlocksWatcher_;

	float frameTimeWatcher_;
	float mainUsageTimeWatcher_;
	float mainUsagePercentWatcher_;
	float consumeUsageTimeWatcher_;
	float consumeUsagePercentWatcher_;
	float loadingUsageTimeWatcher_;
	float loadingUsagePercentWatcher_;

	float sumOfCoresTimeWatcher_;
	float sumOfCoresPercentWatcher_;

	float consumeWaitOnBufferWatcher_;
	float consumeWaitOnJobsWatcher_;

	float jobUsageTimeWatcher_[30];
	float jobUsagePercentWatcher_[30];
};

//---------------------------------------------------------------------------

inline CommandBuffer* JobSystem::getConsumptionCommands()
{
	return &consumptionCommands_;
}

//---------------------------------------------------------------------------

template<class C> inline C* JobSystem::allocJob( uint extraDataSize )
{
	// Allocate and construct the job
	uint size = sizeof( C ) + extraDataSize;
	jobCommands_.write( size );
	jobCommands_.writePadding();
	C* job = (C*)jobCommands_.getCurrentWrite();
	jobCommands_.seekWrite( size );
	new( job ) C;

	// Link to sync block and count
	job->syncBlock_ = curSyncBlock_;
	curSyncBlock_->numJobs_++;

	numJobs_++;

	return job;
}

//---------------------------------------------------------------------------

template<class C> inline C* JobSystem::allocSyncBlock()
{
	// Swap the output buffer
	outputIndex_ ^= 1;
	outputPos_ = 0;

	// Back patch current sync block now that we know where it ends
	curSyncBlock_->next_ = consumptionCommands_.getCurrentWrite();

	// Allocate and construct the new sync block
	consumptionCommands_.write( sizeof( C ) );
	C* syncBlock = (C*)consumptionCommands_.getCurrentWrite();
	consumptionCommands_.seekWrite( sizeof( C ) );
	new( syncBlock ) C;
	syncBlock->numJobs_ = 0;	// As jobs are allocated this will increment

	// Write it into the job commands
	jobCommands_.write( syncBlock );

	// The new sync block is now the current. Add to counter and return
	firstSyncBlock_[frameWriteIndex_].numFrameSyncBlocks_++;
	curSyncBlock_ = syncBlock;

	numSyncBlocks_++;

	return syncBlock;
}

//---------------------------------------------------------------------------

template<class C> inline C* JobSystem::allocOutput( uint num )
{
	C* output = (C*)( outputBuffer_[outputIndex_] + outputPos_ );
	outputPos_ += num * sizeof( C );
	MF_ASSERT( outputPos_ <= OUTPUT_SIZE );
	return output;
}

//-----------------------------------------------------------------------------

inline bool JobSystem::canBegin() const
{
	return ( numAllocedFrames_ < CommandBuffer::NUM_BUFFERS );
}
//-----------------------------------------------------------------------------

#endif // JOB_SYSTEM_HPP
