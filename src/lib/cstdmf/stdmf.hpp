/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/* stdmf.hpp: library wide definitions
 */
#ifndef STDMF_HPP
#define STDMF_HPP

#ifdef CODE_INLINE
    #define INLINE    inline
#else
	/// INLINE macro.
    #define INLINE
#endif

/**
 *  360 hack stuff
 */

/**
 *  PS3 hack stuff
 */
#if defined( PLAYSTATION3 )
// _BIG_ENDIAN is a built in define
#include <stdlib.h>
#include <stdint.h>
#include <sys/sys_time.h>
#include <sys/timer.h>
#include <sys/time_util.h>
#include <sys/select.h>
#endif

#ifndef _WIN32
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <limits>
#include <math.h>

#if !defined( PLAYSTATION3 )
#include <malloc.h>
#endif

/* --------------------------------------------------------------------------
 * Constants
 * --------------------------------------------------------------------------
 */

//#define NT_ENABLED

#ifdef __WATCOMC__
#ifndef false
#define false		0
#define true		1
#endif
#endif // __WATCOMC__

/*
#define ASCII_NUL	0x00
#define ASCII_SOH	0x01
#define ASCII_STX	0x02
#define ASCII_ETX	0x03
#define ASCII_EOT	0x04
#define ASCII_ENQ	0x05
#define ASCII_ACK	0x06
#define ASCII_BEL	0x07
#define ASCII_BS	0x08
#define ASCII_HT	0x09
#define ASCII_LF	0x0a
#define ASCII_VT	0x0b
#define ASCII_FF	0x0c
#define ASCII_CR	0x0d
#define ASCII_SO	0x0e
#define ASCII_SI	0x0f
#define ASCII_DLE	0x10
#define ASCII_DC1	0x11
#define ASCII_DC2	0x12
#define ASCII_DC3	0x13
#define ASCII_DC4	0x14
#define ASCII_NAK	0x15
#define ASCII_SYN	0x16
#define ASCII_ETB	0x17
#define ASCII_CAN	0x18
#define ASCII_EM	0x19
#define ASCII_SUB	0x1a
#define ASCII_ESC	0x1b
#define ASCII_FS	0x1c
#define ASCII_GS	0x1d
#define ASCII_RS	0x1e
#define ASCII_US	0x1f
*/



/* --------------------------------------------------------------------------
 * Types
 * --------------------------------------------------------------------------
 */

/// This type is an unsigned character.
typedef unsigned char	uchar;
/// This type is an unsigned short.
typedef unsigned short	ushort;
/// This type is an unsigned integer.
typedef unsigned int	uint;
/// This type is an unsigned longer.
typedef unsigned long	ulong;

#if defined( PLAYSTATION3 )

typedef int8_t			int8;
typedef int16_t			int16;
typedef int32_t			int32;
typedef int64_t			int64;
typedef uint8_t			uint8;
typedef uint16_t		uint16;
typedef uint32_t		uint32;
typedef uint64_t		uint64;

typedef intptr_t		intptr;
typedef uintptr_t		uintptr;

#define PRI64 "lld"
#define PRIu64 "llu"
#define PRIx64 "llx"
#define PRIX64 "llX"
#define PRIzu "lu"
#define PRIzd "ld"

#else


#ifdef _WIN32
typedef __int8				int8;
typedef unsigned __int8		uint8;

typedef __int16				int16;
typedef unsigned __int16	uint16;

typedef __int32				int32;
typedef unsigned __int32	uint32;

typedef __int64				int64;
typedef unsigned __int64	uint64;
// This type is an integer with the size of a pointer.
typedef INT_PTR				intptr;
// This type is an unsigned integer with the size of a pointer.
typedef UINT_PTR        	uintptr;
#define PRI64 "lld"
#define PRIu64 "llu"
#define PRIx64 "llx"
#define PRIX64 "llX"
#define PRIzu "lu"
#define PRIzd "ld"
#else //unix

/// This type is an integer with a size of 8 bits.
typedef int8_t				int8;
/// This type is an unsigned integer with a size of 8 bits.
typedef uint8_t				uint8;

/// This type is an integer with a size of 16 bits.
typedef int16_t				int16;
/// This type is an unsigned integer with a size of 16 bits.
typedef uint16_t			uint16;

/// This type is an integer with a size of 32 bits.
typedef int32_t				int32;
/// This type is an unsigned integer with a size of 32 bits.
typedef uint32_t			uint32;
/// This type is an integer with a size of 64 bits.
typedef int64_t				int64;
/// This type is an unsigned integer with a size of 64 bits.
typedef uint64_t			uint64;

#ifdef _LP64
typedef int64				intptr;
typedef uint64				uintptr;
#define PRI64 "ld"
#define PRIu64 "lu"
#define PRIx64 "lx"
#define PRIX64 "lX"
#else
typedef int32				intptr;
typedef uint32				uintptr;
#define PRI64 "lld"
#define PRIu64 "llu"
#define PRIx64 "llx"
#define PRIX64 "llX"
#endif

#ifndef PRIzd
#define PRIzd "zd"
#endif

#ifndef PRIzu
#define PRIzu "zu"
#endif

#endif

#endif

/// This type is used for a 4 byte file header descriptor.
//typedef uint32			HdrID;		/* 4 byte (file) descriptor */
/// This type is used for a generic 4 byte descriptor.
//typedef uint32			ID;			/* 4 byte generic descriptor */


/* --------------------------------------------------------------------------
 * Macros
 * --------------------------------------------------------------------------
 */



/* array & structure macros
 */
#if 0
#define ARRAYCLR(v)					memset((v), 0x0, sizeof(v))
#define MEMCLR(v)					memset(&(v), 0x0, sizeof(v))
#define MEMCLRP(v)					memset((v), 0x0, sizeof(*v))
#endif

#define ARRAYSZ(v)					(sizeof(v) / sizeof(v[0]))
#define ARRAY_SIZE(v)				(sizeof(v) / sizeof(v[0]))

#if 0
#define offsetof(type, field)		((uint32)&(((type *)NULL)->field))
#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, field)	offsetof(type, field)
#endif
#ifndef FIELD_SIZE
#define FIELD_SIZE(type, field)		(sizeof(((type *)NULL)->field))
#endif
#endif

/* string comparing
 */
/// Returns true if two strings are equal.
#define STR_EQ(s, t)		(!strcmp((s), (t)))
/// Returns true if two strings are the same, ignoring case.
#define STR_EQI(s, t)		(!bw_stricmp((s), (t)))
/// Returns true if the first sz byte of the input string are equal, ignoring
/// case.
#define STRN_EQI(s, t, sz)	(!bw_strnicmp((s), (t), (sz)))
/// Returns true if the first sz byte of the input string are equal, ignoring
/// case.
#define STRN_EQ(s, t, sz)	(!strncmp((s), (t), (sz)))
/// Returns true if all three input string are equal.
#define STR_EQ2(s, t1, t2)	(!(strcmp((s), (t1)) || strcmp((s), (t2))))


/* scalar types comparing
 */

#include <algorithm>

#if defined(_WIN32)

#undef min
#define min min
#undef max
#define max max

template <class T>
inline const T & min( const T & a, const T & b )
{
	return b < a ? b : a;
}

template <class T>
inline const T & max( const T & a, const T & b )
{
	return a < b ? b : a;
}

#define MF_MIN min
#define MF_MAX max

#define NOMINMAX

#else

#define MF_MIN std::min
#define MF_MAX std::max

#endif


/**
 * This macro creates a long out of 4 chars, used for all id's: ID, HdrId.
 * @note byte ordering on 80?86 == 4321
 */
#if 0
#define MAKE_ID(a, b, c, d)	\
	(uint32)(((uint32)(d)<<24) | ((uint32)(c)<<16) | ((uint32)(b)<<8) | (uint32)(a))
#endif


/* Intel Architecture is little endian (low byte presented first)
 * Motorola Architecture is big endian (high byte first)
 */
/// The current architecture is Little Endian.
#define MF_LITTLE_ENDIAN
/*#define MF_BIG_ENDIAN*/

#ifdef MF_LITTLE_ENDIAN
/* accessing individual bytes (int8) and words (int16) within
 * words and long words (int32).
 * Macros ending with W deal with words, L macros deal with longs
 */
/// Returns the high byte of a word.
#define HIBYTEW(b)		(((b) & 0xff00) >> 8)
/// Returns the low byte of a word.
#define LOBYTEW(b)		( (b) & 0xff)

/// Returns the high byte of a long.
#define HIBYTEL(b)		(((b) & 0xff000000L) >> 24)
/// Returns the low byte of a long.
#define LOBYTEL(b)		( (b) & 0xffL)

/// Returns byte 0 of a long.
#define BYTE0L(b)		( (b) & 0xffL)
/// Returns byte 1 of a long.
#define BYTE1L(b)		(((b) & 0xff00L) >> 8)
/// Returns byte 2 of a long.
#define BYTE2L(b)		(((b) & 0xff0000L) >> 16)
/// Returns byte 3 of a long.
#define BYTE3L(b)		(((b) & 0xff000000L) >> 24)

/// Returns the high word of a long.
#define HIWORDL(b)		(((b) & 0xffff0000L) >> 16)
/// Returns the low word of a long.
#define LOWORDL(b)		( (b) & 0xffffL)

/**
 *	This macro takes a dword ordered 0123 and reorder it to 3210.
 */
#define SWAP_DW(a)	  ( (((a) & 0xff000000)>>24) |	\
						(((a) & 0xff0000)>>8) |		\
						(((a) & 0xff00)<<8) |		\
						(((a) & 0xff)<<24) )

#else
/* big endian macros go here */
#endif

/// This macro is used to enter the debugger.
#if defined( _XBOX360 )
#define ENTER_DEBUGGER() DebugBreak()
#elif defined( _WIN32 )
#define ENTER_DEBUGGER() __asm { int 3 }
#elif defined( PLAYSTATION3 )
#define ENTER_DEBUGGER() __asm__ volatile ( ".int 0" )
#else
#define ENTER_DEBUGGER() asm( "int $3" )
#endif



/**
 *	This function returns user id.
 */
inline int getUserId()
{
#ifdef _WIN32
	// VS2005:
	#if _MSC_VER >= 1400
		char uid[16];
		size_t sz;
		return getenv_s( &sz, uid, sizeof( uid ), "UID" ) == 0 ? atoi( uid ) : 0;

	// VS2003:
	#elif _MSC_VER < 1400
		char * uid = getenv( "UID" );
		return uid ? atoi( uid ) : 0;
	#endif
#elif defined( PLAYSTATION3 )
	return 123;
#else
// Linux:
	char * uid = getenv( "UID" );
	return uid ? atoi( uid ) : getuid();
#endif
}


/**
 *	This function returns the username the process is running under.
 */
inline const char * getUsername()
{
#ifdef _WIN32
	return "";
#else
	// Note: a string in a static area is returned. Do not store this pointer.
	// See cuserid for details.
	char * pUsername = cuserid( NULL );

	return pUsername ? pUsername : "";
#endif
}


/**
 *	This function returns the process id.
 */
inline int mf_getpid()
{
#if defined(unix)
	return getpid();
#elif defined(_XBOX) || defined(_XBOX360) || defined( PLAYSTATION3 )
	return -1;
#else
	return (int) GetCurrentProcessId();
#endif
}

#if defined( unix ) || defined( PLAYSTATION3 )

#define bw_isnan isnan
#define bw_isinf isinf
#define bw_snprintf snprintf
#define bw_vsnprintf vsnprintf
#define bw_vsnwprintf vsnwprintf
#define bw_snwprintf swprintf
#define bw_stricmp strcasecmp
#define bw_strnicmp strncasecmp
#define bw_fileno fileno
#define bw_va_copy va_copy

#else

#define bw_isnan _isnan
#define bw_isinf(x) (!_finite(x) && !_isnan(x))
#define bw_snprintf _snprintf
#define bw_vsnprintf _vsnprintf
#define bw_vsnwprintf _vsnwprintf
#define bw_snwprintf _snwprintf
#define bw_stricmp _stricmp
#define bw_strnicmp _strnicmp
#define bw_fileno _fileno
#define bw_va_copy( dst, src) dst = src

#endif // unix

/* --------------------------------------------------------------------------
 * STL type info
 * --------------------------------------------------------------------------
 */

/**
 *  This class helps with using internal STL implementation details in
 *  different compilers.
 */
template <class MAP> struct MapTypes
{
#ifdef _WIN32
#if _MSC_VER>=1300 // VC7
	typedef typename MAP::mapped_type & _Tref;
#else
	typedef typename MAP::_Tref _Tref;
#endif
#else
	typedef typename MAP::mapped_type & _Tref;
#endif
};


#define MF_FLOAT_EQUAL(value1, value2) \
	(abs(value1 - value2) < std::numeric_limits<float>::epsilon())

// use 0.0004 because most existing functions are using it
inline bool almostEqual( const float f1, const float f2, const float epsilon = 0.0004f )
{
	return fabsf( f1 - f2 ) < epsilon;
}

inline bool almostEqual( const double d1, const double d2, const double epsilon = 0.0004 )
{
	return fabs( d1 - d2 ) < epsilon;
}

inline bool almostZero( const float f, const float epsilon = 0.0004f )
{
	return f < epsilon && f > -epsilon;
}

inline bool almostZero( const double d, const double epsilon = 0.0004 )
{
	return d < epsilon && d > -epsilon;
}

template<typename T>
inline bool almostEqual( const T& c1, const T& c2, const float epsilon = 0.0004f )
{
	if( c1.size() != c2.size() )
		return false;
	typename T::const_iterator iter1 = c1.begin();
	typename T::const_iterator iter2 = c2.begin();
	for( ; iter1 != c1.end(); ++iter1, ++iter2 )
		if( !almostEqual( *iter1, *iter2, epsilon ) )
			return false;
	return true;
}

#if !defined(_XBOX360) && !defined(PLAYSTATION3)
#if !defined(MF_SERVER) && !defined(EDITOR_ENABLED) && !defined(BW_EXPORTER) && !BWCLIENT_AS_PYTHON_MODULE && defined(_DLL)
#define ENABLE_MEMTRACKER
#elif defined(_DEBUG) && defined(MF_SERVER)
#define ENABLE_MEMTRACKER
#endif
#endif

// Raw versions
inline void* raw_malloc( size_t count )
{
	return malloc( count );
}

inline void* raw_realloc( void* p, size_t count )
{
	return realloc( p, count );
}

inline void raw_free( void* p )
{
	free( p );
}

#ifdef WIN32
inline char * raw_strdup( const char * s )
{
	return _strdup( s );
}

inline wchar_t * raw_wcsdup( const wchar_t * s )
{
	return _wcsdup( s );
}
#endif // WIN32

#if  !defined( ENABLE_MEMTRACKER ) || defined( EDITOR_ENABLED )
#define bw_virtualAlloc VirtualAlloc
#define bw_virtualFree VirtualFree
#endif

#if defined( ENABLE_MEMTRACKER ) || defined( EDITOR_ENABLED )

#ifdef EDITOR_ENABLED

// In the tools, we use Doug Lea's excellent allocator to avoid fragmentation
// that happens when editing and jumping around a space for example.
#include "dlmalloc.h"
#define bw_malloc_enabled dlenabled
#define bw_malloc dlmalloc
#define bw_realloc dlrealloc
#define bw_free dlfree
#define bw_strdup dlstrdup
#define bw_wcsdup dlwcsdup

#else // !EDITOR_ENABLED, so ENABLE_MEMTRACKER

// Intercept allocations for debugging and profiling, these functions are
// currently defined in memory_trace.cpp.
void* bw_malloc( size_t count );
void* bw_realloc( void* p, size_t count );
void bw_free( void* p );
#ifdef _WIN32
void* bw_virtualAlloc( void* pMemory, size_t size, DWORD allocationType, DWORD flProtect );
bool  bw_virtualFree( void* pMemory, size_t size, DWORD dwFreeType );
#endif
char* bw_strdup( const char* s );
wchar_t* bw_wcsdup( const wchar_t* s );

#endif // EDITOR_ENABLED

// Override new & delete
inline void* operator new(size_t sz)
{
	return bw_malloc(sz);
}

inline void operator delete(void* m)
{
	bw_free(m);
}

inline void* operator new[](size_t sz)
{
	return bw_malloc(sz);
}

inline void operator delete[](void* m)
{
	bw_free(m);
}

// Override malloc/realloc/free

#define malloc bw_malloc
#define realloc bw_realloc
#define free bw_free

// cstring header needs to be included here so that it doesn't get clobbered by
// the macros below.
#include <cstring>
#define strdup bw_strdup
#define _strdup bw_strdup
#define _wcsdup bw_wcsdup


// This macro overrides the _malloca function to make sure it uses the raw malloc function,
// not the memtracker ones, this is because _freea is implemented in a function which uses
// the stock free function.
// The _malloca override is identical to the macro defined in the VS 2005 supplied malloc.h,
// except for the use of the BigWorld raw_malloc function.

#ifdef WIN32
#undef _malloca
#if defined(_DEBUG)
#if !defined(_CRTDBG_MAP_ALLOC)
#define _malloca(size) \
__pragma(warning(suppress: 6255)) \
        _MarkAllocaS(raw_malloc((size) + _ALLOCA_S_MARKER_SIZE), _ALLOCA_S_HEAP_MARKER)
#endif
#else
#define _malloca(size) \
__pragma(warning(suppress: 6255)) \
    ((((size) + _ALLOCA_S_MARKER_SIZE) <= _ALLOCA_S_THRESHOLD) ? \
        _MarkAllocaS(_alloca((size) + _ALLOCA_S_MARKER_SIZE), _ALLOCA_S_STACK_MARKER) : \
        _MarkAllocaS(raw_malloc((size) + _ALLOCA_S_MARKER_SIZE), _ALLOCA_S_HEAP_MARKER))
#endif
#endif

#endif // defined( ENABLE_MEMTRACKER ) || defined( EDITOR_ENABLED )



/**
 *	Static (i.e. compile-time) assert. Based off 
 *	Modern C++ Design: Generic Programming and Design Patterns Applied
 *	Section 2.1
 *	by Andrei Alexandrescu
 */
template<bool> class BW_compile_time_check
{
public:
	BW_compile_time_check(...) {}
};

template<> class BW_compile_time_check<false>
{
};

#define BW_STATIC_ASSERT(test, errormsg)						\
	do {														\
		struct ERROR_##errormsg {};								\
		typedef BW_compile_time_check< (test) != 0 > TmplImpl;	\
		TmplImpl aTemp = TmplImpl( ERROR_##errormsg() );		\
		size_t x = sizeof( aTemp );								\
		x += 1;													\
	} while (0)

#endif // STDMF_HPP

/*end:stdmf.hpp*/
