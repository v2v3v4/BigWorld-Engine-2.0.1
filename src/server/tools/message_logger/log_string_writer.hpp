/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOG_STRING_WRITER_HPP
#define LOG_STRING_WRITER_HPP

#include "format_data.hpp"

#include "network/file_stream.hpp"
#include "network/forwarding_string_handler.hpp"
#include "network/bsd_snprintf.h"

#include <string>

class LogStringWriter
{
public:
	LogStringWriter( FileStream &blobFile );

	bool isGood() const;

	void onFmtStringSection( const std::string &fmt, int start, int end );

	void onParseComplete();
	void onError();

	void onMinWidth( WidthType w, FormatData &fd );
	void onMaxWidth( WidthType w, FormatData &fd );

	void onChar( char c, const FormatData &fd );
	void onString( const char *s, const FormatData &fd );
	void onPointer( int64 ptr, const FormatData &fd );

	template < class IntType >
	void onInt( IntType i, const FormatData & /* fd */ )
	{
		blobFile_ << i;
	}

	template < class FloatType >
	void onFloat( FloatType f, const FormatData & /* fd */ )
	{
		blobFile_ << f;
	}

private:
	bool isGood_;

	FileStream &blobFile_;
};

#endif // LOG_STRING_WRITER_HPP
