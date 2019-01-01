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
#include "string_utils.hpp"

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif

#include <stdio.h>
#include <time.h>

#include "dprintf.hpp"

/*static*/ DebugFilter* DebugFilter::s_instance_ = NULL;


#if ENABLE_DPRINTF

	// Commented in header file.
	void dprintf( const char * format, ... )
	{
		va_list argPtr;
		va_start( argPtr, format );

		vdprintf( format, argPtr );

		va_end(argPtr);
	}


	/**
	*	This function prints a debug message if the input priorities satisfies the
	*	filter.
	*/
	void dprintf( int componentLevel, int severity, const char * format, ... )
	{
		va_list argPtr;
		va_start( argPtr, format );

		vdprintf( componentLevel, severity, format, argPtr );

		va_end( argPtr );
	}

#endif // ENABLE_DPRINTF

bool DebugFilter::s_shouldWriteTimePrefix_ = false;
#if defined( MF_SERVER ) || defined( PLAYSTATION3 )
bool DebugFilter::s_shouldWriteToConsole_ = true;
#else
bool DebugFilter::s_shouldWriteToConsole_ = false;
#endif


#if ENABLE_DPRINTF

	// Commented in header file.
	void vdprintf( const char * format, va_list argPtr, const char * prefix )
	{
	#ifdef _WIN32
			char buf[ 2048 ];
			char * pBuf = buf;
			int len = 0;
			if (prefix)
			{
				len = strlen( prefix );
				memcpy( pBuf, prefix, len );
				pBuf += len;
				if (DebugFilter::shouldWriteToConsole())
				{
					fprintf( stderr, "%s", prefix );
				}
			}

			_vsnprintf( pBuf, sizeof(buf) - len, format, argPtr );
			buf[sizeof(buf)-1]=0;

			std::wstring wbuf;
			if ( !bw_acptow(buf, wbuf) )
			{
				wbuf = L"[Error converting message string to wide]\n";
			}

 			OutputDebugString( wbuf.c_str() );
			if (DebugFilter::shouldWriteToConsole())
			{
				vfprintf( stderr, format, argPtr );
			}
	#else
			if (DebugFilter::shouldWriteToConsole())
			{
				if (DebugFilter::shouldWriteTimePrefix())
				{
					time_t ttime = time( NULL );
					char timebuff[32];

					if (0 != strftime( timebuff, sizeof(timebuff),
						"%F %T: ", localtime( &ttime ) ))
					{
						fprintf( stderr, timebuff );
					}
				}

				if (prefix)
				{
					fprintf( stderr, "%s", prefix );
				}

				// Need to make a copy of the va_list here to avoid crashing on 64bit
				va_list tmpArgPtr;
				bw_va_copy( tmpArgPtr, argPtr );
				vfprintf( stderr, format, tmpArgPtr );
				va_end( tmpArgPtr );
			}
	#endif
	}


	/**
	*	This function prints a debug message if the input priorities satisfies the
	*	filter.
	*/
	void vdprintf( int componentPriority, int messagePriority,
				const char * format, va_list argPtr,
				const char * prefix )
	{
		vdprintf( format, argPtr, prefix );
	}

#endif // ENABLE_DPRINTF

void DebugFilter::addCriticalCallback( CriticalMessageCallback * pCallback )
{
	pCriticalCallbacks_.push_back(pCallback);
}

void DebugFilter::deleteCriticalCallback( CriticalMessageCallback *pCallback )
{
	CriticalCallbacks::iterator it = pCriticalCallbacks_.begin();
	for ( ; it != pCriticalCallbacks_.end(); ++it )
	{
		if ( (*it) == pCallback )
		{
			pCriticalCallbacks_.erase( it );
			return;
		}
	}
}

void DebugFilter::addMessageCallback( DebugMessageCallback * pCallback )
{
	pMessageCallbacks_.push_back(pCallback);
}

void DebugFilter::deleteMessageCallback( DebugMessageCallback * pCallback )
{
	DebugCallbacks::iterator it = pMessageCallbacks_.begin();
	for ( ; it != pMessageCallbacks_.end(); ++it )
	{
		if ( (*it) == pCallback )
		{
			pMessageCallbacks_.erase( it );
			if (pMessageCallbacks_.size() == 0) { fini(); }
			return;
		}
	}
}

// dprintf.cpp
