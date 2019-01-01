/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef QUERY_RANGE_HPP
#define QUERY_RANGE_HPP

#include "cstdmf/smartpointer.hpp"

class QueryRange;
typedef SmartPointer< QueryRange > QueryRangePtr;

#include "log_entry_address_reader.hpp"
#include "log_time.hpp"
#include "query_params.hpp"
#include "query_range_iterator.hpp"
#include "user_log_reader.hpp"
#include "log_entry.hpp"

/**
 * An iterator over a specified range of a user's log.
 */
struct QueryRange : public SafeReferenceCount
{
public:
	typedef QueryRangeIterator iterator;

	//QueryRange( UserLog &userLog, QueryParams &params );
	QueryRange( QueryParamsPtr pParams, UserLogReaderPtr pUserLog );

	const SearchDirection & getDirection() const { return direction_; }

	bool getNextEntry( LogEntry &entry );

	BinaryIStream* getArgStream();

	void resume();
	bool seek( int segmentNum, int entryNum, int metaOffset,
				int postIncrement = 0 );

	// This is exposed to be used by the QueryRangeIterator.
	const UserSegmentReader *getUserSegmentFromIndex( int index ) const;
	int getUserLogNumSegments() const;

	void updateArgs( int segmentNum, int argsOffset );

	iterator iter_curr() const;
	iterator iter_end() const;

	bool areBoundsGood() const;

	int getNumEntriesVisited() const;
	int getTotalEntries() const;

	// Move the curr_ iterator
	void step_back();
	void step_forward();

	std::string asString() const;

private:
	iterator findSentinel( int direction );
	iterator findLeftSentinel();
	iterator findRightSentinel();
	int findEntryInSegment( const UserSegmentReader *userSegment,
			int segmentIndexNum, SearchDirection direction );

	UserLogReaderPtr pUserLog_;

	LogTime startTime_;
	LogTime endTime_;

	LogEntryAddressReader startAddress_;
	LogEntryAddressReader endAddress_;

	SearchDirection direction_;

	iterator begin_;
	iterator curr_;
	iterator end_;
	iterator args_;
};

#endif // QUERY_RANGE_HPP
