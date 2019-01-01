/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "log_string_printer.hpp"


void LogStringPrinter::setResultString( std::string &result )
{
	pStr_ = &result;
}


void LogStringPrinter::onParseComplete()
{
	pStr_ = NULL;
}


void LogStringPrinter::onError()
{
	pStr_ = NULL;
}


void LogStringPrinter::onFmtStringSection( const std::string &fmt,
	int start, int end )
{
	bsdFormatString( fmt.c_str() + start, 0, 0, end-start, *pStr_ );
}


void LogStringPrinter::onMinWidth( WidthType w, FormatData &fd )
{
	fd.min_ = w;
}


void LogStringPrinter::onMaxWidth( WidthType w, FormatData &fd )
{
	fd.max_ = w;
}


void LogStringPrinter::onString( const char *s, const FormatData &fd )
{
	bsdFormatString( s, fd.flags_, fd.min_, fd.max_, *pStr_ );
}


void LogStringPrinter::onPointer( int64 ptr, const FormatData &fd )
{
	char buf[ 128 ];
	bw_snprintf( buf, sizeof( buf ), "0x%"PRIx64, uint64( ptr ) );
	this->onString( buf, fd );
}


void LogStringPrinter::onChar( char c, const FormatData &fd )
{
	char buf[2] = { c, 0 };
	this->onString( buf, fd );
}
