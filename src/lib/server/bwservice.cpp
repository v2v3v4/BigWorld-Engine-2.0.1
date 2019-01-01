/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// #include "bwservice.hpp"
#include "cstdmf/dprintf.hpp"
#include "cstdmf/watcher.hpp"

void bwParseCommandLine( int argc, char **argv )
{
	MF_WATCH( "config/shouldWriteToConsole", DebugFilter::shouldWriteToConsole );

	for (int i=1; i < argc; i++)
	{
		if (strcmp( argv[i], "-machined" ) == 0)
		{
			DebugFilter::shouldWriteToConsole( false );
		}
	}
}

// bwservice.cpp
