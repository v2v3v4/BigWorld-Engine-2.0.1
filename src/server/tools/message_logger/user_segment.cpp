/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "user_segment.hpp"

#include "logging_component.hpp"
#include "log_entry.hpp"

#include <stdio.h>
#include <dirent.h>
#include <string.h>

UserSegment::UserSegment( const std::string userLogPath, const char *suffix ) :
	pEntries_( NULL ),
	pArgs_( NULL ),
	numEntries_( 0 ),
	argsSize_( 0 ),
	isGood_( true ),
	userLogPath_( userLogPath )
{
	// Generate a suffix if none provided
	if (suffix == NULL)
	{
		time_t now = time( NULL );
		struct tm *pTime = localtime( &now );
		MF_ASSERT( pTime != NULL);
		this->buildSuffixFrom( *pTime, suffix_ );
	}
	else
	{
		suffix_ = suffix;
	}

	return;
}


UserSegment::~UserSegment()
{
	if (pEntries_)
	{
		delete pEntries_;
	}

	if (pArgs_)
	{
		delete pArgs_;
	}
}



void UserSegment::updateEntryBounds()
{
	numEntries_ = pEntries_->length() / sizeof( LogEntry );
	argsSize_ = pArgs_->length();

	LogEntry entry;

	// Work out current start and end times
	if (numEntries_ > 0)
	{
		this->readEntry( 0, entry );
		start_ = entry.time_;
		this->readEntry( numEntries_ - 1, entry );
		end_ = entry.time_;
	}
}


/**
 * Retrieves a specific LogEntry from the UserSegment files.
 */
bool UserSegment::readEntry( int n, LogEntry &entry )
{
	pEntries_->seek( n * sizeof( LogEntry ) );
	*pEntries_ >> entry;
	if (pEntries_->error())
	{
		ERROR_MSG( "UserSegment::readEntry: Failed to read entry: %s\n",
			pEntries_->strerror() );
		return false;
	}

	return true;
}


/**
 *	This method creates a string to be used as a filename suffix using
 *	the provided time structure to format the date from.
 *
 *	@param pTime	The time structure to create the string with.
 *	@param newSuffix	The newly created suffix string.
 *
 *	@returns true if the suffix was successfully created, false otherwise.
 */
bool UserSegment::buildSuffixFrom( struct tm & pTime,
	std::string & newSuffix ) const
{
	char tmpBuff[ 1024 ];

	if (strftime( tmpBuff, sizeof( tmpBuff ) - 1, "%Y-%m-%d-%T", &pTime ) == 0)
	{
		ERROR_MSG( "UserSegment::buildSuffixFrom: "
			"Unable to format provided time to build suffix with.\n" );
		return false;
	}

	newSuffix = tmpBuff;

	return true;
}

// user_segment.cpp
