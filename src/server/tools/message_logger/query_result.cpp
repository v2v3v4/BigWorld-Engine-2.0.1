/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "query_result.hpp"

#include "log_entry.hpp"
#include "logging_component.hpp"
#include "user_log.hpp"

char QueryResult::s_linebuf_[ 2048 ];


QueryResult::QueryResult() :
	host_( NULL )
{ }


QueryResult::QueryResult( const LogEntry &entry, BWLogCommon *pBWLog,
	const UserLog *pUserLog, const LoggingComponent *pComponent,
	const std::string &message ) :

	time_( entry.time_.asDouble() ),
	host_( pBWLog->getHostByAddr( pComponent->getAddress().ip ) ),
	pid_(   pComponent->msg_.pid_ ),
	appid_( pComponent->appid_ ),
	username_( pUserLog->getUsername().c_str() ),
	component_( pBWLog->getComponentByID( pComponent->typeid_ ) ),
	severity_( entry.messagePriority_ ),
	message_( message ),
	stringOffset_( entry.stringOffset_ )
{ }


/*
 * The following functions used to be macros in the body of the following
 * methods but are implemented as functions to avoid macro ugliness.
 */
static int buf_remain( const char *cursor, const char *end )
{
	return end - cursor - 1;
}


static bool truncate( char *end, int *&pLen )
{
	const char truncmsg[] = "<== message truncated!\n";
	strcpy( end - strlen( truncmsg ) - 1, truncmsg );
	if (pLen != NULL) *pLen = sizeof( QueryResult::s_linebuf_ ) - 1;
	return false;
}


static bool seek_cursor( char *&cursor, char *end, bool &previous, int *&pLen,
	int n=0 )
{
	if (n)
	{
		if (cursor + n >= end)
			return truncate( end, pLen );
		cursor += n;
	}
	else
	{
		while (cursor < end && *cursor) { cursor++; }
		if (cursor == end)
			return truncate( end, pLen );
	}
	previous = true;
	return true;
}


static void pad_prior( char *&cursor, char *end, bool &previous, int *&pLen )
{
	if (previous)
	{
		*cursor = ' ';
		seek_cursor( cursor, end, previous, pLen, 1 );
	}
}


/**
 * Format this log result according to the supplied display flags (see
 * DisplayFlags in constants.hpp).
 *
 * Uses a static buffer, so you must copy the result of this call away if you
 * want to preserve it. Writes the length of the formatted string to pLen if
 * it is non-NULL.
 */
const char *QueryResult::format( unsigned flags, int *pLen )
{
	char *cursor = s_linebuf_;
	char *end = s_linebuf_ + sizeof( s_linebuf_ );
	bool previous = false;

	// If this is a pad line, just chuck in -- like grep does
	if (host_ == NULL)
	{
		bw_snprintf( s_linebuf_, sizeof( s_linebuf_ ), "--\n" );
		if (pLen)
		{
			*pLen = 3;
		}
		return s_linebuf_;
	}

	if (flags & (SHOW_DATE | SHOW_TIME))
	{
		struct tm breakdown;
		LogTime time( time_ );
		// This assignment allows compilation on 32 bit systems.
		const time_t seconds = time.secs_;
		localtime_r( &seconds, &breakdown );
		if (flags & SHOW_DATE)
		{
			strftime( cursor, buf_remain( cursor, end ), "%a %d %b %Y ",
				&breakdown );
			seek_cursor( cursor, end, previous, pLen );
		}
		if (flags & SHOW_TIME)
		{
			strftime( cursor, buf_remain( cursor, end ), "%T", &breakdown );
			if (!seek_cursor( cursor, end, previous, pLen ))
				return s_linebuf_;
			snprintf( cursor, buf_remain( cursor, end ), ".%03d ",
				time.msecs_ );
			if (!seek_cursor( cursor, end, previous, pLen, 5 ))
				return s_linebuf_;
		}
	}

	if (flags & SHOW_HOST)
	{
		pad_prior( cursor, end, previous, pLen );
		snprintf( cursor, buf_remain( cursor, end ), "%-15s", host_ );
		if (!seek_cursor( cursor, end, previous, pLen, 15 ))
			return s_linebuf_;
	}

	if (flags & SHOW_USER)
	{
		pad_prior( cursor, end, previous, pLen );
		snprintf( cursor, buf_remain( cursor, end ), "%-10s", username_ );
		if (!seek_cursor( cursor, end, previous, pLen, 10 ))
			return s_linebuf_;
	}

	if (flags & SHOW_PID)
	{
		pad_prior( cursor, end, previous, pLen );
		snprintf( cursor, buf_remain( cursor, end ), "%-5d", pid_ );
		if (!seek_cursor( cursor, end, previous, pLen, 5 ))
			return s_linebuf_;
	}

	if (flags & SHOW_PROCS)
	{
		pad_prior( cursor, end, previous, pLen );
		snprintf( cursor, buf_remain( cursor, end ), "%-10s", component_ );
		if (!seek_cursor( cursor, end, previous, pLen, 10 ))
			return s_linebuf_;
	}

	if (flags & SHOW_APPID)
	{
		pad_prior( cursor, end, previous, pLen );

		if (appid_)
			snprintf( cursor, buf_remain( cursor, end ), "%-3d", appid_ );
		else
			snprintf( cursor, buf_remain( cursor, end ), "   " );

		if (!seek_cursor( cursor, end, previous, pLen, 3 ))
			return s_linebuf_;
	}

	if (flags & SHOW_SEVERITY)
	{
		pad_prior( cursor, end, previous, pLen );
		snprintf( cursor, buf_remain( cursor, end ), "%-8s",
			messagePrefix( (DebugMessagePriority)severity_ ) );
		if (!seek_cursor( cursor, end, previous, pLen, 8 ))
			return s_linebuf_;
	}

	if (flags & SHOW_MESSAGE)
	{
		pad_prior( cursor, end, previous, pLen );
		int msgcol = (int)(cursor - s_linebuf_);
		const char *c = message_.c_str();

		while (*c)
		{
			*cursor = *c++;
			if (!seek_cursor( cursor, end, previous, pLen, 1 ))
				return s_linebuf_;

			// If the character just read was a newline and we're not done, fill
			// out the left hand side so the message columns line up
			if (cursor[-1] == '\n' && *c)
			{
				char *padpoint = cursor;
				if (!seek_cursor( cursor, end, previous, pLen, msgcol ))
					return s_linebuf_;
				memset( padpoint, ' ', msgcol );
			}
		}
	}

	if (cursor[-1] != '\n')
	{
		*cursor = '\n';
		if (!seek_cursor( cursor, end, previous, pLen, 1 ))
			return s_linebuf_;
		*cursor = '\0';
	}
	else
		*cursor = '\0';

	if (pLen != NULL)
		*pLen = (int)(cursor - s_linebuf_);

	return s_linebuf_;
}


const std::string & QueryResult::getMessage() const
{
	return message_;
}


uint32 QueryResult::getStringOffset() const
{
	return stringOffset_;
}


double QueryResult::getTime() const
{
	return time_;
}


const char * QueryResult::getHost() const
{
	return host_;
}


int QueryResult::getPID() const
{
	return pid_;
}


int QueryResult::getAppID() const
{
	return appid_;
}


const char * QueryResult::getUsername() const
{
	return username_;
}


const char * QueryResult::getComponent() const
{
	return component_;
}


int QueryResult::getSeverity() const
{
	return severity_;
}
