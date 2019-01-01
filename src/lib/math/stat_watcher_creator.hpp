/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef STAT_WATCHER_CREATOR_HPP
#define STAT_WATCHER_CREATOR_HPP

#include "stat_with_rates_of_change.hpp"
#include "cstdmf/smartpointer.hpp"

#if ENABLE_WATCHERS
class Watcher;
typedef SmartPointer< Watcher > WatcherPtr;

typedef StatWithRatesOfChange< unsigned int > UintStatWithRatesOfChange;
typedef IntrusiveStatWithRatesOfChange< unsigned int >::Container UintStatWithRatesOfChangeContainer;

namespace StatWatcherCreator
{
	void initRatesOfChangeForStats(
			const UintStatWithRatesOfChangeContainer & stats );

	void initRatesOfChangeForStat( UintStatWithRatesOfChange & stat );

	void addWatchers( WatcherPtr pWatcher,
			const char * name, UintStatWithRatesOfChange & stat );

} // namespace StatWatcherCreator

#endif // ENABLE_WATCHERS

#endif // STAT_WATCHER_CREATOR_HPP
