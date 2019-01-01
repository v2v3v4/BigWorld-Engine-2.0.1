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

#include "rescheduled_sender.hpp"

#include "event_dispatcher.hpp"
#include "network_interface.hpp"

namespace Mercury
{

/**
 *	Constructor for RescheduledSender.
 */
RescheduledSender::RescheduledSender( NetworkInterface & networkInterface,
		const Address & addr, Packet * pPacket, int latencyMilli ) :
	interface_( networkInterface ),
	addr_( addr ),
	pPacket_( pPacket )
{
	interface_.dispatcher().addOnceOffTimer( latencyMilli*1000, this );
}


void RescheduledSender::handleTimeout( TimerHandle, void * )
{
	Channel * pChannel = pPacket_->hasFlags( Packet::FLAG_ON_CHANNEL ) ?
		interface_.findChannel( addr_ ) : NULL;

	interface_.sendRescheduledPacket( addr_, pPacket_.get(), pChannel );

}

void RescheduledSender::onRelease( TimerHandle, void * )
{
	delete this;
}
} // namespace Mercury

// rescheduled_sender.cpp
