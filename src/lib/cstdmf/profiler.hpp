/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *	@internal
 *	@file
 */

#ifndef PROFILER_HPP
#define PROFILER_HPP

#include "cstdmf/stdmf.hpp"
#include "cstdmf/concurrency.hpp"
#include "cstdmf/config.hpp"
#include "cstdmf/timestamp.hpp"

#include <stdio.h>
#include <map>

// -----------------------------------------------------------------------------
// Section: Macros
// -----------------------------------------------------------------------------

#define PROFILER_DECLARE( id, name )			\
	int g_profilerSlot_##id = Profiler::instance().declareSlot( name )

#define PROFILER_DECLARE2( id, name, flags )	\
	int g_profilerSlot_##id = Profiler::instance().declareSlot( name, flags )


#define PROFILER_SCOPED( id )			\
	extern int g_profilerSlot_##id;		\
	ScopedProfiler scopedProfiler_##id( g_profilerSlot_##id )

// -----------------------------------------------------------------------------
// Section: Profiler
// -----------------------------------------------------------------------------

/**
 *	@internal
 */

class Profiler
{
private:
	enum
	{
		NUM_FRAMES			= 64,
		MAX_SLOTS			= 256,
		SLOT_STACK_DEPTH	= 64,
	};

public:
	enum
	{
		FLAG_WATCH			= ( 1 << 0 ),
	};

private:
	struct Slot
	{
		const char*			name_;
		float				curTimeMs_;
		int					curCount_;
		uint64				times_[NUM_FRAMES];
		int					counts_[NUM_FRAMES];
	};

#ifdef _WIN32
	struct ThreadInfo
	{
							std::string name_;
							uint64 base_;
							float time_;
	};
#endif

public:
						Profiler();
						~Profiler();

	static Profiler&	instance();
	static Profiler&	instanceNoCreate();
	static Profiler*	instanceNoCreateP() { return instance_ ; }
	void				tick();

	int					declareSlot( const char* name, uint32 flags = 0 );
	void				begin( int slotId );
	void				end();

#ifdef _WIN32
	void				addThread( HANDLE thread, const char* name );
	void				removeThread( HANDLE thread );
#endif

	void				setNewHistory( const char* historyFileName );
	void				closeHistory();
	void				flushHistory();

private:
	void				addTimeToCurrentSlot();

private:
	static Profiler*	instance_;

	unsigned long		threadId_;

	uint64				curTime_;
	uint64				prevTime_;

	FILE*				historyFile_;
	bool				slotNamesWritten_;

	int					frameCount_;

	int					numSlots_;
	int					curSlot_;

	int					slotStack_[SLOT_STACK_DEPTH];
	int					slotStackPos_;
	Slot				slots_[MAX_SLOTS];

#ifdef _WIN32
	typedef std::map<HANDLE, ThreadInfo*> ThreadInfoMap;
	ThreadInfoMap		threads_;
#endif
};

// -----------------------------------------------------------------------------
// Section: ScopeProfiler
// -----------------------------------------------------------------------------

/**
 *	@internal
 */

class ScopedProfiler
{
public:
	ScopedProfiler( int id )
	{
		Profiler::instanceNoCreate().begin( id );
	}

	~ScopedProfiler()
	{
		Profiler::instanceNoCreate().end();
	}
};


// This class is always inlined, sorry.
#include "profiler.ipp"


#endif // PROFILER_HPP
