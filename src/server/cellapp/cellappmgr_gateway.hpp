/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CELLAPPMGR_GATEWAY_HPP
#define CELLAPPMGR_GATEWAY_HPP

#include "cstdmf/stdmf.hpp"
#include "cstdmf/watcher.hpp"
#include "network/basictypes.hpp"
#include "network/channel_owner.hpp"
#include "network/misc.hpp"

#include "server/common.hpp"
#include "server/manager_app_gateway.hpp"

typedef uint8 SharedDataType;

class CellApp;
class Cells;

/**
 * 	This is a simple helper class that is used to represent the remote cell
 * 	manager.
 */
class CellAppMgrGateway : public ManagerAppGateway
{
public:
	CellAppMgrGateway( Mercury::NetworkInterface & interface );

	void add( const Mercury::Address & addr, uint16 viewerPort,
			Mercury::ReplyMessageHandler * pReplyHandler );

	void informOfLoad( float load );

	void handleCellAppDeath( const Mercury::Address & addr );

	void setSharedData( const std::string & key, const std::string & value,
		   SharedDataType type );
	void delSharedData( const std::string & key, SharedDataType type );

	void ackCellAppDeath( const Mercury::Address & deadAddr );
	void ackShutdown( ShutDownStage stage );

	void updateBounds( const Cells & cells, int maxEntityOffload );

	void onManagerRebirth( CellApp & cellApp, const Mercury::Address & addr );

	void shutDownSpace( SpaceID spaceID );

	// TODO:BAR Remove these
	const Mercury::Address & addr() const 	{ return channel_.addr(); }
	Mercury::Bundle & bundle() 				{ return channel_.bundle(); }
	void send() 							{ channel_.send(); }
};


#endif // CELLAPPMGR_GATEWAY_HPP
