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

#include "fini_job.hpp"

#include <list>

namespace
{

typedef std::list< FiniJob * > FiniJobs;
std::list< FiniJob * > * s_pFiniJobs = NULL;

class FiniRunner
{
public:
	~FiniRunner()
	{
		FiniJob::runAll();
	}
};

FiniRunner finiRunner;

} // anonymous namespace


/**
 *	This static method runs all of the FiniJob jobs.
 */
bool FiniJob::runAll()
{
	bool result = true;

	if (s_pFiniJobs != NULL)
	{
		FiniJobs::iterator iter = s_pFiniJobs->begin();

		while (iter != s_pFiniJobs->end())
		{
			FiniJob * pJob = *iter;
			result &= pJob->fini();
			delete pJob;

			++iter;
		}

		delete s_pFiniJobs;
		s_pFiniJobs = NULL;
	}

	return result;
}


/**
 *	Constructor.
 */
FiniJob::FiniJob()
{
	if (s_pFiniJobs == NULL)
	{
		s_pFiniJobs = new FiniJobs;
	}

	s_pFiniJobs->push_back( this );
}


// fini_job.cpp
