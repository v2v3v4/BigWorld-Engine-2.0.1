/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "file_transfer_progress_reporter.hpp"

#include <sstream>

/**
 * 	Override SluggishProgressReporter method
 */
void FileTransferProgressReporter::reportProgressNow()
{
	std::stringstream ss;
	if (startedFiles_ < numFiles_)
	{
		ss << "Data consolidation is waiting for "
			<< " all secondary database file transfers to start ("
			<< startedFiles_ << '/' << numFiles_ << " started)";
	}
	else
	{
		ss << "Transfering secondary databases for consolidation ("
			<< doneBytes_ << '/' << numBytes_ << " bytes) ("
			<< doneFiles_ << '/' << numFiles_ << " files completed)";
	}

	this->reporter().onStatus( ss.str() );
}

// file_transfer_progress_reporter.cpp
