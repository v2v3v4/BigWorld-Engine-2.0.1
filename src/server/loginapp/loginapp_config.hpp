/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "server/external_app_config.hpp"
#include "server/server_app_config.hpp"

class LoginAppConfig :
	public ServerAppConfig,
	public ExternalAppConfig
{
public:
	static ServerAppOption< bool > shouldShutDownIfPortUsed;
	static ServerAppOption< bool > verboseExternalInterface;
	static ServerAppOption< float > maxLoginDelay;

	static uint64 maxLoginDelayInStamps()
	{
		return secondsToStamps( maxLoginDelay() );
	}

	static ServerAppOption< std::string > privateKey;
	static ServerAppOption< bool > shutDownSystemOnExit;

	static ServerAppOption< bool > allowLogin;
	static ServerAppOption< bool > allowProbe;
	static ServerAppOption< bool > logProbes;

	static ServerAppOption< bool > registerExternalInterface;
	static ServerAppOption< bool > allowUnencryptedLogins;

	static ServerAppOption< int > loginRateLimit;
	static ServerAppOption< int > rateLimitDuration;

	static uint64 rateLimitDurationInStamps()
	{
		return secondsToStamps( rateLimitDuration() );
	}

	static bool rateLimitEnabled()
	{
		return (rateLimitDuration() > 0);
	}

	static bool postInit();
};

// loginapp_config.hpp
