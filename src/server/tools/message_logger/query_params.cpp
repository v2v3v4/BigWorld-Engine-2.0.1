/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "query_params.hpp"

#include "constants.hpp"
#include "bwlog_reader.hpp"
#include "user_log_reader.hpp"
#include "log_entry.hpp"

#include <sys/types.h>
#include <regex.h>


// Helper function for compiling regexs
static regex_t *reCompile( const char *pattern, bool casesens )
{
	regex_t *re = new regex_t();
	int reFlags = REG_EXTENDED | REG_NOSUB | (casesens ? 0 : REG_ICASE);

	int reError = regcomp( re, pattern, reFlags );
	if (reError != 0)
	{
		char reErrorBuf[ 256 ];
		regerror( reError, re, reErrorBuf, sizeof( reErrorBuf ) );

		PyErr_Format( PyExc_SyntaxError,
			"Failed to compile regex '%s': %s\n",
			pattern, reErrorBuf );

		delete re;
		return NULL;
	}

	return re;
}


/**
 * The only mandatory argument to this method is the uid.  Everything else has
 * (reasonably) sensible defaults.
 *
 * Also, be aware that if you are searching backwards (i.e. end time > start
 * time) then your results will be in reverse order.  This is because results
 * are generated on demand, not in advance.
 *
 * This class now automatically swaps start/end and startaddr/endaddr if they
 * are passed in reverse order.  This is so we can avoid repeating all the
 * reordering logic in higher level apps.
 *
 * @param addr	IP Address of the records to find (0 for all records)
 * @param max   The maximum number of records to return (0 for not limit)
 */
QueryParams::QueryParams() :
	uid_( 0 ),
	start_( LOG_BEGIN ),
	end_( LOG_END ),
	addr_( 0 ),
	pid_( 0 ),
	appid_( 0 ),
	procs_( -1 ),
	severities_( -1 ),
	pInclude_( NULL ),
	pExclude_( NULL ),
	interpolate_( PRE_INTERPOLATE ),
	casesens_( true ),
	direction_( QUERY_FORWARDS ),
	numLinesOfContext_( 0 ),
	isGood_( false )
{ }


bool QueryParams::initHost( const char *host, const BWLogReader *pLogReader )
{
	if (*host)
	{
		addr_ = pLogReader->getAddressFromHost( host );
	}
	else
	{
		addr_ = 0;
	}

	if (*host && (addr_ == 0))
	{
		PyErr_Format( PyExc_LookupError,
			"Queried host '%s' was not known in the logs", host );
		return false;
	}

	return true;
}


bool QueryParams::initRegex( const char *include, const char *exclude )
{
	// PyErr messages are set in reCompile on failure.

	// Regex compilation
	if (*include)
	{
		pInclude_ = reCompile( include, casesens_ );
		if (pInclude_ == NULL)
		{
			return false;
		}
	}

	if (*exclude)
	{
		pExclude_ = reCompile( exclude, casesens_ );
		if (pExclude_ == NULL)
		{
			return false;
		}
	}

	return true;
}


bool QueryParams::initStartPeriod( PyObject *startAddr, double startTime,
	UserLogReaderPtr pUserLog, const std::string &period )
{
	// Start address take precedences over start time if both are specified.
	if (startAddr != NULL)
	{
		if (!startAddress_.fromPyTuple( startAddr ))
		{
			return false;
		}

		// Obtain the start time here to ensure that inference of direction
		// based on start and end time is always correct.
		LogEntry entry;

		if (!pUserLog->getEntry( startAddress_, entry ))
		{
			PyErr_Format( PyExc_RuntimeError,
				"Couldn't determine time for %s's entry address %s:%d",
				pUserLog->getUsername().c_str(),
				startAddress_.getSuffix(),
				startAddress_.getIndex() );
			return false;
		}

		start_ = entry.time_;
	}
	else
	{
		start_ = startTime;

		// If start time was given as either extremity and we're using a fixed
		// period (i.e. period isn't to beginning or present), we need the
		// actual time for the same reason as above.
		// TODO: hard coded "to beginning" .. etc should be changed to constants
		if (period.size() &&
			period != "to beginning" &&
			period != "to present" &&
			(startTime == LOG_BEGIN || startTime == LOG_END))
		{
			LogEntry entry;

			if (!pUserLog->getEntry( startTime, entry ))
			{
				char buf[1024];
				bw_snprintf( buf, sizeof( buf ),
					"Couldn't determine time for %s's extremity %f",
					pUserLog->getUsername().c_str(), startTime );
				PyErr_SetString( PyExc_RuntimeError, buf );
				return false;
			}

			start_ = entry.time_;
		}
	}

	return true;
}


bool QueryParams::initEndPeriod( PyObject *endAddr, double endTime,
	UserLogReaderPtr pUserLog, const std::string &period )
{
	// endAddr takes the highest priority for figuring out the endpoint,
	// followed by period, and then by end time.
	if (endAddr != NULL)
	{
		if (!endAddress_.fromPyTuple( endAddr ))
		{
			return false;
		}

		// Obtain the end time here to ensure that inference of direction
		// based on start and end time is always correct.
		LogEntry entry;

		if (!pUserLog->getEntry( endAddress_, entry ))
		{
			PyErr_Format( PyExc_RuntimeError,
				"Couldn't determine time for %s's entry address %s:%d",
				pUserLog->getUsername().c_str(),
				endAddress_.getSuffix(),
				endAddress_.getIndex() );
			return false;
		}

		end_ = entry.time_;
	}
	else if (period.size())
	{
		if (period == "to beginning")
		{
			end_ = 0;
		}
		else if (period == "to present")
		{
			end_ = -1;
		}
		else
		{
			end_ = start_.asDouble() + atof( period.c_str() );

			if (period[0] != '+'&& period[0] != '-')
			{
				start_ = start_.asDouble() - atof( period.c_str() );
			}
		}
	}
	else
	{
		end_ = endTime;
	}

	return true;
}


bool QueryParams::init( PyObject *args, PyObject *kwargs,
	const BWLogReader *pLogReader )
{
	static const char *kwlist[] = { "uid", "start", "end", "startaddr",
							"endaddr", "period", "host", "pid", "appid",
							"procs", "severities", "message", "exclude",
							"interpolate", "casesens", "direction", "context",
							NULL };

	int direction = QUERY_FORWARDS;
	double start = LOG_BEGIN;
	double end = LOG_END;
	const char *host = "", *include = "", *exclude = "", *cperiod = "";
	PyObject *startAddr = NULL, *endAddr = NULL;

	if (!PyArg_ParseTupleAndKeywords( args, kwargs, "H|ddO!O!ssHHiissibii",
			const_cast<char **>( kwlist ),
			&uid_, &start, &end,
			&PyTuple_Type, &startAddr, &PyTuple_Type, &endAddr,
			&cperiod, &host, &pid_, &appid_, &procs_, &severities_,
			&include, &exclude, &interpolate_, &casesens_, &direction, 
			&numLinesOfContext_ ))
	{
		// No need to set PyErr here, taken care of by python
		return false;
	}

	if ((direction != QUERY_FORWARDS) && (direction != QUERY_BACKWARDS))
	{
		PyErr_Format( PyExc_ValueError,
			"Invalid direction (%d)", direction );
		return false;
	}
	direction_ = (SearchDirection)direction;

	if (!this->initHost( host, pLogReader ))
	{
		// PyErr is set in initHost
		return false;
	}

	if (!this->initRegex( include, exclude ))
	{
		// PyErr messages are set in reCompile on failure.
		return false;
	}

	UserLogReaderPtr pUserLog =
		const_cast< BWLogReader * >( pLogReader )->getUserLog( uid_ );

	if (pUserLog == NULL)
	{
		PyErr_Format( PyExc_LookupError,
			"UID %d doesn't have any entries in this log", uid_ );
		return false;
	}

	// If this user's log has no segments (i.e. they have been rolled away) then
	// bail out now
	if (!pUserLog->hasActiveSegments())
	{
		WARNING_MSG( "QueryParams::init: "
			"%s's log has no segments, they may have been rolled\n",
			pUserLog->getUsername().c_str() );

		isGood_ = true;
		return isGood_;
	}

	std::string period = cperiod;

	if (!this->initStartPeriod( startAddr, start, pUserLog, period ))
	{
		// PyErr set in initStartPeriod
		return false;
	}

	if (!this->initEndPeriod( endAddr, end, pUserLog, period ))
	{
		// PyErr set in initEndPeriod
		return false;
	}

	// Re-order times if passed in reverse order.  TODO: Actually verify that
	// the segment numbers are in order instead of using lex cmp on suffixes.
	bool hasValidTimes = (end_ < start_);
	bool hasValidAddresses = startAddress_.isValid() &&
						endAddress_.isValid() && endAddress_ < startAddress_;

	if (hasValidTimes || hasValidAddresses)
	{
		LogTime lt = end_;
		end_ = start_;
		start_ = lt;

		LogEntryAddressReader ea = endAddress_;
		endAddress_ = startAddress_;
		startAddress_ = ea;

		// Reverse the direction now
		//direction_ *= -1;
		direction_ = (direction_ == QUERY_FORWARDS) ?
							QUERY_BACKWARDS : QUERY_FORWARDS;
	}

	if (start_ > end_)
	{
		PyErr_Format( PyExc_RuntimeError, "QueryParams::init: "
			"Start time (%ld.%d) is greater than end time (%ld.%d)",
			(LogTime::Seconds)start_.secs_, start_.msecs_,
			(LogTime::Seconds)end_.secs_, end_.msecs_ );
	}
	else
	{
		isGood_ = true;
	}

	return isGood_;
}


QueryParams::~QueryParams()
{
	if (pInclude_ != NULL)
	{
		regfree( pInclude_ );
		delete pInclude_;
	}

	if (pExclude_ != NULL)
	{
		regfree( pExclude_ );
		delete pExclude_;
	}
}


uint16 QueryParams::getUID() const
{
	return uid_;
}


void QueryParams::setUID( uint16 uid )
{
	uid_ = uid;
}


LogTime QueryParams::getStartTime() const
{
	return start_;
}


LogTime QueryParams::getEndTime() const
{
	return end_;
}


LogEntryAddressReader QueryParams::getStartAddress() const
{
	return startAddress_;
}


LogEntryAddressReader QueryParams::getEndAddress() const
{
	return endAddress_;
}


SearchDirection QueryParams::getDirection() const
{
	return direction_;
}


int QueryParams::getNumLinesOfContext() const
{
	return numLinesOfContext_;
}


bool QueryParams::isPreInterpolate() const
{
	return (interpolate_ == PRE_INTERPOLATE); 
}


bool QueryParams::isPostInterpolate() const
{
	return (interpolate_ == POST_INTERPOLATE); 
}


// TODO: these methods need to be double checked during commit check time
// also think of better names for them
/**
 * Validates whether we have an address set, and if so, whether the address
 * provided matches.
 *
 * It is valid for there to be no address set in the parameters.
 */
bool QueryParams::validateAddress( uint32 addr ) const
{
	// If no address has been specific then we can continue
	if (!addr_)
		return true;

	return (addr_ == addr);
}


bool QueryParams::validatePID( uint16 pid ) const
{
	if (!pid_)
		return true;

	return (pid_ == pid);
}


bool QueryParams::validateAppID( uint16 appid ) const
{
	if (!appid_)
		return true;

	return (appid_ == appid);
}


bool QueryParams::validateProcessType( int procType ) const
{
	if (procs_ == -1)
		return true;

	return (procs_ & (1 << procType));
}


bool QueryParams::validateMessagePriority( int priority ) const
{
	if (severities_ == -1)
		return true;

	return (severities_ & (1 << priority));
}


bool QueryParams::validateIncludeRegex( const char *searchStr ) const
{
	if (pInclude_ == NULL)
		return true;

	return (regexec( pInclude_, searchStr, 0, NULL, 0 ) == 0);
}


bool QueryParams::validateExcludeRegex( const char *searchStr ) const
{
	if (pExclude_ == NULL)
		return true;

	return (regexec( pExclude_, searchStr, 0, NULL, 0 ) != 0);
}
