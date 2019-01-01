/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "nat_config.hpp"

// These need to be defined before including server_app_option_macros.hpp
#define BW_CONFIG_CLASS NATConfig
#define BW_CONFIG_PREFIX "networkAddressTranslation/"
#include "server/server_app_option_macros.hpp"

// -----------------------------------------------------------------------------
// Section: General settings
// -----------------------------------------------------------------------------

BW_OPTION_RO( std::string, localNetMask, "" );
BW_OPTION_RO( std::string, externalAddress, "" );

// nat_config.cpp
