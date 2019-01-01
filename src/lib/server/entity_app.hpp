/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_APP_HPP
#define ENTITY_APP_HPP

#include "server_app.hpp"

#include "cstdmf/time_queue.hpp"

class ManagerAppGateway;
class Watcher;

#define ENTITY_APP_HEADER SERVER_APP_HEADER

/**
 *	This class is a comman base class for BaseApp and CellApp.
 */
class EntityApp : public ServerApp
{
public:
	EntityApp( Mercury::EventDispatcher & mainDispatcher,
			Mercury::NetworkInterface & interface );
	virtual ~EntityApp();

	virtual bool init( int argc, char * argv[] );

	virtual void requestRetirement();
	
	TimeQueue & timeQueue()			{ return timeQueue_; }

	virtual void onSignalled( int sigNum );

protected:
	void addWatchers( Watcher & watcher );
	void tickStats();

	virtual ManagerAppGateway & managerAppGateway() = 0;

private:
	TimeQueue timeQueue_;

	uint32 tickStatsPeriod_;
};

#endif // ENTITY_APP_HPP
