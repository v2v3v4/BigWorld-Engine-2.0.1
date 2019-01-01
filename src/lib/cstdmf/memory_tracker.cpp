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
#include "memory_tracker.hpp"

#include "bw_util.hpp"
#include "fini_job.hpp"
#include "watcher.hpp"

//-----------------------------------------------------------------------------

#ifdef ENABLE_MEMTRACKER

#ifdef WIN32
#pragma warning(disable: 4073)
#pragma init_seg(lib)				// Ensure we get constructed first
#endif

#undef malloc
#undef realloc
#undef free
#undef strdup
#undef _strdup
#undef _wcsdup

#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#define NO_NED_NAMESPACE
#define NO_MALLINFO 1
#include "third_party/nedalloc/nedmalloc.h"
#define MALLOC ::nedmalloc
#define REALLOC ::nedrealloc
#define FREE ::nedfree
#else
#define MALLOC ::malloc
#define REALLOC ::realloc
#define FREE ::free
#endif

//-----------------------------------------------------------------------------

#ifdef WIN32
MemTracker g_memTracker;
#else
MemTracker g_memTracker __attribute__ ((init_priority(101)));
#endif

//-----------------------------------------------------------------------------

static bool s_initDone = false;

/*static*/ THREADLOCAL( MemTracker::ThreadState ) MemTracker::s_threadState;

//-----------------------------------------------------------------------------

// Dummy declaration of default slot - set by MemTracker constructor. This is 
// required for using MEMTRACKER_BREAK_ON_ALLOC(slotId,allocId) on default slot.
int g_memTrackerSlot_Default = -1;

// Modify default slot behaviour flags here
#ifdef _DEBUG
#define DEFAULT_SLOT_FLAGS		FLAG_TRACK_BLOCKS
#else
#define DEFAULT_SLOT_FLAGS		0
#endif

int s_callStackPos;
char s_callStack[65536];

//-----------------------------------------------------------------------------

// Constants that the MemTracker uses to trash memory on alloc, free or
// expanding realloc
#ifdef _DEBUG
static const uint32 s_trashMalloc = 0xcafebeef;
static const uint32 s_trashRealloc = 0xcafedead;
static const uint32 s_trashFree = 0xdeadbeef;
#endif

//-----------------------------------------------------------------------------

#ifdef WIN32
#define Print OutputDebugStringA
#else
#define Print printf
#endif

//-----------------------------------------------------------------------------

// An internal assert. We need this to guarantee that no memory is allocated as
// this could cause infinite recursion.

#define MEMTRACKER_ASSERT( cond ) cond ? (void)0 : assertHandler( #cond, __FILE__, __LINE__ )

static void assertHandler( const char* cond, const char* file, uint line )
{
	char assertString[256];
	bw_snprintf( assertString, 256, "MemTracker Assert: %s\n%s : %d\n", cond, file, line );
	Print( assertString );
	ENTER_DEBUGGER();
}

//-----------------------------------------------------------------------------

// Construct MemTracker, it should be the first object constructed and therefore
// "default" should be the first slot created (id = 0).

MemTracker::MemTracker()
:	numBreaks_(0),
	reportOnExit_( true ),
	crashOnLeak_( false ),
	maxSize_( 0 )
{
#ifdef ENABLE_MEMORY_MAP
	virtualAllocs_.assign( VirtualAllocs::ElementType( NULL, 0 ) );
#endif

	list_.setAsRoot();

	slots_.resize( 0 );
	g_memTrackerSlot_Default = declareSlot( "Default", DEFAULT_SLOT_FLAGS );
	MF_ASSERT( g_memTrackerSlot_Default == 0 );

	s_initDone = true;
}

//-----------------------------------------------------------------------------

// The MemTracker destructor. It prints out all blocks that are still allocated
// with call stack if one was recorded.
MemTracker::~MemTracker()
{
	FiniJob::runAll();

	if ( !reportOnExit_ )
		return;

	#ifdef _DEBUG

	bool leaks = reportAllocations();

	s_initDone = false;

	if ( leaks && crashOnLeak_ )
		*((volatile int *)0) = 0;

	#endif
}

//-----------------------------------------------------------------------------

MemTracker& MemTracker::instance()
{
	return g_memTracker;
}

//-----------------------------------------------------------------------------

void* MemTracker::malloc( size_t size )
{
	MEMTRACKER_ASSERT( s_initDone );

	ThreadState* ts = &s_threadState;

	// Allocate memory and header
	size = ( size + 3 ) & ~3;

	void* mem;
	uint len = 0;

	if ( maxSize_ && ( stats_.curBytes_ + size ) > maxSize_ )
	{
		mem = NULL;
	}
	else
	{
		if ( slots_[ts->curSlot_].flags_ & FLAG_CALLSTACK )
		{
			this->getCallStack();
			len = ( strlen( s_callStack ) + 1 + 3 ) & ~3;
		}

		mem = MALLOC( size + len + sizeof( Header ) );
	}

	if ( mem )
	{
		Header* header = ( Header* )mem;

		// Fill the header and add to list
		header->slot = ts->curSlot_;
		#ifdef WIN32
		header->id = InterlockedIncrement( &g_memTracker.slots_[ts->curSlot_].allocCounter_ ) - 1;
		#else
		header->id = 0;
		#endif
		header->size = size;
		header->callStackSize = len;

		if ( slots_[ts->curSlot_].flags_ & FLAG_TRACK_BLOCKS )
		{
			mutex_.grab();
			header->node.addThisBefore( &list_ );
			mutex_.give();
		}
		else
		{
			header->node.setAsRoot();
		}

		this->updateStats( size, ts->curSlot_ );

		#ifdef _DEBUG
		breakIfRequested( header->slot, header->id );

		// Trash the memory
		uint32* mem2 = ( uint32* )( header + 1 );
		for ( uint i = 0; i < ( size >> 2 ); i++ )
		{
			*mem2++ = s_trashMalloc;
		}
		#else
		uint32* mem2 = ( uint32* )( header + 1 ) + ( size >> 2 );
		#endif

		// Copy the callstack
		memcpy( mem2, s_callStack, len );

		// Done
		return header + 1;
	}
	else
	{
		ERROR_MSG( "Malloc returned NULL\n" );
#if ENABLE_STACK_TRACKER
		this->getCallStack();
		ERROR_MSG( "Stack trace : %s\n", s_callStack );
#endif

		Slot* slot = &slots_[ts->curSlot_];

		#ifdef WIN32
		InterlockedIncrement( &stats_.failedAllocs_ );
		InterlockedIncrement( &slot->stats_.failedAllocs_ );
		#else
		SimpleMutexHolder mutexHolder( mutex_ );
		stats_.failedAllocs_++;
		slot->stats_.failedAllocs_++;
		#endif

		reportStats();
		return NULL;
	}
}

//-----------------------------------------------------------------------------

void* MemTracker::realloc( void* mem, size_t size )
{
	MEMTRACKER_ASSERT( s_initDone );

	ThreadState* ts = &s_threadState;

	Header* header = NULL;
	uint oldSize = 0;

	if ( mem )
	{
		// Remove header
		header = ( Header* )mem;
		header--;

		if ( header->node.getNext() != &header->node )
		{
			mutex_.grab();
			header->node.remove();
			mutex_.give();
		}

		this->updateStats( -int( header->size ), header->slot );

		oldSize = header->size;
	}

	// Reallocate memory and new header
	size = ( size + 3 ) & ~3;

	uint len = 0;

	if ( maxSize_ && ( stats_.curBytes_ + size ) > maxSize_ )
	{
		mem = NULL;
	}
	else
	{
		if ( slots_[ts->curSlot_].flags_ & FLAG_CALLSTACK )
		{
			this->getCallStack();
			len = ( strlen( s_callStack ) + 1 + 3 ) & ~3;
		}

		mem = REALLOC( header, size + len + sizeof( Header ) );
	}

	if ( mem )
	{
		header = ( Header* )mem;

		// Fill the header and add to list
		header->slot = ts->curSlot_;
		#ifdef WIN32
		header->id = InterlockedIncrement( &g_memTracker.slots_[ts->curSlot_].allocCounter_ ) - 1;
		#else
		header->id = 0;
		#endif
		header->size = size;
		header->callStackSize = len;

		if ( slots_[ts->curSlot_].flags_ & FLAG_TRACK_BLOCKS )
		{
			mutex_.grab();
			header->node.addThisBefore( &list_ );
			mutex_.give();
		}
		else
		{
			header->node.setAsRoot();
		}

		this->updateStats( size, ts->curSlot_ );

		#ifdef _DEBUG
		breakIfRequested( header->slot, header->id );

		// Trash the memory
		if ( size > oldSize )
		{
			uint32* mem2 = ( uint32* )( header + 1 ) + ( oldSize >> 2 );
			for ( uint i = ( oldSize >> 2 ); i < ( size >> 2 ); i++ )
			{
				*mem2++ = s_trashRealloc;
			}
		}
		#endif

		// Copy the callstack
		uint32* mem2 = ( uint32* )( header + 1 ) + ( size >> 2 );
		memcpy( mem2, s_callStack, len );

		// Done
		return header + 1;
	}
	else
	{
		ERROR_MSG( "Realloc returned NULL\n" );
#if ENABLE_STACK_TRACKER
		this->getCallStack();
		ERROR_MSG( "Stack trace : %s\n", s_callStack );
#endif

		Slot* slot = &slots_[ts->curSlot_];

		#ifdef WIN32
		InterlockedIncrement( &stats_.failedAllocs_ );
		InterlockedIncrement( &slot->stats_.failedAllocs_ );
		#else
		SimpleMutexHolder mutexHolder( mutex_ );
		stats_.failedAllocs_++;
		slot->stats_.failedAllocs_++;
		#endif

		reportStats();
		return NULL;
	}
}

//-----------------------------------------------------------------------------

void MemTracker::free( void* mem )
{
	MEMTRACKER_ASSERT( s_initDone );

	// Remove the header
	Header* header = ( Header* )mem;
	header--;

	if ( header->node.getNext() != &header->node )
	{
		mutex_.grab();
		header->node.remove();
		mutex_.give();
	}

	this->updateStats( -int( header->size ), header->slot );

	#ifdef _DEBUG
	// Trash the memory
	uint32* mem2 = ( uint32* )( header );
	uint size = header->size + sizeof( Header );
	for ( uint i = 0; i < ( size >> 2 ); i++ )
	{
		*mem2++ = s_trashFree;
	}
	#endif

	// Free it
	FREE( header );
}

//-----------------------------------------------------------------------------

#ifndef MF_SERVER

void* MemTracker::virtualAlloc( void* pMemory, size_t size, DWORD allocationType, DWORD flProtect )
{
	void* pRet = NULL;
	pRet = VirtualAlloc( pMemory, size, allocationType, flProtect );
#ifdef ENABLE_MEMORY_MAP
	// If the memory map is enabled we store the virtual allocs in a list so that
	// we can save them out to the memory map
	if (pRet != NULL && 
		allocationType & MEM_RESERVE)
	{
		mutex_.grab();
		size_t index = 0;
		for (; index < virtualAllocs_.size(); index++)
		{
			if (virtualAllocs_[index].first == NULL)
			{
				break;
			}
		}
		if (index != virtualAllocs_.size())
		{
			virtualAllocs_[index] = VirtualAllocs::ElementType( pRet, size );
		}
		mutex_.give();
	}
#endif
	return pRet;
}

#endif

//-----------------------------------------------------------------------------

#ifndef MF_SERVER

bool  MemTracker::virtualFree( void* pMemory, size_t size, DWORD dwFreeType )
{
#ifdef ENABLE_MEMORY_MAP
	// If we are using the memory map and we are releasing
	// all the memory in an allocation, remove it from our list
	if (dwFreeType & MEM_RELEASE && 
		size == 0)
	{
		mutex_.grab();
		size_t index = 0;
		for (; index < virtualAllocs_.size(); index++)
		{
			if (virtualAllocs_[index].first == pMemory)
			{
				virtualAllocs_[index] = VirtualAllocs::ElementType( NULL, 0 );
			}
		}
		mutex_.give();
	}
#endif
	bool ret = VirtualFree( pMemory, size, dwFreeType ) == TRUE;
	return ret;
}

#endif

//-----------------------------------------------------------------------------

char* MemTracker::strdup( const char* s )
{
	uint len = strlen( s ) + 1;
	char* ns = ( char* )malloc( len );
	memcpy( ns, s, len );
	return ns;
}

//-----------------------------------------------------------------------------

wchar_t* MemTracker::wcsdup( const wchar_t* s )
{
	uint len = wcslen( s ) + 1;
	size_t size = len*sizeof(wchar_t);
	wchar_t* ns = ( wchar_t* )malloc( size );
	memcpy( ns, s, size );
	return ns;
}

//-----------------------------------------------------------------------------

int MemTracker::declareSlot( const char* name, uint32 flags )
{
	SimpleMutexHolder mutexHolder( mutex_ );

	int slot = slots_.size();

	slots_.resize( slots_.size() + 1 );
	slots_[ slot ].name_ = name;
#ifndef ENABLE_MEMORY_MAP
	slots_[ slot ].flags_ = flags;
#else
	// If the memory map is enabled we track all blocks
	// so that we can output them to the memory map file
	slots_[ slot ].flags_ = flags | FLAG_TRACK_BLOCKS;
#endif

	return slot;
}

//-----------------------------------------------------------------------------

int MemTracker::declareBreak( uint slotId, uint allocId )
{
	SimpleMutexHolder mutexHolder( mutex_ );

	MEMTRACKER_ASSERT( numBreaks_ < ARRAY_SIZE( breaks_ ) );

	breaks_[numBreaks_].slotId	= slotId;
	breaks_[numBreaks_].allocId	= allocId;

	return numBreaks_++;
}


//-----------------------------------------------------------------------------

void MemTracker::begin( uint slotId )
{
	ThreadState* ts = &s_threadState;

	MEMTRACKER_ASSERT( slotId < slots_.size() );
	MEMTRACKER_ASSERT( ts->slotStackPos_ < ARRAY_SIZE( ts->slotStack_ ) );

	ts->slotStack_[ts->slotStackPos_++] = ts->curSlot_;
	ts->curSlot_ = slotId;
}

//-----------------------------------------------------------------------------

void MemTracker::end( )
{
	ThreadState* ts = &s_threadState;

	MEMTRACKER_ASSERT( ts->slotStackPos_ > 0 );

	ts->curSlot_ = ts->slotStack_[--ts->slotStackPos_];
}

//-----------------------------------------------------------------------------

void MemTracker::readStats( AllocStats& stats ) const
{
	stats = stats_;
}

//-----------------------------------------------------------------------------

void MemTracker::readStats( AllocStats& stats, uint slotId ) const
{
	stats = slots_[slotId].stats_;
}

//-----------------------------------------------------------------------------

void MemTracker::reportStats() const
{
	DEBUG_MSG( "\n\nMemTracker Stats\n" );

	DEBUG_MSG( "\nTotal:\n" );
	DEBUG_MSG( "Current Bytes : %ld\n", stats_.curBytes_ );
	DEBUG_MSG( "Current Blocks: %ld\n", stats_.curBlocks_ );
	DEBUG_MSG( "Failed Allocations: %ld\n", stats_.failedAllocs_ );

	for ( Slots::const_iterator iter = slots_.begin(); 
		  iter != slots_.end(); ++iter )
	{
		DEBUG_MSG( "\nSlot %s:\n", iter->name_ );
		DEBUG_MSG( "Current Bytes : %ld\n", iter->stats_.curBytes_ );
		DEBUG_MSG( "Current Blocks: %ld\n", iter->stats_.curBlocks_ );
		DEBUG_MSG( "Failed Allocations: %ld\n", iter->stats_.failedAllocs_ );
	}

	DEBUG_MSG( "\n\n" );
}

//-----------------------------------------------------------------------------

bool MemTracker::reportAllocations() const
{
	bool leaks = false;

	char	out[256];
	uint32	totalBytes	= 0;
	uint32	totalBlocks = 0;
	FILE*	leakFile;

	for ( ListNode* node = list_.getNext(); node != &list_; node = node->getNext() )
	{
		//Header* header = CAST_NODE( node, Header, node );
		Header* header = ( Header* )( node );

		// If this is the first leak
		if (!leaks)
		{
			Print( "MemTracker detected the following leaks:\n" );
			leaks = true;
			leakFile = bw_fopen( "leaks.txt", "w" );
		}

		bw_snprintf( out, 256, "Slot: %s, Id: %d - %d bytes\n", slots_[header->slot].name_, header->id, header->size );
		Print( out );
		fprintf( leakFile, "%s", out );

		totalBytes += header->size;
		totalBlocks++;

		if ( slots_[ header->slot ].flags_ & FLAG_CALLSTACK )
		{
			char* callstack = ( char* )( header + 1 ) + header->size;
			Print( callstack );
			fprintf( leakFile, "%s\n", callstack );
		}
	}

	if ( leaks )
	{
		bw_snprintf( out, ARRAY_SIZE(out), 
					"MemTracker total leaks: %d bytes in %d blocks.\n",
					totalBytes, totalBlocks );
		Print( out );
		fprintf( leakFile, "%s", out );

		fclose( leakFile );
	}

	return leaks;
}

//-----------------------------------------------------------------------------

#ifdef WIN32

void MemTracker::getCallStack()
{
	MEMTRACKER_SCOPED( Default );
#if ENABLE_STACK_TRACKER
	strcpy( s_callStack, StackTracker::buildReport().c_str() );
#endif
}

#else

void MemTracker::getCallStack()
{
	s_callStackPos = 0;
	s_callStack[0] = 0;

	const int maxEntries = 200;
	void * buffer[maxEntries];

	int numEntries = backtrace( buffer, maxEntries );
	char ** symbols = backtrace_symbols( buffer, numEntries );

	if ( !symbols )
		return;

	for ( int i = 0; i < numEntries; i++ )
	{
		if ( strlen( symbols[i] ) >= ( ARRAY_SIZE( s_callStack ) - 1 - s_callStackPos ) )
			break;

		strcpy( s_callStack + s_callStackPos, symbols[i] );
		s_callStackPos += strlen( symbols[i] );
		s_callStack[ s_callStackPos++ ] = '\n';
		s_callStack[ s_callStackPos ] = 0;
	}

	raw_free( symbols );
}

#endif

//-----------------------------------------------------------------------------

void MemTracker::updateStats( int size, uint slotId )
{
	int blocksChange = ( size > 0 ) ? 1 : -1;

	Slot* slot = &slots_[slotId];

	#ifdef WIN32
	InterlockedExchangeAdd( &stats_.curBytes_, size );
	InterlockedExchangeAdd( &stats_.curBlocks_, blocksChange );

	InterlockedExchangeAdd( &slot->stats_.curBytes_, size );
	InterlockedExchangeAdd( &slot->stats_.curBlocks_, blocksChange );
	#else
	SimpleMutexHolder mutexHolder( mutex_ );

	stats_.curBytes_ += size;
	stats_.curBlocks_ += blocksChange;

	slot->stats_.curBytes_ += size;
	slot->stats_.curBlocks_ += blocksChange;
	#endif
}

//-----------------------------------------------------------------------------

// This issues a break when the user has requested a BREAK_ON_ALLOC within
// the given slot and given allocation id. Trace up the callstack to see who the
// caller is.
void MemTracker::breakIfRequested( uint slotId, uint allocId )
{
	for ( uint i = 0; i < numBreaks_; i++ )
	{
		if ( breaks_[i].slotId == slotId 
				&& breaks_[i].allocId == allocId )
		{
			char outString[256];

			bw_snprintf( outString, ARRAY_SIZE(outString) , 
				"MemTracker: User break in slot '%s' - allocation %d.\n",
				slots_[slotId].name_, allocId );
			Print( outString );

			ENTER_DEBUGGER();
			break;
		}
	}
}

//-----------------------------------------------------------------------------

#if ENABLE_WATCHERS
void MemTracker::addWatchers()
{
	MemTracker * pNull = NULL;
	SequenceWatcher< Slots > * pWatchSlots = 
		new SequenceWatcher< Slots >( pNull->slots_ );

	DirectoryWatcher * pSlotWatcher = new DirectoryWatcher();

	pSlotWatcher->addChild( "name", makeWatcher( &Slot::name_ ) );
	pSlotWatcher->addChild( "flags", makeWatcher( &Slot::flags_ ) );

/* TODO: Cast the volatile away so the watcher works
	pSlotWatcher->addChild( "allocCounter", 
						   makeWatcher( &Slot::allocCounter_ ) );
*/
	Slot * pNullSlot = NULL;
	pSlotWatcher->addChild( "curBytes", 
							 makeWatcher( (long &)pNullSlot->stats_.curBytes_ ) );
	pSlotWatcher->addChild( "curBlocks", 
							 makeWatcher( (long &)pNullSlot->stats_.curBlocks_ ) );
	pSlotWatcher->addChild( "failedAllocs", 
							 makeWatcher( (long &)pNullSlot->stats_.failedAllocs_ ) );

	pWatchSlots->addChild( "*", pSlotWatcher );
	pWatchSlots->setLabelSubPath( "name" );

	Watcher::rootWatcher().addChild( "MemoryTracker", pWatchSlots, 
									 &g_memTracker );

	MF_WATCH( "Memory/Maximum Size", g_memTracker.maxSize_ );

}
#endif

#ifdef ENABLE_MEMORY_MAP

//-----------------------------------------------------------------------------

namespace
{
	// Add values together with location value
	void addMemoryValue( uint16& value, uint16 amount, uint16 location)
	{
		value = (value + amount) | location;
	}

	// Add the memory allocation to the memory map
	void addToMemoryMap( uint32 start, uint32 size, uint16* memMap, uint16 location )
	{
		uint32 index = start >> 12;
		if (start & 0xfff)
		{
			uint32 blockSize = ((index + 1) << 12) - start;
			if (blockSize > size)
			{
				addMemoryValue( memMap[index], size, location );
				size = 0;
			}
			else
			{
				addMemoryValue( memMap[index], blockSize, location );
				size -= blockSize;
				++index;
			}
		}

		while (size != 0)
		{
			if (size > 4096)
			{
				addMemoryValue( memMap[index++], 4096, location );
				size -= 4096;
			}
			else
			{
				addMemoryValue( memMap[index], size, location );
				size = 0;
			}
		}
	}

	// The location identifiers for the different types of memory
	const uint16 LOCATION_VIRTUALALLOC = 1 << 15;
	const uint16 LOCATION_MEMTRACKER = 1 << 14;
	const uint16 LOCATION_HEAP = 1 << 13;
}

//-----------------------------------------------------------------------------


// Save out the memory map
void MemTracker::saveMemoryMap( const char* filename )
{
	FILE* f = fopen( filename, "wb+" );
	if (f)
	{
		// Allocate a buffer for the memory map
		uint16* memMap = new uint16[512*1024];
		memset(memMap, 0, 512 * 1024 * sizeof(uint16));

		mutex_.grab();

		// Get our virtual allocs
		for ( VirtualAllocs::iterator it = virtualAllocs_.begin();
			it != virtualAllocs_.end(); it++)
		{
			if (it->first != NULL)
			{
				addToMemoryMap( uint32(it->first), it->second, memMap, LOCATION_VIRTUALALLOC );
			}
		}

		// Get all the memory allocations that have passed through the MemTracker
		for ( ListNode* node = list_.getNext(); node != &list_; node = node->getNext() )
		{
			Header* header = ( Header* )( node );
			addToMemoryMap( uint32(header), header->size + header->callStackSize + sizeof(Header), memMap, LOCATION_MEMTRACKER );
		}
		mutex_.give();

		// Get the current heaps
		DWORD numHeaps = GetProcessHeaps( 0, NULL );

		std::vector<HANDLE> heaps;
		heaps.resize( numHeaps );

		numHeaps = GetProcessHeaps( numHeaps, &heaps[0] );
		
		if (numHeaps < heaps.size())
			heaps.resize( numHeaps );

		// Iterate over the heaps
		for (uint32 i = 0; i < heaps.size(); i++)
		{
			if (heaps[i])
			{
				HANDLE hHeap = heaps[i];

				if (HeapLock(hHeap))
				{
					// Init the first heap entry
					PROCESS_HEAP_ENTRY hEntry;
					memset( &hEntry, 0, sizeof(hEntry) );

					// Walk the heap and gather all busy blocks
					while (HeapWalk( hHeap, &hEntry ))
					{
						if (hEntry.wFlags & PROCESS_HEAP_ENTRY_BUSY)
						{
							addToMemoryMap( (uint32)hEntry.lpData, hEntry.cbData, memMap, LOCATION_HEAP );
						}
					}

					HeapUnlock( hHeap );
				}
			}

		}
		fwrite( memMap, 2, 512*1024, f );
		fclose(f);
		delete [] memMap;
	}
}

#endif


//-----------------------------------------------------------------------------

void* bw_malloc(size_t count)
{
	void* rv = NULL;
	rv = g_memTracker.malloc( count );
	return rv;
}

//-----------------------------------------------------------------------------

void* bw_realloc(void* p, size_t count)
{
	void* rv = NULL;
	rv = g_memTracker.realloc( p, count );
	return rv;
}

//-----------------------------------------------------------------------------

void bw_free( void* p )
{
	if (!p)
	{
		return;
	}

	g_memTracker.free( p );
}

//-----------------------------------------------------------------------------

#ifndef MF_SERVER
void* bw_virtualAlloc( void* pMemory, size_t size, DWORD allocationType, DWORD flProtect )
{
	void * rv = NULL;
	rv = g_memTracker.virtualAlloc( pMemory, size, allocationType, flProtect );
	return rv;
}
#endif

//-----------------------------------------------------------------------------

#ifndef MF_SERVER
bool  bw_virtualFree( void* pMemory, size_t size, DWORD dwFreeType )
{
	if (!pMemory)
	{
		return false;
	}
	return g_memTracker.virtualFree( pMemory, size, dwFreeType );
}
#endif

//-----------------------------------------------------------------------------

char* bw_strdup( const char * s )
{
	return g_memTracker.strdup( s );
}

//-----------------------------------------------------------------------------

wchar_t* bw_wcsdup( const wchar_t * s )
{
	return g_memTracker.wcsdup( s );
}

//-----------------------------------------------------------------------------
// Thread finish function

void memTrackerThreadFinish()
{
	#ifdef WIN32
	neddisablethreadcache( NULL );
	#endif
}

#else // ENABLE_MEMTRACKER

//-----------------------------------------------------------------------------
// Stub for the thread finish function

void memTrackerThreadFinish()
{
}

#endif // ENABLE_MEMTRACKER
