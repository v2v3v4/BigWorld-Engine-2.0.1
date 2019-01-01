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
 *	@file
 */
 
// -----------------------------------------------------------------------------
// Section: StackTracker
// -----------------------------------------------------------------------------

inline void StackTracker::push(const char* name, const char*file, uint line)
{
	assert( stackPos_ < THREAD_STACK_DEPTH );
	uint stackPos = stackPos_;
	stack_[ stackPos ].name = name;
	stack_[ stackPos ].file = file;
	stack_[ stackPos ].line = line;
	stackPos_++;
	
#ifdef _DEBUG
	if (stackPos_ > maxStackPos_)
	{
		maxStackPos_ = stackPos_;
	}
#endif
}

inline void StackTracker::pop()
{
	assert( stackPos_ > 0 );
	--stackPos_;
}

inline uint StackTracker::stackSize()
{
	return stackPos_;
}

// 0 == top of stack, stackSize-1 == bottom
inline std::string StackTracker::getStackItem(uint idx)
{
	assert( idx < stackPos_ );
	return stack_[ stackPos_-1 - idx ].name;
}
