/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "unit_test_lib/unit_test.hpp"
#include "cstdmf/cstdmf.hpp"
#include "cstdmf/memory_tracker.hpp"
#include "cstdmf/timestamp.hpp"

int main( int argc, char* argv[] )
{
	new CStdMf;

#ifdef ENABLE_MEMTRACKER
	MemTracker::instance().setReportOnExit( false );
#endif

	// Initialise stampsPerSecond and timing method
	stampsPerSecond();

	// Turn off output for network testing to avoid the spam generated.
	// DebugFilter::instance().filterThreshold( MESSAGE_PRIORITY_ERROR );

	int ret = BWUnitTest::runTest( "network", argc, argv );

#ifdef ENABLE_MEMTRACKER
	MemTracker::instance().setReportOnExit( true );
	MemTracker::instance().setCrashOnLeak( true );
#endif

	delete CStdMf::pInstance();

	return ret;
}

// main.cpp
