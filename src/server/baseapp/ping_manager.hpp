/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PING_MANAGER_HPP
#define PING_MANAGER_HPP

namespace Mercury
{
class EventDispatcher;
class NetworkInterface;
}

namespace PingManager
{ 
	bool init( Mercury::EventDispatcher & dispatcher,
			Mercury::NetworkInterface & networkInterface );
	void fini();
}

#endif // PING_MANAGER_HPP
