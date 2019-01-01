/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef USER_LOG_WRITER_HPP
#define USER_LOG_WRITER_HPP

#include "cstdmf/smartpointer.hpp"

#include <map>

class UserLogWriter;
typedef SmartPointer< UserLogWriter > UserLogWriterPtr;
typedef std::map< uint16, UserLogWriterPtr > UserLogs;


#include "user_log.hpp"
#include "log_component_names.hpp"
#include "log_string_interpolator.hpp"

#include "network/basictypes.hpp"

#include <string>
#include <vector>

class BWLogWriter;
class BWLogCommon;
class UserSegmentWriter;

class UserLogWriter : public UserLog
{
public:
	UserLogWriter( uint16 uid, const std::string &username );

	bool init( const std::string rootPath );

	LoggingComponent * findLoggingComponent( const Mercury::Address &addr );
	LoggingComponent * findLoggingComponent( const LoggerComponentMessage &msg,
		const Mercury::Address &addr, LogComponentNames &logComponents );

	bool addEntry( LoggingComponent *component, LogEntry &entry,
		LogStringInterpolator &handler, MemoryIStream &is,
		BWLogWriter *pLogWriter, uint8 version );

	bool updateComponent( LoggingComponent *component );
	bool removeUserComponent( const Mercury::Address &addr );

	void rollActiveSegment();

	const char *logEntryToString( const LogEntry &entry,
		BWLogCommon *pBWLog, const LoggingComponent *component,
		LogStringInterpolator &handler, MemoryIStream &is, uint8 version ) const;

private:
	UserSegmentWriter * getLastSegment();
};

#endif // USER_LOG_WRITER_HPP
