/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOG_TIME_HPP
#define LOG_TIME_HPP

#include "constants.hpp"

#include "cstdmf/stdmf.hpp"

#include <time.h>

#pragma pack( push, 1 )
class LogTime
{
public:
	typedef int64 Seconds;
	typedef uint16 Milliseconds;

	LogTime();
	LogTime( double ftime );

	bool operator>( const LogTime &other ) const;
	bool operator>=( const LogTime &other ) const;
	bool operator<( const LogTime &other ) const;
	bool operator<=( const LogTime &other ) const;

	double asDouble() const;

	Seconds secs_;
	Milliseconds msecs_;
};
#pragma pack( pop )

#endif // LOG_TIME_HPP
