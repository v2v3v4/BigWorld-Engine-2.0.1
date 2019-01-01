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
#include "stack_tracker.hpp"

#if ENABLE_STACK_TRACKER
StackTracker::StackItem	StackTracker::stack_[THREAD_STACK_DEPTH];
uint StackTracker::stackPos_ = 0;

#ifdef _DEBUG
uint StackTracker::maxStackPos_ = 0;
#endif


std::string StackTracker::buildReport()
{
	std::string res;

	for (uint i = 0; i < stackSize(); i++)
	{
		res += getStackItem(i);
		if (i < stackSize()-1)
		{
			res += " <- ";
		}
	}

	return res;
}

#ifdef _DEBUG
uint StackTracker::getMaxStackPos() { return maxStackPos_; }
#endif

#endif // #if ENABLE_STACK_TRACKER
