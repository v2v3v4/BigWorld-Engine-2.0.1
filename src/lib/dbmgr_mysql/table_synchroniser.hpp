/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LIB__DBMGR_MYSQL__TABLE_SYNCHRONISER_HPP
#define LIB__DBMGR_MYSQL__TABLE_SYNCHRONISER_HPP

#include <unistd.h>

#include <string>

namespace Mercury
{
	class EventDispatcher;
} // end namespace Mercury


/**
 *	Class to run sync_db.
 */
class TableSynchroniser
{
public:
	/**
	 *	Constructor.
	 */
	TableSynchroniser():
		pid_( -1 ),
		running_( false ),
		status_( 0 )
	{}

	~TableSynchroniser();

	bool run( Mercury::EventDispatcher & dispatcher );
	
	void onProcessExited( int status );

	void abort();

	bool isFinished() const
		{ return pid_ != -1 && running_ == false; }

	bool wasSuccessful() const
	{ 
		return this->didExit() && 
			this->exitCode() == 0; 
	}
	
	bool didExit() const;
	int exitCode() const;

	bool wasSignalled() const;
	int signal() const;

private:
	void execSyncDB( const std::string & syncdbPath );

	pid_t pid_;
	
	bool running_;
	int status_;
};

#endif // LIB__DBMGR_MYSQL__TABLE_SYNCHRONISER_HPP
