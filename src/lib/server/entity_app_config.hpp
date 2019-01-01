/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_APP_CONFIG_HPP
#define ENTITY_APP_CONFIG_HPP

#include "server_app_config.hpp"

/**
 *	This class contains the configuration options for an EntityApp.
 */
class EntityAppConfig : public ServerAppConfig
{
public:
	static ServerAppOption< int > numStartupRetries;

protected:
	static bool postInit();
};

#endif // ENTITY_APP_CONFIG_HPP
