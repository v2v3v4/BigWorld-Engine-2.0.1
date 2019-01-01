/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "command_line_parser.hpp"
#include "consolidate_dbs_app.hpp"

#include "cstdmf/dprintf.hpp"
#ifdef ENABLE_MEMTRACKER
#include "cstdmf/memory_tracker.hpp"
#endif

#include "resmgr/bwresource.hpp"

#include "server/bwconfig.hpp"
#include "server/bwservice.hpp"


DECLARE_DEBUG_COMPONENT( 0 )


int main( int argc, char * argv[] )
{
#ifdef ENABLE_MEMTRACKER
	g_memTracker.setReportOnExit( false );
#endif

	BWResource bwresource;
	BWResource::init( argc, (const char **)argv );
	BWConfig::init( argc, argv );

	if (BWConfig::isBad())
	{
		return EXIT_SUCCESS;
	}

	CommandLineParser options;
	if (!options.init( argc, argv ))
	{
		ERROR_MSG( "Failed to parse command line options.\n" );
		return EXIT_FAILURE;
	}

	DebugFilter::shouldWriteToConsole( options.isVerbose() );

	ConsolidateDBsApp app( !options.shouldIgnoreSqliteErrors() );

	bool shouldReportToDBMgr = options.hadNonOptionArgs();

	if (!app.init( options.isVerbose(), shouldReportToDBMgr,
			options.primaryDatabaseInfo() ))
	{
		return EXIT_FAILURE;
	}

	START_MSG( "ConsolidateDBs" );

	if (options.shouldClear())
	{
		if (app.clearSecondaryDBEntries())
		{
			return EXIT_SUCCESS;
		}
		else
		{
			ERROR_MSG( "Failed to clear secondary databases\n" );
			return EXIT_FAILURE;
		}
	}

	if (!app.checkPrimaryDBEntityDefsMatch())
	{
		return EXIT_FAILURE;
	}

	if (options.hadNonOptionArgs())
	{
		// We were run with the secondary databases already transferred by the
		// snapshot_helper and are specified in a file list, the path to which
		// is specified on the command line.
		return app.consolidateSecondaryDBs( options.secondaryDatabases() ) ?
			EXIT_SUCCESS : EXIT_FAILURE;
	}
	else
	{
		// We were run with no specific primary database or secondary
		// databases specified, the consolidator must transfer those that are
		// registered in the primary database, before consolidating.
		return app.transferAndConsolidate() ? EXIT_SUCCESS : EXIT_FAILURE;
	}
}

// main.cpp
