/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "consolidation_progress_reporter.hpp"

#include <sstream>

/**
 * 	Override SluggishProgressReporter method
 */
void ConsolidationProgressReporter::reportProgressNow()
{
	// Generate string
	std::stringstream ss;
	ss << "Consolidating " << curDBName_ << " (" << doneEntitiesInCurDB_
		<< '/' << numEntitiesInCurDB_ << " entities)"
		<< " (" << doneDBs_ << '/' << numDBs_ << " databases)";

	this->reporter().onStatus( ss.str() );
}


// consolidation_progress_reporter.cpp
