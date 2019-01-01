/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "server/server_app_config.hpp"

class ReviverConfig : public ServerAppConfig
{
public:
	static ServerAppOption< float > reattachPeriod;
	static ServerAppOption< float > pingPeriod;
	static ServerAppOption< bool > shutDownOnRevive;
	static ServerAppOption< int > timeoutInPings;

	// static bool postInit();
};

// reviver_config.hpp
