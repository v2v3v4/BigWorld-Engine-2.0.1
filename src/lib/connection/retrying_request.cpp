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

#include "retrying_request.hpp"

#include "login_handler.hpp"
#include "server_connection.hpp"

#include "network/misc.hpp"
#include "network/network_interface.hpp"
#include "network/nub_exception.hpp"

/**
 *  Constructor.
 */
RetryingRequest::RetryingRequest( LoginHandler & parent,
		const Mercury::Address & addr,
		const Mercury::InterfaceElement & ie,
		int retryPeriod,
		int timeoutPeriod,
		int maxAttempts,
		bool useParentNetworkInterface ) :
	pParent_( &parent ),
	pInterface_( NULL ),
	addr_( addr ),
	ie_( ie ),
	timerHandle_(),
	done_( false ),
	retryPeriod_( retryPeriod ),
	timeoutPeriod_( timeoutPeriod ),
	numAttempts_( 0 ),
	numOutstandingAttempts_( 0 ),
	maxAttempts_( maxAttempts )
{
	if (useParentNetworkInterface)
	{
		this->setInterface( &parent.pServerConnection()->networkInterface() );
	}

	parent.addChildRequest( this );
}


/**
 *  Destructor.
 */
RetryingRequest::~RetryingRequest()
{
	MF_ASSERT_DEV( !timerHandle_.isSet() );
}


/**
 *  This method handles a reply to one of our requests.
 */
void RetryingRequest::handleMessage( const Mercury::Address & srcAddr,
	Mercury::UnpackedMessageHeader & header,
	BinaryIStream & data,
	void * arg )
{
	if (!done_)
	{
		this->onSuccess( data );
		this->cancel();
	}

	--numOutstandingAttempts_;
	this->decRef();
}


/**
 *  This method handles an exception, usually a timeout.  Whatever happens, it
 *  indicates that one of our attempts is over.
 */
void RetryingRequest::handleException( const Mercury::NubException & exc,
	void * arg )
{
	--numOutstandingAttempts_;

	// If the last attempt has failed, we're done.
	if (!done_ && numOutstandingAttempts_ == 0)
	{
		// If this request had a maxAttempts of 1, we assume that retrying is
		// taking place by spawning multiple instances of this request (a la
		// BaseAppLoginRequest), so we don't emit this error message.
		if (maxAttempts_ > 1)
		{
			ERROR_MSG( "RetryingRequest::handleException( %s ): "
				"Final attempt of %d has failed (%s), aborting\n",
				ie_.name(), maxAttempts_,
				Mercury::reasonToString( exc.reason() ) );
		}

		this->onFailure( exc.reason() );
		this->cancel();
	}

	this->decRef();
}


/**
 *  This method handles an internally scheduled timeout, which causes another
 *  attempt to be sent.
 */
void RetryingRequest::handleTimeout( TimerHandle handle, void * arg )
{
	this->send();
}


/**
 *  This method sets the network interface to be used by this object.  You
 *  cannot call send() until you have called this.
 */
void RetryingRequest::setInterface( Mercury::NetworkInterface * pInterface )
{
	MF_ASSERT_DEV( pInterface_ == NULL );
	pInterface_ = pInterface;
	timerHandle_ = this->dispatcher().addTimer( retryPeriod_, this );
}


/**
 *  This method sends the request once.  This method should be called as the
 *  last statement in the constructor of a derived class.
 */
void RetryingRequest::send()
{
	if (done_) return;

	if (numAttempts_ < maxAttempts_)
	{
		++numAttempts_;

		Mercury::Bundle bundle;
		bundle.startRequest( ie_, this, NULL, timeoutPeriod_,
			Mercury::RELIABLE_NO );

		this->addRequestArgs( bundle );

		// Calling send may decrement this if an exception occurs.
		RetryingRequestPtr pThis = this;

		this->incRef();
		++numOutstandingAttempts_;

		pInterface_->send( addr_, bundle );
	}
}


/**
 *  This method removes this request from the parent's childRequests_ list,
 *  which means it will be destroyed as soon as all of its outstanding requests
 *  have either been replied to (and ignored) or timed out.
 */
void RetryingRequest::cancel()
{
	timerHandle_.cancel();
	pParent_->removeChildRequest( this );
	done_ = true;

	pInterface_->cancelRequestsFor( this, Mercury::REASON_SUCCESS );
}



Mercury::EventDispatcher & RetryingRequest::dispatcher()
{ 
	return pParent_->pServerConnection()->dispatcher(); 
}

// retrying_request.cpp
