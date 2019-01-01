/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ANONYMOUS_CHANNEL_CLIENT_HPP
#define ANONYMOUS_CHANNEL_CLIENT_HPP

#include "network/interfaces.hpp"

namespace Mercury
{
class ChannelOwner;
class InterfaceMinder;
class NetworkInterface;
}

#include <string>

/**
 *	This class is used to handle channels to processes that may not have
 *	explicit knowledge of us.
 */
class AnonymousChannelClient : public Mercury::InputMessageHandler
{
public:
	AnonymousChannelClient();
	~AnonymousChannelClient();

	bool init( Mercury::NetworkInterface & interface,
		Mercury::InterfaceMinder & interfaceMinder,
		const Mercury::InterfaceElement & birthMessage,
		const char * componentName,
		int numRetries );

	Mercury::ChannelOwner * pChannelOwner() const	{ return pChannelOwner_; }

private:
	virtual void handleMessage( const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data );

	Mercury::ChannelOwner * pChannelOwner_;
	std::string				interfaceName_;
};

#define BW_INIT_ANONYMOUS_CHANNEL_CLIENT( INSTANCE, INTERFACE,				\
		CLIENT_INTERFACE, SERVER_INTERFACE, NUM_RETRIES )					\
	INSTANCE.init( INTERFACE,												\
		CLIENT_INTERFACE::gMinder,											\
		CLIENT_INTERFACE::SERVER_INTERFACE##Birth,							\
		#SERVER_INTERFACE,													\
		NUM_RETRIES )														\


#define BW_ANONYMOUS_CHANNEL_CLIENT_MSG( SERVER_INTERFACE )					\
	MERCURY_FIXED_MESSAGE( SERVER_INTERFACE##Birth,							\
		sizeof( Mercury::Address ),											\
		NULL /* Handler set by BW_INIT_ANONYMOUS_CHANNEL_CLIENT */ )

#endif // ANONYMOUS_CHANNEL_CLIENT_HPP
