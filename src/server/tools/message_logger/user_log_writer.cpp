/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "user_log_writer.hpp"

#include "mlutil.hpp"
#include "user_segment_writer.hpp"
#include "log_string_interpolator.hpp"
#include "bwlog_writer.hpp"
#include "query_result.hpp"


/**
 * Constructor.
 */
UserLogWriter::UserLogWriter( uint16 uid, const std::string &username ) :
 UserLog( uid, username )
{ }


bool UserLogWriter::init( const std::string rootPath )
{
	if (!UserLog::init( rootPath ))
	{
		return false;
	}

	static const char *mode = "a+";

	if (!MLUtil::softMkDir( path_.c_str() ))
	{
		ERROR_MSG( "UserLogWriter::init: User log directory is not "
			"accessible in write mode: %s\n", path_.c_str() );
		return false;
	}

	if (!uidFile_.init( uidFile_.join( path_.c_str(), "uid" ), mode, uid_ ))
	{
		ERROR_MSG( "UserLogWriter::UserLogWriter: "
			"Failed to init uid file in %s\n", path_.c_str() );
		return false;
	}

	if (!userComponents_.init( path_.c_str(), mode ))
	{
		ERROR_MSG( "UserLogWriter::UserLogWriter: "
			"Failed to read components mapping from %s\n",
			userComponents_.filename() );
		return false;
	}

	isGood_ = true;
	return isGood_;
}


LoggingComponent * UserLogWriter::findLoggingComponent(
	const Mercury::Address &addr )
{
	return userComponents_.getComponentByAddr( addr );
}


LoggingComponent * UserLogWriter::findLoggingComponent(
	const LoggerComponentMessage &msg, const Mercury::Address &addr,
	LogComponentNames &logComponents )
{
	return userComponents_.getComponentFromMessage( msg, addr, logComponents );
}


/**
 * Removes the current active segment which will ensure a new segment file
 * is created when the next log message comes in.
 */
void UserLogWriter::rollActiveSegment()
{
	if (!this->hasActiveSegments())
	{
		return;
	}

	UserSegment *pUserSegment = userSegments_.back();
	delete pUserSegment;

	// Why is this being done? This was copied from BWLog::roll, but
	// seems strange to do..
	userSegments_.pop_back();
}


/**
 * Adds a LogEntry to the end of the UserSegment file.
 */
bool UserLogWriter::addEntry( LoggingComponent *component, LogEntry &entry,
		LogStringInterpolator &handler, MemoryIStream &is,
		BWLogWriter *pLogWriter, uint8 version )
{
	// Make sure segment is ready to be written to
	if (userSegments_.empty() || this->getLastSegment()->isFull( pLogWriter ))
	{
		// If userSegments_ is empty, there's a potential race condition here.
		// For the time between the call to 'new Segment()' and the call to
		// 'activeFiles_.write()', the newly created segment could be
		// accidentally rolled by mltar.  To avoid this, we blow away
		// active_files so that mltar knows not to roll at the moment.  That's
		// OK because activeFiles_.write() regenerates the whole file anyway.
		if (userSegments_.empty())
		{
			pLogWriter->deleteActiveFiles();
		}

		UserSegmentWriter *pSegment = new UserSegmentWriter( path_, NULL );
		pSegment->init();
		if (pSegment->isGood())
		{
			// Drop full segments as we don't need em around anymore and they're
			// just eating up memory and file handles
			while (!userSegments_.empty())
			{
				delete userSegments_.back();
				userSegments_.pop_back();
			}

			userSegments_.push_back( pSegment );

			// Update active_files
			if (!pLogWriter->updateActiveFiles())
			{
				ERROR_MSG( "UserLog::addEntry: "
					"Unable to update active_files\n" );
				return false;
			}
		}
		else
		{
			ERROR_MSG( "UserLog::addEntry: "
				"Couldn't create new segment %s; dropping msg with fmt '%s'\n",
				pSegment->getSuffix().c_str(), handler.fmt().c_str() );
			delete pSegment;
			return false;
		}
	}

	MF_ASSERT( userSegments_.size() == 1 );

	UserSegmentWriter *pSegment = this->getLastSegment();
	return pSegment->addEntry( component, this, entry, handler, is, version );
}


UserSegmentWriter * UserLogWriter::getLastSegment()
{
	return static_cast< UserSegmentWriter * >( userSegments_.back() );
}


bool UserLogWriter::updateComponent( LoggingComponent *component )
{
	MF_ASSERT( component != NULL );
	return userComponents_.write( component );
}


bool UserLogWriter::removeUserComponent( const Mercury::Address &addr )
{
	return userComponents_.erase( addr );
}


const char * UserLogWriter::logEntryToString( const LogEntry &entry,
		BWLogCommon *pBWLog, const LoggingComponent *component, 
		LogStringInterpolator &handler, MemoryIStream &is, uint8 version ) const
{
	std::string msg;
	MemoryIStream args( is.data(), is.remainingLength() );

	handler.streamToString( args, msg, version );
	QueryResult result( entry, pBWLog, this, component, msg );

	// This dodgy return is OK because the pointer is to a static buffer
	const char *text = result.format();
	return text;
}


// user_log_writer.cpp
