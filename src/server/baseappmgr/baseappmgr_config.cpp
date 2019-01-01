/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "baseappmgr_config.hpp"

#include "cstdmf/timestamp.hpp"

#define BW_CONFIG_CLASS BaseAppMgrConfig
#define BW_CONFIG_PREFIX "baseAppMgr/"
#include "server/server_app_option_macros.hpp"

// -----------------------------------------------------------------------------
// Section: General settings
// -----------------------------------------------------------------------------

BW_OPTION( float, baseAppOverloadLevel, 0.8f );
BW_OPTION( float, createBaseRatio, 4.f );
BW_OPTION_RO( float, updateCreateBaseInfoPeriod, 5.f );
DERIVED_BW_OPTION( int, updateCreateBaseInfoPeriodInTicks );

BW_OPTION( bool, hardKillDeadBaseApps, true );

BW_OPTION_RO( float, baseAppTimeout, 5.f );
DERIVED_BW_OPTION( uint64, baseAppTimeoutInStamps );

BW_OPTION_RO( float, overloadTolerancePeriod, 5.f );
DERIVED_BW_OPTION( uint64, overloadTolerancePeriodInStamps );

BW_OPTION( int, overloadLogins, 10 );


// -----------------------------------------------------------------------------
// Section: Post initialisation
// -----------------------------------------------------------------------------

/**
 *
 */
bool BaseAppMgrConfig::postInit()
{
	bool result = ManagerAppConfig::postInit();

	updateCreateBaseInfoPeriodInTicks.set(
			secondsToTicks( updateCreateBaseInfoPeriod(), 1 ) );

	baseAppTimeoutInStamps.set(
			uint64( stampsPerSecondD() * baseAppTimeout() ) );
	overloadTolerancePeriodInStamps.set(
			uint64( stampsPerSecondD() * overloadTolerancePeriod() ) );

	return result;
}

// baseappmgr_config.cpp
