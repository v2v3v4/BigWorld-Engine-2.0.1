/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MANAGER_APP_HPP
#define MANAGER_APP_HPP

#include "server_app.hpp"

#define MANAGER_APP_HEADER SERVER_APP_HEADER

/**
 *	This class is a comman base class for BaseAppMgr and CellAppMgr.
 */
class ManagerApp : public ServerApp
{
public:
	ManagerApp( Mercury::EventDispatcher & mainDispatcher,
			Mercury::NetworkInterface & interface );

protected:
	void addWatchers( Watcher & watcher );

private:
};

#endif // MANAGER_APP_HPP
