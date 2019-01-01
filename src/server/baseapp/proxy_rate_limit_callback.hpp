/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PROXY_RATE_LIMIT_CALLBACK_HPP
#define PROXY_RATE_LIMIT_CALLBACK_HPP

#include "rate_limit_message_filter.hpp"

class Proxy;

/**
 *	Callback for the RateLimitMessageFilter.
 */
class ProxyRateLimitCallback :
		public RateLimitMessageFilter::Callback
{
public:
	/**
	 *	Constructor.
	 *
	 *	@param pProxy 	The associated proxy for this callback.
	 */
	ProxyRateLimitCallback( Proxy * pProxy ): pProxy_( pProxy )
	{}

	/**
	 *	Destructor.
	 */
	virtual ~ProxyRateLimitCallback()
	{}

	/**
	 *	Return the associated proxy.
	 */
	Proxy * pProxy() { return pProxy_; }


public: // From RateLimitMessageFilter::Callback

	virtual BufferedMessage * createBufferedMessage(
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data, Mercury::InputMessageHandler * pHandler );

	virtual void onFilterLimitsExceeded( const Mercury::Address & srcAddr,
		BufferedMessage * pMessage );

private:
	Proxy * pProxy_;

};

#endif // PROXY_RATE_LIMIT_CALLBACK_HPP
