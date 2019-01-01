/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LIB__DBMGR_MYSQL__DATABASE_TOOL_APP
#define LIB__DBMGR_MYSQL__DATABASE_TOOL_APP

#include "dbmgr_lib/db_entitydefs.hpp"

#include "dbmgr_mysql/db_config.hpp"
#include "dbmgr_mysql/locked_connection.hpp"

#include "dbmgr_mysql/mappings/entity_type_mappings.hpp"

#include "network/event_dispatcher.hpp"

#include <memory>

namespace Mercury
{
	class EventDispatcher;
}
class LoggerMessageForwarder;
class MySqlLockedConnection;
class SignalHandler;
class WatcherNub;

class DatabaseToolApp
{
public:
	DatabaseToolApp();

	virtual ~DatabaseToolApp();

	virtual void onSignalled( int sigNum )
	{}

protected:
	Mercury::EventDispatcher & dispatcher()
		{ return eventDispatcher_; }

	MySql & connection()
		{ return *(pLockedConn_->connection()); }

	const EntityDefs & entityDefs() const
		{ return entityDefs_; }

	bool init( const char * appName, bool isVerbose, bool shouldLock=true, 
		const DBConfig::ConnectionInfo & connectionInfo = 
			DBConfig::connectionInfo() );
	
	bool connect( const DBConfig::ConnectionInfo & connectionInfo, 
		bool shouldLock );
	bool initLogger( const char * appName, const std::string & loggerID, 
			bool isVerbose );
	bool initScript();
	bool initEntityDefs();

	void enableSignalHandler( int sigNum, bool enable = true );

	WatcherNub & watcherNub()
		{ return *pWatcherNub_; }

// Member data
private:
	Mercury::EventDispatcher eventDispatcher_;

	std::auto_ptr< SignalHandler > pSignalHandler_;
	
	std::auto_ptr< WatcherNub >
							pWatcherNub_;
	std::auto_ptr< LoggerMessageForwarder > 
							pLoggerMessageForwarder_;

	std::auto_ptr< MySqlLockedConnection >
							pLockedConn_;
	EntityDefs 				entityDefs_;
};

#endif // LIB__DBMGR_MYSQL__DATABASE_TOOL_APP
