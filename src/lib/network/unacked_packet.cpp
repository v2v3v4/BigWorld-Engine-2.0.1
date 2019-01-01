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

#include "unacked_packet.hpp"

namespace Mercury
{

// -----------------------------------------------------------------------------
// Section: UnackedPacket
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
Channel::UnackedPacket::UnackedPacket( Packet * pPacket ) :
	pPacket_( pPacket )
{
}


/**
 *	This method reads this object from the input stream.
 */
Channel::UnackedPacket * Channel::UnackedPacket::initFromStream(
	BinaryIStream & data, uint64 timeNow )
{
	PacketPtr pPacket = Packet::createFromStream( data, Packet::UNACKED_SEND );

	if (pPacket)
	{
		UnackedPacket * pInstance = new UnackedPacket( pPacket.getObject() );

		data >> pInstance->lastSentAtOutSeq_;

		pInstance->lastSentTime_ = timeNow;
		pInstance->wasResent_ = false;

		return pInstance;
	}
	else
	{
		return NULL;
	}
}


/**
 *	This method adds this object to the input stream.
 */
void Channel::UnackedPacket::addToStream(
	UnackedPacket * pInstance, BinaryOStream & data )
{
	if (pInstance)
	{
		Packet::addToStream( data, pInstance->pPacket_.getObject(),
			Packet::UNACKED_SEND );

		data << pInstance->lastSentAtOutSeq_;
	}
	else
	{
		Packet::addToStream( data, (Packet*)NULL, Packet::UNACKED_SEND );
	}
}

} // namespace Mercury

// unacked_packet.cpp
