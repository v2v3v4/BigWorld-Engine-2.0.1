/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MESSAGE_LOGGER_CONSTANTS_HPP
#define MESSAGE_LOGGER_CONSTANTS_HPP

// This constant represents the format version of the log directory and
// contained files. It is independant of MESSAGE_LOGGER_VERSION as defined
// in 'src/lib/network/logger_message_forwarder.hpp' which represents the
// protocol version of messages sent between components and the logger.
// Version 6: * LogTime seconds changed from time_t to int64 to prevent
//              architecture differences.
static const int LOG_FORMAT_VERSION = 6;

static const double LOG_BEGIN = 0;
static const double LOG_END = -1;

static const int DEFAULT_SEGMENT_SIZE_MB = 100;

enum DisplayFlags
{
	SHOW_DATE = 1 << 0,
	SHOW_TIME = 1 << 1,
	SHOW_HOST = 1 << 2,
	SHOW_USER = 1 << 3,
	SHOW_PID = 1 << 4,
	SHOW_APPID = 1 << 5,
	SHOW_PROCS = 1 << 6,
	SHOW_SEVERITY = 1 << 7,
	SHOW_MESSAGE = 1 << 8,
	SHOW_ALL = 0x1FF
};


enum SearchDirection
{
	QUERY_FORWARDS = 1,
	QUERY_BACKWARDS = -1
};


enum InterpolateFlags
{
	DONT_INTERPOLATE = 0,
	POST_INTERPOLATE = 1,
	PRE_INTERPOLATE = 2
};

#endif // MESSAGE_LOGGER_CONSTANTS_HPP
