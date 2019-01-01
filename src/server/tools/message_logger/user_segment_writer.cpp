/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "user_segment_writer.hpp"

#include "log_string_writer.hpp"
#include "bwlog_writer.hpp"

#include "cstdmf/debug.hpp"

UserSegmentWriter::UserSegmentWriter( const std::string userLogPath,
	const char *suffix ) :
	UserSegment( userLogPath, suffix )
{ }


bool UserSegmentWriter::init()
{
	char buf[ 1024 ];
	static const char *mode = "a+";

	bw_snprintf( buf, sizeof( buf ), "%s/entries.%s",
		userLogPath_.c_str(), suffix_.c_str() );

	pEntries_ = new FileStream( buf, mode );
	if (!pEntries_->good())
	{
		ERROR_MSG( "UserSegmentWriter::init: "
			"Couldn't open entries file %s for writing: %s\n",
			buf, pEntries_->strerror() );
		isGood_ = false;
		return false;
	}

	// Generate args filename
	bw_snprintf( buf, sizeof( buf ), "%s/args.%s",
		userLogPath_.c_str(), suffix_.c_str() );
	pArgs_ = new FileStream( buf, mode );
	if (!pArgs_->good())
	{
		ERROR_MSG( "UserSegmentWriter::init: "
			"Couldn't open args file %s for writing: %s\n",
			buf, pArgs_->strerror() );
		isGood_ = false;
		return false;
	}

	this->updateEntryBounds();

	return true;
}


/**
 *  Add an entry to this segment.
 */
bool UserSegmentWriter::addEntry( LoggingComponent *pComponent,
	UserLogWriter *pUserLog, LogEntry &entry, LogStringInterpolator &handler,
	MemoryIStream &is, uint8 version )
{
	entry.argsOffset_ = pArgs_->length();

	LogStringWriter parser( *pArgs_ );
	if (!handler.streamToLog( parser, is, version ))
	{
		ERROR_MSG( "UserSegmentWriter::addEntry: "
			"Error whilst destreaming args from %s\n",
			pComponent->getString().c_str() );
		return false;
	}

	argsSize_ = pArgs_->length();
	entry.argsLen_ = argsSize_ - entry.argsOffset_;

	// If this is the component's first log entry, we need to write the
	// component to disk as well.
	if (!pComponent->written())
	{
		pComponent->updateFirstEntry( suffix_, numEntries_ );

 		pUserLog->updateComponent( pComponent );
				//components_.write( component );

		if (!pComponent->written())
		{
			ERROR_MSG( "UserSegment::addEntry: "
				"Failed to write %s to user components file.\n",
				pComponent->getString().c_str() );
		}
	}

	*pEntries_ << entry;
	pEntries_->commit();

	if (numEntries_ == 0)
	{
		start_ = entry.time_;
	}
	end_ = entry.time_;
	numEntries_++;

	return true;
}


/**
 * This method returns whether the current segment has been completely filled.
 *
 * @returns true if segment is full, false if not full.
 */
bool UserSegmentWriter::isFull( const BWLogWriter *pLogWriter ) const
{
	int currSegmentSize = int( numEntries_ * sizeof( LogEntry ) ) + argsSize_;

	return (currSegmentSize >= pLogWriter->getMaxSegmentSize());
}
