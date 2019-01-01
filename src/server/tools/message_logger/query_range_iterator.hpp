/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef QUERY_RANGE_ITERATOR_HPP
#define QUERY_RANGE_ITERATOR_HPP

#include "constants.hpp"

#include "log_entry_address.hpp"

class QueryRange;
class UserSegmentReader;

class QueryRangeIterator
{
public:
	QueryRangeIterator( const QueryRange *queryRange,
			int segmentNum = -1, int entryNum = -1, int metaOffset = 0 );

	static QueryRangeIterator error( QueryRange &queryRange )
	{
		return QueryRangeIterator( &queryRange );
	}

	bool isGood() const;

	const UserSegmentReader * getSegment() const;

	int getSegmentNumber() const;
	int getEntryNumber() const;
	int getArgsOffset() const;
	int getMetaOffset() const;

	LogEntryAddress getAddress() const;

	std::string asString() const;

	bool operator<(  const QueryRangeIterator &other ) const;
	bool operator<=( const QueryRangeIterator &other ) const;
	bool operator==( const QueryRangeIterator &other ) const;
	int operator-(   const QueryRangeIterator &other ) const;
	QueryRangeIterator& operator++();
	QueryRangeIterator& operator--();


private:
	void step( SearchDirection direction );

	// The parent QueryRange that we are iterating over
	const QueryRange *pQueryRange_;

	// Index into the UserLog list of UserSegments
	int segmentNum_;

	// TODO: is there any good reason for this to be a union?
	union
	{
		int entryNum_;
		int argsOffset_;
	};

	int metaOffset_;
};

#endif // QUERY_RANGE_ITERATOR_HPP
