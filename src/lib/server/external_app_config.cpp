/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "external_app_config.hpp"

// These need to be defined before including server_app_option_macros.hpp
#define BW_CONFIG_CLASS ExternalAppConfig
#define BW_CONFIG_PREFIX ""
#include "server_app_option_macros.hpp"

// -----------------------------------------------------------------------------
// Section: ExternalAppConfig
// -----------------------------------------------------------------------------

BW_OPTION_RO( float, externalLatencyMin, 0.f );
BW_OPTION_RO( float, externalLatencyMax, 0.f );
BW_OPTION_RO( float, externalLossRatio, 0.f );

BW_OPTION_RO( std::string, externalInterface, "" );

bool ExternalAppConfig::postInit()
{
	return true;
}

// external_app_config.cpp
