/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_MEMBER_STATS_HPP
#define ENTITY_MEMBER_STATS_HPP

#include "cstdmf/timestamp.hpp"
#include "math/stat_with_rates_of_change.hpp"
#include "math/stat_watcher_creator.hpp"

class Watcher;
typedef SmartPointer<Watcher> WatcherPtr;

/**
 *	This class is a base class for MethodDescription and DataDescription. It is
 *	used to store statistics about these instances.
 */
class EntityMemberStats
{
public:
#if ENABLE_WATCHERS
	static WatcherPtr pWatcher();

	void countSentToOwnClient( int bytes )
	{
		sentToOwnClient_.count( bytes );
	}

	void countSentToOtherClients( int bytes )
	{
		sentToOtherClients_.count( bytes );
	}

	void countAddedToHistoryQueue( int bytes )
	{
		addedToHistoryQueue_.count( bytes );
	}

	void countSentToGhosts( int bytes )
	{
		sentToGhosts_.count( bytes );
	}

	void countSentToBase( int bytes )
	{
		sentToBase_.count( bytes );
	}

	void countReceived( int bytes )
	{
		received_.count( bytes );
	}

	static void limitForBaseApp()
	{
		s_limitForBaseApp_ = true;
	}

	class Stat
	{
		typedef IntrusiveStatWithRatesOfChange< unsigned int >  SubStat;
		SubStat messages_;
		SubStat bytes_;
		static SubStat::Container * s_pContainer;
	public:
		Stat():
			messages_( s_pContainer ),
			bytes_( s_pContainer )
		{
			StatWatcherCreator::initRatesOfChangeForStat( messages_ );
			StatWatcherCreator::initRatesOfChangeForStat( bytes_ );
		}
		Stat & operator=( const Stat & other )
		{
			return *this;
		}
		void count( uint32 bytes )
		{
			messages_++;
			bytes_ += bytes;
		}

		static void tickAll( double deltaTime = 1.0 );
		static void tickSome( size_t index, size_t total, double deltaTime );

		void addWatchers( WatcherPtr watchMe, std::string name );
	};

private:
	Stat sentToOwnClient_;
	Stat sentToOtherClients_;
	Stat addedToHistoryQueue_;
	Stat sentToGhosts_;
	Stat sentToBase_;
	Stat received_;

	static bool s_limitForBaseApp_;
#endif // ENABLE_WATCHERS
};

#endif // ENTITY_MEMBER_STATS_HPP
