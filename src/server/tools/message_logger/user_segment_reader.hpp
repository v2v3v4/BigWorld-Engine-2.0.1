/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef USER_SEGMENT_READER_HPP
#define USER_SEGMENT_READER_HPP

#include "user_segment.hpp"

#include <string>
#include <vector>

class PyQueryResult;
class LogStringInterpolator;

class UserSegmentReader : public UserSegment
{
public:
	UserSegmentReader( const std::string userLogPath, const char *suffix );

	bool init();

	static int filter( const struct dirent *ent );

	bool isDirty() const;

	int findEntryNumber( LogTime &time, SearchDirection direction );

	bool seek( int n );

	const LogTime & getStartLogTime() const;
	const LogTime & getEndLogTime() const;


	bool interpolateMessage( const LogEntry &entry,
			const LogStringInterpolator *pHandler, std::string &result );

	// Candidate for cleanup. QueryRange currently requires this.
	FileStream * getArgStream();

	int getEntriesLength() const;
	int getArgsLength() const;
};

#endif // USER_SEGMENT_READER_HPP
