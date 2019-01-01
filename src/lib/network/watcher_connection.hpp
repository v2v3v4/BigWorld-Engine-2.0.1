/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WATCHER_CONNECTION_HPP
#define WATCHER_CONNECTION_HPP

#include "cstdmf/config.hpp"

#if ENABLE_WATCHERS

#include "interfaces.hpp"

class Endpoint;
class WatcherNub;

namespace Mercury
{
class EventDispatcher;
}

// -----------------------------------------------------------------------------
// Section: WatcherConnection
// -----------------------------------------------------------------------------

/**
 *
 */
class WatcherConnection : public Mercury::InputNotificationHandler
{
public:
	WatcherConnection( WatcherNub & nub,
			Mercury::EventDispatcher & dispatcher,
			Endpoint * pEndpoint );

	~WatcherConnection();

	int handleInputNotification( int fd );

private:
	bool recvSize();
	bool recvMsg();

	WatcherNub & nub_;
	Mercury::EventDispatcher & dispatcher_;
	Endpoint * pEndpoint_;

	uint32 receivedSize_;
	uint32 messageSize_;

	char * pBuffer_;
};

#endif /* ENABLE_WATCHERS */

#endif // WATCHER_CONNECTION_HPP
