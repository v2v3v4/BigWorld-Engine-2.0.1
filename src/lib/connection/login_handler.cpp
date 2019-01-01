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

#include "login_handler.hpp"

#include "loginapp_login_request.hpp"
#include "server_connection.hpp"

#include "network/network_interface.hpp"

namespace // anonymous
{

/// How many times should the BaseApp login message be sent before giving up.
const int MAX_BASEAPP_LOGIN_ATTEMPTS = 10;

} // namespace (anonymous)

// -----------------------------------------------------------------------------
// Section: LoginHandler
// -----------------------------------------------------------------------------

/**
 *	Constructor
 */
LoginHandler::LoginHandler(
		ServerConnection* pServerConnection, LogOnStatus loginNotSent ) :
	loginAppAddr_( Mercury::Address::NONE ),
	baseAppAddr_( Mercury::Address::NONE ),
	pParams_( NULL ),
	pServerConnection_( pServerConnection ),
	replyRecord_(),
	done_( loginNotSent != LogOnStatus::NOT_SET ),
	status_( loginNotSent ),
	errorMsg_(),
	numBaseAppLoginAttempts_( 0 ),
	childRequests_()
{
}


LoginHandler::~LoginHandler()
{
}


void LoginHandler::start( const Mercury::Address & loginAppAddr,
	LogOnParamsPtr pParams )
{
	loginAppAddr_ = loginAppAddr;
	pParams_ = pParams;
	this->sendLoginAppLogin();
}


void LoginHandler::startWithBaseAddr( const Mercury::Address & baseAppAddr,
	SessionKey sessionKey )
{
	baseAppAddr_ = baseAppAddr;
	replyRecord_.sessionKey = sessionKey;
	replyRecord_.serverAddr = baseAppAddr;
	this->sendBaseAppLogin();
}


void LoginHandler::finish()
{
	// Clear out all child requests
	while (!childRequests_.empty())
	{
		(*childRequests_.begin())->cancel();
	}
	pServerConnection_->dispatcher().breakProcessing();

	// Recreate the ServerConnection's network interface if necessary.
	if (!pServerConnection_->hasInterface())
	{
		Mercury::NetworkInterface * pInterface =
			new Mercury::NetworkInterface( 
				&pServerConnection_->dispatcher(),
				Mercury::NETWORK_INTERFACE_EXTERNAL );
		pServerConnection_->pInterface( pInterface );
	}

	done_ = true;
}


/**
 *  This method sends the login request to the server.
 */
void LoginHandler::sendLoginAppLogin()
{
	new LoginAppLoginRequest( *this );
}


/**
 *	This method handles the login reply message from the LoginApp.
 */
void LoginHandler::onLoginReply( BinaryIStream & data )
{
	data >> status_;

	if (status_ == LogOnStatus::LOGGED_ON)
	{
		// The reply record is symmetrically encrypted.
#ifdef USE_OPENSSL
		if (pServerConnection_->pFilter())
		{
			MemoryOStream clearText;
			pServerConnection_->pFilter()->decryptStream( data, clearText );
			clearText >> replyRecord_;
		}
		else
#endif
		{
			data >> replyRecord_;
		}

		// Correct sized reply
		if (!data.error())
		{
			baseAppAddr_ = replyRecord_.serverAddr;
			INFO_MSG( "LoginHandler::onLoginReply: "
					"Connecting to BaseApp at %s\n",
				baseAppAddr_.c_str() );
			this->sendBaseAppLogin();
			errorMsg_ = "";
		}
		else
		{
			ERROR_MSG( "LoginHandler::handleMessage: "
				"Got reply of unexpected size (%d)\n",
				data.remainingLength() );

			status_ = LogOnStatus::CONNECTION_FAILED;
			errorMsg_ = "Mercury::REASON_CORRUPTED_PACKET";

			this->finish();
		}
	}
	else
	{
		data >> errorMsg_;

		if (errorMsg_.empty())	// this really shouldn't happen
		{
			if (status_ == LogOnStatus::LOGIN_CUSTOM_DEFINED_ERROR)
			{
				errorMsg_ = "Unspecified error.";
			}
			else
			{
				errorMsg_ = "Unelaborated error.";
			}
		}

		this->finish();
	}
}


/**
 *	This method sends a login request to the BaseApp. There can be multiple
 *	login requests outstanding at any given time (sent from different sockets).
 *	Only one will win.
 */
void LoginHandler::sendBaseAppLogin()
{
	if (numBaseAppLoginAttempts_ < MAX_BASEAPP_LOGIN_ATTEMPTS)
	{
		new BaseAppLoginRequest( *this );
		++numBaseAppLoginAttempts_;
	}
	else
	{
		status_ = LogOnStatus::CONNECTION_FAILED;
		errorMsg_ =
			"Unable to connect to BaseApp: "
			"A NAT or firewall error may have occurred?";

		this->finish();
	}
}


/**
 *	This method is called when a reply to baseAppLogin is received from the
 *	BaseApp (or an exception occurred - likely a timeout).
 *
 *	The first successful reply wins and we do not care about the rest.
 */
void LoginHandler::onBaseAppReply( BaseAppLoginRequestPtr pHandler,
	SessionKey sessionKey )
{
	pServerConnection_->pInterface( &pHandler->networkInterface() );
	pServerConnection_->channel( pHandler->channel() );

	// This is the session key that authenticate message should send.
	replyRecord_.sessionKey = sessionKey;
	pServerConnection_->sessionKey( sessionKey );

	// Set the servconn as the bundle primer
	pServerConnection_->channel().bundlePrimer( *this->pServerConnection() );

	this->finish();
}


/**
 *	This method handles a network level failure.
 */
void LoginHandler::onFailure( Mercury::Reason reason )
{
	status_ = LogOnStatus::CONNECTION_FAILED;
	errorMsg_ = "Mercury::";
	errorMsg_ += Mercury::reasonToString( reason );

	this->finish();
}


/**
 *  This method add a RetryingRequest to this LoginHandler.
 */
void LoginHandler::addChildRequest( RetryingRequestPtr pRequest )
{
	childRequests_.insert( pRequest );
}


/**
 *  This method removes a RetryingRequest from this LoginHandler.
 */
void LoginHandler::removeChildRequest( RetryingRequestPtr pRequest )
{
	childRequests_.erase( pRequest );
}


/**
 *  This method adds a condemned network interface to this LoginHandler.  This
 *  is used by BaseAppLoginRequests to clean up their network interfaces as
 *  they are unable to do it themselves.
 */
void LoginHandler::addCondemnedInterface( Mercury::NetworkInterface * pInterface )
{
	pServerConnection_->addCondemnedInterface( pInterface );
}

// login_handler.cpp
