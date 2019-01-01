/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "third_party/CppUnitLite2/src/CppUnitLite2.h"
#include "common_interface.hpp"

#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "network/channel_finder.hpp"

Mercury::Channel * pFromChannel;
Mercury::Channel * pToChannel;

int g_numSent = 0;
const int NUM_SENDS = 1000;

namespace
{

class LocalHandler : public CommonHandler, public TimerHandler
{
public:
	LocalHandler( Mercury::EventDispatcher & dispatcher ) :
		dispatcher_( dispatcher )
	{
	}

protected:
	virtual void on_msg1( const Mercury::Address & srcAddr,
			const CommonInterface::msg1Args & args )
	{
		if (args.data != 0)
		{
			dispatcher_.breakProcessing();
		}
	}

	void handleTimeout( TimerHandle handle, void * arg )
	{
		if (arg)
		{
			this->recreateChannel( handle, 
								   static_cast<Mercury::Channel **>(arg) );
		}
		else
		{
			this->sendMsg1();
		}
	}

	void recreateChannel( TimerHandle handle, Mercury::Channel ** ppChannel )
	{	
		Mercury::Channel * pOldChannel = *ppChannel;
		Mercury::Channel * pNewChannel =
			new Mercury::Channel( pOldChannel->networkInterface(), 
								  pOldChannel->addr(),
								  Mercury::Channel::INTERNAL, 1.f, NULL, 1 );
		MemoryOStream mos;
		pOldChannel->addToStream( mos );
		pOldChannel->destroy();

		pNewChannel->initFromStream( mos, pNewChannel->addr() );
		pNewChannel->isLocalRegular( false );
		pNewChannel->isRemoteRegular( false );

		*ppChannel = pNewChannel;
	}

	void sendMsg1()
	{
		if (g_numSent < NUM_SENDS)
		{
			Mercury::Bundle & bundle = pFromChannel->bundle();
			CommonInterface::msg1Args & args =
				CommonInterface::msg1Args::start( bundle );
			args.seq = g_numSent;
			// The last one indicates "disconnect".
			args.data = (g_numSent == NUM_SENDS - 1);
			g_numSent++;
			pFromChannel->send();	
		}
	}

private:
	Mercury::EventDispatcher & dispatcher_;
};


class MyChannelFinder : public Mercury::ChannelFinder
{
public:
	MyChannelFinder ( Mercury::Channel*& myChannel ):
		myChannel_( myChannel )
	{
	}

	Mercury::Channel* find( Mercury::ChannelID id, 
							const Mercury::Address & srcAddr,
							const Mercury::Packet * pPacket,
							bool & rHasBeenHandled )
	{
		if ( id == 1 )
		{
			rHasBeenHandled = false;
			return myChannel_;
		}
		return NULL;
	}

	Mercury::Channel *& myChannel_;
};

} // anonymous namespace



TEST( Channel_stream )
{
	Mercury::EventDispatcher dispatcher;

	Mercury::NetworkInterface fromInterface( &dispatcher,
		Mercury::NETWORK_INTERFACE_INTERNAL );
	Mercury::NetworkInterface toInterface( &dispatcher,
		Mercury::NETWORK_INTERFACE_INTERNAL );

	LocalHandler handler( dispatcher );

	fromInterface.pExtensionData( &handler );
	toInterface.pExtensionData( &handler );

	MyChannelFinder fromChannelFinder ( pFromChannel );
	MyChannelFinder toChannelFinder ( pToChannel );
	fromInterface.registerChannelFinder( &fromChannelFinder );
	toInterface.registerChannelFinder( &toChannelFinder );

	CommonInterface::registerWithInterface( fromInterface );
	CommonInterface::registerWithInterface( toInterface );

	pFromChannel = 
		new Mercury::Channel( fromInterface, toInterface.address(),
							  Mercury::Channel::INTERNAL, 1.f, NULL, 1 );
	pFromChannel->isLocalRegular( false );
	pFromChannel->isRemoteRegular( false );

	pToChannel = 
		new Mercury::Channel( toInterface, fromInterface.address(),
							  Mercury::Channel::INTERNAL, 1.f, NULL, 1 );
	pToChannel->isLocalRegular( false );
	pToChannel->isRemoteRegular( false );
	TimerHandle h1 = dispatcher.addTimer( 3000, &handler, &pToChannel );
	TimerHandle h2 = dispatcher.addTimer( 5000, &handler, &pFromChannel );
	TimerHandle h3 = dispatcher.addTimer( 7000, &handler, NULL );

	fromInterface.setLossRatio( 0.1 );

	dispatcher.processUntilBreak();

	h1.cancel();
	h2.cancel();
	h3.cancel();

	pFromChannel->destroy();
	pToChannel->destroy();
}

// test_stream.cpp
