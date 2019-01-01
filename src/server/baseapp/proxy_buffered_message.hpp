/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PROXY_BUFFERED_MESSAGE_HPP
#define PROXY_BUFFERED_MESSAGE_HPP

#include "rate_limit_message_filter.hpp"

/**
 *	A buffered message for a proxy from a client.
 */
class ProxyBufferedMessage : public BufferedMessage
{
public:
	/**
	 * 	Constructor.
	 *
	 * 	@param header 		the message header
	 * 	@param data 		the message data
	 * 	@param pHandler 	the destination input message handler instance
	 * 	@parma callback 	the associated proxy's RateLimitCallback object
	 */
	ProxyBufferedMessage( const Mercury::UnpackedMessageHeader & header,
				BinaryIStream & data,
				Mercury::InputMessageHandler * pHandler ):
			BufferedMessage( header, data, pHandler )
	{}

	/**
	 *	Destructor.
	 */
	virtual ~ProxyBufferedMessage() {}

public: // overridden from BufferedMessage

	virtual void dispatch( RateLimitMessageFilter::Callback * pCallback,
		const Mercury::Address & srcAddr );

};

#endif // PROXY_BUFFERED_MESSAGE_HPP
