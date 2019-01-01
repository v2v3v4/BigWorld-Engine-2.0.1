/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// timeofday.ipp

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif


INLINE float TimeOfDay::startTime() const
{
	return startTime_;
}

INLINE void TimeOfDay::startTime( float time )
{
	startTime_ = time;
}


INLINE float TimeOfDay::secondsPerGameHour() const
{
	return secondsPerGameHour_;
}

INLINE void TimeOfDay::secondsPerGameHour( float t )
{
	secondsPerGameHour_ = t;
	this->timeConfigChanged();
}


INLINE float TimeOfDay::gameTime() const
{
	return time_;
}

INLINE void TimeOfDay::gameTime( float t )
{
	time_ = fmodf( t + 2400.f, 24.f );
	this->timeConfigChanged();
}


INLINE float TimeOfDay::sunAngle() const
{
	return sunAngle_;
}

INLINE void TimeOfDay::sunAngle( float angle )
{
	sunAngle_ = angle;
}


INLINE float TimeOfDay::moonAngle() const
{
	return moonAngle_;
}

INLINE void TimeOfDay::moonAngle( float angle )
{
	moonAngle_ = angle;
}


INLINE const OutsideLighting & TimeOfDay::lighting() const
{
	return now_;
}

INLINE OutsideLighting & TimeOfDay::lighting()
{
	return now_;
}

INLINE void TimeOfDay::updateNotifiersOn( bool on )
{
	updateNotifiersOn_ = on;
}


// time_of_day.ipp