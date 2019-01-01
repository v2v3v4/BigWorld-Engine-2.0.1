/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef QUERY_PARAMS_HPP
#define QUERY_PARAMS_HPP

#include "cstdmf/smartpointer.hpp"

class QueryParams;
typedef SmartPointer< QueryParams > QueryParamsPtr;

// TODO: get rid of this dependency. We should have a way around needing this
#include "Python.h"

#include "bwlog_reader.hpp"
#include "log_time.hpp"
#include "log_entry_address_reader.hpp"
#include "user_log_reader.hpp"

#include "cstdmf/stdmf.hpp"

#include <regex.h> // regex_t


class BWLogReader;

/**
 * This class represents a set of parameters to be used when querying logs.
 */
class QueryParams : public SafeReferenceCount
{
public:
	QueryParams();

	bool init( PyObject *args, PyObject *kwargs,
				const BWLogReader *pLogReader );

	bool isGood() const { return isGood_; }

	uint16 getUID() const;
	void setUID( uint16 uid );

	// These accessors are used to populate the QueryRange
	LogTime getStartTime() const;
	LogTime getEndTime() const;
	LogEntryAddressReader getStartAddress() const;
	LogEntryAddressReader getEndAddress() const;
	SearchDirection getDirection() const;

	int getNumLinesOfContext() const;

	bool isPreInterpolate() const;
	bool isPostInterpolate() const;

	// These methods are used to validate the parameters for a Query
	bool validateAddress( uint32 addr ) const;
	bool validatePID( uint16 pid ) const;
	bool validateAppID( uint16 appid ) const;
	bool validateProcessType( int procType ) const;
	bool validateMessagePriority( int priority ) const;
	bool validateIncludeRegex( const char *searchStr ) const;
	bool validateExcludeRegex( const char *searchStr ) const;


private:
	// Private because we want to let the reference counting handle deletion
	~QueryParams();

	bool initHost( const char *host, const BWLogReader *pLogReader  );
	bool initRegex( const char *include, const char *exclude );
	bool initStartPeriod( PyObject *startAddr, double start,
		UserLogReaderPtr pUserLog, const std::string &period );
	bool initEndPeriod( PyObject *endAddr, double endTime,
		UserLogReaderPtr pUserLog, const std::string &period );

	uint16 uid_;

	LogTime start_;
	LogTime end_;

	LogEntryAddressReader startAddress_;
	LogEntryAddressReader endAddress_;

	uint32 addr_;
	uint16 pid_;
	uint16 appid_;
	int procs_;
	int severities_;

	regex_t *pInclude_;
	regex_t *pExclude_;

	int interpolate_;
	bool casesens_;
	SearchDirection direction_;
	int numLinesOfContext_;

	bool isGood_;
};

#endif // QUERY_PARAMS_HPP
