/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "reviver.hpp"
#include "reviver_config.hpp"

// Needs to be after bwStop
#include "server/bwservice.hpp"


/**
 *	This method prints the usage of this program.
 */
void printHelp( const char * commandName )
{
	printf( "\n\n" );
	printf( "Usage: %s [OPTION]\n", commandName );
	printf(
"Monitors BigWorld server components and spawns a new process if a component\n"
"fails.\n"
"\n"
"  --add {baseAppMgr|cellAppMgr|dbMgr|loginApp}\n"
"  --del {baseAppMgr|cellAppMgr|dbMgr|loginApp}\n"
"\n" );

	printf(
"For example, the following monitors the DBMgr process and starts a new\n"
"instance if that one fails.\n"
"  %s --add dbMgr\n\n",
	 commandName );
}

int BIGWORLD_MAIN( int argc, char * argv[] )
{
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "--help" ) == 0)
		{
			printHelp( argv[0] );
			return 0;
		}
	}

	return bwMainT< Reviver >( argc, argv );
}

// main.cpp
