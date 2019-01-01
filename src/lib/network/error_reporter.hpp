/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ERROR_REPORTER_HPP
#define ERROR_REPORTER_HPP

#include "basictypes.hpp"
#include "interfaces.hpp"

#include <map>
#include <utility> // For std::pair

namespace Mercury
{

class EventDispatcher;

/**
 *  Accounting structure for keeping track of the number of exceptions reported
 *  in a given period.
 */
struct ErrorReportAndCount
{
	uint64 lastReportStamps;	//< When this error was last reported
	uint64 lastRaisedStamps;	//< When this error was last raised
	uint count;					//< How many of this exception have been
								//	reported since
};


/**
 *	Key type for ErrorsAndCounts.
 */

typedef std::pair< Address, std::string > AddressAndErrorString;

/**
 *	Accounting structure that keeps track of counts of Mercury exceptions
 *	in a given period per pair of address and error message.
 *
 */
typedef std::map< AddressAndErrorString, ErrorReportAndCount >
	ErrorsAndCounts;


/**
 *	This class is responsible for handling error messages that may be delayed
 *	and aggregated.
 */
class ErrorReporter : public TimerHandler
{
public:
	ErrorReporter( EventDispatcher & dispatcher );
	~ErrorReporter();

	void reportException( Reason reason, const Address & addr = Address::NONE,
			const char * prefix = NULL );
	void reportPendingExceptions( bool reportBelowThreshold = false );

private:
	void reportException( const NubException & ne, const char * prefix = NULL );

	void reportError( const Address & address, const char* format, ... );

	/**
	 *	The minimum time that an exception can be reported from when it was
	 *	first reported.
	 */
	static const uint ERROR_REPORT_MIN_PERIOD_MS;

	/**
	 *	The nominal maximum time that a report count for a Mercury address and
	 *	error pair is kept.
	 */
	static const uint ERROR_REPORT_COUNT_MAX_LIFETIME_MS;

	void addReport( const Address & address, const std::string & error );

	static std::string addressErrorToString(
			const Address & address,
			const std::string & errorString );

	static std::string addressErrorToString(
			const Address & address,
			const std::string & errorString,
			const ErrorReportAndCount & reportAndCount,
			const uint64 & now );

	virtual void handleTimeout( TimerHandle handle, void * arg );

	TimerHandle reportLimitTimerHandle_;
	ErrorsAndCounts errorsAndCounts_;
};

} // namespace Mercury

#endif // ERROR_REPORTER_HPP
