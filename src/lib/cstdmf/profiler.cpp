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

#include "profiler.hpp"
#include "bw_util.hpp"
#include "debug.hpp"
#include "watcher.hpp"

DECLARE_DEBUG_COMPONENT2( "CStdMF", 0 )

// -----------------------------------------------------------------------------
// Section Profiler
// -----------------------------------------------------------------------------

Profiler* Profiler::instance_ = NULL;

static const float DAMPING_RATIO = 0.97f;

static void DestroyProfiler()
{
	if ( Profiler* profiler = &Profiler::instanceNoCreate() )
		delete profiler;
}

#ifdef _WIN32

static bool s_resetThreadTimes;
static float s_kernelTime;
static float s_processTime;

void Profiler::addThread( HANDLE thread, const char* name )
{
	ThreadInfo* info = new ThreadInfo;

	info->name_ = std::string( "Profiler Threads/" ) + std::string( name );
	info->base_ = 0;
	info->time_ = 0.0f;

	threads_[thread] = info;

	MF_WATCH( info->name_.c_str(), info->time_, Watcher::WT_READ_ONLY );
}

#endif

/**
 *
 */
Profiler::Profiler() :
	threadId_( OurThreadID() ),
	prevTime_( timestamp() ),
	historyFile_( NULL ),
	slotNamesWritten_( false ),
	frameCount_( 0 ),
	numSlots_( 1 ),
	curSlot_( 0 ),
	slotStackPos_( 0 )
{
	memset( slots_, 0, sizeof( slots_ ) );
	slots_[0].name_ = "Unaccounted";

#ifdef _WIN32
	MF_WATCH( "Profiler/Unaccounted", slots_[0].curTimeMs_, Watcher::WT_READ_ONLY );
	MF_WATCH( "Profiler Threads/Reset", s_resetThreadTimes );
	MF_WATCH( "Profiler Threads/Time Kernel", s_kernelTime, Watcher::WT_READ_ONLY );
	MF_WATCH( "Profiler Threads/Time Process", s_processTime, Watcher::WT_READ_ONLY );

	addThread( GetCurrentThread(), "Main" );
#endif

	instance_ = this;

	atexit( DestroyProfiler );

#if ENABLE_PROFILER
//	historyFile_ = bw_fopen( "profiler.csv", "w" );
#endif

	this->begin( 0 );
}


/**
 *
 */
Profiler::~Profiler()
{
	while ( slotStackPos_ )
		this->end();

	closeHistory();

	Profiler::instance_ = NULL;
}


/**
 *
 */
void Profiler::tick()
{
	this->end();
	MF_ASSERT( slotStackPos_ == 0 );

#ifdef _WIN32
	// Thread profiling
	FILETIME dummy;
	FILETIME processKernel;
	FILETIME processUser;
	FILETIME threadKernel;
	FILETIME threadUser;

	static uint64 kernelBase;
	static uint64 processBase;

	// Compute process time
	GetProcessTimes( GetCurrentProcess(), &dummy, &dummy, &processUser, &processKernel );

	uint64 kernelTime	= ( uint64( processKernel.dwHighDateTime ) << 32 )
						+ uint64( processKernel.dwLowDateTime );

	uint64 processTime	= ( uint64( processKernel.dwHighDateTime ) << 32 )
						+ uint64( processKernel.dwLowDateTime )
						+ ( uint64( processUser.dwHighDateTime ) << 32 )
						+ uint64( processUser.dwLowDateTime );

	s_kernelTime = ( float )( double( kernelTime - kernelBase ) / 10000000.0 );
	s_processTime = ( float )( double( processTime - processBase ) / 10000000.0 );

	if ( s_resetThreadTimes )
	{
		kernelBase = kernelTime;
		processBase = processTime;
	}

	// Compute thread times
	for ( ThreadInfoMap::iterator it = threads_.begin(); it != threads_.end(); ++it )
	{
		ThreadInfo* info = it->second;

		BOOL b = GetThreadTimes( it->first, &dummy, &dummy, &threadUser, &threadKernel );

		uint64 threadTime	= ( uint64( threadKernel.dwHighDateTime ) << 32 )
							+ uint64( threadKernel.dwLowDateTime )
							+ ( uint64( threadUser.dwHighDateTime ) << 32 )
							+ uint64( threadUser.dwLowDateTime );

		info->time_ = ( float )( double( threadTime - info->base_ ) / 10000000.0 );

		if ( s_resetThreadTimes )
		{
			info->base_ = threadTime;
		}
	}

	s_resetThreadTimes = false;
#endif

	// Update current times for watchers
	double stampsPerMs = stampsPerSecondD() / 1000.0;

	for ( int i = 0; i < numSlots_; i++ )
	{
		uint64 slotTime = slots_[i].times_[frameCount_];
		float slotTimeF = ( float )( ( double )slotTime / stampsPerMs );
		slots_[i].curTimeMs_ =
			( DAMPING_RATIO * slots_[i].curTimeMs_ ) + ( ( 1.f - DAMPING_RATIO ) * slotTimeF );

		slots_[i].curCount_ = slots_[i].counts_[frameCount_];
	}

	frameCount_++;

	// Flush
	if ( frameCount_ == NUM_FRAMES )
	{
		this->flushHistory();
		frameCount_ = 0;
	}

	// Start next frame
	this->begin( 0 );
}


/**
 *
 */
int Profiler::declareSlot( const char* name, uint32 flags )
{
#if ENABLE_PROFILER
	slotNamesWritten_ = false;
	slots_[numSlots_].name_ = name;

	if ( flags & FLAG_WATCH )
	{
		std::string watcherName( std::string( "Profiler/" ) + std::string( name ) );
		MF_WATCH( watcherName.c_str(), slots_[numSlots_].curTimeMs_, Watcher::WT_READ_ONLY );

		std::string watcherName2( std::string( "Profiler Count/" ) + std::string( name ) );
		MF_WATCH( watcherName2.c_str(), slots_[numSlots_].curCount_, Watcher::WT_READ_ONLY );
	}

	return numSlots_++;
#else
	return 0;
#endif
}


/**
 *
 */
void Profiler::setNewHistory( const char* historyFileName )
{
#if ENABLE_PROFILER
	closeHistory();

	historyFile_ = bw_fopen( historyFileName, "w" );
	slotNamesWritten_ = false;
	frameCount_ = 0;

#ifdef _WIN32
	s_resetThreadTimes = true;
#endif
#endif
}


/**
 *
 */
void Profiler::closeHistory()
{
	flushHistory();

	if ( historyFile_ )
	{
		fclose( historyFile_ );
		historyFile_ = NULL;
	}
}


/**
 *
 */
void Profiler::flushHistory()
{
	// Save
	if ( historyFile_ )
	{
		// Save slot names (twice - for times and counts, skipping unaccounted for counts)
		if ( !slotNamesWritten_ )
		{
			fprintf( historyFile_, "Total," );

			for ( int i = 0; i < numSlots_; i++ )
				fprintf( historyFile_, "%s,", slots_[i].name_ );

			fprintf( historyFile_, "," );

			for ( int i = 1; i < numSlots_; i++ )
				fprintf( historyFile_, "%s,", slots_[i].name_ );

			fprintf( historyFile_, "\n" );

			slotNamesWritten_ = true;
		}

		// Calculate total for each frame
		uint64 frameTotals[NUM_FRAMES];
		for ( int i = 0; i < frameCount_; i++ )
		{
			frameTotals[i] = 0;
			for ( int j = 0; j < numSlots_; j++ )
				frameTotals[i] += slots_[j].times_[i];
		}

		double stampsPerMs = stampsPerSecondD() / 1000.0;

		// Save each frame
		for ( int i = 0; i < frameCount_; i++ )
		{
			float frameTotal = ( float )( ( double )frameTotals[i] / stampsPerMs );
			fprintf( historyFile_, "%f,", frameTotal );

			for ( int j = 0; j < numSlots_; j++ )
			{
				uint64 slotTime = slots_[j].times_[i];
				float slotTimeF = ( float )( ( double )slotTime / stampsPerMs );
				fprintf( historyFile_, "%f,", slotTimeF );
			}

			fprintf( historyFile_, "," );

			for ( int j = 1; j < numSlots_; j++ )
			{
				fprintf( historyFile_, "%d,", slots_[j].counts_[i] );
			}

			fprintf( historyFile_, "\n" );
		}
	}

	// Clear
	for ( int i = 0; i < frameCount_; i++ )
	{
		for ( int j = 0; j < numSlots_; j++ )
		{
			slots_[j].counts_[i] = 0;
			slots_[j].times_[i] = 0;
		}
	}
}



// profiler.cpp
