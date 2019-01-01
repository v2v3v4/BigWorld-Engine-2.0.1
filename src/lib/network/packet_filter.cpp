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

#include "packet_filter.hpp"

#ifndef CODE_INLINE
#include "packet_filter.ipp"
#endif

#include "channel.hpp"
#include "network_interface.hpp"
#include "packet_receiver.hpp"

namespace Mercury
{

// ----------------------------------------------------------------
// Section: PacketFilter
// ----------------------------------------------------------------

/*
 *	Documented in header file.
 */
Reason PacketFilter::send( NetworkInterface & networkInterface,
		const Address & addr, Packet * pPacket )
{
	return networkInterface.basicSendWithRetries( addr, pPacket );
}


/**
 *	Documented in header file.
 */
Reason PacketFilter::recv( PacketReceiver & receiver,
							const Address & addr, Packet * pPacket,
	   						ProcessSocketStatsHelper * pStatsHelper )
{
	return receiver.processFilteredPacket( addr, pPacket, pStatsHelper );
}

} // namespace Mercury

// packet_filter.cpp
