/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FINI_JOB_HPP
#define FINI_JOB_HPP

/**
 *	This class is used to implement functionality that needs to be run during
 *	static destruction. It can be promoted to occur before other tasks by
 *	calling FiniJob::runAll.
 *
 *	This is currently called by MemTracker to ensure memory has been cleaned up
 *	before it does any accounting.
 *
 *	Objects of this type are deleted immediately after their fini method is
 *	called.
 */
class FiniJob
{
public:
	FiniJob();
	virtual ~FiniJob() {};

	static bool runAll();

protected:
	virtual bool fini() = 0;
};

typedef FiniJob * FiniJobPtr;

#endif // FINI_JOB_HPP
