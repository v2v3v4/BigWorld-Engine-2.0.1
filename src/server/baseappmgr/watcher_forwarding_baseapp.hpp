/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BA_WATCHER_FORWARDING_HPP
#define BA_WATCHER_FORWARDING_HPP

#include "network/basictypes.hpp"
#include "server/watcher_forwarding.hpp"

class WatcherPathRequestV2;
class ForwardingCollector;

/**
 * This class implments a BaseAppMgr specific Forwarding Watcher.
 */
class BAForwardingWatcher : public ForwardingWatcher
{
public:
	ForwardingCollector *newCollector(
		WatcherPathRequestV2 & pathRequest,
		const std::string & destWatcher, const std::string & targetInfo );


	AddressList * getComponentAddressList( const std::string & targetInfo );
};

#endif
