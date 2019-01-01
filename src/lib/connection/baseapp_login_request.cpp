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

#include "baseapp_login_request.hpp"

#include "baseapp_ext_interface.hpp"
#include "login_handler.hpp"
#include "server_connection.hpp"


#include "cstdmf/binary_stream.hpp"

#include "network/network_interface.hpp"


// -----------------------------------------------------------------------------
// Section: BaseAppLoginRequest
// -----------------------------------------------------------------------------


/**
 *  Constructor.
 */
BaseAppLoginRequest::BaseAppLoginRequest( LoginHandler & parent ) :
	RetryingRequest( parent, parent.baseAppAddr(),
		BaseAppExtInterface::baseAppLogin,
		DEFAULT_RETRY_PERIOD,
		DEFAULT_TIMEOUT_PERIOD,
		/* maxAttempts: */ 1,
		/* useParentNetworkInterface: */ false ),
	attempt_( parent.numBaseAppLoginAttempts() )
{
	ServerConnection * pServConn = parent.pServerConnection();

	// Each instance has its own network interface.  We need to do this to cope
	// with strange multi-level NATing issues in China.
	if (attempt_ == 0)
	{
		this->setInterface( &(pServConn->networkInterface()) );
		pServConn->pInterface( NULL );
	}
	else
	{
		Mercury::NetworkInterface * pInterface =
			new Mercury::NetworkInterface( &this->dispatcher(),
				Mercury::NETWORK_INTERFACE_EXTERNAL );
		this->setInterface( pInterface );

		// This temporary interface must serve all the interfaces that the main
		// interface serves, because if the initial downstream packets are lost
		// and the reply to the baseAppLogin request actually comes back as a
		// piggyback on a packet with other ClientInterface messages, this
		// interface will need to know how to process them.
		pServConn->registerInterfaces( *pInterface_ );
	}

	pChannel_ = new Mercury::Channel(
		*pInterface_, parent.baseAppAddr(),
		Mercury::Channel::EXTERNAL,
		/* minInactivityResendDelay: */ 1.0,
		pServConn->pFilter().get() );

	// The channel is irregular until we get cellPlayerCreate (i.e. entities
	// enabled).
	pChannel_->isLocalRegular( false );
	pChannel_->isRemoteRegular( false );
	this->send();
}


/**
 *  Destructor.
 */
BaseAppLoginRequest::~BaseAppLoginRequest()
{
	// The winner's pChannel_ will be NULL since it has been transferred to the
	// servconn.
	// delete pChannel_;
	if (pChannel_)
	{
		pChannel_->destroy();
		pChannel_ = NULL;
		pParent_->addCondemnedInterface( pInterface_ );
		pInterface_ = NULL;
	}
}


/**
 *  This method creates another instance of this class, instead of the default
 *  behaviour which is to just resend another request from this object.
 */
void BaseAppLoginRequest::handleTimeout( TimerHandle handle, void * arg )
{
	// Each request should only spawn one other request
	timerHandle_.cancel();

	if (!done_)
	{
		pParent_->sendBaseAppLogin();
	}
}


/**
 *  Streams on required args for baseapp login.
 */
void BaseAppLoginRequest::addRequestArgs( Mercury::Bundle & bundle )
{
	// Send the loginKey and number of attempts (for debugging reasons)
	bundle << pParent_->replyRecord().sessionKey << attempt_;
}


/**
 *	This method handles the reply to the baseAppLogin message. Getting this
 *	called means that this is the winning BaseAppLoginHandler.
 */
void BaseAppLoginRequest::onSuccess( BinaryIStream & data )
{
	SessionKey sessionKey = 0;
	data >> sessionKey;

	pParent_->onBaseAppReply( this, sessionKey );

	// Forget about our Channel because it has been transferred to the
	// ServerConnection.
	pChannel_ = NULL;
}


// baseapp_login_request.cpp
