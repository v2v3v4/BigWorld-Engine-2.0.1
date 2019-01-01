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

#include "cstdmf/bw_util.hpp"
#include "cstdmf/memory_tracker.hpp"
#include "resmgr/bwresource.hpp"
#include "pyscript/py_import_paths.hpp"
#include "pyscript/script.hpp"
#include "unit_test_lib/unit_test.hpp"

#include <string>

int main( int argc, char* argv[] )
{
#ifdef ENABLE_MEMTRACKER
	MemTracker::instance().setCrashOnLeak( true );
#endif

	BWResource bwresource;

	std::string binDir = BWUtil::executableDirectory();
	binDir += "/../../bigworld/res";

	const char *resPaths[3] = { "--res", binDir.c_str(), NULL };
	if (!BWResource::init( ARRAY_SIZE( resPaths ), resPaths ))
	{
		fprintf( stderr, "Could not initialise BWResource\n" );
		return 1;
	}

	// TODO: Could be good to support mutliply start and stopping the Script
	// module.
	PyImportPaths importPaths;
	importPaths.addPath( "." );

	if (!Script::init( importPaths ))
	{
		fprintf( stderr, "Could not initialise Script module" );
		return 1;
	}

	int returnValue = BWUnitTest::runTest( "pyscript", argc, argv );
	Script::fini();

	// TODO: It would be good to not require these.
	BWResource::fini();
	DebugFilter::fini();

	return returnValue;
}

// main.cpp
