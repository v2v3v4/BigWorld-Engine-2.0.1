/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MF_CONSOLIDATE_DBMGR_HPP
#define MF_CONSOLIDATE_DBMGR_HPP

#include "network/basictypes.hpp"
#include "network/machine_guard.hpp"
#include "network/watcher_nub.hpp"

#include <string>

/**
 *
 */
class DBMgr : public MachineGuardMessage::ReplyHandler
{
public:
	DBMgr( WatcherNub & watcherNub );

	bool init();

	void setStatus( const std::string & status );

	// MachineGuardMessage::ReplyHandler interface
	bool onProcessStatsMessage( ProcessStatsMessage & psm, uint32 addr );

private:
	WatcherNub &		watcherNub_;
	Mercury::Address	addr_;
};

#endif // MF_CONSOLIDATE_DBMGR_HPP
