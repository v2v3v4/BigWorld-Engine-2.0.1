/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "dbmgr.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/watcher.hpp"

#include "dbmgr_lib/db_status.hpp"

#include "network/basictypes.hpp"
#include "network/machine_guard.hpp"
#include "network/watcher_nub.hpp"

#include <string>


DBMgr::DBMgr( WatcherNub & watcherNub ) :
	watcherNub_( watcherNub )
{
}


/**
 *	Initialise this DBMgr instance.
 */
bool DBMgr::init()
{
	ProcessStatsMessage	psm;
	psm.param_ = ProcessMessage::PARAM_USE_CATEGORY |
				ProcessMessage::PARAM_USE_UID |
				ProcessMessage::PARAM_USE_NAME;
	psm.category_ = psm.WATCHER_NUB;
	psm.uid_ = getUserId();
	psm.name_ = "dbmgr";

	// onProcessStatsMessage() will be called inside sendAndRecv().
	if (psm.sendAndRecv( 0, BROADCAST, this ) != Mercury::REASON_SUCCESS)
	{
		ERROR_MSG( "DBMgr::DBMgr: Unable to query BWMachined.\n" );
		return false;
	}
	else if (addr_.isNone())
	{
		INFO_MSG( "DBMgr::DBMgr: No DBMgr process found.\n" );
		return false;
	}

	return true;
}


/**
 * 	Sets DBMgr detailed status watcher.
 */
void DBMgr::setStatus( const std::string & status )
{
	if (addr_.isNone())
	{
		return;
	}

	MemoryOStream	strm( status.size() + 32 );
	// Stream on WatcherDataMsg
	strm << int( WATCHER_MSG_SET2 ) << int( 1 ); // message type and count
	strm << uint32( 0 );	// Sequence number. We don't care about it.
	// Add watcher path
	strm.addBlob( DBSTATUS_WATCHER_STATUS_DETAIL_PATH,
			strlen( DBSTATUS_WATCHER_STATUS_DETAIL_PATH ) + 1 );
	// Add data
	strm << uchar( WATCHER_TYPE_STRING );
	strm << status;

	watcherNub_.udpSocket().sendto( strm.data(), strm.size(),
			addr_.port, addr_.ip );
}


/**
 *	This method is called to provide us with information about the DBMgr
 * 	running on our cluster.
 */
bool DBMgr::onProcessStatsMessage( ProcessStatsMessage & psm, uint32 addr )
{
	// DBMgr not found on the machine
	if (psm.pid_ == 0)
	{
		return true;
	}

	if (addr_.isNone())
	{
		addr_.ip = addr;
		addr_.port = psm.port_;

		TRACE_MSG( "DBMgr::onProcessStatsMessage: "
				"Found DBMgr at %s\n",
			addr_.c_str() );
	}
	else
	{
		Mercury::Address dbMgrAddr( addr, psm.port_ );

		WARNING_MSG( "DBConsolidator::onProcessStatsMessage: "
				"Already found a DBMgr. Ignoring DBMgr at %s\n",
			dbMgrAddr.c_str() );
	}

	return true;
}

// dbmgr.cpp
