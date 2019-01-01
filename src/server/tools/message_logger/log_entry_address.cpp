/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "log_entry_address.hpp"

#include "cstdmf/binary_stream.hpp"

LogEntryAddress::LogEntryAddress()
{ }

LogEntryAddress::LogEntryAddress( const char *suffix, int index ) :
	suffix_( suffix ),
	index_( index )
{ }


LogEntryAddress::LogEntryAddress( const std::string &suffix, int index ) :
	suffix_( suffix ),
	index_( index )
{ }


void LogEntryAddress::write( BinaryOStream &os ) const
{
	os << suffix_ << index_;
}


// Note: the read functionality is required by the log writer
void LogEntryAddress::read( BinaryIStream &is )
{
	is >> suffix_ >> index_;
}


bool LogEntryAddress::operator<( const LogEntryAddress &other ) const
{
	return suffix_ < other.suffix_ ||
		(suffix_ == other.suffix_ && index_ < other.index_);
}


const char *LogEntryAddress::getSuffix() const
{
	return suffix_.c_str();
}


int LogEntryAddress::getIndex() const
{
	return index_;
}
