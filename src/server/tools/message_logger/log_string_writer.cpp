/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "log_string_writer.hpp"


LogStringWriter::LogStringWriter( FileStream &blobFile ) :
	isGood_( false ),
	blobFile_( blobFile )
{ }


bool LogStringWriter::isGood() const
{
	return (isGood_ && blobFile_.good());
}

void LogStringWriter::onParseComplete()
{
	blobFile_.commit();
	isGood_ = true;
}


void LogStringWriter::onError()
{
	isGood_ = false;
}


void LogStringWriter::onFmtStringSection( const std::string &fmt,
	int start, int end )
{ }


void LogStringWriter::onMinWidth( WidthType w, FormatData & /* fd */ )
{
	blobFile_ << w;
}

void LogStringWriter::onMaxWidth( WidthType w, FormatData & /* fd */ )
{
	blobFile_ << w;
}


void LogStringWriter::onString( const char *s, const FormatData & /* fd */ )
{
	blobFile_ << s;
}


void LogStringWriter::onPointer( int64 ptr, const FormatData & /* fd */ )
{
	blobFile_ << ptr;
}


void LogStringWriter::onChar( char c, const FormatData & /* fd */ )
{
	blobFile_ << c;
}
