/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "dbmgr_config.hpp"

#define BW_CONFIG_CLASS DBMgrConfig
#define BW_CONFIG_PREFIX "dbMgr/"
#include "server/server_app_option_macros.hpp"

// -----------------------------------------------------------------------------
// Section: General settings
// -----------------------------------------------------------------------------

BW_OPTION( bool, allowEmptyDigest, false );

BW_OPTION_RO( int, dumpEntityDescription, 0 );

BW_OPTION_AT( uint32, desiredBaseApps, 1, "" );
BW_OPTION_AT( uint32, desiredCellApps, 1, "" );

BW_OPTION( float, overloadLevel, 1.f );

BW_OPTION( float, overloadTolerancePeriod, 5.f );

BW_OPTION_RO( std::string, type, "xml" );


/**
 *
 */
bool DBMgrConfig::postInit()
{
	bool result = ServerAppConfig::postInit();

	return result;
}

// dbmgr_config.cpp
