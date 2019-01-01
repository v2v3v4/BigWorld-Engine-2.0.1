/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "logger.hpp"

#include "cstdmf/dprintf.hpp"

#include "server/bwservice.hpp"

#include <signal.h>

DECLARE_DEBUG_COMPONENT( 0 )

// ----------------------------------------------------------------------------
// Section: Signal handlers
// ----------------------------------------------------------------------------

namespace
{
Logger * gLogger;
bool g_finished = false;

class SetWriteToConsoleStaticInitialiser
{
public:
	SetWriteToConsoleStaticInitialiser( bool value )
		{ DebugFilter::shouldWriteToConsole( value ); }
};

SetWriteToConsoleStaticInitialiser s_setWriteToConsoleStaticInitialiser( false );

}


void sigint( int /* sig */ )
{
	g_finished = true;
}


void sighup( int /* sig */ )
{
	if (gLogger != NULL)
	{
		gLogger->shouldRoll( true );
	}
}


// ----------------------------------------------------------------------------
// Section: Main
// ----------------------------------------------------------------------------

int BIGWORLD_MAIN( int argc, char * argv[] )
{
	Logger logger;
	gLogger = &logger;

	signal( SIGHUP, sighup );
	signal( SIGINT, sigint );
	signal( SIGTERM, sigint );

	// Enable error messages to go to syslog
	DebugMsgHelper::shouldWriteToSyslog( true );
	INFO_MSG( "---- Logger is running ----\n" );

	if (!logger.init( argc, argv ))
	{
		return 1;
	}

	while (!g_finished)
	{
		logger.handleNextMessage();
	}

	return 0;
}

// main.cpp
