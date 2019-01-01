/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "Python.h"		// See http://docs.python.org/api/includes.html

#include "mysql_synchronise.hpp"

#include "cstdmf/debug.hpp"

#include "network/event_dispatcher.hpp"
#include "network/logger_message_forwarder.hpp"
#include "network/machined_utils.hpp"

#include "resmgr/bwresource.hpp"

#include "server/bwconfig.hpp"
#include "server/bwservice.hpp"


// -----------------------------------------------------------------------------
// Section: Main
// -----------------------------------------------------------------------------

static const char * USAGE_MESSAGE =
	"Usage: sync_db [options]\n\n"							
	"Options:\n"														
	" --verbose | -v      Display verbose program output to the console.\n"
	" --help              Program usage.\n";

int main( int argc, char * argv[] )
{
	bool isVerbose = false;
	bool shouldLock = true;

	for (int i = 1; i < argc; ++i)
	{
		if (((strcmp( argv[i], "--res" ) == 0) ||
			 (strcmp( argv[i], "-r" ) == 0)) && i < (argc - 1))
		{
			++i;
		}
		else if ((strcmp( argv[i], "--verbose" ) == 0) ||
				(strcmp( argv[i], "-v" ) == 0))
		{
			isVerbose = true;
		}
		else if (strcmp( argv[i], "--help" ) == 0)
		{
			puts( USAGE_MESSAGE );
			return EXIT_SUCCESS;
		}
		else if (strcmp( argv[i], "--run-from-dbmgr" ) == 0)
		{
			shouldLock = false;
		}
	}

	DebugFilter::shouldWriteToConsole( isVerbose );

	BWResource bwresource;
	BWResource::init( argc, const_cast< const char ** >( argv ) );
	BWConfig::init( argc, argv );

	MySqlSynchronise mysqlSynchronise;

	if (!mysqlSynchronise.init( isVerbose, shouldLock ))
	{
		ERROR_MSG( "Initialisation failed\n" );
		return EXIT_FAILURE;
	}

	START_MSG( "SyncDB" );

	if (!mysqlSynchronise.run())
	{
		ERROR_MSG( "Sync to database failed\n" );
		return EXIT_FAILURE;
	}

	INFO_MSG( "Sync to database successful\n" );

	return EXIT_SUCCESS;
}

// sync_db.cpp
