/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SCRIPT_TIMERS_HPP
#define SCRIPT_TIMERS_HPP

#include <map>

#include "cstdmf/timer_handler.hpp"

class BinaryIStream;
class BinaryOStream;
class EntityApp;

/**
 *	This class stores a collection of timers that have an associated script id.
 */
class ScriptTimers
{
public:
	typedef int32 ScriptID;

	static void init( EntityApp & app );
	static void fini( EntityApp & app );

	ScriptID addTimer( float initialOffset, float repeatOffset, int userArg,
			TimerHandler * pHandler );
	bool delTimer( ScriptID timerID );

	void releaseTimer( TimerHandle handle );

	void cancelAll();

	void writeToStream( BinaryOStream & stream ) const;
	void readFromStream( BinaryIStream & stream,
			uint32 numTimers, TimerHandler * pHandler );

	ScriptID getIDForHandle( TimerHandle handle ) const;

	bool isEmpty() const	{ return map_.empty(); }

private:
	typedef std::map< ScriptID, TimerHandle > Map;

	ScriptID getNewID();
	Map::const_iterator findTimer( TimerHandle handle ) const;
	Map::iterator findTimer( TimerHandle handle );

	Map map_;
};


/**
 *	This namespace contains helper methods so that ScriptTimers can be a pointer
 *	instead of an instance. These functions handle the creation and destruction
 *	of the object.
 */
namespace ScriptTimersUtil
{
	ScriptTimers::ScriptID addTimer( ScriptTimers ** ppTimers,
		float initialOffset, float repeatOffset, int userArg,
		TimerHandler * pHandler );
	bool delTimer( ScriptTimers * pTimers, ScriptTimers::ScriptID timerID );

	void releaseTimer( ScriptTimers ** ppTimers, TimerHandle handle );

	void cancelAll( ScriptTimers * pTimers );

	void writeToStream( ScriptTimers * pTimers, BinaryOStream & stream );
	void readFromStream( ScriptTimers ** ppTimers, BinaryIStream & stream,
			TimerHandler * pHandler );

	ScriptTimers::ScriptID getIDForHandle( ScriptTimers * pTimers,
			TimerHandle handle );
}

#endif // SCRIPT_TIMERS_HPP
