/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DB_STATUS_HPP
#define DB_STATUS_HPP

#include <string>

class Watcher;

#define DBSTATUS_WATCHER_STATUS_DETAIL_PATH "statusDetail"

class DBStatus
{
public:
	enum Status
	{
		STARTING,				// Starting up
		STARTUP_CONSOLIDATING,	// Running consolidation during start-up
		WAITING_FOR_APPS,		// Waiting for apps to become ready.
		RESTORING_STATE,		// Restoring entities, spaces, etc.
		RUNNING,				// Running
		SHUTTING_DOWN,			// Shutting down
		SHUTDOWN_CONSOLIDATING	// Running consolidation during shutdown.	
	};

public:
	DBStatus( Status status = STARTING, 
			const std::string& detail = "Starting" );

	Status status() const 				{ return Status( status_ ); }
	const std::string& detail() const 	{ return detail_; }
	void detail( const std::string& detail ) { detail_ = detail; }

	void registerWatchers( Watcher & watcher );

	void set( Status status, const std::string& detail );

	// Watchers
	bool hasStarted() const		{ return status_ == RUNNING; }

private:
	int			status_;
	std::string	detail_;
};

#endif 	// DB_STATUS_HPP
