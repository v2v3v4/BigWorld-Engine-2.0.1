/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "stdafx.h"

#include "unit_test_lib/unit_test.hpp"

int 		g_cmdArgC;
char ** 	g_cmdArgV;

int main( int argc, char* argv[] )
{
	// saved away for the test harness
	g_cmdArgC = argc;
	g_cmdArgV = argv;

	return BWUnitTest::runTest( "resmgr", argc, argv );
}

// main.cpp
