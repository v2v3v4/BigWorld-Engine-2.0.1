/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MESSAGE_HANDLERS_HPP
#define MESSAGE_HANDLERS_HPP

#include "server_connection.hpp"

#include "network/interfaces.hpp"
#include "network/network_interface.hpp"

// -----------------------------------------------------------------------------
// Section: Various Mercury InputMessageHandler implementations
// -----------------------------------------------------------------------------


class BinaryIStream;

namespace Mercury
{
	class Address;
	class UnpackedMessageHeader;
} // namespace Mercury

// -----------------------------------------------------------------------------
// Section: EntityMessageHandler declaration
// -----------------------------------------------------------------------------

/**
 *	This class is used to handle handleDataFromServer message from the server.
 */
class EntityMessageHandler : public Mercury::InputMessageHandler
{
public:
	EntityMessageHandler()
	{}

protected:
	/// This method handles messages from Mercury.
	virtual void handleMessage( const Mercury::Address & /* srcAddr */,
			Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data )
	{
		ServerConnection * pServConn =
			(ServerConnection *)header.pInterface->pExtensionData();
		pServConn->handleEntityMessage( header.identifier & 0x7F, data );
	}
};

/**
 * 	@internal
 *	Objects of this type are used to handle messages destined for the client
 *	application.
 */
template <class ARGS>
class ClientMessageHandler : public Mercury::InputMessageHandler
{
public:
	/// This typedef declares a member function on the ServerConnection
	/// class that handles a single message.
	typedef void (ServerConnection::*Handler)( const ARGS & args );

	/// This is the constructor
	ClientMessageHandler( Handler handler ) : handler_( handler ) {}

private:
	/**
	 * 	This method handles a Mercury message, and dispatches it to
	 * 	the correct member function of the ServerConnection object.
	 */
	virtual void handleMessage( const Mercury::Address & /*srcAddr*/,
			Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data )
	{
#ifndef _BIG_ENDIAN
		ARGS & args = *(ARGS*)data.retrieve( sizeof(ARGS) );
#else
		// Poor old big-endian clients can't just refer directly to data
		// on the network stream, they have to stream it off.
		ARGS args;
		data >> args;
#endif

		ServerConnection * pServConn =
			(ServerConnection *)header.pInterface->pExtensionData();
		(pServConn->*handler_)( args );
	}

	Handler handler_;
};


/**
 * 	@internal
 *	Objects of this type are used to handle variable length messages destined
 *	for the client application.
 */
class ClientVarLenMessageHandler : public Mercury::InputMessageHandler
{
public:
	/// This typedef declares a member function on the ServerConnection
	/// class that handles a single variable lengthmessage.
	typedef void (ServerConnection::*Handler)( BinaryIStream & stream );

	/// This is the constructor.
	ClientVarLenMessageHandler( Handler handler ) : handler_( handler ) {}

private:
	/**
	 * 	This method handles a Mercury message, and dispatches it to
	 * 	the correct member function of the ServerConnection object.
	 */
	virtual void handleMessage( const Mercury::Address & /*srcAddr*/,
			Mercury::UnpackedMessageHeader & header,
			BinaryIStream & data )
	{
		ServerConnection * pServConn =
			(ServerConnection *)header.pInterface->pExtensionData();
		(pServConn->*handler_)( data );
	}

	Handler handler_;
};


/**
 * 	@internal
 *	Objects of this type are used to handle variable length messages destined
 *	for the client application. The handler is also passed the source address.
 */
class ClientVarLenWithAddrMessageHandler : public Mercury::InputMessageHandler
{
	public:
		/// This typedef declares a member function on the ServerConnection
		/// class that handles a single variable lengthmessage.
		typedef void (ServerConnection::*Handler)(
				const Mercury::Address & srcAddr,
				BinaryIStream & stream );

		/// This is the constructor.
		ClientVarLenWithAddrMessageHandler( Handler handler ) :
			handler_( handler ) {}

	private:
		/**
		 * 	This method handles a Mercury message, and dispatches it to
		 * 	the correct member function of the ServerConnection object.
		 */
		virtual void handleMessage( const Mercury::Address & srcAddr,
				Mercury::UnpackedMessageHeader & header,
				BinaryIStream & data )
		{
			ServerConnection * pServConn =
				(ServerConnection *)header.pInterface->pExtensionData();
			(pServConn->*handler_)( srcAddr, data );
		}

		Handler handler_;
};


#endif // MESSAGE_HANDLERS_HPP
