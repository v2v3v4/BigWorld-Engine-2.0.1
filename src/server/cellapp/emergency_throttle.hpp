/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EMERGENCY_THROTTLE_HPP
#define EMERGENCY_THROTTLE_HPP

#include "cstdmf/smartpointer.hpp"

class Watcher;
typedef SmartPointer< Watcher > WatcherPtr;

class EmergencyThrottle
{
public:
	EmergencyThrottle();

	void update( float numSecondsBehind, float spareTime, float tickPeriod );
	float value() const	{ return value_; }
	float estimatePersistentLoadTime( float persistentLoadTime,
										float throttledLoadTime) const;

	static WatcherPtr pWatcher();

private:
	bool shouldScaleBack( float timeToNextTick, float spareTime ) const;

	bool scaleBack( float fraction );
	bool scaleForward( float fraction );

	float value_;

	bool shouldPrintScaleForward_;
	uint64 startTime_;
	float maxTimeBehind_;
};

#endif // EMERGENCY_THROTTLE_HPP
