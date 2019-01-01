/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BWLOG_WRITER_HPP
#define BWLOG_WRITER_HPP

#include "bwlog_common.hpp"
#include "active_files.hpp"
#include "unary_integer_file.hpp"
#include "user_log_writer.hpp"

#include "server/config_reader.hpp"
#include "network/basictypes.hpp"
#include "network/logger_message_forwarder.hpp"


class BWLogWriter : public BWLogCommon
{
public:
	BWLogWriter();
	~BWLogWriter();

	bool init( const ConfigReader &config, const char *root );
	bool onUserLogInit( uint16 uid, const std::string &username );

	bool roll();

	bool setAppInstanceID( const Mercury::Address &addr, int id );

	bool stopLoggingFromComponent( const Mercury::Address &addr );

	bool addLogMessage( const LoggerComponentMessage &msg,
		const Mercury::Address &addr, MemoryIStream &is );

	bool updateActiveFiles();
	void deleteActiveFiles();

	void writeToStdout( bool status );

	int getMaxSegmentSize() const;

private:
	bool initFromConfig( const ConfigReader &config );

	UserLogWriterPtr createUserLog( uint16 uid, const std::string &username );
	UserLogWriterPtr getUserLog( uint16 uid );

	LoggingComponent* findLoggingComponent( const Mercury::Address &addr );

	Mercury::Reason resolveUID( uint16 uid, uint32 addr,
		std::string &result );

	bool writeToStdout_;

	// The maximum size allowed for UserLog segment files (in bytes)
	int maxSegmentSize_;

	std::string logDir_;

	UnaryIntegerFile pid_;

	// Both BWLogWriter and BWLogReader need a UserLog list, however they both
	// handle the descruction of these lists differently so are kept in their
	// respective subclasses.
	UserLogs userLogs_;

	// The ActiveFiles contains a pointer to the UserLogs so it
	// is able to update itself whenever it's necessary.
	ActiveFiles activeFiles_;
};

#endif // BWLOG_WRITER_HPP
