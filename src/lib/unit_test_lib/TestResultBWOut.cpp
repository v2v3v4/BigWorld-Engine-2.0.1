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

#include "TestResultBWOut.hpp"
#include "third_party/CppUnitLite2/src/Failure.h"
#include <iostream>
#include <iomanip>


TestResultBWOut::TestResultBWOut( const std::string & name, 
								 bool useXML /*= false*/ ) :
	TestResult(),
	name_( name ),
	usingXML_( useXML )
{
}


void TestResultBWOut::StartTests ()
{
	TestResult::StartTests();
	if (usingXML_)
	{
		this->StartTestsXML();
	}
	else
	{
		this->StartTestsTXT();
	}
}


void TestResultBWOut::AddFailure (const Failure & failure) 
{
    TestResult::AddFailure(failure);
	if (usingXML_)
	{
		this->AddFailureXML( failure );
	}
	else
	{
		this->AddFailureTXT( failure );
	}
}


void TestResultBWOut::EndTests () 
{
    TestResult::EndTests();
	if (usingXML_)
	{
		this->EndTestsXML();
	}
	else
	{
		this->EndTestsTXT();
	}
}


void TestResultBWOut::StartTestsXML()
{
	std::cout << "<TestRun>" << std::endl;
	std::cout << "\t<FailedTests>" << std::endl;
}


void TestResultBWOut::AddFailureXML( const Failure & failure ) 
{
    std::cout << "\t\t<FailedTest>" << std::endl;
	std::cout << "\t\t\t<Name>" << failure.TestName() << "</Name>" << std::endl;
	std::cout << "\t\t\t<Location>" << std::endl;
	std::cout << "\t\t\t\t<File>" << failure.FileName() << "</File>" << std::endl;
	std::cout << "\t\t\t\t<Line>" << failure.LineNumber() << "</Line>" << std::endl;
	std::cout << "\t\t\t</Location>" << std::endl;
	std::cout << "\t\t\t<Message>" << failure.Condition() << "</Message>" << std::endl;
	std::cout << "\t\t</FailedTest>" << std::endl;
}


void TestResultBWOut::EndTestsXML() 
{
	std::cout << "\t</FailedTests>" << std::endl;
	std::cout << "\t<Statistics>" << std::endl;
    std::cout << "\t\t<Tests> " << m_testCount << " </Tests>" << std::endl;
	std::cout << "\t\t<Failures> " << m_failureCount << " </Failures>" << std::endl;
    std::cout << "\t\t<Time> " << std::setprecision(3) << m_secondsElapsed << " </Time>" << std::endl;
	std::cout << "\t</Statistics>" << std::endl;
	std::cout << "</TestRun>" << std::endl;
}


void TestResultBWOut::StartTestsTXT()
{
	std::cout << "==========" << std::endl;
	std::cout << "Running Tests on: " << name_ << std::endl;
}


void TestResultBWOut::AddFailureTXT( const Failure & failure ) 
{
	std::cout << "Failed Test: " << failure.TestName() << std::endl;
	std::cout << "File: " << failure.FileName() << std::endl;
	std::cout << "Line: " << failure.LineNumber() << std::endl;
	std::cout << "Condition: " << failure.Condition() << std::endl;
	std::cout << "----------" << std::endl;
}


void TestResultBWOut::EndTestsTXT() 
{
	std::cout << "Tests run: " << m_testCount << std::endl;
	std::cout << "Failures: " << m_failureCount << std::endl;
	std::cout << "Time: " << std::setprecision(3) << m_secondsElapsed << std::endl;
}
