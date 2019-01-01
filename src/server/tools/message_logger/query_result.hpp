/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef QUERY_RESULT_HPP
#define QUERY_RESULT_HPP

#include "bwlog_common.hpp"
#include "user_log.hpp"

#include "cstdmf/stdmf.hpp"

#include <string>


/**
 * This is the python object returned from a Query.
 */
class QueryResult
{
public:
	QueryResult();
	QueryResult( const LogEntry &entry, BWLogCommon *pBWLog,
		const UserLog *pUserLog, const LoggingComponent *pComponent,
		const std::string &message );

	// Static buffer used for generating result output.
	static char s_linebuf_[];

	const char *format( unsigned flags = SHOW_ALL, int *pLen = NULL );

	const std::string & getMessage() const;
	uint32 getStringOffset() const;
	double getTime() const;
	const char * getHost() const;
	int getPID() const;
	int getAppID() const;
	const char * getUsername() const;
	const char * getComponent() const;
	int getSeverity() const;

private:
	double time_;
	const char * host_;
	int pid_;
	int appid_;
	const char * username_;
	const char * component_;
	int severity_;

	// message_ needs its own memory since an outer context may well
	// overwrite the buffer provided during construction
	std::string message_;

	// We also store the format string offset as this makes computing
	// histograms faster.
	uint32 stringOffset_;
};

#endif // QUERY_RESULT_HPP
