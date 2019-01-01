/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TIME_GLOBALS_HPP
#define TIME_GLOBALS_HPP



/*
 Wraps up the "Client Settings/Time of Day" and "Client Settings/Secs Per
 Hour" watcher values in a Singleton class. This way, they can reflect the
 current camera space.
 */
class TimeGlobals
{
private:
	TimeGlobals();

public:
	static TimeGlobals & instance();
	void setupWatchersFirstTimeOnly();
	TimeOfDay * getTimeOfDay() const;
	std::string timeOfDayAsString() const;
	void timeOfDayAsString( std::string newTime );
	float secondsPerGameHour() const;
	void secondsPerGameHour( float t );
};


#endif // TIME_GLOBALS_HPP



// time_globals.hpp
