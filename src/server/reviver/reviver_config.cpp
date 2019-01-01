/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "reviver_config.hpp"

#define BW_CONFIG_CLASS ReviverConfig
#define BW_CONFIG_PREFIX "reviver/"
#include "server/server_app_option_macros.hpp"

// -----------------------------------------------------------------------------
// Section: General options
// -----------------------------------------------------------------------------

BW_OPTION_RO( float, reattachPeriod, 10.f );
BW_OPTION( float, pingPeriod, 0.1f );
BW_OPTION( bool, shutDownOnRevive, true );
BW_OPTION( int, timeoutInPings, 5 );


#if 0
/**
 *
 */
bool ReviverConfig::postInit()
{
	bool result = ServerAppConfig::postInit();

	return result;
}
#endif

// reviver_config.cpp
