/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "loginapp.hpp"

// -----------------------------------------------------------------------------
// Section: InputMessageHandlers
// -----------------------------------------------------------------------------

/**
 *	This class is used to handle messages from this process Mercury interfaces.
 */
class LoginAppRawMessageHandler : public Mercury::InputMessageHandler
{
	public:
		typedef void (LoginApp::*Handler)(
			const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header,
			BinaryIStream & stream );

		// Constructors
		LoginAppRawMessageHandler( Handler handler ) :
			handler_( handler )
		{}

	private:
		virtual void handleMessage( const Mercury::Address & srcAddr,
				Mercury::UnpackedMessageHeader & header,
				BinaryIStream & data )
		{
			(LoginApp::instance().*handler_)( srcAddr, header, data );
		}

		Handler handler_;
};


/**
 *	Class for struct-style Mercury message handler objects.
 */
template <class Args> class LoginAppStructMessageHandler :
	public Mercury::InputMessageHandler
{
public:
	typedef void (LoginApp::*Handler)( const Args & args );

	LoginAppStructMessageHandler( Handler handler ) :
		handler_( handler )
	{}

private:
	virtual void handleMessage( const Mercury::Address & srcAddr,
		Mercury::UnpackedMessageHeader & header, BinaryIStream & data )
	{
		Args * pArgs = (Args*)data.retrieve( sizeof(Args) );
		(LoginApp::instance().*handler_)( *pArgs );
	}

	Handler handler_;
};


namespace // anonymous
{

// These are used in the server definitions of the interfaces below.

// LoginInterface
LoginAppRawMessageHandler gLoginHandler( &LoginApp::login );
LoginAppRawMessageHandler gProbeHandler( &LoginApp::probe );

// LoginIntInterface
LoginAppRawMessageHandler gShutDownHandler( &LoginApp::controlledShutDown );

} // namespace (anonymous)


#define DEFINE_SERVER_HERE
#include "login_int_interface.hpp"

#define DEFINE_SERVER_HERE
#include "connection/login_interface.hpp"

// message_handlers.cpp
