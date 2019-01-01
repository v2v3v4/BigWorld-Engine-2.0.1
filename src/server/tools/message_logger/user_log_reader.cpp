/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "user_log_reader.hpp"

#include "user_segment_reader.hpp"
#include "mlutil.hpp"

UserLogReader::UserLogReader( uint16 uid, const std::string &username ) :
	UserLog( uid, username )
{ }


bool UserLogReader::init( const std::string rootPath )
{
	if (!UserLog::init( rootPath ))
	{
		return false;
	}

	if (!MLUtil::isPathAccessible( path_.c_str() ))
	{
		ERROR_MSG( "UserLogReader::init: User log directory is not "
			"accessible in read mode: %s\n", path_.c_str() );
		return false;
	}

	static const char *mode = "r";
	const char *uidFilePath = uidFile_.join( path_.c_str(), "uid" );
	if (!uidFile_.init( uidFilePath, mode, uid_ ))
	{
		ERROR_MSG( "UserLogReader::init: "
			"Failed to initialise uid file in %s\n", path_.c_str() );
		return false;
	}

	if (!userComponents_.init( path_.c_str(), mode ))
	{
		ERROR_MSG( "UserLogReader::init: "
			"Failed to read components mapping from %s\n",
			userComponents_.filename() );
		return false;
	}

	if (!this->loadSegments())
	{
		ERROR_MSG( "UserLogReader::init: Failed to load segments\n" );
		return false;
	}


	isGood_ = true;
	return isGood_;
}


/**
 * This is a helper function to assist in releasing the directory entries
 * read in from UserLogReader::loadSegments().
 */
static void free_dirents( struct dirent **list, int numentries )
{
	for (int i=0; i < numentries; i++)
	{
		free( list[i] );
	}
	free( list );
}


bool UserLogReader::loadSegments()
{
	struct dirent **namelist = NULL;
	int numEntries = scandir( path_.c_str(), &namelist,
				UserSegmentReader::filter, NULL );

	if (numEntries == -1)
	{
		ERROR_MSG( "UserLogReader::loadSegments: Failed to scan user log "
			"directory to load existing 'entries' segments.\n");
		return false;
	}

	for (int i=0; i < numEntries; i++)
	{
		const char *filename = namelist[i]->d_name;

		// Everything in the filename after the '.' should be the time.
		// Both the entries file and args file are assumed to have the
		// same suffix.
		const char *suffix = strchr( filename, '.' );
		if (suffix == NULL)
		{
			ERROR_MSG( "UserLogReader::loadSegments: "
				"Entries file found with bad filename: '%s'\n", filename );
			free_dirents( namelist, numEntries );
			return false;
		}
		else
		{
			suffix++;
		}

		int existingIndex = this->getSegmentIndexFromSuffix( suffix );
		if (existingIndex != -1)
		{
			UserSegmentReader *pSegment = static_cast< UserSegmentReader *>(
											userSegments_[ existingIndex ] );
			if (pSegment->isDirty())
			{
				pSegment->updateEntryBounds();
			}
		}
		else
		{
			UserSegmentReader *pSegment = new UserSegmentReader( path_, suffix );
			pSegment->init();

			if (pSegment->isGood())
			{
				userSegments_.push_back( pSegment );
			}
			else
			{
				ERROR_MSG( "UserLogReader::loadSegments: "
					"Dropping segment %s due to load error.\n",
					pSegment->getSuffix().c_str() );
				delete pSegment;
			}
		}
	}

	// We order the segments by sorting on their start times instead of doing an
	// alphasort in scandir() above, because the filenames are generated from
	// localtime() and may not be strictly in the right order around daylight
	// savings or other similar time changes.
	UserSegmentComparator USCompare;
	std::sort( userSegments_.begin(), userSegments_.end(), USCompare );

	free_dirents( namelist, numEntries );
	return true;
}


/**
 * Returns the index of an already loaded segment with the given suffix.
 *
 * @returns Index of loaded segment, -1 if no segment with provided suffix is
 *          loaded.
 */
int UserLogReader::getSegmentIndexFromSuffix( const char *suffix ) const
{
	for (unsigned i=0; i < userSegments_.size(); i++)
	{
		if (userSegments_[i]->getSuffix() == suffix)
		{
			return i;
		}
	}

	return -1;
}


int UserLogReader::getNumSegments() const
{
	return userSegments_.size();
}


/**
 *
 */
const UserSegmentReader* UserLogReader::getUserSegment( int segmentIndex ) const
{
	return static_cast< UserSegmentReader * >( userSegments_[ segmentIndex ] );
}


/**
 * Extract the LogEntry corresponding to the provided LogEntryAddress.
 *
 * The provided QueryRange is also updated to point to the location that the
 * LogEntry came from.
 *
 * @returns true on success, false on error.
 */
bool UserLogReader::getEntryAndQueryRange( const LogEntryAddress &addr,
	LogEntry &result, QueryRangePtr pRange )
{
	const char *logEntrySuffix = addr.getSuffix();

	// get the segment num
	int segmentNum = this->getSegmentIndexFromSuffix( logEntrySuffix );
	if (segmentNum == -1)
	{
		ERROR_MSG( "UserLogReader::getEntry: "
			"There is no segment with suffix '%s' in %s's log\n",
			logEntrySuffix, username_.c_str() );

		return false;
	}

	// get the segment and read off the entry
	UserSegmentReader *pSegment =
		static_cast< UserSegmentReader * >( userSegments_[ segmentNum ] );

	if (!pSegment->readEntry( addr.getIndex(), result ))
	{
		ERROR_MSG( "UserLogReader::getEntry: "
			"Couldn't read entry %d from log segment %s\n",
			addr.getIndex(), logEntrySuffix );
		return false;
	}

	// If a range was provided, set its args iterator to the right place
	if (pRange != NULL)
	{
		pRange->updateArgs( segmentNum, result.argsOffset_ );
	}

	return true;
}


/**
 * Extract the LogEntry corresponding to the provided LogEntryAddress.
 *
 * @returns true on success, false on error.
 */
bool UserLogReader::getEntry( const LogEntryAddress &addr, LogEntry &result )
{
	UserSegmentReader *pSegment; // used as a throw away
	return this->getEntryAndSegment( addr, result, pSegment, true );
}


/**
 * Extract the LogEntry corresponding to the provided LogEntryAddress.
 *
 * The UserSegment the LogEntry exists in is stored in pSegmentResult.
 *
 * @returns true on success, false on error.
 */
bool UserLogReader::getEntryAndSegment( const LogEntryAddress &addr,
	LogEntry &result, UserSegmentReader * &pSegmentResult, bool warn )
{
	const char *logEntrySuffix = addr.getSuffix();
	pSegmentResult = NULL;

	int segmentNum = this->getSegmentIndexFromSuffix( logEntrySuffix );
	if (segmentNum == -1)
	{
		if (warn)
		{
			ERROR_MSG( "UserLogReader::getEntry: "
				"There is no segment with suffix '%s' in %s's log\n",
				logEntrySuffix, username_.c_str() );
		}

		return false;
	}

	// Read off the entry
	UserSegmentReader *pSegment =
		static_cast< UserSegmentReader *>( userSegments_[ segmentNum ] );

	if (!pSegment->readEntry( addr.getIndex(), result ))
	{
		ERROR_MSG( "UserLogReader::getEntry: "
			"Couldn't read entry %d from log segment %s\n",
			addr.getIndex(), logEntrySuffix );
		return false;
	}
	pSegmentResult = pSegment;

	return true;
}



/**
 * This method locates the LogEntry at the specified time extremity.
 *
 * Either LOG_BEGIN or LOG_END must be passed as the first argument to this
 * method.
 *
 * @returns true on success, false on error.
 */
bool UserLogReader::getEntry( double time, LogEntry &result )
{
	if (time != LOG_BEGIN && time != LOG_END)
	{
		ERROR_MSG( "UserLogReader::getEntry: "
			"Invalid time passed as first argument: %f\n", time );
		return false;
	}

	// If we encounter the extremely unlikely case where a user's log directory
	// has been created but the first log segment hasn't been written yet, abort
	// here.
	if (!this->hasActiveSegments())
	{
		ERROR_MSG( "UserLog::getEntry: "
			"User's log is currently empty, can't proceed with getEntry()\n" );
		return false;
	}

	UserSegment *pSegment = NULL;
	if (time == LOG_BEGIN)
	{
		pSegment = userSegments_.front();
	}
	else
	{
		pSegment = userSegments_.back();
	}

	int entryNum;
	if (time == LOG_END)
	{
		entryNum = 0;
	}
	else
	{
		entryNum = pSegment->getNumEntries() - 1;
	}

	return pSegment->readEntry( entryNum, result );
}


const LoggingComponent *UserLogReader::getComponentByID( int componentID )
{
	return userComponents_.getComponentByID( componentID );
}


bool UserLogReader::reloadFiles()
{
	if (!this->loadSegments())
	{
		ERROR_MSG( "UserLogReader::reloadFiles: Failed to reload segments\n" );
		return false;
	}

	if (!userComponents_.refresh())
	{
		ERROR_MSG( "UserLogReader::reloadFiles: Error reloading components\n" );
		return false;
	}
	
	return true;
}


bool UserLogReader::getUserComponents( UserComponentVisitor &visitor ) const
{
	return userComponents_.visitAllWith( visitor );
}


bool UserLogReader::visitAllSegmentsWith( UserSegmentVisitor &visitor ) const
{
	unsigned int i=0;
	bool status = true;

	while ((i < userSegments_.size()) && (status == true))
	{
		const UserSegmentReader *pSegment =
			static_cast< const UserSegmentReader * >( userSegments_[ i ] );
		status = visitor.onSegment( pSegment );
		++i;
	}

	return status;
}

// user_log_reader.cpp
