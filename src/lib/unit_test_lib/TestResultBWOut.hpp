/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TESTRESULTBWOUT_H
#define TESTRESULTBWOUT_H

#include "third_party/CppUnitLite2/src/TestResult.h"
#include <string>


class TestResultBWOut : public TestResult
{
public:
	TestResultBWOut( const std::string & name, bool useXML = false );

	virtual void StartTests ();
    virtual void AddFailure (const Failure & failure);
    virtual void EndTests ();

	void StartTestsXML();
	void AddFailureXML( const Failure & failure );
	void EndTestsXML();

	void StartTestsTXT();
	void AddFailureTXT( const Failure & failure );
	void EndTestsTXT();

private:
	std::string name_;
	bool usingXML_;
};


#endif
