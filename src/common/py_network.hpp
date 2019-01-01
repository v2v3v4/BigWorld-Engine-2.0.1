/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_NETWORK_HPP
#define PY_NETWORK_HPP

#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"

namespace PyNetwork
{
	bool init( Mercury::EventDispatcher & dispatcher,
			Mercury::NetworkInterface & interface );
}

#endif // PY_NETWORK_HPP
