/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "event_history_stats.hpp"

#include <sstream>

namespace // anon
{
/**
 *	This class implements a read-only Watcher interface and interprets the
 *	base pointer as an INTTYPE pointer. It dereferences to pointer and
 * 	provides the value as the watcher value.
 *
 * 	Mainly useful as a child of MapWatcher.
 */
template <class INTTYPE>
class BaseIntegerWatcher : public Watcher
{
	virtual bool getAsString( const void * base, const char * path,
			std::string & result, std::string & desc, Mode & mode ) const
	{
		std::stringstream ss;
		ss << *reinterpret_cast< const INTTYPE* >( base );
		result = ss.str();
		mode = WT_READ_ONLY;
		return true;
	}

	virtual bool getAsStream( const void * base,
			const char * path, WatcherPathRequestV2 & pathRequest ) const
	{
		std::string result;
		Mode mode = WT_READ_ONLY;

		bool ret = this->Watcher::getAsString( base, path, result, mode );
		if (ret)
		{
			watcherValueToStream( pathRequest.getResultStream(), result, mode );
			pathRequest.setResult( comment_, mode, this, base );
		}
		return ret;
	}

	virtual bool setFromString( void * base, const char * path,
			const char * valueStr )
	{
		return false;
	}

	virtual bool setFromStream( void * base,
			const char * path,
			WatcherPathRequestV2 & pathRequest )
	{
		return false;
	}
};

} // anon namespace


/**
 *	This function exposes watchers at the given parent path.
 */
void EventHistoryStats::init( const std::string & parentPath )
{
	// Setup watchers
	CountsWatcher* 	pCountsWatcher = new CountsWatcher( counts_ );
	pCountsWatcher->addChild( "*", new BaseIntegerWatcher<uint32> );

	SizesWatcher*	pSizesWatcher = new SizesWatcher( sizes_ );
	pSizesWatcher->addChild( "*", new BaseIntegerWatcher<uint32> );

	MemberWatcher< bool,EventHistoryStats >*	pEnabledWacher =
			new MemberWatcher< bool, EventHistoryStats >( *this,
					&EventHistoryStats::isEnabled,
					&EventHistoryStats::setEnabled );

	DirectoryWatcher* pDir = new DirectoryWatcher;
	pDir->addChild( "counts", pCountsWatcher );
	pDir->addChild( "sizes", pSizesWatcher );
	pDir->addChild( "enabled", pEnabledWacher );

	Watcher::rootWatcher().addChild( parentPath.c_str(), pDir );
}


// event_history_stats.cpp
