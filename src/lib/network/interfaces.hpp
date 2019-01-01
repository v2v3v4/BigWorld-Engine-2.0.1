/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef NETWORK_INTERFACES_HPP
#define NETWORK_INTERFACES_HPP

#include "misc.hpp"

namespace Mercury
{

class Channel;

/**
 *	This class declares an interface for receiving Mercury messages.
 *	Classes that can handle general messages from Mercury needs to
 *	implement this.
 *
 *	@ingroup mercury
 */
class InputMessageHandler
{
public:
	virtual ~InputMessageHandler() {};

	/**
	 * 	This method is called by Mercury to deliver a message.
	 *
	 * 	@param source	The address at which the message originated.
	 * 	@param header	This contains the message type, size, and flags.
	 * 	@param data		The actual message data.
	 */
	virtual void handleMessage( const Address & source,
		UnpackedMessageHeader & header,
		BinaryIStream & data ) = 0;
};


/**
 *	This class declares an interface for receiving reply messages.
 *	When a client issues a request, an interface of this type should
 *	be provided to handle the reply.
 *
 *	@see Bundle::startRequest
 *	@see Bundle::startReply
 *
 *	@ingroup mercury
 */
class ReplyMessageHandler
{
public:
	virtual ~ReplyMessageHandler() {};

	/**
	 * 	This method is called by Mercury to deliver a reply message.
	 *
	 * 	@param source	The address at which the message originated.
	 * 	@param header	This contains the message type, size, and flags.
	 * 	@param data		The actual message data.
	 * 	@param arg		This is user-defined data that was passed in with
	 * 					the request that generated this reply.
	 */
	virtual void handleMessage( const Address & source,
		UnpackedMessageHeader & header,
		BinaryIStream & data,
		void * arg ) = 0;

	/**
	 * 	This method is called by Mercury when the request fails. The
	 * 	normal reason for this happening is a timeout.
	 *
	 * 	@param exception	The reason for failure.
	 * 	@param arg			The user-defined data associated with the request.
	 */
	virtual void handleException( const NubException & exception,
		void * arg ) = 0;

	virtual void handleShuttingDown( const NubException & exception,
		void * arg )
	{
		WARNING_MSG( "ReplyMessageHandler::handleShuttingDown: "
				"Not handled. Possible memory leak.\n" );
	}
};


/**
 *	This class implements ReplyMessageHandler and can be used by handlers with
 *	a handleException method that is safe to call while shutting down.
 */
class ShutdownSafeReplyMessageHandler : public ReplyMessageHandler
{
public:
	virtual void handleShuttingDown( const NubException & exception,
		void * arg )
	{
		this->handleException( exception, arg );
	}
};


/**
 * 	This class defines an interface for receiving socket events.
 * 	Since Mercury runs the event loop, it is useful to be able
 * 	to register additional file descriptors, and receive callbacks
 * 	when they are ready for IO.
 *
 * 	@see EventDispatcher::registerFileDescriptor
 *
 * 	@ingroup mercury
 */
class InputNotificationHandler
{
public:
	virtual ~InputNotificationHandler() {};

	/**
	 *	This method is called when a file descriptor is ready for
	 *	reading.
	 *
	 *	@param fd	The file descriptor
	 *
	 * 	@return The return value is ignored. Implementors should return 0.
	 */
	virtual int handleInputNotification( int fd ) = 0;
};


/**
 *	This class declares an interface for receiving notifications when a bundle
 *	is processed.
 *
 *	@see InterfaceTable::pBundleEventHandler
 *
 *	@ingroup mercury
 */
class BundleEventHandler
{
public:
	virtual ~BundleEventHandler() {}


	/**
	 * 	This method is called when the bundle has started processing, before
	 * 	any of its messages are delivered.
	 */
	virtual void onBundleStarted( Channel * pChannel ) {}


	/**
	 * 	This method is called after all messages in a bundle have
	 * 	been delivered.
	 */
	virtual void onBundleFinished( Channel * pChannel ) {}
};


/**
 *  This class defines an interface for objects used for priming bundles on
 *  channels with data.  It is used by ServerConnection and Proxy to write the
 *  'authenticate' message to the start of each bundle.
 *
 *  @see Channel::bundlePrimer
 */
class BundlePrimer
{
public:
	virtual ~BundlePrimer() {}

	/**
	 *  This method is called by the channel just after the bundle is cleared.
	 */
	virtual void primeBundle( Bundle & bundle ) = 0;

	/**
	 *  This method should return the number of non RELIABLE_DRIVER messages
	 *  that the primer writes to the bundle.
	 */
	virtual int numUnreliableMessages() const = 0;
};


/**
 *	This interface is used to receive notification that a channel has timed out.
 */
class ChannelTimeOutHandler
{
public:
	virtual void onTimeOut( Channel * pChannel ) = 0;
};


} // namespace Mercury

#endif // NETWORK_INTERFACES_HPP
