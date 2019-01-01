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

#include "chunk/chunk.hpp"
#include "cstdmf/bw_util.hpp"
#include "cstdmf/memory_tracker.hpp"

#include "entitydef/data_description.hpp"

#include "pyscript/py_import_paths.hpp"
#include "pyscript/py_output_writer.hpp"
#include "pyscript/script.hpp"

#include "resmgr/bwresource.hpp"

#include "unit_test_lib/unit_test.hpp"

#include <string>

extern int Math_token;
extern int ResMgr_token;
extern int PyScript_token;
extern int ChunkUserDataObject_token;
extern int PyUserDataObject_token;
extern int UserDataObjectDescriptionMap_Token;

int s_tokens =
	Math_token |
	ResMgr_token |
	PyScript_token |
	ChunkUserDataObject_token |
	PyUserDataObject_token |
	UserDataObjectDescriptionMap_Token;


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

	PyOutputWriter::overrideSysMembers( false );

	int returnValue = BWUnitTest::runTest( "entitydef", argc, argv );
	Script::fini();

	// Hmmm... it would be good not to require these
	MetaDataType::fini();

	// Double hmmm...
	Chunk::fini();
	DebugFilter::fini();

	return returnValue;
}

// main.cpp
