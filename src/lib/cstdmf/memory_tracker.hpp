/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __CSTDMF_MEMORY_TRACKER_HPP__
#define __CSTDMF_MEMORY_TRACKER_HPP__

#include "stdmf.hpp"
#include "concurrency.hpp"
#include "list_node.hpp"
#include "config.hpp"
#include "static_array.hpp"

// -----------------------------------------------------------------------------
// Section: Macros
// -----------------------------------------------------------------------------

#ifndef ENABLE_MEMTRACKER
#define MEMTRACKER_DECLARE( id, name, flags )
#define MEMTRACKER_BEGIN( id )
#define MEMTRACKER_END()
#define MEMTRACKER_SCOPED( id )
#endif

//-----------------------------------------------------------------------------

#ifdef ENABLE_MEMTRACKER

//-----------------------------------------------------------------------------

#define MEMTRACKER_DECLARE( id, name, flags )	\
	int g_memTrackerSlot_##id = g_memTracker.declareSlot( name, flags )

#define MEMTRACKER_BEGIN( id )				\
	extern int g_memTrackerSlot_##id;		\
	g_memTracker.begin( g_memTrackerSlot_##id )

#define MEMTRACKER_END()					\
	g_memTracker.end()

#define MEMTRACKER_SCOPED( id )				\
	extern int g_memTrackerSlot_##id;		\
	ScopedMemTracker scopedMemTracker_##id( g_memTrackerSlot_##id )

#define MEMTRACKER_BREAK_ON_ALLOC( slotId, allocId )					\
	int break_##slotId_##allocId =										\
		g_memTracker.declareBreak( g_memTrackerSlot_##slotId, allocId )

//-----------------------------------------------------------------------------

#ifdef MF_SERVER
#include "execinfo.h"
#endif

//-----------------------------------------------------------------------------

#undef malloc
#undef realloc
#undef free
#undef strdup
#undef _strdup

// Enable this if you want to save the memory map to inspect
// memory fragmentation
// The memory map can be saved through MemTracker::saveMemoryMap
// or from python through BigWorld.saveMemoryMap()
//#define ENABLE_MEMORY_MAP

class MemTracker
{
public:
	// Slot flags, controlling the behaviour of a slot
	enum FLAG
	{
		FLAG_CALLSTACK		= ( 1 << 0 ),
		FLAG_TRACK_BLOCKS	= ( 1 << 1 ),
	};

	// Allocation statistics. Represents a particular slot or global memory
	struct AllocStats
	{
		volatile long		curBytes_;		// Bytes currently allocated
		volatile long		curBlocks_;		// Number of blocks currently allocated
		volatile long		failedAllocs_;	// Number of times an allocation failed
	};

private:
	enum
	{
		MAX_SLOTS			= 256,			// The maximum number of slots
		MAX_THREADS			= 16,			// The maximum number of threads
		SLOT_STACK_DEPTH	= 64,			// The slot stack size
		MAX_BREAKS			= 16			// The maximum number of user breaks
	};

	// The header represents a single block of tracked memory
	struct Header
	{
		ListNode			node;			// Node for list of allocated blocks
		uint				slot;			// The user assigned slot for this allocation
		uint				id;				// The allocation id, unique for this slot
		uint				size;			// Size of the block, not counting overhead
		uint				callStackSize;	// Size of the callstack data
	};

	// The slot represents a collection of blocks. Each tracked block belongs
	// to exactly one slot.
	struct Slot
	{
		const char*			name_;
		uint32				flags_;
		volatile long		allocCounter_;
		AllocStats			stats_;
	};

	// Stores the slot stack for each thread
	struct ThreadState
	{
		int					curSlot_;			// Current slot
		int					slotStack_[SLOT_STACK_DEPTH];	// The stack of slots
		uint				slotStackPos_;					// The number of slots on the stack
	};

	// Stores a user defined break on an allocation within a slot
	struct Break
	{
		uint				slotId;
		uint				allocId;
	};

public:
						MemTracker();
						~MemTracker();

	static MemTracker&	instance();

	// Functions that allocate and free tracked memory
	void*				malloc( size_t size );
	void*				realloc( void* mem, size_t size );
	void				free( void* mem );

#ifndef MF_SERVER
	void*				virtualAlloc( void* pMemory, size_t size, DWORD allocationType, DWORD flProtect );
	bool				virtualFree( void* pMemory, size_t size, DWORD dwFreeType );
#endif

	char*				strdup( const char* s );
	wchar_t*			wcsdup( const wchar_t* s );

	// These functions are used by the MEMTRACKER_ macros to control
	// declaration and usage of slots
	int					declareSlot( const char* name, uint32 flags );
	void				begin( uint slotId );
	void				end();

	// User defined break on an allocation within a slot.
	int					declareBreak( uint slotId, uint allocId );

	// Query functions - they fill an AllocStats structure for global
	// memory usage or for a particular slot
	void				readStats( AllocStats& stats ) const;
	void				readStats( AllocStats& stats, uint slotId ) const;

	// Prints various memory stats
	void				reportStats() const;
	bool				reportAllocations() const;

	// Set various behaviours for testing and reporting
	void				setReportOnExit( bool reportOnExit );
	void				setCrashOnLeak( bool crashOnLeak );
	void				setMaxSize( uint maxSize );

#if ENABLE_WATCHERS
	static void			addWatchers();
#endif

#ifdef ENABLE_MEMORY_MAP
	void saveMemoryMap( const char* filename );
#endif

private:
	void				getCallStack();
	void				updateStats( int size, uint slotId );
	void				breakIfRequested( uint slotId, uint allocId );

private:
	ListNode			list_;				// List of all allocated blocks
	SimpleMutex			mutex_;

	AllocStats			stats_;				// Global allocation stats

	typedef StaticArray< Slot,MAX_SLOTS > Slots;
	Slots				slots_;				// Array of all slots

	static THREADLOCAL( ThreadState ) s_threadState;

	uint				numBreaks_;			// Number of declared breaks
	Break				breaks_[MAX_BREAKS];// Array of all breaks

	bool				reportOnExit_;		// Report leaks from destructor
	bool				crashOnLeak_;		// Will cause unit tests to fail if they leak
	uint				maxSize_;			// Artificial memory limit

#ifdef ENABLE_MEMORY_MAP
	typedef std::pair<void*, size_t> SingleVirtualAlloc;
	typedef StaticArray<SingleVirtualAlloc,16> VirtualAllocs;
	VirtualAllocs		virtualAllocs_;
#endif
};

#define malloc bw_malloc
#define realloc bw_realloc
#define free bw_free
#define strdup bw_strdup
#define _strdup bw_strdup
#define _wcsdup bw_wcsdup

extern MemTracker	g_memTracker;
extern int			g_memTrackerSlot_Default; // Dummy declaration, see .cpp

// -----------------------------------------------------------------------------

inline void MemTracker::setReportOnExit( bool reportOnExit )
{
	reportOnExit_ = reportOnExit;
}

inline void MemTracker::setCrashOnLeak( bool crashOnLeak )
{
	crashOnLeak_ = crashOnLeak;
}

inline void MemTracker::setMaxSize( uint maxSize )
{
	maxSize_ = maxSize;
}

// -----------------------------------------------------------------------------

// Class that pushes and pops a slot while its in scope, used by the
// MEMTRACKER_SCOPED macro.

class ScopedMemTracker
{
public:
	ScopedMemTracker( int id )
	{
		g_memTracker.begin( id );
	}

	~ScopedMemTracker()
	{
		g_memTracker.end();
	}
};

#endif	// ENABLE_MEMTRACKER

// -----------------------------------------------------------------------------

#endif
