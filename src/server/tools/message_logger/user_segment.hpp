/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef USER_SEGMENT_HPP
#define USER_SEGMENT_HPP

#include <vector>

// This is defined prior to 'user_log.hpp' inclusion, so that UserLog will
// know what UserSegments are.
class UserSegment;
typedef std::vector< UserSegment * > UserSegments;

#include "user_log.hpp"
#include "log_time.hpp"

#include "network/file_stream.hpp"

#include <stdlib.h>
#include <dirent.h>

#include <string>

class LogEntry;
class LoggingComponent;
class UserLog;
class LoggingStringHandler;
class MemoryIStream;

/**
 * A segment of a user's log.  This really means a pair of entries and args
 * files.  NOTE: At the moment, each Segment always has two FileStreams
 * open, which means that two Queries can't be executed at the same time in
 * the same process, and also that if a log has many segments, then the
 * number of open file handles for this process will be excessive.
 */
class UserSegment
{
public:
	UserSegment( const std::string userLogPath, const char *suffix );
	virtual ~UserSegment();

	const std::string & getSuffix() const { return suffix_; }

	bool isGood() const { return isGood_; }

	bool readEntry( int n, LogEntry &entry );

	void updateEntryBounds();

	int getNumEntries() const { return numEntries_; }

	

protected:
	bool buildSuffixFrom( struct tm & pTime, std::string & newSuffix ) const;

	std::string suffix_;

	// File streams representing the currently open args.* / entries.* files.
	FileStream *pEntries_;
	FileStream *pArgs_;

	int numEntries_;
	int argsSize_;
	LogTime start_, end_;

	bool isGood_;

	std::string userLogPath_;

private:
	friend class UserSegmentComparator;
};


/**
 * This class is a helper to allow std::sort operations on UserSegments.
 */
class UserSegmentComparator
{
public:
	bool operator() ( const UserSegment *a, const UserSegment *b )
	{
		return a->start_ < b->start_;
	}
};

#endif // USER_SEGMENT_HPP
