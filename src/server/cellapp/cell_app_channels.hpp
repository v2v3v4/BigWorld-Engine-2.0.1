/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CELL_APP_CHANNELS_HPP
#define CELL_APP_CHANNELS_HPP

#include "cstdmf/singleton.hpp"
#include "cstdmf/timer_handler.hpp"

#include "network/basictypes.hpp"

#include <map>
#include <set>

class CellAppChannel;

namespace Mercury
{
class EventDispatcher;
}


/**
 *	This class is the collection of CellAppChannel instances.
 */
class CellAppChannels : public Singleton< CellAppChannels >,
	public TimerHandler
{
public:
	CellAppChannels( int microseconds, Mercury::EventDispatcher & dispatcher );
	~CellAppChannels();

	CellAppChannel * get( const Mercury::Address & addr,
		bool shouldCreate = true );

	void remoteFailure( const Mercury::Address & addr );

	void setCapacities( int ghostingCapacity, int offloadCapacity );

private:
	void sendAll();

	virtual void handleTimeout( TimerHandle handle, void * arg );

	typedef std::map< Mercury::Address, CellAppChannel * > Map;
	typedef Map::iterator iterator;

	Map map_;

	static const int RECENTLY_DEAD_PERIOD = 10;

	typedef std::set< Mercury::Address > RecentlyDead;
	RecentlyDead recentlyDead_;

	uint64 lastTimeOfDeath_;
	TimerHandle timer_;
};

#endif // CELL_APP_CHANNELS_HPP
