/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "entity_app.hpp"

#include "entity_app_config.hpp"
#include "manager_app_gateway.hpp"

#include "cstdmf/watcher.hpp"

#include "entitydef/entity_member_stats.hpp"

#include "server/bwconfig.hpp"
#include "server/common.hpp"
#include "server/script_timers.hpp"

#include <signal.h>

namespace
{

class RetireAppCommand : public NoArgCallableWatcher
{
public:
	RetireAppCommand( EntityApp & app ) :
		NoArgCallableWatcher( CallableWatcher::LEAST_LOADED,
				"Retire the least loaded app." ),
		app_( app )
	{
	}

private:
	virtual bool onCall( std::string & output, std::string & value )
	{
		INFO_MSG( "Requesting to retire this app.\n" );
		app_.requestRetirement();
		return true;
	}

	EntityApp & app_;
};

} // anonymous namespace

// -----------------------------------------------------------------------------
// Section: EntityApp
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
EntityApp::EntityApp( Mercury::EventDispatcher & mainDispatcher,
			Mercury::NetworkInterface & interface ) :
	ServerApp( mainDispatcher, interface ),
	timeQueue_(),
	tickStatsPeriod_( 0 ) // Set below
{
	ScriptTimers::init( *this );

	float tickStatsPeriod = BWConfig::get( "tickStatsPeriod", 2.f );
	tickStatsPeriod_ =
		uint32( tickStatsPeriod * EntityAppConfig::updateHertz() );
}


/**
 *	Destructor.
 */
EntityApp::~EntityApp()
{
	ScriptTimers::fini( *this );
}


/**
 * 	Initialisation function.
 *
 *	This must be called from subclasses's init().
 */
bool EntityApp::init( int argc, char * argv[] )
{
	if (!this->ServerApp::init( argc, argv ))
	{
		return false;
	}

	this->enableSignalHandler( SIGQUIT );
	return true;
}


/**
 *	This method requests that this application should be retired.
 */
void EntityApp::requestRetirement()
{
	this->managerAppGateway().retireApp();
}


/**
 *	This method adds the watchers associated with this class.
 */
void EntityApp::addWatchers( Watcher & watcher )
{
	ServerApp::addWatchers( watcher );

	char buf[ 256 ];
	snprintf( buf, sizeof( buf ), "command/retire%s",
			this->getAppName() );

	watcher.addChild( buf, new RetireAppCommand( *this ) );
}


/**
 *	This method ticks any stats that maintain a moving average.
 */
void EntityApp::tickStats()
{
	if (tickStatsPeriod_ > 0)
	{
		AUTO_SCOPED_PROFILE( "tickStats" );
		EntityMemberStats::Stat::tickSome( time_ % tickStatsPeriod_,
				tickStatsPeriod_,
				double( tickStatsPeriod_ ) / EntityAppConfig::updateHertz() );
	}
}


/**
 *	Signal handler.
 *
 *	This should be called from subclasses' onSignalled() overrides.
 */
void EntityApp::onSignalled( int sigNum )
{
	this->ServerApp::onSignalled( sigNum );
	switch (sigNum)
	{
	case SIGQUIT:
		CRITICAL_MSG( "Received QUIT signal. This is likely caused by the "
				"%sMgr killing this %s because it has been "
				"unresponsive for too long. Look at the callstack from "
				"the core dump to find the likely cause.\n",
			this->getAppName(), this->getAppName() );
	default: break;
	}
}


// entity_app.hpp
