/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef USER_SEGMENT_WRITER_HPP
#define USER_SEGMENT_WRITER_HPP

#include "user_segment.hpp"

#include "logging_component.hpp"
#include "log_entry.hpp"
#include "log_string_interpolator.hpp"
#include "user_log_writer.hpp"

#include <string>
#include <vector>

class BWLogWriter;

class UserSegmentWriter : public UserSegment
{
public:
	UserSegmentWriter( const std::string userLogPath, const char *suffix );
	//~UserSegmentWriter();

	bool init();

	bool addEntry( LoggingComponent *component, UserLogWriter *pUserLog,
		LogEntry &entry, LogStringInterpolator &handler, MemoryIStream &is,
		uint8 version );

	bool isFull( const BWLogWriter *pLogWriter ) const;
};

#endif // USER_SEGMENT_WRITER_HPP
