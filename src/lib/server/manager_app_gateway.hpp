/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MANAGER_APP_GATEWAY_HPP
#define MANAGER_APP_GATEWAY_HPP

#include "network/channel_owner.hpp"

namespace Mercury
{
	class InterfaceElement;
}
class Watcher;

class ManagerAppGateway
{
public:

	ManagerAppGateway( Mercury::NetworkInterface & networkInterface,
		const Mercury::InterfaceElement & retireAppIE );
	virtual ~ManagerAppGateway();


	bool init( const char * interfaceName, int numRetries );


	void addWatchers( const char * name, Watcher & watcher );

	void isRegular( bool localValue, bool remoteValue = false )
	{
		channel_.channel().isLocalRegular( localValue );
		channel_.channel().isRemoteRegular( remoteValue );
	}

	bool isInitialised() const
	{ return channel_.channel().addr() != Mercury::Address::NONE; }

	void retireApp();

protected:
	Mercury::ChannelOwner channel_;

private:
	const Mercury::InterfaceElement & retireAppIE_;
};


#endif // MANAGER_APP_GATEWAY_HPP
