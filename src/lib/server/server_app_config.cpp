/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "server_app_config.hpp"

#include "cstdmf/timestamp.hpp"
#include "pyscript/personality.hpp"

// These need to be defined before including server_app_option_macros.hpp
#define BW_CONFIG_CLASS ServerAppConfig
#define BW_CONFIG_PREFIX ""
#include "server_app_option_macros.hpp"

// -----------------------------------------------------------------------------
// Section: ServerAppConfig
// -----------------------------------------------------------------------------

ServerAppOption< int > ServerAppConfig::updateHertz( DEFAULT_GAME_UPDATE_HERTZ,
			"gameUpdateHertz", "gameUpdateHertz", Watcher::WT_READ_ONLY );

BW_OPTION_RO( std::string, personality, DEFAULT_PERSONALITY_NAME );

ServerAppOption< bool >
	ServerAppConfig::isProduction( true, "production", "isProduction" );

BW_OPTION_RO( float, timeSyncPeriod, 60.f );
DERIVED_BW_OPTION( int, timeSyncPeriodInTicks );

BW_OPTION_RO( bool, useDefaultSpace, false );

/**
 *	This method initialises the configuration options of a ServerApp derived
 *	App. This is called by the bwMainT< APP >() template method.
 */
bool ServerAppConfig::init( bool (*postInitFn)() )
{
	ServerAppOptionIniter::initAll();
	bool result = (*postInitFn)();
	ServerAppOptionIniter::printAll();
	ServerAppOptionIniter::deleteAll();

	return result;
}


//-----------------------------------------------------------------------------
// Section: Post initialisation
//-----------------------------------------------------------------------------

/**
 *	This method is called just after initialisation but before the options are
 *	printed or used.
 */
bool ServerAppConfig::postInit()
{
	timeSyncPeriodInTicks.set( secondsToTicks( timeSyncPeriod(), 1 ) );

	return true;
}

// -----------------------------------------------------------------------------
// Section: Helper functions
// -----------------------------------------------------------------------------

/**
 *	This method converts a time duration from seconds to ticks.
 *
 *	@param seconds The number of seconds to convert.
 *	@param lowerBound A lower bound on the resulting value.
 */
int ServerAppConfig::secondsToTicks( float seconds, int lowerBound )
{
	return std::max( lowerBound,
			int( floorf( seconds * updateHertz() + 0.5f ) ) );
}


/**
 *	This method converts a time duration from seconds to stamps. Stamps are the
 *	units used by timestamp().
 */
uint64 ServerAppConfig::secondsToStamps( float seconds )
{
	return uint64( seconds * stampsPerSecondD() );
}

// server_app_config.cpp
