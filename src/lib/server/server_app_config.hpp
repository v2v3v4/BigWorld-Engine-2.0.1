/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SERVER_APP_CONFIG_HPP
#define SERVER_APP_CONFIG_HPP

#include "server_app_option.hpp"

/**
 *	This class contains the configuration options for a ServerApp.
 */
class ServerAppConfig
{
public:
	static ServerAppOption< int > updateHertz;

	static ServerAppOption< std::string > personality;
	static ServerAppOption< bool > isProduction;

	static ServerAppOption< float > timeSyncPeriod;
	static ServerAppOption< int > timeSyncPeriodInTicks;

	static ServerAppOption< bool > useDefaultSpace;

	static bool postInit();
	static bool init( bool (*postInitFn)() );

	// Helper functions
	static uint64 secondsToStamps( float seconds );
	static int secondsToTicks( float seconds, int lowerBound );
	static float expectedTickPeriod() { return 1.f/updateHertz(); }

private:
	ServerAppConfig();
};

#endif // SERVER_APP_CONFIG_HPP
