/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "loginapp_config.hpp"

// These need to be defined before including server_app_option_macros.hpp
#define BW_CONFIG_CLASS LoginAppConfig
#define BW_CONFIG_PREFIX "loginApp/"
#include "server/server_app_option_macros.hpp"

// -----------------------------------------------------------------------------
// Section: General settings
// -----------------------------------------------------------------------------

BW_OPTION_RO( bool, shouldShutDownIfPortUsed, true );
BW_OPTION_RO( bool, verboseExternalInterface, false );
BW_OPTION( float, maxLoginDelay, 10.f );

BW_OPTION_RO( std::string, privateKey, "server/loginapp.privkey" );

BW_OPTION( bool, shutDownSystemOnExit, false );

BW_OPTION( bool, allowLogin, false );
BW_OPTION( bool, allowProbe, false );
BW_OPTION( bool, logProbes, true );

BW_OPTION_RO( bool, registerExternalInterface, false );
BW_OPTION( bool, allowUnencryptedLogins, false );

BW_OPTION( int, loginRateLimit, 0 );
BW_OPTION( int, rateLimitDuration, 0 );

// -----------------------------------------------------------------------------
// Section: Post initialisation
// -----------------------------------------------------------------------------

/**
 *
 */
bool LoginAppConfig::postInit()
{
	if (!ServerAppConfig::postInit() &&
			ExternalAppConfig::postInit())
	{
		return false;
	}

	BWConfig::update( "loginApp/externalLatencyMin", externalLatencyMin.getRef() );
	BWConfig::update( "loginApp/externalLatencyMax", externalLatencyMax.getRef() );
	BWConfig::update( "loginApp/externalLossRatio", externalLossRatio.getRef() );

	BWConfig::update( "loginApp/externalInterface", externalInterface.getRef() );

	return true;
}

// loginapp_config.cpp
