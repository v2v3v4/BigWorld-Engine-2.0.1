/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENGINE_STATISTICS_HPP
#define ENGINE_STATISTICS_HPP

#include <iostream>

#include "romp/console.hpp"

/**
 *	This class implements a console displaying various
 *	engine statistics.  The main displays are :
 *
 *	memory and performance indications from the Moo library
 *	timing displays from DogWatches throughout the system.
 *
 *	Most often, this console can be viewed by pressing F5
 */
class EngineStatistics : public StatisticsConsole::Handler
{
public:
	~EngineStatistics();

	static EngineStatistics & instance();

	// Accessors
	void tick( float dTime );

	static bool logSlowFrames_;

private:
	EngineStatistics();

	virtual void displayStatistics( XConsole & console );

	EngineStatistics( const EngineStatistics& );
	EngineStatistics& operator=( const EngineStatistics& );

	friend std::ostream& operator<<( std::ostream&, const EngineStatistics& );

	float lastFrameTime_;
	float timeToNextUpdate_;

	static EngineStatistics instance_;
};

#ifdef CODE_INLINE
#include "engine_statistics.ipp"
#endif

#endif // ENGINE_STATISTICS_HPP
