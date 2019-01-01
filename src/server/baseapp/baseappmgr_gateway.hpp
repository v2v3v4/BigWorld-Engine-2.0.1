/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BASEAPPMGR_GATEWAY
#define BASEAPPMGR_GATEWAY

#include "server/manager_app_gateway.hpp"

class BackupHash;
class BaseApp;

namespace Mercury
{
class Address;
class NetworkInterface;
class ReplyMessageHandler;
}

class BaseAppMgrGateway : public ManagerAppGateway
{
public:
	BaseAppMgrGateway( Mercury::NetworkInterface & interface );

	// TODO:BAR This is a good candidate for pushing up to ManagerAppGateway,
	// and have EntityApp have a pure virtual method e.g.
	// addManagerAppRebirthData().
	void onManagerRebirth( BaseApp & baseApp, const Mercury::Address & addr );

	void add( const Mercury::Address & addrForCells,
		const Mercury::Address & addrForClients,
		Mercury::ReplyMessageHandler * pHandler );

	void useNewBackupHash( const BackupHash & entityToAppHash,
		const BackupHash & newEntityToAppHash );

	void informOfArchiveComplete( const Mercury::Address & deadBaseAppAddr );

	void registerBaseGlobally( const std::string & pickledKey,
		const EntityMailBoxRef & mailBox, 
		Mercury::ReplyMessageHandler * pHandler );

	void deregisterBaseGlobally( const std::string & pickledKey );


	// TODO:BAR Remove these eventually
	const Mercury::Address & addr() const 	{ return channel_.addr(); }
	Mercury::Bundle & bundle() 				{ return channel_.bundle(); }
	void send() 							{ channel_.send(); }
};

#endif // BASEAPPMGR_GATEWAY
