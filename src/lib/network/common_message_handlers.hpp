/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef COMMON_MESSAGE_HANDLERS_HPP
#define COMMON_MESSAGE_HANDLERS_HPP

template <class OBJECT_TYPE>
class MessageHandlerFinder
{
public:
	// This should be specialised for non-ServerApp types
	static OBJECT_TYPE * find( const Mercury::Address & srcAddr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & stream )
	{
		return &ServerApp::getApp< OBJECT_TYPE >( header );
	}
};

// -----------------------------------------------------------------------------
// Section: Message Handler implementation
// -----------------------------------------------------------------------------

/**
 *	Objects of this type are used to handle messages destined for the cellapp, a
 *	space or a cell.
 */
template <class OBJECT_TYPE, class ARGS_TYPE,
		 class FIND_POLICY = MessageHandlerFinder< OBJECT_TYPE > >
class StructMessageHandler : public Mercury::InputMessageHandler
{
	public:
		/**
		 *	This type is the function pointer type that handles the incoming
		 *	message.
		 */
		typedef void (OBJECT_TYPE::*Handler)( const ARGS_TYPE & args );

		/**
		 *	Constructor.
		 */
		StructMessageHandler( Handler handler ) : handler_( handler ) {}

	private:
		// Override
		virtual void handleMessage( const Mercury::Address & srcAddr,
				Mercury::UnpackedMessageHeader & header,
				BinaryIStream & data )
		{
			OBJECT_TYPE * pObject = FIND_POLICY::find( srcAddr, header, data );

			if (pObject != NULL)
			{
				ARGS_TYPE args;
				data >> args;

				(pObject->*handler_)( args );
			}
			else
			{
				ERROR_MSG( "MessageHandler::handleMessage: "
					"Could not find object\n" );

				data.finish();
			}
		}

		Handler handler_;
};


/**
 *	Objects of this type are used to handle messages destined for the cellapp, a
 *	space or a cell.
 */
template <class OBJECT_TYPE, class ARGS_TYPE,
		 class FIND_POLICY = MessageHandlerFinder< OBJECT_TYPE > >
class StructMessageHandlerEx : public Mercury::InputMessageHandler
{
	public:
		/**
		 *	This type is the function pointer type that handles the incoming
		 *	message.
		 */
		typedef void (OBJECT_TYPE::*Handler)(
					const Mercury::Address & srcAddr,
					const Mercury::UnpackedMessageHeader & header,
					const ARGS_TYPE & args );

		/**
		 *	Constructor.
		 */
		StructMessageHandlerEx( Handler handler ) : handler_( handler ) {}

	private:
		// Override
		virtual void handleMessage( const Mercury::Address & srcAddr,
				Mercury::UnpackedMessageHeader & header,
				BinaryIStream & data )
		{
			OBJECT_TYPE * pObject = FIND_POLICY::find( srcAddr, header, data );

			if (pObject != NULL)
			{
				ARGS_TYPE args;
				data >> args;

				(pObject->*handler_)( srcAddr, header, args );
			}
			else
			{
				ERROR_MSG( "MessageHandler::handleMessage: "
					"Could not find object\n" );

				data.finish();
			}
		}

		Handler handler_;
};


/**
 *	Objects of this type are used to handle variable length messages destined
 *	for a cellapp, cell or space.
 */
template <class OBJECT_TYPE,
		 class FIND_POLICY = MessageHandlerFinder< OBJECT_TYPE > >
class StreamMessageHandler : public Mercury::InputMessageHandler
{
	public:
		/**
		 *	This type is the function pointer type that handles the incoming
		 *	message.
		 */
		typedef void (OBJECT_TYPE::*Handler)( BinaryIStream & stream );

		/**
		 *	Constructor.
		 */
		StreamMessageHandler( Handler handler ) : handler_( handler ) {}

	private:
		// Override
		virtual void handleMessage( const Mercury::Address & srcAddr,
				Mercury::UnpackedMessageHeader & header,
				BinaryIStream & data )
		{
			OBJECT_TYPE * pObject = FIND_POLICY::find( srcAddr, header, data );

			if (pObject != NULL)
			{
				(pObject ->*handler_)( data );
			}
			else
			{
				ERROR_MSG( "StreamMessageHandler::handleMessage: "
					"Could not find object\n" );
			}
		}

		Handler handler_;
};

/**
 *	Objects of this type are used to handle variable length messages destined
 *	for the cell app, a cell or a space. They simply pass all the mercury
 *	parameters to the handler function.
 */
template <class OBJECT_TYPE,
		 class FIND_POLICY = MessageHandlerFinder< OBJECT_TYPE > >
class StreamMessageHandlerEx : public Mercury::InputMessageHandler
{
	public:
		/**
		 *	This type is the function pointer type that handles the incoming
		 *	message.
		 */
		typedef void (OBJECT_TYPE::*Handler)(
			const Mercury::Address & addr,
			const Mercury::UnpackedMessageHeader & header,
			BinaryIStream & stream );

		/**
		 *	Constructor.
		 */
		StreamMessageHandlerEx( Handler handler ) : handler_( handler ) {}

	private:
		// Override
		virtual void handleMessage( const Mercury::Address & srcAddr,
				Mercury::UnpackedMessageHeader & header,
				BinaryIStream & data )
		{
			OBJECT_TYPE * pObject = FIND_POLICY::find( srcAddr, header, data );

			if (pObject != NULL)
			{
				(pObject->*handler_)( srcAddr, header, data );
			}
			else
			{
				// OK we give up then
				ERROR_MSG( "StreamMessageHandlerEx::handleMessage: "
						"Do not have object for message from %s\n",
					srcAddr.c_str() );
			}
		}

		Handler handler_;
};

#endif // COMMON_MESSAGE_HANDLERS_HPP
