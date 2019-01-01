/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "manager_app_gateway.hpp"

#include "cstdmf/watcher.hpp"
#include "network/bundle.hpp"
#include "network/machined_utils.hpp"

ManagerAppGateway::ManagerAppGateway( Mercury::NetworkInterface & networkInterface,
			const Mercury::InterfaceElement & retireAppIE ) :
		channel_( networkInterface, Mercury::Address::NONE ),
		retireAppIE_( retireAppIE )
{}


ManagerAppGateway::~ManagerAppGateway()
{}

bool ManagerAppGateway::init( const char * interfaceName, int numRetries )
{
	Mercury::Address addr;

	Mercury::Reason reason =
		Mercury::MachineDaemon::findInterface( interfaceName,
			0, addr, numRetries );

	if (reason == Mercury::REASON_SUCCESS)
	{
		channel_.addr( addr );
	}

	// This channel is irregular until we start the game tick timer.
	this->isRegular( false );

	return reason == Mercury::REASON_SUCCESS;
}


void ManagerAppGateway::retireApp()
{
	Mercury::Bundle & bundle = channel_.bundle();
	bundle.startMessage( retireAppIE_ );
	bundle << int8( 0 ); // dummy
	channel_.send();
}


void ManagerAppGateway::addWatchers( const char * name, Watcher & watcher )
{
	watcher.addChild( name,	Mercury::Channel::pWatcher(), &channel_ );
}

// manager_app_gateway.cpp
