/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MANAGER_APP_CONFIG_HPP
#define MANAGER_APP_CONFIG_HPP

#include "server_app_config.hpp"

/**
 *	This class contains the configuration options for an EntityApp.
 */
class ManagerAppConfig : public ServerAppConfig
{
public:
	static ServerAppOption< bool > shutDownServerOnBadState;
	static ServerAppOption< bool > shutDownServerOnBaseAppDeath;
	static ServerAppOption< bool > shutDownServerOnCellAppDeath;

protected:
	// static bool postInit() { return ServerAppConfig::postInit(); }
};

#endif // MANAGER_APP_CONFIG_HPP
