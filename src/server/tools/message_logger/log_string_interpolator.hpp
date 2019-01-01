/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOG_STRING_INTERPOLATOR_HPP
#define LOG_STRING_INTERPOLATOR_HPP

#include "string_offset.hpp"
#include "format_data.hpp"

#include "network/bsd_snprintf.h"
#include "network/file_stream.hpp"
#include "network/format_string_handler.hpp"
#include "network/logger_message_forwarder.hpp"

#include <string>
#include <vector>

class BinaryFile;
class LogStringWriter;

/**
 * This class handles both reading and writing bwlogs.
 */
class LogStringInterpolator : public FormatStringHandler
{
public:
	LogStringInterpolator() {}
	LogStringInterpolator( const std::string &fmt );
	~LogStringInterpolator() {}

	// FormatStringHandler interface
	void onString( int start, int end );
	void onToken( char type, int cflags, int min, int max,
		int flags, uint8 base, int vflags );

	uint32 fileOffset() const { return fileOffset_; }
	void write( FileStream &fs );
	void read( FileStream &fs );

	const std::string &fmt() const { return fmt_; }

	bool streamToLog( LogStringWriter &writer, 
		BinaryIStream &is, uint8 version = MESSAGE_LOGGER_VERSION );
	bool streamToString( BinaryIStream &is, std::string &str,
		uint8 version = MESSAGE_LOGGER_VERSION );

protected:
	std::string fmt_;
	std::string components_;
	StringOffsetList stringOffsets_;
	std::vector< FormatData > fmtData_;
	uint32 fileOffset_;

private:
	template < class Handler >
	bool interpolate( Handler &handler, BinaryIStream &is, uint8 version );

};

#endif // LOG_STRING_INTERPOLATOR_HPP
