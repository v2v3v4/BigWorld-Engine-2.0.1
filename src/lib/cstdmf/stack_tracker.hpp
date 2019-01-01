/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *	@internal
 *	@file
 */

#ifndef STACK_TRACKER__HPP
#define STACK_TRACKER__HPP


#include "cstdmf/debug.hpp"
#include "cstdmf/stdmf.hpp"
#include "cstdmf/concurrency.hpp"
#include "cstdmf/config.hpp"

#include <string>

// Using a C assert here, because MF_ASSERT calls things that can cause another push/pop 
// to happen, causing this to assert again, ad infinitum.
#include <cassert>

#if ENABLE_STACK_TRACKER
class StackTracker
{
private:
	enum
	{
		THREAD_STACK_DEPTH	= 1024			// The maximum stack size for a thread
	};

	struct StackItem
	{
		const char*		name;
		const char*		file;
		uint			line;
	};

public:

	static void push(const char* name, const char*file=NULL, uint line=0);
	static void pop();
	static uint stackSize();

	// 0 == top of stack, stackSize-1 == bottom
	static std::string getStackItem(uint idx);

	static std::string buildReport();

#ifdef _DEBUG
	static uint getMaxStackPos();
#endif

private:
	static THREADLOCAL(StackItem)	stack_[THREAD_STACK_DEPTH];
	static THREADLOCAL(uint)		stackPos_;
#ifdef _DEBUG
	static THREADLOCAL(uint)		maxStackPos_;
#endif
};


class ScopedStackTrack
{
public:
	inline ScopedStackTrack(const char* name, const char* file=NULL, uint line=0)
	{
		StackTracker::push(name, file, line);
	}

	inline ~ScopedStackTrack()
	{
		StackTracker::pop();
	}
};

#include "stack_tracker.ipp"

#endif // #if ENABLE_STACK_TRACKER

#endif
