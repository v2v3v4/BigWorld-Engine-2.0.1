/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOG_ENTRY_ADDRESS_HPP
#define LOG_ENTRY_ADDRESS_HPP

#include "cstdmf/stdmf.hpp"

#include <string>

class BinaryIStream;
class BinaryOStream;

#pragma pack( push, 1 )

/**
 * This class represents the on disk address of a log entry. Notice that we
 * reference by suffix instead of segment number to handle segment deletion on
 * disk.
 */
class LogEntryAddress
{
public:
	LogEntryAddress();
	LogEntryAddress( const char *suffix, int index );
	LogEntryAddress( const std::string &suffix, int index );

	void write( BinaryOStream &os ) const;
	void read( BinaryIStream &is );

	bool operator<( const LogEntryAddress &other ) const;

	const char *getSuffix() const;
	int getIndex() const;

protected:
	std::string suffix_;
	int32 index_;
};
#pragma pack( pop )

#endif // LOG_ENTRY_ADDRESS_HPP
