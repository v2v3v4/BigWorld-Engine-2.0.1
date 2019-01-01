/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BASE_MESSAGE_FORWARDER_HPP
#define BASE_MESSAGE_FORWARDER_HPP

#include "network/basictypes.hpp"
#include "network/channel.hpp"

#include <map>

class BinaryIStream;
namespace Mercury
{
	class NetworkInterface;
	class UnpackedMessageHeader;
}

/**
 *	This class is responsible for maintaining a mapping for bases that have
 *	been offloaded to their offloaded destination addresses, and forwarding
 *	messages destined for those bases to their new location.
 */
class BaseMessageForwarder
{
public:
	BaseMessageForwarder( Mercury::NetworkInterface & networkInterface );

	void addForwardingMapping( EntityID entityID, 
			const Mercury::Address & destAddr );
	
	bool forwardIfNecessary( EntityID entityID, 
			const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	Mercury::ChannelPtr getForwardingChannel( EntityID entityID );

private:
	typedef std::map< EntityID, Mercury::Address > Map;

	Map 							map_;
	Mercury::NetworkInterface & 	networkInterface_;
};

#endif // BASE_MESSAGE_FORWARDER_HPP
