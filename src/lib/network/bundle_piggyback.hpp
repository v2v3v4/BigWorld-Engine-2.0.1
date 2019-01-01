/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BUNDLE_PIGGYBACK_HPP
#define BUNDLE_PIGGYBACK_HPP

#ifdef USE_PIGGIES

#include "network/packet.hpp"
#include "network/reliable_order.hpp"

#include <vector>

namespace Mercury
{

/**
 *  @internal
 *  A Piggyback is the data structure used to represent a piggyback packet
 *  between the call to Bundle::piggyback() and the data actually being
 *  streamed onto the packet footers during NetworkInterface::send().
 */
class BundlePiggyback
{
public:
	BundlePiggyback( Packet * pPacket,
			Packet::Flags flags,
			SeqNum seq,
			int16 len ) :
		pPacket_( pPacket ),
		flags_( flags ),
		seq_( seq ),
		len_( len )
	{}

	PacketPtr		pPacket_; 	///< Original packet messages come from
	Packet::Flags	flags_;     ///< Header for the piggyback packet
	SeqNum			seq_;       ///< Sequence number of the piggyback packet
	int16			len_;       ///< Length of the piggyback packet
	ReliableVector	rvec_;      ///< Reliable messages to go onto the packet
};


typedef std::vector< BundlePiggyback* > BundlePiggybacks;

} // end namespace Mercury

#endif // USE_PIGGIES

#endif // BUNDLE_PIGGYBACK_HPP
