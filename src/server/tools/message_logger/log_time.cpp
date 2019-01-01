/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "log_time.hpp"

#include "cstdmf/debug.hpp"

#include <limits.h>

/**
 * Default Constructor.
 */
LogTime::LogTime ()
{ }


/**
 * Constructor.
 *
 * Pass LOG_END here to generate the positive infinity time
 */
LogTime::LogTime( double ftime )
{
	if (ftime == LOG_END)
	{
		secs_ = LONG_MAX;
		msecs_ = 0;
	}
	else
	{
		secs_ = (Seconds)ftime;
		msecs_ = (Milliseconds)((ftime - secs_) * 1000 + 0.5);
	}
}


bool LogTime::operator>( const LogTime &other ) const
{
	return secs_ > other.secs_ ||
		(secs_ == other.secs_ && msecs_ > other.msecs_);
}


bool LogTime::operator>=( const LogTime &other ) const
{
	return (secs_ == other.secs_ && msecs_ == other.msecs_) ||
		*this > other;
}


bool LogTime::operator<( const LogTime &other ) const
{
	return secs_ < other.secs_ ||
		(secs_ == other.secs_ && msecs_ < other.msecs_);
}


bool LogTime::operator<=( const LogTime &other ) const
{
	return (secs_ == other.secs_ && msecs_ == other.msecs_) ||
		*this < other;
}


double LogTime::asDouble() const
{
	return secs_ + msecs_/1000.0;
}

// log_time.cpp
