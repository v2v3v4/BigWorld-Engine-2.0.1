/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "db_status.hpp"

#include "cstdmf/watcher.hpp"

/**
 * 	Initialiser
 */
DBStatus::DBStatus( Status status, const std::string& detail ) :
	status_( status ),
	detail_( detail )
{}


/**
 * 	Register our watchers.
 */
void DBStatus::registerWatchers( Watcher & watcher )
{
	WatcherPtr pWatcher =
		makeWatcher( &DBStatus::status_, Watcher::WT_READ_ONLY );
	pWatcher->setComment( "Status of this process. Mainly relevant during "
			"startup and shutdown" );
	watcher.addChild( "status", pWatcher, this );

	pWatcher = makeWatcher( &DBStatus::detail_ );
	pWatcher->setComment(
			"Human readable information about the current status of this "
			"process. Mainly relevant during startup and shutdown." );
	watcher.addChild( DBSTATUS_WATCHER_STATUS_DETAIL_PATH,
			pWatcher, this );

	watcher.addChild( "hasStarted",
			makeWatcher( &DBStatus::hasStarted ), this );
}


/**
 * 	Sets the current status
 */
void DBStatus::set( Status status, const std::string& detail )
{
	status_ = status;
	detail_ = detail;
}

// db_status.cpp
