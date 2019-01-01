/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "unit_test_lib/unit_test.hpp"

// TODO: Temporary hack until a library is made for Windows.
#ifdef _WIN32
#include "unit_test_lib/unit_test.cpp"
#endif

int main( int argc, char* argv[] )
{
	return BWUnitTest::runTest( "unit_test", argc, argv );
}

// main.cpp
