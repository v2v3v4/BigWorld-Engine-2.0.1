/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EXTERNAL_APP_CONFIG_HPP
#define EXTERNAL_APP_CONFIG_HPP

#include "server_app_option.hpp"

/**
 *	This class contains the configuration options for an EntityApp.
 */
class ExternalAppConfig
{
public:
	static ServerAppOption< float > externalLatencyMin;
	static ServerAppOption< float > externalLatencyMax;
	static ServerAppOption< float > externalLossRatio;

	static ServerAppOption< std::string > externalInterface;

protected:
	static bool postInit();
};

#endif // EXTERNAL_APP_CONFIG_HPP
