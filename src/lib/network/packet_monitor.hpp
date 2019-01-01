/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PACKET_MONITOR_HPP
#define PACKET_MONITOR_HPP

namespace Mercury
{

class Address;
class Packet;


/**
 *	This class defines an interface that can be used to receive
 *	a callback whenever an incoming or outgoing packet passes
 *	through Mercury.
 *
 *	@see NetworkInterface::setPacketMonitor
 *
 *	@ingroup mercury
 */
class PacketMonitor
{
public:
	virtual ~PacketMonitor() {};

	/**
	 *	This method is called when Mercury sends a packet.
	 *
	 *	@param addr 	The destination address of the packet.
	 *	@param packet	The actual packet.
	 */
	virtual void packetOut( const Address & addr, const Packet & packet ) = 0;

	/**
	 * 	This method is called when Mercury receives a packet, before
	 * 	it is processed.
	 *
	 * 	@param addr		The source address of the packet.
	 * 	@param packet	The actual packet.
	 */
	virtual void packetIn( const Address& addr, const Packet& packet ) = 0;
};

} // namespace Mercury

#endif // PACKET_MONITOR_HPP
