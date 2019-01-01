/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __EVENT_HISTORY_STATS_HPP__
#define __EVENT_HISTORY_STATS_HPP__

#include "cstdmf/watcher.hpp"

#include <map>
#include <string>

/**
 *	This class is used to track EventHistory usage. Statistics are exposed
 *	as watchers.
 */
class EventHistoryStats
{
	typedef std::map< std::string, uint32 > CountsMap;
	typedef MapWatcher< CountsMap > 		CountsWatcher;
	typedef std::map< std::string, uint32 > SizesMap;
	typedef MapWatcher< SizesMap > 			SizesWatcher;

	bool 			isEnabled_;
	CountsMap 		counts_;
	SizesMap		sizes_;

public:
	EventHistoryStats() : isEnabled_( false ) {}

	void init( const std::string & parentPath );

	/**
	 *	This function is called by functions dealing with EventHistory
	 *	to build up the statistics exposed by this class.
	 */
	void trackEvent( const std::string& typeName,
			const std::string& memberName, uint32 size )
	{
		if (isEnabled_)
		{
			std::string key( typeName );
			key.reserve( typeName.size() + memberName.size() + 1 );
			key += '.';
			key += memberName;
			counts_[ key ]++;
			sizes_[ key ] += size;
		}
	}

	bool isEnabled() const	{ return isEnabled_; }
	void setEnabled( bool enable )
	{
		// Enabling the watcher has side effect of clearing stats. This is
		// probably a useful thing for debugging so that people can perform
		// different actions and look at how much it affects the amount (and
		// size) of events.
		if (enable)
		{
			counts_.clear();
			sizes_.clear();
		}
		isEnabled_ = enable;
	}
};


#endif // __EVENT_HISTORY_STATS_HPP__
