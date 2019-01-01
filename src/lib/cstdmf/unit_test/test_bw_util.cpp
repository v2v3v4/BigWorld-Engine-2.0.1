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

#include "cstdmf/debug.hpp"
#include "cstdmf/bw_util.hpp"

#include <stdio.h>


TEST( FormatPath )
{
	std::string validPath( "/home/tester/mf/" );
	std::string workingPathWithout( "/home/tester/mf" );
	std::string workingPathWith( "/home/tester/mf/" );

	CHECK_EQUAL( validPath, BWUtil::formatPath( workingPathWith ) );
	CHECK_EQUAL( validPath, BWUtil::formatPath( workingPathWithout ) );
}


TEST( SanitisePath )
{
	std::string validPath( "/home/tester/mf" );

#ifdef _WIN32
	std::string windowsPath( "\\home\\tester\\mf" );
	BWUtil::sanitisePath( windowsPath );
	CHECK_EQUAL( validPath, windowsPath );
#else
	std::string linuxPath( "/home/tester/mf" );
	BWUtil::sanitisePath( linuxPath );
	CHECK_EQUAL( validPath, linuxPath );
#endif
}



// Currently commented out as the implementation for windows and linux
// differ in behaviour
#if 0
TEST( FilePath )
{
	std::string longPathToFile( "/home/tester/mf/bigworld/bin/Hybrid/cellapp" );
	std::string longPathToDirSlash( "/home/tester/mf/bigworld/bin/Hybrid/" );
	std::string longPathToDir( "/home/tester/mf/bigworld/bin/Hybrid" );
	std::string shortPathToFile( "/testFile" );
	std::string shortPathToDir( "/" );

	printf( "res1: %s\n", BWUtil::getFilePath( longPathToFile ).c_str() );
	printf( "res2: %s\n", BWUtil::getFilePath( longPathToDirSlash ).c_str() );
	printf( "res3: %s\n", BWUtil::getFilePath( longPathToDir ).c_str() );
	printf( "res4: %s\n", BWUtil::getFilePath( shortPathToFile ).c_str() );
	printf( "res5: %s\n", BWUtil::getFilePath( shortPathToDir ).c_str() );

	printf( "path dir: %s\n--\n", BWUtil::executableDirectory().c_str() );
	printf( "path    : %s\n", BWUtil::executablePath().c_str() );
}
#endif

// test_bw_util.cpp
