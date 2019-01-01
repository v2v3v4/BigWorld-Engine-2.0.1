/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOG_ENTRY_HPP
#define LOG_ENTRY_HPP

#include "log_time.hpp"

#include "cstdmf/stdmf.hpp"


/**
 * The fixed-length portion of a log entry (i.e. the bit that gets written
 * to the 'entries' file).
 */
struct LogEntry
{
	LogTime time_;
	int componentID_;
	uint8 messagePriority_;
	uint32 stringOffset_;
	uint32 argsOffset_;
	uint16 argsLen_;
};

#endif // LOG_ENTRY_HPP
