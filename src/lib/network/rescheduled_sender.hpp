/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RESCHEDULED_SENDER_HPP
#define RESCHEDULED_SENDER_HPP

#include "basictypes.hpp"
#include "packet.hpp"

namespace Mercury
{

class NetworkInterface;

/**
 *	This class is responsible for sending packets that have artificial latency.
 */
class RescheduledSender : public TimerHandler
{
public:
	RescheduledSender( NetworkInterface & networkInterface, const Address & addr,
			Packet * pPacket, int latencyMilli );

	virtual void handleTimeout( TimerHandle handle, void * arg );
	virtual void onRelease( TimerHandle handle, void * arg );
private:
	NetworkInterface & interface_;
	Address addr_;
	PacketPtr pPacket_;
};

} // namespace Mercury

#endif // RESCHEDULED_SENDER_HPP
