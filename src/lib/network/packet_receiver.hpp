/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PACKET_RECEIVER_HPP
#define PACKET_RECEIVER_HPP

#include "interfaces.hpp"

#include "fragmented_bundle.hpp"
#include "once_off_packet.hpp"
#include "packet.hpp"
#include "packet_receiver_stats.hpp"

namespace Mercury
{

class Channel;
class EventDispatcher;
class NetworkInterface;
class ProcessSocketStatsHelper;

/**
 *	This class is responsible for receiving Mercury packets from a network
 *	socket.
 */
class PacketReceiver : public InputNotificationHandler
{
public:
	PacketReceiver( Endpoint & socket, NetworkInterface & networkInterface );
	~PacketReceiver();

	Reason processPacket( const Address & addr, Packet * p,
			ProcessSocketStatsHelper * pStatsHelper );
	Reason processFilteredPacket( const Address & addr, Packet * p,
			ProcessSocketStatsHelper * pStatsHelper );

	PacketReceiverStats & stats()		{ return stats_; }

#if ENABLE_WATCHERS
	static WatcherPtr pWatcher();
#endif // ENABLE_WATCHERS

private:
	virtual int handleInputNotification( int fd );
	bool processSocket( bool expectingPacket );
	bool checkSocketErrors( int len, bool expectingPacket );

	Reason processOrderedPacket( const Address & addr, Packet * p,
		Channel * pChannel, ProcessSocketStatsHelper * pStatsHelper );

	bool processPiggybacks( const Address & addr,
			Packet * p, ProcessSocketStatsHelper * pStatsHelper );
	EventDispatcher & dispatcher();


	// Member data
	Endpoint & socket_;
	NetworkInterface & networkInterface_;

	// Stores the packet as an optimisation for processSocket.
	PacketPtr pNextPacket_;
	PacketReceiverStats stats_;
	OnceOffReceiver onceOffReceiver_;
};

} // namespace Mercury

#endif //  PACKET_RECEIVER_HPP
