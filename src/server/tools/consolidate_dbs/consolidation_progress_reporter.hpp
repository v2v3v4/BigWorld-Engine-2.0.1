/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONSOLIDATE_DBS__CONSOLIDATION_PROGRESS_REPORTER_HPP
#define CONSOLIDATE_DBS__CONSOLIDATION_PROGRESS_REPORTER_HPP

#include "sluggish_progress_reporter.hpp"

#include <string>

/**
 * 	This object is passed around to various operations to so that there is a
 * 	a single object that knows about the progress of consolidation and can
 * 	report it to DBMgr.
 */
class ConsolidationProgressReporter : private SluggishProgressReporter
{
public:

	/**
	 *	Constructor.
	 */
	ConsolidationProgressReporter( DBMgrStatusReporter & reporter, int numDBs ) :
		SluggishProgressReporter( reporter ),
		numDBs_( numDBs ),
		doneDBs_( 0 ),
		numEntitiesInCurDB_( 0 ),
		doneEntitiesInCurDB_( 0 )
	{}


	void onStartConsolidateDB( const std::string & dbName, int numEntities )
	{
		++doneDBs_;
		curDBName_ = dbName;
		numEntitiesInCurDB_ = numEntities;
		doneEntitiesInCurDB_ = 0;

		this->reportProgressNow();
	}


	void onConsolidatedRow()
	{
		++doneEntitiesInCurDB_;
		this->reportProgress();	// SluggishProgressReporter method
	}

private:
	virtual void reportProgressNow();

private:
	int			numDBs_;
	int			doneDBs_;	// Actualy counts the one currently being done
	std::string curDBName_;
	int			numEntitiesInCurDB_;
	int			doneEntitiesInCurDB_;
};

#endif // CONSOLIDATE_DBS__CONSOLIDATION_PROGRESS_REPORTER_HPP
