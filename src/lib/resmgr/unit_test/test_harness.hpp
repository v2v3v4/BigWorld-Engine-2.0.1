/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TEST_HARNESS_HPP
#define TEST_HARNESS_HPP

#include "cstdmf/memory_tracker.hpp"
#include "resmgr/bwresource.hpp"

#ifdef linux
#include <libgen.h>
#endif

#include <string>


// Saved away in main.cpp
extern int 		g_cmdArgC;
extern char ** 	g_cmdArgV; 

class ResMgrUnitTestHarness
{
public:
	ResMgrUnitTestHarness():
		isOK_( false )
	{
#ifdef ENABLE_MEMTRACKER
		MemTracker::instance().setCrashOnLeak( true );
#endif

#ifdef linux
		this->changeToExecutableDir( g_cmdArgV[0] );
#endif

		new BWResource();

		// If res arguments are not provided then create our own res paths.
		bool resPathSpecified = false;
		for (int i = 1; i < g_cmdArgC; ++i)
		{
			if (strncmp( g_cmdArgV[i], "--res", 5 ) == 0)
			{
				resPathSpecified = true;
			}
		}
		if (!resPathSpecified)
		{
			const char * myResPath = "../../bigworld/src/lib/resmgr/unit_test/res/";
			const char *myargv[] =
			{
				"resmgr_unit_test",
				"--res",
				myResPath
			};
			int myargc = ARRAY_SIZE( myargv );
			myargv[0] = g_cmdArgV[0]; // patch in the real application


			if (!BWResource::init( myargc, myargv ))
			{
				fprintf( stderr, "could not initialise BWResource\n" );
				return;
			}
		}
		// If arguments are provided then use the supplied res paths.
		else
		{
			if (!BWResource::init( g_cmdArgC, (const char**)g_cmdArgV ))
			{
				fprintf( stderr, "could not initialise BWResource\n" );
				return;
			}
		}

		isOK_ = true;
	}

	virtual ~ResMgrUnitTestHarness() 
	{
		delete BWResource::pInstance();
		BWResource::fini();

		DebugMsgHelper::fini();
	}

	bool isOK() const { return isOK_; }

private:
	
#ifdef linux
	static void changeToExecutableDir( const char * arg0 )
	{
		// dirname may modify its argument
		char * absPathCopy = NULL;
		const char * dirName = NULL;
		
		if (*arg0 != '/')
		{
			// convert to absolute path name
			std::string absPath( ::getenv( "PWD" ) );
			absPath += "/";
			absPath += arg0;
			absPathCopy = strdup( absPath.c_str() );
		}
		else
		{
			absPathCopy = strdup( arg0 );
		}
		
		dirName = dirname( absPathCopy );
		chdir( dirName );

		free( absPathCopy );
	}
#endif

	bool isOK_;
};

#endif // TEST_HARNESS_HPP
