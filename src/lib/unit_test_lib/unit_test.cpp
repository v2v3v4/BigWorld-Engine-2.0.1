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

#include "unit_test.hpp"

#include "cstdmf/dprintf.hpp"

#define USE_CPP_UNIT_LITE
#ifdef USE_CPP_UNIT_LITE

#include "third_party/CppUnitLite2/src/CppUnitLite2.h"
#include "third_party/CppUnitLite2/src/TestResultStdErr.h"
#include "TestResultBWOut.hpp"
#else

// CppUnit related
#include "cppunit/CompilerOutputter.h"
#include "cppunit/TestResult.h"
#include "cppunit/TestResultCollector.h"
#include "cppunit/TestRunner.h"
#include "cppunit/TestFailure.h"
#include "cppunit/Exception.h"
#include "cppunit/TextTestProgressListener.h"
#include "cppunit/extensions/TestFactoryRegistry.h"

#endif


#include <cstring>
#include <map>

namespace BWUnitTest
{

#ifdef USE_CPP_UNIT_LITE

int runTest( const std::string & testName, int argc, char* argv[] )
{
	DebugFilter::shouldWriteToConsole( false );

	for (int i = 1; i < argc; ++i)
	{
		if ((strcmp( "-v", argv[i] ) == 0) ||
				(strcmp( "--verbose", argv[i] ) == 0))
		{
			DebugFilter::shouldWriteToConsole( true );
		}
	}

#ifdef MF_SERVER
	// Output using CppUnitLite2's standard error outputter
	TestResultStdErr result;
#else
	bool useXML = false;

	for (int i = 0; i < argc; ++i)
	{
		if (strcmp( argv[ i ], "--xml" ) == 0 ||
			strcmp( argv[ i ], "-x" ) == 0)
		{
			useXML = true;
		}
	}

	// Output using BigWorld's outputter 
	TestResultBWOut result( testName, useXML );
#endif // MF_SERVER

	TestRegistry::Instance().Run( result );
	TestRegistry::Destroy();
	return result.FailureCount();	
}

#else

class BWTestProgressListener: public CppUnit::TestListener
{
public:
	BWTestProgressListener();
	virtual ~BWTestProgressListener() {}
public: // from CppUnit::TestListener
	virtual void startTest( CppUnit::Test * test );
	virtual void addFailure( const CppUnit::TestFailure & failure );
	virtual void endTest( CppUnit::Test * test );

private:
	bool currentTestFailed_;

};

BWTestProgressListener::BWTestProgressListener():
		currentTestFailed_ ( false )
{

}


void BWTestProgressListener::startTest( CppUnit::Test * test )
{
	printf( "%s:", test->getName().c_str() );
	fflush( stdout );
	currentTestFailed_ = false;
}

void BWTestProgressListener::addFailure( const CppUnit::TestFailure & failure )
{
	currentTestFailed_ = true;

}

void BWTestProgressListener::endTest( CppUnit::Test * test )
{
	const std::string & testName = test->getName();
	const int MAX_TESTNAME_LENGTH = 50;
	printf( "%*s\n",
		MAX_TESTNAME_LENGTH - testName.size(),
		currentTestFailed_ ? "FAILED" : "OK" );
}

int runTest( const std::string & testName, int argc, char* argv[] )
{
	DebugFilter::shouldWriteToConsole( false );

	for (int i = 1; i < argc; ++i)
	{
		if ((strcmp( "-v", argv[i] ) == 0) ||
			(strcmp( "--verbose", argv[i] ) == 0))
		{
			DebugFilter::shouldWriteToConsole( true );
		}
	}

	// Create the event manager and test controller
	CppUnit::TestResult controller;

	// Add a listener that collects test result
	CppUnit::TestResultCollector result;
	controller.addListener( &result );

#if 0 // original CppUnit progress listener
	// Add a listener that print dots as test run.
	CppUnit::TextTestProgressListener progress;
#else
	BWTestProgressListener progress;
#endif

	controller.addListener( &progress );

	CppUnit::TestRunner runner;
	runner.addTest( CppUnit::TestFactoryRegistry::getRegistry().makeTest() );

	std::cout << "Running "  <<  testName << ":\n";
	runner.run( controller );
	std::cerr << std::endl;

	// Print test in a compiler compatible format.
	CppUnit::CompilerOutputter outputter( &result, std::cout );
	outputter.write();

	return result.testFailures();
}

#endif // USE_CPP_UNIT_LITE

} // namespace BWUnitTest

// unit_test.cpp
