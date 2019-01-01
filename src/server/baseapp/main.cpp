/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "baseapp.hpp"
#include "baseapp_config.hpp"

#include "server/bwservice.hpp"

int BIGWORLD_MAIN( int argc, char * argv[] )
{
	return bwMainT< BaseApp >( argc, argv );
}

// main.cpp
