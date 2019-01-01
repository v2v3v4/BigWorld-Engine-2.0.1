/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "balance_config.hpp"

// These need to be defined before including server_app_option_macros.hpp
#define BW_CONFIG_CLASS BalanceConfig
#define BW_CONFIG_PREFIX ""
#define BW_COMMON_PREFIX "balance/"
#include "server_app_option_macros.hpp"

// -----------------------------------------------------------------------------
// Section: General configuration
// -----------------------------------------------------------------------------

BW_OPTION( float, maxCPUOffload, 0.02f );
BW_OPTION( int, minEntityOffload, 1 );
BW_OPTION( float, minMovement, 0.1f );
BW_OPTION( float, slowApproachFactor, 0.1f );

BW_OPTION_FULL( bool, demo, false, "balance/demo/enable",
		"config/balance/demo/enable" );
BW_OPTION_FULL( float, demoNumEntitiesPerCell, 100.f,
		"balance/demo/numEntitiesPerCell",
		"config/balance/demo/numEntitiesPerCell" );

#if 0
bool BalanceConfig::postInit()
{
	return true;
}
#endif

// balance_config.cpp
