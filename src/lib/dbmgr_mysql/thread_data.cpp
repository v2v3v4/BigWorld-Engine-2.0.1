/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "thread_data.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/time_queue.hpp"

#include "connection_info.hpp"

#include "network/event_dispatcher.hpp"

DECLARE_DEBUG_COMPONENT( 0 );

// -----------------------------------------------------------------------------
// Section: MySqlThreadData
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
MySqlThreadData::MySqlThreadData( const DBConfig::ConnectionInfo & connInfo ) :
	pConnection_( NULL ),
	connectionInfo_( connInfo )
{
}


/**
 *	This method is called in the background thread when the thread has just
 *	started.
 */
void MySqlThreadData::onStart( BackgroundTaskThread & thread )
{
	mysql_thread_init();
	pConnection_ = new MySql( connectionInfo_ );
}


/**
 *	This method is called in the background thread when the thread is just about
 *	to stop.
 */
void MySqlThreadData::onEnd( BackgroundTaskThread & thread )
{
	delete pConnection_;
	pConnection_ = NULL;
	mysql_thread_end();
}


/**
 *	This method will attempt to reconnect this thread to the database.
 */
bool MySqlThreadData::reconnect()
{
	return pConnection_ ? pConnection_->reconnectTo( connectionInfo_ ) : false;
}


// thread_data.cpp
