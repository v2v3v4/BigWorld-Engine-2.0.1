/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef USER_LOG_READER_HPP
#define USER_LOG_READER_HPP

#include "cstdmf/smartpointer.hpp"

#include <map>

class UserLogReader;
typedef SmartPointer< UserLogReader > UserLogReaderPtr;
typedef std::map< uint16, UserLogReaderPtr > UserLogs;

#include "user_log.hpp"
#include "query_range.hpp"
#include "user_segment_reader.hpp"

#include <vector>


class UserSegmentVisitor
{
public:
	virtual bool onSegment( const UserSegmentReader * pSegment ) = 0;
};


class UserLogReader : public UserLog
{
public:
	UserLogReader( uint16 uid, const std::string &username );

	bool init( const std::string rootPath );

	// Log retrieval
	bool getEntryAndSegment( const LogEntryAddress &addr, LogEntry &result,
		UserSegmentReader * &pSegment, bool warn = true );
	bool getEntryAndQueryRange( const LogEntryAddress &addr, LogEntry &result,
		QueryRangePtr pRange );

	bool getEntry( double time, LogEntry &result );
	bool getEntry( const LogEntryAddress &addr, LogEntry &result );

	// This method is public to allow QueryRange to be able to
	// retrieve segments.
	// Candidate for cleanup. A longer term improvement would separate the
	// knowledge in QueryRange of how UserLog actually stores the UserSegments.
	int getSegmentIndexFromSuffix( const char *suffix ) const;
	int getNumSegments() const;
	const UserSegmentReader *getUserSegment( int segmentIndex ) const;

	const LoggingComponent *getComponentByID( int componentID );

	bool reloadFiles();

	bool getUserComponents( UserComponentVisitor &visitor ) const;

	bool visitAllSegmentsWith( UserSegmentVisitor &visitor ) const;

private:
	// UserLogReaders should always be passed around as SmartPointers
	//~UserLogReader();

	bool loadSegments();
};

#endif // USER_LOG_READER_HPP
