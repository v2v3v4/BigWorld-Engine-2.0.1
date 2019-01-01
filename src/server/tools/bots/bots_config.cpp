/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "bots_config.hpp"

// These need to be defined before including server_app_option_macros.hpp
#define BW_CONFIG_CLASS BotsConfig
#define BW_CONFIG_PREFIX "bots/"
#include "server/server_app_option_macros.hpp"

// -----------------------------------------------------------------------------
// Section: Standard options
// -----------------------------------------------------------------------------

BW_OPTION	( std::string, 		username, 				"Bot" )
BW_OPTION	( std::string, 		password, 				"" )
BW_OPTION	( std::string, 		serverName, 			"" )
BW_OPTION	( std::string, 		tag,		 			"Default" )
BW_OPTION	( uint16, 			port, 					0 )
BW_OPTION	( bool, 			shouldUseRandomName, 	true )
BW_OPTION	( bool, 			shouldUseScripts, 		true )
BW_OPTION	( std::string, 		standinEntity, 			"DefaultEntity" )
BW_OPTION	( std::string, 		controllerType, 		"Patrol" )
BW_OPTION	( std::string, 		controllerData, 		"server/bots/test.bwp" )
BW_OPTION	( std::string,		publicKey, 				"" )
BW_OPTION	( std::string,		loginMD5Digest, 		"" )



// bots_config.cpp
