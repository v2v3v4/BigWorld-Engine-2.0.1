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
#include "time_globals.hpp"


#include "chunk/chunk_manager.hpp"
#include "romp/time_of_day.hpp"




TimeGlobals::TimeGlobals()
{

}


TimeGlobals & TimeGlobals::instance()
{
	static TimeGlobals tg;
	return tg;
}


void TimeGlobals::setupWatchersFirstTimeOnly()
{
	BW_GUARD;
	static bool beenHere = false;

	if ( !beenHere )
	{
		MF_WATCH( "Client Settings/Time of Day",
			TimeGlobals::instance(),
			MF_ACCESSORS( std::string, TimeGlobals, timeOfDayAsString ), "Current time-of-day" );
		MF_WATCH( "Client Settings/Secs Per Hour",
			TimeGlobals::instance(),
			MF_ACCESSORS( float, TimeGlobals, secondsPerGameHour ), "Rate of actual seconds per game hour" );
	}

	beenHere = true;
}


TimeOfDay * TimeGlobals::getTimeOfDay() const
{
	BW_GUARD;
	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (!pSpace)
	{
		ERROR_MSG( "app.cpp::getTimeOfDay: pSpace null!" );
		return NULL;
	}

	EnviroMinder & enviro = pSpace->enviro();

	TimeOfDay * tod = enviro.timeOfDay();

	return tod;
}


std::string TimeGlobals::timeOfDayAsString() const
{
	BW_GUARD;
	TimeOfDay * timeOfDay = getTimeOfDay();
	if (timeOfDay != NULL)
		return timeOfDay->getTimeOfDayAsString();
	return "";
}


void TimeGlobals::timeOfDayAsString( std::string newTime )
{
	BW_GUARD;
	TimeOfDay * timeOfDay = getTimeOfDay();
	if (timeOfDay != NULL)
		timeOfDay->setTimeOfDayAsString( newTime );
}


float TimeGlobals::secondsPerGameHour() const
{
	BW_GUARD;
	TimeOfDay * timeOfDay = getTimeOfDay();
	if (timeOfDay != NULL)
		return timeOfDay->secondsPerGameHour();
	return 0.f;
}


void TimeGlobals::secondsPerGameHour( float t )
{
	BW_GUARD;
	TimeOfDay * timeOfDay = getTimeOfDay();
	if (timeOfDay != NULL)
		timeOfDay->secondsPerGameHour( t );
}
