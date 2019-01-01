/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "query_range.hpp"

#include "user_segment_reader.hpp"
#include "log_entry.hpp"

/**
 *	Constructor.
 */
QueryRange::QueryRange( QueryParamsPtr pParams, UserLogReaderPtr pUserLog ) :
	pUserLog_( pUserLog ),
	startTime_(    pParams->getStartTime() ),
	endTime_(      pParams->getEndTime() ),
	startAddress_( pParams->getStartAddress() ),
	endAddress_(   pParams->getEndAddress() ),
	direction_(    pParams->getDirection() ),
	begin_( this ),
	curr_(  this ),
	end_(   this ),
	args_(  this )
{
	// Find the start point for the query
	begin_ = curr_ = this->findSentinel( direction_ );

	// Find the end point for the query. Even though this might seem expensive
	// because there's no guarantee our search will actually reach the end,
	// it's better to do this now as it makes checking for search termination
	// during each call to getNextEntry() cheap.
	end_ = this->findSentinel( -direction_ );
}


/**
 * Attempts to locate a valid log entry matching the query parameters within
 * the provided UserSegment.
 *
 * @returns Entry number within the UserSegment on success, -1 when not found.
 */
int QueryRange::findEntryInSegment( const UserSegmentReader *userSegment,
	int segmentIndexNum, SearchDirection direction )
{
	MF_ASSERT( userSegment != NULL );

//	UserSegment &segment = *userLog_.segments_[i];

	// This segment is a candidate if either the start or end falls within
	// its limits, or if it's spanned by the start and end
	bool starteq;
	if (startAddress_.isValid())
	{
		starteq = (startAddress_.getSuffix() == userSegment->getSuffix());
	}
	else
	{
		starteq = ((userSegment->getStartLogTime() <= startTime_) &&
					(startTime_ <= userSegment->getEndLogTime()));
	}

	bool endeq;
	if (endAddress_.isValid())
	{
		endeq = (endAddress_.getSuffix() == userSegment->getSuffix());
	}
	else
	{
		endeq = ((userSegment->getStartLogTime() <= endTime_) &&
					(endTime_ <= userSegment->getEndLogTime()));
	}

	bool startlt;
	if (startAddress_.isValid())
	{
		startlt = (startAddress_.getSuffix() < userSegment->getSuffix());
	}
	else
	{
		startlt = (startTime_ < userSegment->getStartLogTime());
	}

	bool endgt;
	if (endAddress_.isValid())
	{
		endgt = (userSegment->getSuffix() < endAddress_.getSuffix());
	}
	else
	{
		endgt = (userSegment->getEndLogTime() < endTime_);
	}

	int entryNum = -1;
	if (starteq || endeq || (startlt && endgt))
	{
		if (direction == QUERY_FORWARDS)
		{
			entryNum = const_cast< UserSegmentReader * >( userSegment )->findEntryNumber( startTime_, direction );
		}
		else
		{
			entryNum = const_cast< UserSegmentReader * >( userSegment )->findEntryNumber( endTime_, direction );
		}
	}

	return entryNum;
}


QueryRange::iterator QueryRange::findLeftSentinel()
{
	// Check for manually specified offsets first
	if (startAddress_.isValid())
	{
		const char *startSuffix = startAddress_.getSuffix();
		int segmentNum = pUserLog_->getSegmentIndexFromSuffix( startSuffix );

		return iterator( this, segmentNum, startAddress_.getIndex() );
	}


	int i = 0;
	int numSegments = pUserLog_->getNumSegments();
	int entryNum = -1;
	// Do a linear search through the list of segments and return an iterator
	// pointing into the right one
	while (i < numSegments)
	{
		const UserSegmentReader *pSegment = pUserLog_->getUserSegment( i );
		
		entryNum = this->findEntryInSegment( pSegment, i, QUERY_FORWARDS );
		if (entryNum != -1)
		{
			return iterator( this, i, entryNum );
		}

		++i;
	}

	return iterator::error( *this );
}


QueryRange::iterator QueryRange::findRightSentinel()
{
	// Check for manually specified offsets first
	if (endAddress_.isValid())
	{
		const char *endSuffix = endAddress_.getSuffix();
		int segmentNum = pUserLog_->getSegmentIndexFromSuffix( endSuffix );
		return iterator( this, segmentNum, endAddress_.getIndex() );
	}


	int i = pUserLog_->getNumSegments() - 1;
	int entryNum = -1;
	// Do a linear search through the list of segments and return an iterator
	// pointing into the right one
	while (i >= 0)
	{
		const UserSegmentReader *pSegment = pUserLog_->getUserSegment( i );
		
		entryNum = this->findEntryInSegment( pSegment, i, QUERY_BACKWARDS );
		if (entryNum != -1)
		{
			return iterator( this, i, entryNum );
		}

		--i;
	}

	return iterator::error( *this );
}


/**
 * Used to locate the first entry that this range should inspect, either coming
 * from the left (1) or right (-1) direction.
 */
QueryRange::iterator QueryRange::findSentinel( int direction )
{
	if (direction == QUERY_FORWARDS)
	{
		return this->findLeftSentinel();
	}
	else if (direction == QUERY_BACKWARDS)
	{
		return this->findRightSentinel();
	}

	ERROR_MSG( "QueryRange::findSentinel: Invalid direction %d\n", direction );

	return iterator::error( *this );
}


const UserSegmentReader * QueryRange::getUserSegmentFromIndex( int index ) const
{
	return pUserLog_->getUserSegment( index );
}


int QueryRange::getUserLogNumSegments() const
{
	return pUserLog_->getNumSegments();
}


bool QueryRange::getNextEntry( LogEntry &entry )
{
	if (!begin_.isGood() || !end_.isGood() ||
		!curr_.isGood()  || !(curr_ <= end_))
	{
		return false;
	}

	// Read off entry and set args iterator
	const UserSegmentReader *pSegment = curr_.getSegment();

	bool status = const_cast< UserSegmentReader * >( pSegment )->readEntry(
											curr_.getEntryNumber(), entry );
	if (status)
	{
		args_ = QueryRangeIterator( this, curr_.getSegmentNumber(),
									entry.argsOffset_ );

		this->step_forward();
	}

	return status;
}


/**
 *  Returns a FileStream positioned at the args blob corresponding to the most
 *  recent entry fetched by getNextEntry().
 */
BinaryIStream* QueryRange::getArgStream()
{
	const UserSegmentReader *pSegment = args_.getSegment();

	FileStream *pFileStream =
		const_cast< UserSegmentReader *>( pSegment )->getArgStream();

	if (pFileStream)
	{
		pFileStream->seek( args_.getArgsOffset() );
		if (!pFileStream->good())
		{
			pFileStream = NULL;
		}
	}

	return pFileStream;
}


void QueryRange::updateArgs( int segmentNum, int argsOffset )
{
	args_ = QueryRangeIterator( this, segmentNum, argsOffset );
}


QueryRangeIterator QueryRange::iter_curr() const
{
	return curr_;
}


QueryRangeIterator QueryRange::iter_end() const
{
	return end_;
}


bool QueryRange::areBoundsGood() const
{
	return (begin_.isGood() && end_.isGood());
}


int QueryRange::getNumEntriesVisited() const
{
	return (curr_ - begin_);
}


int QueryRange::getTotalEntries() const
{
	return (end_ - begin_) + 1;
}


void QueryRange::step_forward()
{
	++curr_;
}


void QueryRange::step_back()
{
	--curr_;
}


void QueryRange::resume()
{
	end_ = this->findSentinel( -direction_ );

	if (!begin_.isGood())
	{
		begin_ = curr_ = this->findSentinel( direction_ );
	}

	// If we had exhausted the previous range, then we need to step off the
	// last record
	if (curr_.getMetaOffset() == direction_)
	{
		this->step_forward();
	}
}


bool QueryRange::seek( int segmentNum, int entryNum,
	int metaOffset, int postIncrement )
{
	bool status = false;
	iterator query( this, segmentNum, entryNum );

	if ((begin_ <= query) && (query <= end_))
	{
		curr_ = iterator( this, segmentNum, entryNum, metaOffset );

		for (int i=0; i < postIncrement; i++)
		{
			this->step_forward();
		}

		status = true;
	}

	return status;
}


std::string QueryRange::asString() const
{
	char buf[ 256 ];

	bw_snprintf( buf, sizeof( buf ), "(%s) -> (%s)",
		begin_.asString().c_str(), end_.asString().c_str() );

	return std::string( buf );
}
