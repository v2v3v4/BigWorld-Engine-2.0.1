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
 * 	@file description.cpp
 *
 *	This file provides a common base class for descriptions, mainly for stats.
 *
 *	@ingroup entity
 */
#include "pch.hpp"
#include "cstdmf/watcher.hpp"
#include "entitydef/entity_member_stats.hpp"

#if ENABLE_WATCHERS
bool EntityMemberStats::s_limitForBaseApp_;

WatcherPtr EntityMemberStats::pWatcher()
{
	static WatcherPtr watchMe = NULL;

	if (!watchMe)
	{
		watchMe = new DirectoryWatcher();
		EntityMemberStats * pNull = NULL;
		pNull->sentToOwnClient_.addWatchers( watchMe, "sentToOwnClient" );
		if (!s_limitForBaseApp_)
		{
			pNull->sentToOtherClients_.addWatchers( watchMe, "sentToOtherClients" );
			pNull->addedToHistoryQueue_.addWatchers( watchMe, "addedToHistoryQueue" );
		}

		pNull->sentToGhosts_.addWatchers( watchMe,
				s_limitForBaseApp_ ? "sentToCell" : "sentToGhosts" );

		pNull->sentToBase_.addWatchers( watchMe, "sentToBase" );
		pNull->received_.addWatchers( watchMe, "received" );
	}
	return watchMe;
}

EntityMemberStats::Stat::SubStat::Container * EntityMemberStats::Stat::s_pContainer;

void EntityMemberStats::Stat::addWatchers( WatcherPtr watchMe, std::string name )
{
	StatWatcherCreator::addWatchers( watchMe, 
		(name + std::string( "Messages" )).c_str(), messages_ );
	StatWatcherCreator::addWatchers( watchMe, 
		(name + std::string( "Bytes" ) ).c_str(), bytes_ );
}

void EntityMemberStats::Stat::tickAll( double deltaTime )
{
	SubStat::Container::iterator iter = s_pContainer->begin();

	while (iter != s_pContainer->end())
	{
		(*iter)->tick( deltaTime );
		++iter;
	}
}


/**
 *	This method ticks a fraction of the statistics. It is useful for spreading
 *	out the cost of updating statistics.
 *
 *	@param index The index of the portion to tick. Value in range [0, total).
 *	@param total The total number of portions.
 *	@param deltaTime The time since this portion was last ticked.
 */
void EntityMemberStats::Stat::tickSome( size_t index, size_t total,
		double deltaTime )
{
	size_t size = s_pContainer ? s_pContainer->size() : 0;

	size_t start = index * size / total;
	size_t end = (index + 1) * size / total;

	for (size_t i = start; i < end; ++i)
	{
		(*s_pContainer)[ i ]->tick( deltaTime );
	}
}

#endif
