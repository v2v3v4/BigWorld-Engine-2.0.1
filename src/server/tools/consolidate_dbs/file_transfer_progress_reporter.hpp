/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONSOLIDATE_DBS__FILE_TRANSFER_PROGRESS_REPORTER_HPP
#define CONSOLIDATE_DBS__FILE_TRANSFER_PROGRESS_REPORTER_HPP

#include "sluggish_progress_reporter.hpp"

/**
 * 	This object is passed around to various operations to so that there is a
 * 	a single object that knows about the progress of file transfer and can
 * 	report it to DBMgr.
 */
class FileTransferProgressReporter : private SluggishProgressReporter
{
public:
	/**
	 *	Constructor.
	 */
	FileTransferProgressReporter( DBMgrStatusReporter & reporter,
			int numFiles ) :
		SluggishProgressReporter( reporter ),
		numFiles_( numFiles ),
		startedFiles_( 0 ), 
		doneFiles_( 0 ),
		numBytes_( 0 ), 
		doneBytes_( 0 )
	{
		this->reportProgressNow();
	}

	void onStartTransfer( int numBytesInFile )	// Start transfer of a file
	{
		++startedFiles_;
		numBytes_ += numBytesInFile;

		this->reportProgress();
	}

	void onReceiveData( int numBytes )
	{
		doneBytes_ += numBytes;
		this->reportProgress();
	}

	void onFinishTransfer()	// Finish transfer of a file
	{
		++doneFiles_;
	}

private:
	virtual void reportProgressNow();

private:

	int numFiles_;
	int startedFiles_;
	int	doneFiles_;
	int	numBytes_;
	int doneBytes_;
};

#endif 	// CONSOLIDATE_DBS__FILE_TRANSFER_PROGRESS_REPORTER_HPP
