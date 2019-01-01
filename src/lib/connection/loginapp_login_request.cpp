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

#include "loginapp_login_request.hpp"

#include "login_handler.hpp"
#include "log_on_params.hpp"
#include "server_connection.hpp"

// -----------------------------------------------------------------------------
// Section: LoginAppLoginRequest
// -----------------------------------------------------------------------------

/**
 *  Constructor.
 */
LoginAppLoginRequest::LoginAppLoginRequest( LoginHandler & parent ) :
	RetryingRequest( parent, parent.loginAddr(), LoginInterface::login )
{
	this->send();
}


/**
 *	Override from RetryingRequest. Stream request arguments on to the bundle.
 */
void LoginAppLoginRequest::addRequestArgs( Mercury::Bundle & bundle )
{
	LogOnParamsPtr pParams = pParent_->pParams();

	bundle << LOGIN_VERSION;

	if (!pParams->addToStream( bundle, LogOnParams::HAS_ALL,
			pParent_->pServerConnection()->pLogOnParamsEncoder() ))
	{
		ERROR_MSG( "LoginAppLoginRequest::addRequestArgs: "
			"Failed to assemble login bundle\n" );

		pParent_->onFailure( Mercury::REASON_CORRUPTED_PACKET );
	}
}


/**
 * 	Override from RetryingRequest. Callback when the request succeeds.
 */
void LoginAppLoginRequest::onSuccess( BinaryIStream & data )
{
	pParent_->onLoginReply( data );
}


/**
 *	Override from RetryingRequest. Callback when the request fails.
 */
void LoginAppLoginRequest::onFailure( Mercury::Reason reason )
{
	pParent_->onFailure( reason );
}


// loginapp_login_request.cpp
