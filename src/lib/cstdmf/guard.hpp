/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CSTDBW_GUARD__H
#define CSTDBW_GUARD__H


#include "stack_tracker.hpp"
#include "profiler.hpp"
#include "memory_tracker.hpp"
#include "config.hpp"


#if ENABLE_STACK_TRACKER
	#define BW_GUARD		ScopedStackTrack _BW_GUARD(__FUNCTION__, __FILE__, __LINE__)
	#define BW_GUARD_BEGIN	StackTracker::push(__FUNCTION__, __FILE__, __LINE__)
	#define BW_GUARD_END	StackTracker::pop()
#else
#	define BW_GUARD
#	define BW_GUARD_BEGIN
#	define BW_GUARD_END
#endif


#define BW_GUARD_PROFILER( id ) \
	BW_GUARD; \
	PROFILER_SCOPED(id)

#define BW_GUARD_MEMTRACKER( id ) \
	BW_GUARD; \
	MEMTRACKER_SCOPED( id )

#define BW_GUARD_PROFILER_MEMTRACKER( id ) \
	BW_GUARD; \
	PROFILER_SCOPED( id ); \
	MEMTRACKER_SCOPED( id )


#endif
