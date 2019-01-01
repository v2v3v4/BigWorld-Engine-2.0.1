/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "manager_app_config.hpp"

// These need to be defined before including server_app_option_macros.hpp
#define BW_CONFIG_CLASS ManagerAppConfig
#define BW_CONFIG_PREFIX ""
#include "server_app_option_macros.hpp"

// -----------------------------------------------------------------------------
// Section: ManagerAppConfig
// -----------------------------------------------------------------------------

BW_OPTION( bool, shutDownServerOnBadState, true );
BW_OPTION( bool, shutDownServerOnBaseAppDeath, false );
BW_OPTION( bool, shutDownServerOnCellAppDeath, false );

//  manager_app_config.cpp
