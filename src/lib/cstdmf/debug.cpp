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

#include "cstdmf/string_utils.hpp"

#include "bw_util.hpp"
#include "concurrency.hpp"
#include "debug.hpp"
#include "dprintf.hpp"
#include "stack_tracker.hpp"
#include "timestamp.hpp"

#include <string.h>
#if defined(_WIN32)
#include <windows.h>
#include <time.h>
#pragma warning( disable: 4995 ) // disable warning for unsafe string functions
#include "critical_message_box.hpp"
#pragma comment( lib, "winmm" )
#elif defined( PLAYSTATION3 )
#else
#include <unistd.h>
#include <syslog.h>
#endif

DECLARE_DEBUG_COMPONENT2( "CStdMF", 0 );

bool g_shouldWriteToSyslog = false;
std::string g_syslogAppName;

#define BW_DEBUG_BUFSIZ 2048

static const char DEV_ASSERT_MSG[] = 
	"Development assertions may indicate failures caused by incorrect "
	"engine usage.\n"
	"In "
#ifdef MF_SERVER
	"production mode"
#else
	"Release builds"
#endif
	", they do not cause the application to exit.\n"
	"Please investigate potential misuses of the engine at the time of the "
	"failure.\n";

//-------------------------------------------------------
//	Section: MainThreadTracker
//-------------------------------------------------------

/**
 *	This static thread-local variable is initialised to false, and set to true
 *	in the constructor of the static s_mainThreadTracker object below
 */
static THREADLOCAL( bool ) s_isCurrentMainThread_( false );


/**
 *	Constructor
 */
MainThreadTracker::MainThreadTracker()
{
	s_isCurrentMainThread_ = true;
}


/**
 *	Static method that returns true if the current thread is the main thread,
 *  false otherwise.
 *
 *	@returns      true if the current thread is the main thread, false if not
 */
/* static */ bool MainThreadTracker::isCurrentThreadMain()
{
	return s_isCurrentMainThread_;
}


// Instantiate it, so it initialises the flag to the main thread
static MainThreadTracker s_mainThreadTracker;


//-------------------------------------------------------
//	Section: debug funtions
//-------------------------------------------------------

/**
 *	This function is used to strip the path and return just the basename from a
 *	path string.
 */
const char * mf_debugMunge( const char * path, const char * module )
{
	static char	staticSpace[128];

	const char * pResult = path;

	const char * pSeparator;

	pSeparator = strrchr( pResult, '\\' );
	if (pSeparator != NULL)
	{
		pResult = pSeparator + 1;
	}

	pSeparator = strrchr( pResult, '/' );
	if (pSeparator != NULL)
	{
		pResult = pSeparator + 1;
	}

	strcpy( staticSpace, "logger/cppThresholds/" );

	if (module != NULL)
	{
		strcat( staticSpace, module );
		strcat( staticSpace, "/" );
	}

	strcat(staticSpace,pResult);
	return staticSpace;
}


//-------------------------------------------------------
//	Section: DebugMsgHelper
//-------------------------------------------------------

/**
 *	This is a helper function used by the CRITICAL_MSG macro.
 */
void DebugMsgHelper::criticalMessage( const char * format, ... )
{
	va_list argPtr;
	va_start( argPtr, format );
	this->criticalMessageHelper( false, format, argPtr );
	va_end( argPtr );
}


/**
 *	This is a helper function used by the CRITICAL_MSG macro. If
 *	DebugFilter::hasDevelopmentAssertions() is true, this will cause a proper
 *	critical message otherwise, it'll behaviour similar to a normal error
 *	message.
 */
void DebugMsgHelper::devCriticalMessage( const char * format, ... )
{
	va_list argPtr;
	va_start( argPtr, format );
	this->criticalMessageHelper( true, format, argPtr );
	va_end( argPtr );
}


/**
 *	This is a helper function used by the CRITICAL_MSG macro.
 */
void DebugMsgHelper::criticalMessageHelper( bool isDevAssertion,
		const char * format, va_list argPtr )
{
	char buffer[ BW_DEBUG_BUFSIZ * 2 ];

	DebugMsgHelper::criticalMsgOccurs( true );

	bw_vsnprintf( buffer, sizeof(buffer), format, argPtr );
	buffer[sizeof(buffer)-1] = '\0';

#if ENABLE_STACK_TRACKER

#if defined( _WIN32 )
	#define NEW_LINE "\r\n"
#else//WIN32
	#define NEW_LINE "\n"
#endif//WIN32

	if (StackTracker::stackSize() > 0)
	{
		std::string stack = StackTracker::buildReport();

		strcat( buffer, NEW_LINE );
		strcat( buffer, "Stack trace: " );
		strcat( buffer, stack.c_str() );
		strcat( buffer, NEW_LINE );
	}
#endif

#if !defined( _WIN32 ) && !defined( PLAYSTATION3 )
	// send to syslog if it's been initialised
	if (g_shouldWriteToSyslog)
	{
		syslog( LOG_CRIT, "%s", buffer );
	}
#endif

	// output it as a normal message
	this->message( "%s", buffer );
	if (isDevAssertion)
	{
		this->message( "%s", DEV_ASSERT_MSG );
	}

	this->messageBackTrace();

	if (isDevAssertion && !	DebugFilter::instance().hasDevelopmentAssertions())
	{
		// dev assert and we don't have dev asserts enabled
		return;
	}

	// now do special critical message stuff
	if (DebugFilter::instance().getCriticalCallbacks().size() != 0)
	{
		DebugFilter::CriticalCallbacks::const_iterator it =
			DebugFilter::instance().getCriticalCallbacks().begin();
		DebugFilter::CriticalCallbacks::const_iterator end =
			DebugFilter::instance().getCriticalCallbacks().end();

		for (; it!=end; ++it)
		{
			(*it)->handleCritical( buffer );
		}
	}

#ifdef _XBOX360
	{
		OutputDebugStringA( buffer );

		LPCWSTR buttons[] = { L"Exit" };
		XOVERLAPPED         overlapped;					// Overlapped object for message box UI
		MESSAGEBOX_RESULT   result;						// Message box button pressed result

		ZeroMemory( &overlapped, sizeof( XOVERLAPPED ) );

		char tcbuffer[ BW_DEBUG_BUFSIZ * 2 ];
		WCHAR wcbuffer[ BW_DEBUG_BUFSIZ * 2 ];

		vsnprintf( tcbuffer, ARRAY_SIZE(tcbuffer), format, argPtr );
		tcbuffer[sizeof(tcbuffer)-1] = '\0';

		MultiByteToWideChar( CP_UTF8, 0, tcbuffer, -1, wcbuffer, ARRAYSIZE(wcbuffer) );

		DWORD dwRet = XShowMessageBoxUI( 0,
					L"Critical Error",					// Message box title
					wcbuffer,							// Message string
					ARRAYSIZE(buttons),					// Number of buttons
					buttons,							// Button captions
					0,									// Button that gets focus
					XMB_ERRORICON,						// Icon to display
					&result,							// Button pressed result
					&overlapped );

		//assert( dwRet == ERROR_IO_PENDING );

		while( !XHasOverlappedIoCompleted( &overlapped ) )
		{
			extern IDirect3DDevice9 *		g_pd3dDevice;
			g_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, D3DCOLOR_XRGB(0,0,0), 1.0f, 0L );
			g_pd3dDevice->Present( 0, 0, NULL, 0 );
		}

		for( int i=0; i<60; i++ )
		{
			extern IDirect3DDevice9 *		g_pd3dDevice;
			g_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, D3DCOLOR_XRGB(0,0,0), 1.0f, 0L );
			g_pd3dDevice->Present( 0, 0, NULL, 0 );
		}

		XLaunchNewImage( "", 0 );

		//ENTER_DEBUGGER();
	}
#elif defined ( PLAYSTATION3 )
	printf( buffer );
	printf( "\n" );
	ENTER_DEBUGGER();
#elif defined(_WIN32)

	if ( automatedTest_ )
	{
		_set_abort_behavior( 0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT );
		logToFile( "Critical Error, aborting test." );
		logToFile( buffer );
		abort();
	}

	#if ENABLE_ENTER_DEBUGGER_MESSAGE
		// Disable all abort() behaviour in case we call it
		_set_abort_behavior( 0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT );

		if( CriticalErrorHandler::get() )
		{
			switch( CriticalErrorHandler::get()->ask( buffer ) )
			{
			case CriticalErrorHandler::ENTERDEBUGGER:
				CriticalErrorHandler::get()->recordInfo( false );
				ENTER_DEBUGGER();
				break;
			case CriticalErrorHandler::EXITDIRECTLY:
				CriticalErrorHandler::get()->recordInfo( true );
				abort();
				break;
			}
		}
		else
			abort();
	#else // ENABLE_ENTER_DEBUGGER_MESSAGE
		strcat( buffer, "\n\nThe application must exit.\n" );
		::MessageBox( 0, bw_utf8tow( buffer ).c_str(), L"Critical Error Occurred", MB_ICONHAND | MB_OK );
		abort();
	#endif// ENABLE_ENTER_DEBUGGER_MESSAGE

#else // defined(_WIN32)

	char	filename[512],	hostname[256];
	if (gethostname( hostname, sizeof(hostname) ) != 0)
		hostname[0] = 0;

	char exeName[512];
	const char * pExeName = "unknown";

	int len = readlink( "/proc/self/exe", exeName, sizeof(exeName) - 1 );
	if (len > 0)
	{
		exeName[ len ] = '\0';

		char * pTemp = strrchr( exeName, '/' );
		if (pTemp != NULL)
		{
			pExeName = pTemp + 1;
		}
	}

	bw_snprintf( filename, sizeof(filename), "assert.%s.%s.%d.log", pExeName, hostname, getpid() );

	FILE * assertFile = bw_fopen( filename, "a" );
	fprintf( assertFile, "%s", buffer );
	fclose( assertFile );

	volatile uint64 crashTime = timestamp(); // For reference in the coredump.
	crashTime = crashTime; // Disable compiler warning about unused variable.

	*(int*)NULL = 0;
	typedef void(*BogusFunc)();
	((BogusFunc)NULL)();

#endif // defined(_WIN32)
}

namespace
{
	const char * const prefixes[] =
	{
		"TRACE: ",
		"DEBUG: ",
		"INFO: ",
		"NOTICE: ",
		"WARNING: ",
		"ERROR: ",
		"CRITICAL: ",
		"HACK: ",
		NULL	// Script
	};
}

/*static*/ bool DebugMsgHelper::showErrorDialogs_ = true;
/*static*/ bool DebugMsgHelper::criticalMsgOccurs_ = false;
/*static*/ SimpleMutex* DebugMsgHelper::mutex_ = NULL;

#ifdef WIN32
/*static*/ bool DebugMsgHelper::automatedTest_ = false;
#endif


/*static*/ void DebugMsgHelper::shouldWriteToSyslog( bool state )
{
	g_shouldWriteToSyslog = state;
}

/**
 *	This method allow tools to have a common method to set whether to show error dialogs or not
 *  Do this in a thread-safe way.
 */
/*static*/ void DebugMsgHelper::showErrorDialogs( bool show )
{
	if (!mutex_) mutex_ = new SimpleMutex;
	mutex_->grab();
	showErrorDialogs_ = show;
	mutex_->give();
}

/**
 *	This method allow tools to have a common method to determine whether to show error dialogs
 */
/*static*/ bool DebugMsgHelper::showErrorDialogs()
{
	if (!mutex_) mutex_ = new SimpleMutex;
	mutex_->grab();
	bool showErrorDialogs = showErrorDialogs_;
	mutex_->give();
	return showErrorDialogs;
}


/*static*/ void DebugMsgHelper::fini()
{
	if (mutex_)
		delete mutex_;
	mutex_=NULL;

	DebugFilter::fini();
}

/**
 *	This function is a helper to the *_MSG macros.
 */
void DebugMsgHelper::message( const char * format, ... )
{
	bool handled = false;

	// Break early if this message should be filtered out.
	if (!DebugFilter::shouldAccept( componentPriority_, messagePriority_ ))
	{
		return;
	}

	va_list argPtr;
	va_start( argPtr, format );


#if !defined( _WIN32 ) && !defined( PLAYSTATION3 )
	// send to syslog if it's been initialised
	if ((g_shouldWriteToSyslog) &&
		( (messagePriority_ == MESSAGE_PRIORITY_ERROR) ||
	      (messagePriority_ == MESSAGE_PRIORITY_CRITICAL) ))
	{
		char buffer[ BW_DEBUG_BUFSIZ * 2 ];

		// Need to make a copy of the va_list here to avoid crashing on 64bit
		va_list tmpArgPtr;
		bw_va_copy( tmpArgPtr, argPtr );
		vsnprintf( buffer, sizeof(buffer), format, tmpArgPtr );
		buffer[sizeof(buffer)-1] = '\0';
		va_end( tmpArgPtr );

		syslog( LOG_CRIT, "%s", buffer );
	}
#endif

	DebugFilter::DebugCallbacks::const_iterator it =
		DebugFilter::instance().getMessageCallbacks().begin();
	DebugFilter::DebugCallbacks::const_iterator end =
		DebugFilter::instance().getMessageCallbacks().end();

	for (; it!=end; ++it)
	{
		if (!handled)
		{
			// Need to make a copy of the va_list here to avoid crashing on 64bit
			va_list tmpArgPtr;
			bw_va_copy( tmpArgPtr, argPtr );
			handled = (*it)->handleMessage(
				componentPriority_, messagePriority_, format, tmpArgPtr );
			va_end( tmpArgPtr );
		}
	}

	if (!handled)
	{
		if (0 <= messagePriority_ &&
			messagePriority_ < int(sizeof(prefixes) / sizeof(prefixes[0])) &&
			prefixes[messagePriority_] != NULL)
		{
			vdprintf( componentPriority_, messagePriority_,
					format, argPtr,
					prefixes[messagePriority_] );
		}
		else
		{
			vdprintf( componentPriority_, messagePriority_,
				format, argPtr );
		}
	}
	va_end( argPtr );
}


#ifdef unix
#define MAX_DEPTH 50
#include <execinfo.h>
#include <cxxabi.h>

void DebugMsgHelper::messageBackTrace()
{
	void ** traceBuffer = new void*[MAX_DEPTH];
	uint32 depth = backtrace( traceBuffer, MAX_DEPTH );
	char ** traceStringBuffer = backtrace_symbols( traceBuffer, depth );
	for (uint32 i = 0; i < depth; i++)
	{
		// Format: <executable path>(<mangled-function-name>+<function
		// instruction offset>) [<eip>]
		std::string functionName;

		std::string traceString( traceStringBuffer[i] );
		std::string::size_type begin = traceString.find( '(' );
		bool gotFunctionName = (begin >= 0);

		if (gotFunctionName)
		{
			// Skip the round bracket start.
			++begin;
			std::string::size_type bracketEnd = traceString.find( ')', begin );
			std::string::size_type end = traceString.rfind( '+', bracketEnd );
			std::string mangled( traceString.substr( begin, end - begin ) );

			int status = 0;
			size_t demangledBufferLength = 0;
			char * demangledBuffer = abi::__cxa_demangle( mangled.c_str(), 0, 
				&demangledBufferLength, &status );

			if (demangledBuffer)
			{
				functionName.assign( demangledBuffer, demangledBufferLength );

				// __cxa_demangle allocates the memory for the demangled
				// output using malloc(), we need to free it.
#ifdef ENABLE_MEMTRACKER
				raw_free( demangledBuffer );
#else
				free( demangledBuffer );
#endif
			}
			else
			{
				// Didn't demangle, but we did get a function name, use that.
				functionName = mangled;
			}
		}

		this->message( "Stack: #%d %s\n", 
			i, 
			(gotFunctionName) ? functionName.c_str() : traceString.c_str() );
	}

#ifdef ENABLE_MEMTRACKER
	raw_free( traceStringBuffer );
#else
	free( traceStringBuffer );
#endif
	delete[] traceBuffer;
}

#else

void DebugMsgHelper::messageBackTrace()
{
}

#endif

#ifdef _WIN32
void DebugMsgHelper::logToFile( const char* line )
{
	if ( !automatedTest_ )
	{
		return;
	}

	FILE* log = fopen( "debug.log", "a" );
	if ( !log )
	{
		return;
	}

	time_t rawtime;
	time( &rawtime );

	fprintf( log, "\n%s%s\n", ctime( &rawtime ), line );

	fclose( log );
}
#endif

char __scratch[] = "DebugLibTestString Tue Nov 29 11:54:35 EST 2005";

#if defined(_WIN32) && !defined(_XBOX)

class DefaultCriticalErrorHandler : public CriticalErrorHandler
{
	virtual Result ask( const char* msg )
	{
#ifdef BUILT_BY_BIGWORLD
		CriticalMsgBox mb( msg, true );
#else//BUILT_BY_BIGWORLD
		CriticalMsgBox mb( msg, false );
#endif//BUILT_BY_BIGWORLD
		if (mb.doModal())
			return ENTERDEBUGGER;

		return EXITDIRECTLY;
	}
	virtual void recordInfo( bool willExit )
	{}
}
DefaultCriticalErrorHandler;

CriticalErrorHandler* CriticalErrorHandler::handler_ = &DefaultCriticalErrorHandler;

#else//defined(_WIN32)

CriticalErrorHandler* CriticalErrorHandler::handler_;

#endif//defined(_WIN32)

CriticalErrorHandler* CriticalErrorHandler::get()
{
	return handler_;
}

CriticalErrorHandler* CriticalErrorHandler::set( CriticalErrorHandler* handler )
{
	CriticalErrorHandler* old = handler_;
	handler_ = handler;
	return old;
}



#ifdef WIN32

#define EXCEPTION_CASE(e) case e: return #e

static const char* exceptionCodeAsString(DWORD code)
{
	switch(code)
	{
	EXCEPTION_CASE( EXCEPTION_ACCESS_VIOLATION );
	EXCEPTION_CASE( EXCEPTION_INVALID_HANDLE );
	EXCEPTION_CASE( EXCEPTION_STACK_OVERFLOW );
	EXCEPTION_CASE( EXCEPTION_PRIV_INSTRUCTION );
	EXCEPTION_CASE( EXCEPTION_ILLEGAL_INSTRUCTION );
	EXCEPTION_CASE( EXCEPTION_BREAKPOINT );
	default:
		return "EXCEPTION";
	}
}

static const char* accessViolationTypeAsString( ULONG_PTR type )
{
	switch( type )
	{
		case 0:
			return "Read";
		case 1:
			return "Write";
		case 8:
			return "Data Execution";
		default:
			return "Unknown";
	}
}

//flags to control exiting without msg and without error on exception
//used by open automate:
//On exception we want an error code (and not a message)
bool exitWithErrorCodeOnException = false;
//On exit even with exception we want to return 0 (as the exception was probably 
//caused during exit due to singletons order)
int errorCodeForExitOnException = 1;

#if ENABLE_STACK_TRACKER

DWORD ExceptionFilter(DWORD exceptionCode, struct _EXCEPTION_POINTERS *ep)
{
	PEXCEPTION_RECORD er = ep->ExceptionRecord;

	char extraMsg[1024];
	extraMsg[0] = '\0';

	if ( exceptionCode == EXCEPTION_ACCESS_VIOLATION && er->NumberParameters >= 2 )
	{
		bw_snprintf( extraMsg, sizeof( extraMsg ), "%s @ 0x%08X",
			accessViolationTypeAsString( er->ExceptionInformation[0] ), er->ExceptionInformation[1] );
	}

	if (exitWithErrorCodeOnException) 
	{
		ERROR_MSG("Exception (%s : 0x%08X @ 0x%08X) received, exiting.",
			exceptionCodeAsString(exceptionCode), exceptionCode, er->ExceptionAddress);

		_exit(errorCodeForExitOnException );
	}

	// The critical message handler will append the stack trace for us.
	CRITICAL_MSG("The BigWorld Client has encountered an unhandled exception and must close (%s : 0x%08X @ 0x%08X) (%s)", 
		exceptionCodeAsString(exceptionCode), exceptionCode, er->ExceptionAddress, extraMsg);

	return EXCEPTION_CONTINUE_SEARCH;
}

#endif
#ifndef _XBOX360

/**
* This function exists with an error code when an exception happens
*/
LONG WINAPI ErrorCodeExceptionFilter(_EXCEPTION_POINTERS *exceptionInfo)
{
	ERROR_MSG("Exception (%s : 0x%08X) received, exiting.", exceptionCodeAsString(exceptionInfo->ExceptionRecord->ExceptionCode), exceptionInfo->ExceptionRecord->ExceptionCode);
	_exit(1);
}

/**
* This function exists with 0 code (success) when an exception happens
*/
LONG WINAPI SuccessExceptionFilter(_EXCEPTION_POINTERS *exceptionInfo)
{
	ERROR_MSG("Exception (%s : 0x%08X) received, exiting with success (0) error code.", exceptionCodeAsString(exceptionInfo->ExceptionRecord->ExceptionCode), exceptionInfo->ExceptionRecord->ExceptionCode);
	_exit(0);
}

#endif
#endif


// debug.cpp
