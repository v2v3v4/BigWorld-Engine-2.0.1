/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FORMAT_STRINGS_HPP
#define FORMAT_STRINGS_HPP

#include "binary_file_handler.hpp"

#include "log_string_interpolator.hpp"
#include "log_entry.hpp"

#include <map>

typedef std::vector< std::string > FormatStringList;

/**
 * Handles the global format strings mapping and file.
 */
class FormatStrings : public BinaryFileHandler
{
public:
	virtual ~FormatStrings();

	bool init( const char *root, const char *mode );

	virtual bool read();
	virtual void flush();

	LogStringInterpolator * resolve( const std::string &fmt );

	const LogStringInterpolator * getHandlerForLogEntry( const LogEntry &entry );

	FormatStringList getFormatStrings() const;

private:
	// Mapping from format string -> handler (used when writing log entries)
	typedef std::map< std::string, LogStringInterpolator* > FormatMap;
	FormatMap formatMap_;

	// Mapping from strings file offset -> handler (for reading)
	typedef std::map< uint32, LogStringInterpolator* > OffsetMap;
	OffsetMap offsetMap_;
};

#endif // FORMAT_STRINGS_HPP
