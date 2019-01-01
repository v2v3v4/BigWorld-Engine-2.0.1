/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MF_DEBUG_HPP
#define MF_DEBUG_HPP

/**
 *	@file debug.hpp
 *
 *	This file contains macros and functions related to debugging.
 *
 *	A number of macros are defined to display debug information. They should be
 *	used as you would use the printf function. To use these macros,
 *	DECLARE_DEBUG_COMPONENT needs to be used in the cpp file. Its argument is
 *	the initial component priority.
 *
 *	With each message, there is an associated message priority. A message is
 *	only displayed if its priority is not less than
 *	DebugFilter::filterThreshold() plus the component's priority.
 *
 *	@param TRACE_MSG	Used to display trace information. That is, when you enter a
 *					method.
 *	@param DEBUG_MSG	Used to display debug information such as what a variable is
 *					equal to.
 *	@param INFO_MSG	Used to display general information such as when a
 *					particular process is started.
 *	@param NOTICE_MSG	Used to display information that is more important than an
 *					INFO_MSG message but is not a possible error.
 *	@param WARNING_MSG	Used to display warning messages. These are messages
 *					that could be errors and should be looked into.
 *	@param ERROR_MSG	Used to display error messages. These are messages that are
 *					errors and need to be fixed. The software should hopefully
 *					be able to survive these situations.
 *	@param CRITICAL_MSG Used to display critical error messages. These are message
 *					that are critical and cause the program not to continue.
 *	@param DEV_CRITICAL_MSG Used to display development time only critical
 *					error messages. These are message that are critical and
 *					cause the program not to continue.
 *	@param HACK_MSG	Used to display temporary messages. This is the highest
 *					priority message that can be used to temporarily print a
 *					message. No code should be commited with this in it.
 *	@param SCRIPT_MSG	Used to display messages printed from Python script.
 *
 *	@param MF_ASSERT 	is a macro that should be used instead of assert or ASSERT.
 */

#include <assert.h>

#include "config.hpp"
#include "dprintf.hpp"

/**
 *	This enumeration is used to indicate the priority of a message. The higher
 *	the enumeration's value, the higher the priority.
 */
enum DebugMessagePriority
{
	MESSAGE_PRIORITY_TRACE,
	MESSAGE_PRIORITY_DEBUG,
	MESSAGE_PRIORITY_INFO,
	MESSAGE_PRIORITY_NOTICE,
	MESSAGE_PRIORITY_WARNING,
	MESSAGE_PRIORITY_ERROR,
	MESSAGE_PRIORITY_CRITICAL,
	MESSAGE_PRIORITY_HACK,
	MESSAGE_PRIORITY_SCRIPT,
	MESSAGE_PRIORITY_ASSET,
	NUM_MESSAGE_PRIORITY
};

inline const char * messagePrefix( DebugMessagePriority p )
{
	static const char * prefixes[] =
	{
		"TRACE",
		"DEBUG",
		"INFO",
		"NOTICE",
		"WARNING",
		"ERROR",
		"CRITICAL",
		"HACK",
		"SCRIPT",
		"ASSET"
	};

	return (p >= 0 && (size_t)p < ARRAY_SIZE(prefixes)) ? prefixes[(int)p] : "";
}


class SimpleMutex;


/**
 *  This class implements the functionality exposed by BigWorld message macros,
 *	manages calling registered message callbacks, and handles both critical
 *  and non-critical messages.
 */
class DebugMsgHelper
{
public:
	DebugMsgHelper( int componentPriority, int messagePriority ) :
		componentPriority_( componentPriority ),
		messagePriority_( messagePriority )
	{
	}
	DebugMsgHelper() :
		componentPriority_( 0 ),
		messagePriority_( MESSAGE_PRIORITY_CRITICAL )
	{
	}
	static void fini();

#ifndef _WIN32
	void message( const char * format, ... )
        __attribute__ ( (format (printf, 2, 3 ) ) );
    void criticalMessage( const char * format, ... )
        __attribute__ ( (format (printf, 2, 3 ) ) );
    void devCriticalMessage( const char * format, ... )
        __attribute__ ( (format (printf, 2, 3 ) ) );
#else
	void message( const char * format, ... );
	void criticalMessage( const char * format, ... );
	void devCriticalMessage( const char * format, ... );
#endif

	void messageBackTrace();
	static void shouldWriteToSyslog( bool state = true );

	static void showErrorDialogs( bool show );
	static bool showErrorDialogs();

	static void criticalMsgOccurs( bool occurs )	{	criticalMsgOccurs_ = occurs;	}
	static bool criticalMsgOccurs()	{	return criticalMsgOccurs_;	}

#ifdef _WIN32
	static void logToFile( const char* line );
	static void automatedTest( bool isTest ) { automatedTest_ = isTest; }
	static bool automatedTest() { return automatedTest_; }
#endif

private:
	void criticalMessageHelper( bool isDevAssertion, const char * format,
			va_list argPtr );

	int componentPriority_;
	int messagePriority_;

	static bool showErrorDialogs_;
	static bool criticalMsgOccurs_;
	static SimpleMutex* mutex_;
#ifdef _WIN32
	static bool automatedTest_;
#endif
};


/**
 *	This macro is a helper used by the *_MSG macros.
 */
namespace
{
	// This is the default s_componentPriority for files that does not
	// have DECLARE_DEBUG_COMPONENT2(). Useful for hpp and ipp files that
	// uses debug macros. s_componentPriority declared by
	//	DECLARE_DEBUG_COMPONENT2() will have precedence over this one.
	const int s_componentPriority = 0;
}

/// This macro prints a debug message with CRITICAL priority.
/// CRITICAL_MSG is always enabled no matter what the build target is.
#define CRITICAL_MSG													\
	DebugMsgHelper( ::s_componentPriority,								\
					MESSAGE_PRIORITY_CRITICAL ).criticalMessage


#if ENABLE_MSG_LOGGING

#define BACKTRACE_WITH_PRIORITY( PRIORITY )								\
	DebugMsgHelper( ::s_componentPriority, 								\
		PRIORITY ).messageBackTrace

#define TRACE_BACKTRACE		BACKTRACE_WITH_PRIORITY( MESSAGE_PRIORITY_TRACE )
#define DEBUG_BACKTRACE		BACKTRACE_WITH_PRIORITY( MESSAGE_PRIORITY_DEBUG )
#define INFO_BACKTRACE		BACKTRACE_WITH_PRIORITY( MESSAGE_PRIORITY_INFO )
#define NOTICE_BACKTRACE	BACKTRACE_WITH_PRIORITY( MESSAGE_PRIORITY_NOTICE )
#define WARNING_BACKTRACE	BACKTRACE_WITH_PRIORITY( MESSAGE_PRIORITY_WARNING )
#define ERROR_BACKTRACE		BACKTRACE_WITH_PRIORITY( MESSAGE_PRIORITY_ERROR )
#define CRITICAL_BACKTRACE	BACKTRACE_WITH_PRIORITY( MESSAGE_PRIORITY_CRITICAL )
#define HACK_BACKTRACE		BACKTRACE_WITH_PRIORITY( MESSAGE_PRIORITY_HACK )


#define DEBUG_MSG_WITH_PRIORITY( PRIORITY )								\
	DebugMsgHelper( ::s_componentPriority, PRIORITY ).message

// The following macros are used to display debug information. See comment at
// the top of this file.

/// This macro prints a debug message with TRACE priority.
#define TRACE_MSG		DEBUG_MSG_WITH_PRIORITY( MESSAGE_PRIORITY_TRACE )

/// This macro prints a debug message with DEBUG priority.
#define DEBUG_MSG		DEBUG_MSG_WITH_PRIORITY( MESSAGE_PRIORITY_DEBUG )

/// This macro prints a debug message with INFO priority.
#define INFO_MSG		DEBUG_MSG_WITH_PRIORITY( MESSAGE_PRIORITY_INFO )

/// This macro prints a debug message with NOTICE priority.
#define NOTICE_MSG		DEBUG_MSG_WITH_PRIORITY( MESSAGE_PRIORITY_NOTICE )

/// This macro prints a debug message with WARNING priority.
#define WARNING_MSG															\
		DEBUG_MSG_WITH_PRIORITY( MESSAGE_PRIORITY_WARNING )

/// This macro prints a debug message with ERROR priority.
#define ERROR_MSG															\
		DEBUG_MSG_WITH_PRIORITY( MESSAGE_PRIORITY_ERROR )

/// This macro prints a debug message with HACK priority.
#define HACK_MSG		DEBUG_MSG_WITH_PRIORITY( MESSAGE_PRIORITY_HACK )

/// This macro prints a debug message with SCRIPT priority.
#define SCRIPT_MSG		DEBUG_MSG_WITH_PRIORITY( MESSAGE_PRIORITY_SCRIPT )

/// This macro prints a development time only message CRITICAL priority.
#define DEV_CRITICAL_MSG	CRITICAL_MSG

#else	// ENABLE_MSG_LOGGING

#define NULL_MSG(...)														\
	do																		\
	{																		\
		if (false)															\
			printf(__VA_ARGS__);											\
	}																		\
	while (false)

#define TRACE_MSG(...)			NULL_MSG(__VA_ARGS__)
#define DEBUG_MSG(...)			NULL_MSG(__VA_ARGS__)
#define INFO_MSG(...)			NULL_MSG(__VA_ARGS__)
#define NOTICE_MSG(...)			NULL_MSG(__VA_ARGS__)
#define WARNING_MSG(...)		NULL_MSG(__VA_ARGS__)
#define ERROR_MSG(...)			NULL_MSG(__VA_ARGS__)
#define HACK_MSG(...)			NULL_MSG(__VA_ARGS__)
#define SCRIPT_MSG(...)			NULL_MSG(__VA_ARGS__)
#define DEV_CRITICAL_MSG(...)	NULL_MSG(__VA_ARGS__)

#endif	// ENABLE_MSG_LOGGING


/**
 *	This macro used to display trace information. Can be used later to add in
 *	our own callstack if necessary.
 */
#define ENTER( className, methodName )									\
	TRACE_MSG( className "::" methodName "\n" )


/**
 *	This function eats the arguments of *_MSG macros when in Release mode
 */
inline void debugMsgNULL( const char * /*format*/, ... )
{
}



const char * mf_debugMunge( const char * path, const char * module = NULL );


/**
 *  This function determines if a float is a valid and finite
 */
union IntFloat
{
	float f;
	uint32 ui32;
};

inline bool isFloatValid( float f )
{
	IntFloat intFloat;
	intFloat.f = f;
	return ( intFloat.ui32 & 0x7f800000 ) != 0x7f800000;
}

#ifndef _RELEASE
	#if ENABLE_WATCHERS
	extern int bwWatchInt( int & value, const char * path );

	/**
	*	This macro needs to be placed in a cpp file before any of the *_MSG macros
	*	can be used.
	*
	*	@param module	The name (or path) of the watcher module that the component
	*					priority should be displayed in.
	*	@param priority	The initial component priority of the messages in the file.
	*/
		#define DECLARE_DEBUG_COMPONENT2( module, priority )				\
			static int s_componentPriority = priority;						\
			static int IGNORE_THIS_COMPONENT_WATCHER_INIT =					\
				 bwWatchInt( ::s_componentPriority,							\
									mf_debugMunge( __FILE__, module ) );
	#else
		#define DECLARE_DEBUG_COMPONENT2( module, priority )					\
			static int s_componentPriority = priority;
	#endif
#else
	#define DECLARE_DEBUG_COMPONENT2( module, priority )
#endif

/**
 *	This macro needs to be placed in a cpp file before any of the *_MSG macros
 *	can be used.
 *
 *	@param priority	The initial component priority of the messages in the file.
 */
#define DECLARE_DEBUG_COMPONENT( priority )									\
	DECLARE_DEBUG_COMPONENT2( NULL, priority )




#include <assert.h>

#ifdef __ASSERT_FUNCTION
#	define MF_FUNCNAME __ASSERT_FUNCTION
#else
#		define MF_FUNCNAME ""
#endif

#ifdef MF_USE_ASSERTS
#	define MF_REAL_ASSERT assert(0);
#else
#	define MF_REAL_ASSERT
#endif

// The MF_ASSERT macro should used in place of the assert and ASSERT macros.
#if !defined( _RELEASE )
/**
 *	This macro should be used instead of assert.
 *
 *	@see MF_ASSERT_DEBUG
 */
#	define MF_ASSERT( exp )													\
		if (!(exp))															\
		{																	\
			DebugMsgHelper().criticalMessage(								\
				"ASSERTION FAILED: " #exp "\n"								\
					__FILE__ "(%d)%s%s\n", (int)__LINE__,					\
					*MF_FUNCNAME ? " in " : "",								\
					MF_FUNCNAME );											\
																			\
			MF_REAL_ASSERT													\
		}
#else	// _RELEASE

#	define MF_ASSERT( exp )
#endif // !_RELEASE


// The MF_ASSERT_DEBUG is like MF_ASSERT except it is only evaluated
// in debug builds.
#ifdef _DEBUG
#	define MF_ASSERT_DEBUG		MF_ASSERT
#else
/**
 *	This macro should be used instead of assert. It is enabled only
 *	in debug builds, unlike MF_ASSERT which is enabled in both
 *	debug and hybrid builds.
 *
 *	@see MF_ASSERT
 */
#	define MF_ASSERT_DEBUG( exp )
#endif


#if defined( MF_SERVER ) || defined ( EDITOR_ENABLED ) || !defined( _RELEASE )
/**
 *	An assertion which is only lethal when not in a production environment.
 *	These are disabled for client release builds.
 *
 *	@see MF_ASSERT
 */
#	define MF_ASSERT_DEV( exp )													\
			if (!(exp))															\
			{																	\
				DebugMsgHelper().devCriticalMessage(							\
						"MF_ASSERT_DEV FAILED: " #exp "\n"						\
							__FILE__ "(%d)%s%s\n",								\
						(int)__LINE__,											\
						*MF_FUNCNAME ? " in " : "", MF_FUNCNAME );				\
			}

#else

/**
*	Empty versions of above function - not available on client release builds.
*/
#	define MF_ASSERT_DEV( exp )

#endif

/**
 *	An assertion which is only lethal when not in a production environment.
 *	In a production environment, the block of code following the macro will
 *	be executed if the assertion fails.
 *
 *	@see MF_ASSERT_DEV
 */
#define IF_NOT_MF_ASSERT_DEV( exp )												\
		if ((!( exp )) && (														\
			DebugMsgHelper().devCriticalMessage(								\
				"MF_ASSERT_DEV FAILED: " #exp "\n"								\
					__FILE__ "(%d)%s%s\n", (int)__LINE__,						\
					*MF_FUNCNAME ? " in " : "", MF_FUNCNAME ),					\
			true))		// leave trailing block after message


/**
 *	This macro is used to assert a pre-condition.
 *
 *	@see MF_ASSERT
 *	@see POST
 */
#define PRE( exp )	MF_ASSERT( exp )

/**
 *	This macro is used to assert a post-condition.
 *
 *	@see MF_ASSERT
 *	@see PRE
 */
#define POST( exp )	MF_ASSERT( exp )

/**
 *	This macro is used to verify an expression. In non-release it
 *	asserts on failure, and in release the expression is still
 *	evaluated.
 *
 *	@see MF_ASSERT
 */
#ifdef _RELEASE
#define MF_VERIFY( exp ) (exp)
#define MF_VERIFY_DEV( exp ) (exp)
#else
#define MF_VERIFY MF_ASSERT
#define MF_VERIFY_DEV MF_ASSERT_DEV
#endif

// this is a placeholder until a better solution can be implemented
#define MF_EXIT(msg) {\
			DebugMsgHelper().criticalMessage(								\
				"FATAL ERROR: " #msg "\n"									\
					__FILE__ "(%d)%s%s\n", (int)__LINE__,					\
					*MF_FUNCNAME ? " in " : "",								\
					MF_FUNCNAME );											\
																			\
			MF_REAL_ASSERT													\
}


/**
 *	This class is used to query if the current thread is the main thread.
 */
class MainThreadTracker
{
public:
	MainThreadTracker();

	static bool isCurrentThreadMain();
};


/**
 *  This class serves as base class for classes that want to handle critical
 *  messages in different ways, and also keeps track of the current critical
 *  message handler through its static methods.
 */
class CriticalErrorHandler
{
	static CriticalErrorHandler* handler_;
public:
	enum Result
	{
		ENTERDEBUGGER = 0,
		EXITDIRECTLY
	};
	static CriticalErrorHandler* get();
	static CriticalErrorHandler* set( CriticalErrorHandler* );
	virtual ~CriticalErrorHandler(){}
	virtual Result ask( const char* msg ) = 0;
	virtual void recordInfo( bool willExit ) = 0;
};


#ifdef WIN32
#if ENABLE_STACK_TRACKER

DWORD ExceptionFilter(DWORD exceptionCode, struct _EXCEPTION_POINTERS *ep);

template<typename T, typename R>
R CallWithExceptionFilter( T* obj, R (T::*method)() )
{
	if (IsDebuggerPresent())
	{
		return (obj->*method)();
	}

	__try
	{
		return (obj->*method)();
	}
	__except( ExceptionFilter(GetExceptionCode(), GetExceptionInformation()) )
	{}

	return R();
}

template<typename T, typename R, typename P1>
R CallWithExceptionFilter( T* obj, R (T::*method)( P1 p1 ), P1 p1 )
{
	if (IsDebuggerPresent())
	{
		return (obj->*method)( p1 );
	}

	__try
	{
		return (obj->*method)( p1 );
	}
	__except( ExceptionFilter(GetExceptionCode(), GetExceptionInformation()) )
	{}

	return R();
}

template<typename R, typename P1>
R CallWithExceptionFilter( R (*method)( P1 p1 ), P1 p1 )
{
	if (IsDebuggerPresent())
	{
		return method( p1 );
	}

	__try
	{
		return method( p1 );
	}
	__except( ExceptionFilter(GetExceptionCode(), GetExceptionInformation()) )
	{}

	return R();
}

template<typename R, typename P1, typename P2, typename P3>
R CallWithExceptionFilter( R (*method)( P1 p1, P2 p2, P3 p3 ), P1 p1, P2 p2, P3 p3 )
{
	if (IsDebuggerPresent())
	{
		return method( p1, p2, p3, p4 );
	}

	__try
	{
		return method( p1, p2, p3, p4 );
	}
	__except( ExceptionFilter(GetExceptionCode(), GetExceptionInformation()) )
	{}

	return R();
}

template<typename R, typename P1, typename P2, typename P3, typename P4>
R CallWithExceptionFilter( R (*method)( P1 p1, P2 p2, P3 p3, P4 p4 ), P1 p1, P2 p2, P3 p3, P4 p4 )
{
	if (IsDebuggerPresent())
	{
		return method( p1, p2, p3, p4 );
	}

	__try
	{
		return method( p1, p2, p3, p4 );
	}
	__except( ExceptionFilter(GetExceptionCode(), GetExceptionInformation()) )
	{}

	return R();
}

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
R CallWithExceptionFilter( R (*method)( P1 p1, P2 p2, P3 p3, P4 p4, P5 p5 ), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5 )
{
	if (IsDebuggerPresent())
	{
		return method( p1, p2, p3, p4, p5 );
	}

	__try
	{
		return method( p1, p2, p3, p4, p5 );
	}
	__except( ExceptionFilter(GetExceptionCode(), GetExceptionInformation()) )
	{}

	return R();
}

#else//ENABLE_STACK_TRACKER

template<typename T, typename R>
R CallWithExceptionFilter( T* obj, R (T::*method)() )
{
	return (obj->*method)();
}

template<typename T, typename R, typename P1>
R CallWithExceptionFilter( T* obj, R (T::*method)( P1 p1 ), P1 p1 )
{
	return (obj->*method)( p1 );
}

template<typename R, typename P1>
R CallWithExceptionFilter( R (*method)( P1 p1 ), P1 p1 )
{
	return method( p1 );
}

template<typename R, typename P1, typename P2, typename P3>
R CallWithExceptionFilter( R (*method)( P1 p1, P2 p2, P3 p3 ), P1 p1, P2 p2, P3 p3 )
{
	return method( p1, p2, p3, p4 );
}

template<typename R, typename P1, typename P2, typename P3, typename P4>
R CallWithExceptionFilter( R (*method)( P1 p1, P2 p2, P3 p3, P4 p4 ), P1 p1, P2 p2, P3 p3, P4 p4 )
{
	return method( p1, p2, p3, p4 );
}

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
R CallWithExceptionFilter( R (*method)( P1 p1, P2 p2, P3 p3, P4 p4, P5 p5 ), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5 )
{
	return method( p1, p2, p3, p4, p5 );
}

#endif//ENABLE_STACK_TRACKER

#ifndef _XBOX360
LONG WINAPI ErrorCodeExceptionFilter(_EXCEPTION_POINTERS *ExceptionInfo);
LONG WINAPI SuccessExceptionFilter(_EXCEPTION_POINTERS *ExceptionInfo);
#endif
#endif
#endif // MF_DEBUG_HPP
