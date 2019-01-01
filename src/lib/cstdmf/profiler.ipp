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
// Section: Profiler
// -----------------------------------------------------------------------------

/**
 *	@todo Comment
 */
inline Profiler& Profiler::instance()
{
	if ( !Profiler::instance_ )
		new Profiler;

	return *Profiler::instance_;
}


/**
 *	@todo Comment
 */
inline Profiler& Profiler::instanceNoCreate()
{
	return *Profiler::instance_;
}


/**
 *	@todo Comment
 */
#if ENABLE_PROFILER
inline void Profiler::addTimeToCurrentSlot()
{
	uint64 curTime = timestamp();
	uint64 delta = curTime - prevTime_;
	prevTime_ = curTime;
	slots_[curSlot_].times_[frameCount_] += delta;
}
#endif


/**
 *	Add to current and push slotId
 */
inline void Profiler::begin( int slotId )
{
#if ENABLE_PROFILER
	if ( OurThreadID() != Profiler::instanceNoCreate().threadId_ )
		return;
	this->addTimeToCurrentSlot();
	slotStack_[slotStackPos_++] = curSlot_;
	curSlot_ = slotId;
	slots_[slotId].counts_[frameCount_]++;
#endif
}


/**
 *	Add to current and pop
 */
inline void Profiler::end( )
{
#if ENABLE_PROFILER
	if ( OurThreadID() != Profiler::instanceNoCreate().threadId_ )
		return;
	this->addTimeToCurrentSlot();
	curSlot_ = slotStack_[--slotStackPos_];
#endif
}

// profiler.ipp
