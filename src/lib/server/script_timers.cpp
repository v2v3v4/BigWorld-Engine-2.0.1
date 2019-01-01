/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "script_timers.hpp"

#include "entity_app.hpp"
#include "entity_app_config.hpp"

namespace
{
EntityApp * g_pApp = NULL;
}


// -----------------------------------------------------------------------------
// Section: ScriptTimers
// -----------------------------------------------------------------------------

/**
 *	This static method sets up the script timers to be able to access the
 *	EntityApp.
 */
void ScriptTimers::init( EntityApp & app )
{
	MF_ASSERT( g_pApp == NULL );

	g_pApp = &app;
}


void ScriptTimers::fini( EntityApp & app )
{
	MF_ASSERT( g_pApp == &app );
	g_pApp = NULL;
}


/**
 *	This method adds a timer to the collection. It returns an identifier that
 *	should be used by ScriptTimers::delTimer.
 */
ScriptTimers::ScriptID ScriptTimers::addTimer( float initialOffset,
		float repeatOffset, int userArg, TimerHandler * pHandler )
{
	if (initialOffset < 0.f)
	{
		WARNING_MSG( "ScriptTimers::addTimer: Negative timer offset (%f)\n",
				initialOffset );
		initialOffset = 0.f;
	}

	MF_ASSERT( g_pApp );

	int hertz = EntityAppConfig::updateHertz();
	int initialTicks = GameTime( g_pApp->time() +
			initialOffset * hertz );
	int repeatTicks = 0;

	if (repeatOffset > 0.f)
	{
		repeatTicks = GameTime( repeatOffset * hertz );
		if (repeatTicks < 1)
		{
			repeatTicks = 1;
		}
	}

	TimerHandle timerHandle = g_pApp->timeQueue().add(
			initialTicks, repeatTicks,
			pHandler, (void *)userArg );

	if (timerHandle.isSet())
	{
		int id = this->getNewID();

		map_[ id ] = timerHandle;

		return id;
	}

	return 0;
}


/**
 *	This method returns an available ScriptID for a new timer.
 */
ScriptTimers::ScriptID ScriptTimers::getNewID()
{
	ScriptTimers::ScriptID id = 1;

	// Ugly linear search
	while (map_.find( id ) != map_.end())
	{
		++id;
	}

	return id;
}


/**
 *	This method cancels the timer with the given id.
 *
 *	@return True if such a timer exists, otherwise false.
 */
bool ScriptTimers::delTimer( ScriptTimers::ScriptID timerID )
{
	Map::iterator iter = map_.find( timerID );

	if (iter != map_.end())
	{
		// Take a copy so that the TimerHandle in the map is still set when
		// ScriptTimers::releaseTimer is called.
		TimerHandle handle = iter->second;
		handle.cancel();
		// iter->second.cancel();

		return true;
	}

	return false;
}


/**
 *	This should be called when the TimerHandler passed to addTimer has its
 *	onRelease method called.
 */
void ScriptTimers::releaseTimer( TimerHandle handle )
{
	int numErased = 0;

	Map::iterator iter = map_.begin();

	while (iter != map_.end())
	{
		// If this is hit, it's likely that the TimerHandle is cancelled
		// directly.
		MF_ASSERT( iter->second.isSet() );

		if (handle == iter->second)
		{
			map_.erase( iter++ );
			++numErased;
		}
		else
		{
			iter++;
		}
	}

	MF_ASSERT( numErased == 1 );
}


/**
 *	This method cancels on timers in this collection.
 */
void ScriptTimers::cancelAll()
{
	Map::size_type size = map_.size();

	for (Map::size_type i = 0; i < size; ++i)
	{
		MF_ASSERT( i + map_.size() == size );
		// Take a copy so that the TimerHandle in the map is still set when
		// ScriptTimers::releaseTimer is called.
		TimerHandle handle = map_.begin()->second;
		handle.cancel();
		// map_.begin()->second.cancel();
	}
}


/**
 *	This method writes this object to a stream.
 */
void ScriptTimers::writeToStream( BinaryOStream & stream ) const
{
	MF_ASSERT( g_pApp );

	stream << uint32( map_.size() );

	Map::const_iterator iter = map_.begin();

	while (iter != map_.end())
	{
		TimeQueue::TimeStamp time;
		TimeQueue::TimeStamp interval;
		void *				pUser;

		MF_VERIFY( g_pApp->timeQueue().getTimerInfo(
						iter->second,
						time, interval, pUser ) );

		stream << iter->first << time << interval << int32( uintptr(pUser) );

		++iter;
	}
}


/**
 *	This method reads timers from a stream.
 */
void ScriptTimers::readFromStream( BinaryIStream & stream, uint32 numTimers,
		TimerHandler * pHandler )
{
	for (uint32 i = 0; i < numTimers; ++i)
	{
		ScriptID timerID;
		TimeQueue::TimeStamp time;
		TimeQueue::TimeStamp interval;
		int32 userData;

		stream >> timerID >> time >> interval >> userData;

		map_[ timerID ] = g_pApp->timeQueue().add(
			time, interval, pHandler, (void *)userData );
	}
}


/**
 *	This method returns the ScriptID associated with the given TimerHandle.
 */
ScriptTimers::ScriptID ScriptTimers::getIDForHandle( TimerHandle handle ) const
{
	Map::const_iterator iter = this->findTimer( handle );

	return (iter != map_.end()) ? iter->first : 0;
}


/**
 *	This method finds the entry associated with a TimerHandle.
 */
ScriptTimers::Map::const_iterator ScriptTimers::findTimer( TimerHandle handle ) const
{
	Map::const_iterator iter = map_.begin();

	while (iter != map_.end())
	{
		if (iter->second == handle)
		{
			return iter;
		}

		++iter;
	}

	return iter;
}


// -----------------------------------------------------------------------------
// Section: ScriptTimersUtil
// -----------------------------------------------------------------------------

namespace ScriptTimersUtil
{

/**
 *	This function is a wrapper to ScriptTimers::addTimer that handles
 *	ScriptTimers creation.
 */
ScriptTimers::ScriptID addTimer( ScriptTimers ** ppTimers,
	float initialOffset, float repeatOffset, int userArg,
	TimerHandler * pHandler )
{
	ScriptTimers *& rpTimers = *ppTimers;

	if (rpTimers == NULL)
	{
		rpTimers = new ScriptTimers;
	}

	return rpTimers->addTimer( initialOffset, repeatOffset,
			userArg, pHandler );
}


/**
 *	This function is a wrapper to ScriptTimers::delTimer.
 */
bool delTimer( ScriptTimers * pTimers, ScriptTimers::ScriptID timerID )
{
	return pTimers && pTimers->delTimer( timerID );
}


/**
 *	This function is a wrapper to ScriptTimers::cancelAll.
 */
void cancelAll( ScriptTimers * pTimers )
{
	if (pTimers)
	{
		pTimers->cancelAll();
	}
}


/**
 *	This function is a wrapper to ScriptTimers::releaseTimer.
 */
void releaseTimer( ScriptTimers ** ppTimers, TimerHandle handle )
{
	ScriptTimers *& rpTimers = *ppTimers;
	MF_ASSERT( rpTimers );

	rpTimers->releaseTimer( handle );

	if (rpTimers->isEmpty())
	{
		delete rpTimers;
		rpTimers = NULL;
	}
}


/**
 *	This function is a wrapper to ScriptTimers::writeToStream.
 */
void writeToStream( ScriptTimers * pTimers, BinaryOStream & stream )
{
	if (pTimers)
	{
		pTimers->writeToStream( stream );
	}
	else
	{
		stream << uint32( 0 );
	}
}


/**
 *	This function is a wrapper to ScriptTimers::readFromStream.
 */
void readFromStream( ScriptTimers ** ppTimers, BinaryIStream & stream,
		TimerHandler * pHandler )
{
	ScriptTimers *& rpTimers = *ppTimers;
	MF_ASSERT( rpTimers == NULL );

	uint32 numTimers;
	stream >> numTimers;

	if (numTimers != 0)
	{
		rpTimers = new ScriptTimers;
		rpTimers->readFromStream( stream, numTimers, pHandler );
	}
}


/**
 *	This function is a wrapper to ScriptTimers::getIDForHandle.
 */
ScriptTimers::ScriptID getIDForHandle( ScriptTimers * pTimers,
		TimerHandle handle )
{
	if (pTimers == NULL)
	{
		return 0;
	}

	return pTimers->getIDForHandle( handle );
}

} // namespace ScriptTimersUtil

// timers.cpp
