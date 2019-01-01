/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOG_STRING_PRINTER_HPP
#define LOG_STRING_PRINTER_HPP

#include "format_data.hpp"

#include "network/forwarding_string_handler.hpp"
#include "network/bsd_snprintf.h"

#include <string>

/**
 * StreamParsers - for reading either network or file streams of log data
 */
class LogStringPrinter
{
public:
	void setResultString( std::string &result );

	// Parsing methods
	void onFmtStringSection( const std::string &fmt, int start, int end );

	void onParseComplete();
	void onError();

	void onMinWidth( WidthType w, FormatData &fd );
	void onMaxWidth( WidthType w, FormatData &fd );

	void onChar( char c, const FormatData &fd );
	void onString( const char *s, const FormatData &fd );
	void onPointer( int64 ptr, const FormatData &fd );

	template < class IntType >
	void onInt( IntType i, const FormatData &fd )
	{
		bsdFormatInt( i, fd.base_, fd.min_, fd.max_, fd.flags_, *pStr_ );
	}

	template < class FloatType >
	void onFloat( FloatType f, const FormatData &fd )
	{
		bsdFormatFloat( f, fd.min_, fd.max_, fd.flags_, *pStr_ );
	}

private:
	// We are passed a reference to a string to modify. This pointer
	// should never be deleted.
	std::string *pStr_;
};

#endif // LOG_STRING_PRINTER_HPP
