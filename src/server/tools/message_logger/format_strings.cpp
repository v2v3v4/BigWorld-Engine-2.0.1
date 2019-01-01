/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "format_strings.hpp"


FormatStrings::~FormatStrings()
{
	OffsetMap::iterator it = offsetMap_.begin();
	while (it != offsetMap_.end())
	{
		delete it->second;
		++it;
	}
}


bool FormatStrings::init( const char *root, const char *mode )
{
	const char *formatStringsPath = this->join( root, "strings" );
	return BinaryFileHandler::init( formatStringsPath, mode );
}


bool FormatStrings::read()
{
	long len = pFile_->length();
	pFile_->seek( 0 );

	while (pFile_->tell() < len)
	{
		LogStringInterpolator *pHandler = new LogStringInterpolator();
		pHandler->read( *pFile_ );

		// This could happen if we're reading and the logger was halfway
		// through dumping a fmt when we calculated 'len'.
		if (pFile_->error())
		{
			WARNING_MSG( "FormatStrings::read: "
				"Error encountered while reading strings file '%s': %s\n",
				filename_.c_str(), pFile_->strerror() );
			return false;
		}

		formatMap_[ pHandler->fmt() ] = pHandler;
		offsetMap_[ pHandler->fileOffset() ] = pHandler;
	}

	return true;
}


/**
 * This method is invoked from FileHandler::refresh() to clear our current
 * self knowledge prior to re-reading the format string file.
 */
void FormatStrings::flush()
{
	FormatMap::iterator it = formatMap_.begin();
	while (it != formatMap_.end())
	{
		delete it->second;
		++it;
	}

	formatMap_.clear();
	offsetMap_.clear();
}


/**
 * If we're in write mode and the fmt string passed in does not currently exist
 * in the mapping, it will be added to the mapping and written to disk.
 */
LogStringInterpolator* FormatStrings::resolve( const std::string &fmt )
{
	FormatMap::iterator it = formatMap_.find( fmt );
	if (it != formatMap_.end())
	{
		return it->second;
	}

	// If we aren't in append mode then we can't modify the file further, thus
	// no valid result is found.
	if (mode_ != "a+")
	{
		return NULL;
	}

	LogStringInterpolator *pHandler = new LogStringInterpolator( fmt );
	pHandler->write( *pFile_ );
	pFile_->commit();

	formatMap_[ fmt ] = pHandler;
	offsetMap_[ pHandler->fileOffset() ] = pHandler;

	return pHandler;
}


/**
 * This method returns the string handler that should be used to parse
 * the log entry provided.
 *
 * @returns A pointer to a LogStringInterpolator on success, NULL on error.
 */
const LogStringInterpolator* FormatStrings::getHandlerForLogEntry(
	const LogEntry &entry )
{
	OffsetMap::iterator it = offsetMap_.find( entry.stringOffset_ );

	if (it == offsetMap_.end())
	{
		return NULL;
	}

	return it->second;
}


/**
 * This method returns a list of all the format available format strings.
 *
 * @returns A std::vector of std::strings.
 */
FormatStringList FormatStrings::getFormatStrings() const
{
	FormatStringList stringsList;

	FormatMap::const_iterator it = formatMap_.begin();
	while (it != formatMap_.end())
	{
		stringsList.push_back( it->first );
		++it;
	}

	return stringsList;
}
