/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "entity_app_config.hpp"

#include "network/compression_stream.hpp"
#include "server/common.hpp"

// -----------------------------------------------------------------------------
// Section: EntityAppConfig
// -----------------------------------------------------------------------------

ServerAppOption< int > EntityAppConfig::numStartupRetries(
		60, "numStartupRetries", "" );


bool EntityAppConfig::postInit()
{
	if (!ServerAppConfig::postInit())
	{
		return false;
	}

	if (!CompressionOStream::initDefaults(
				BWConfig::getSection( "networkCompression" ) ))
	{
		ERROR_MSG( "EntityAppConfig::postInit: "
				"Failed to initialise networkCompression settings.\n" );
		return false;
	}

	return true;
}

// entity_app_config.cpp
