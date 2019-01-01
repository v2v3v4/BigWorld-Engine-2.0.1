/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

namespace Mercury
{

/**
 *	This method finds the channel with the given address. If none is found, a
 *	new channel is created.
 */
INLINE Channel & NetworkInterface::findOrCreateChannel( const Address & addr )
{
	return *this->findChannel( addr, /* createAnonymous: */ true );
}


// -----------------------------------------------------------------------------
// Section: Accessors
// -----------------------------------------------------------------------------

/**
 *	This method registers a handler to monitor sent/received packets. Only a
 *	single monitor is supported. To remove the monitor, set it to NULL.
 *
 *	@param pPacketMonitor	An object implementing the PacketMonitor interface.
 */
INLINE void NetworkInterface::setPacketMonitor( PacketMonitor * pPacketMonitor )
{
	pPacketMonitor_ = pPacketMonitor;
}


/**
 *	This method returns the address of the interface this is bound to or the
 *	address of the first non-local interface if it's bound to all interfaces.
 *	A zero address is returned if the interface couldn't initialise.
 */
INLINE const Address & NetworkInterface::address() const
{
	return address_;
}


/**
 *	This method sets the minimum and maximum latency associated with this
 *	interface. If non-zero, packets will be randomly delayed before they are
 *	sent.
 *
 *	@param latencyMin	The minimum latency in seconds
 *	@param latencyMax	The maximum latency in seconds
 */
INLINE void NetworkInterface::setLatency( float latencyMin, float latencyMax )
{
	// Convert to milliseconds
	artificialLatencyMin_ = int( latencyMin * 1000 );
	artificialLatencyMax_ = int( latencyMax * 1000 );
}


/**
 * 	This method sets the average packet loss for this interface. If non-zero,
 * 	packets will be randomly dropped, and not sent.
 *
 * 	@param lossRatio	The ratio of packets to drop. Setting to 0.0 disables
 *		artificial packet dropping. 1.0 means all packets are dropped.
 */
INLINE void NetworkInterface::setLossRatio( float lossRatio )
{
	artificialDropPerMillion_ = int( lossRatio * 1000000 );
}

} // namespace Mercury

// network_interface.ipp
