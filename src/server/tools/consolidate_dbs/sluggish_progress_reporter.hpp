/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONSOLIDATE_DBS__SLUGGISH_PROGRESS_REPORTER_HPP
#define CONSOLIDATE_DBS__SLUGGISH_PROGRESS_REPORTER_HPP

#include "dbmgr_status_reporter.hpp"

#include "cstdmf/timestamp.hpp"

/**
 * 	This is a base class for progress reporters. reportProgress() can be called
 * 	very often, but reportProgressNow() will only be called once every
 * 	reportInterval.
 */
class SluggishProgressReporter
{
public:
	/**
	 *	Constructor.
	 */
	SluggishProgressReporter( DBMgrStatusReporter & reporter,
			float reportInterval = 0.5 ) : // Half a second
		reporter_( reporter ),
		reportInterval_( uint64( reportInterval * stampsPerSecondD() ) ),
		lastReportTime_( timestamp() )
	{}

	void reportProgress()
	{
		uint64 now = timestamp();
		if ((now - lastReportTime_) > reportInterval_)
		{
			this->reportProgressNow();
			lastReportTime_ = now;
		}
	}

	// Should be overridden by derived class
	virtual void reportProgressNow() = 0;

protected:
	DBMgrStatusReporter & reporter()
		{ return reporter_; }

private:
	DBMgrStatusReporter & reporter_;

	uint64	reportInterval_;
	uint64	lastReportTime_;
};


#endif // CONSOLIDATE_DBS__SLUGGISH_PROGRESS_REPORTER_HPP
