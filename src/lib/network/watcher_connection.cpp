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

#include "watcher_connection.hpp"

#if ENABLE_WATCHERS

#include "event_dispatcher.hpp"
#include "remote_endpoint.hpp"
#include "watcher_nub.hpp"

#include "cstdmf/profile.hpp"

DECLARE_DEBUG_COMPONENT2( "Network", 0 )


// -----------------------------------------------------------------------------
// Section: WatcherConnection
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
WatcherConnection::WatcherConnection( WatcherNub & nub,
		Mercury::EventDispatcher & dispatcher,
		Endpoint * pEndpoint ) :
	nub_( nub ),
	dispatcher_( dispatcher ),
	pEndpoint_( pEndpoint ),
	receivedSize_( 0 ),
	messageSize_( 0 ),
	pBuffer_( NULL )
{
	dispatcher_.registerFileDescriptor( *pEndpoint, this );
}


WatcherConnection::~WatcherConnection()
{
	dispatcher_.deregisterFileDescriptor( *pEndpoint_ );
	delete pEndpoint_;

	delete pBuffer_; // Generally already NULL
}


int WatcherConnection::handleInputNotification( int fd )
{
	AUTO_SCOPED_PROFILE( "watchersTCP" );

	if (pBuffer_ == NULL)
	{
		if (!this->recvSize())
		{
			delete this;
			return 0;
		}
	}

	if (pBuffer_)
	{
		if (!this->recvMsg())
		{
			delete this;
			return 0;
		}
	}

	return 0;
}


/**
 *	This method attempts to receive the size of the message form the current
 *	connection.
 *
 *	@return false if the connection has been lost.
 */
bool WatcherConnection::recvSize()
{
	MF_ASSERT( receivedSize_ < sizeof( messageSize_ ) );
	MF_ASSERT( pBuffer_ == NULL );

	int size = pEndpoint_->recv( ((char *)&messageSize_) + receivedSize_,
			sizeof( messageSize_ ) - receivedSize_ );

	if (size <= 0)
	{
		// Lost watcher connection
		return false;
	}

	receivedSize_ += size;

	if (receivedSize_ == sizeof( messageSize_ ))
	{
		if (messageSize_ > uint32(WN_PACKET_SIZE_TCP))
		{
			ERROR_MSG( "WatcherConnection::recvSize: "
							"Invalid message size %d from %s\n",
						messageSize_, pEndpoint_->getLocalAddress().c_str() );
			return false;
		}

		pBuffer_ = new char[ messageSize_ ];
		receivedSize_ = 0;
	}

	return true;
}


/**
 *	This method attempts to receive the message from the current connection.
 *
 *	@return false if the connection has been lost.
 */
bool WatcherConnection::recvMsg()
{
	MF_ASSERT( pBuffer_ );

	int size = pEndpoint_->recv( pBuffer_ + receivedSize_,
			messageSize_ - receivedSize_ );

	if (size <= 0)
	{
		// Lost watcher connection
		return false;
	}

	receivedSize_ += size;

	if (receivedSize_ == messageSize_)
	{
		RemoteEndpoint remoteEndpoint( *pEndpoint_ );
		nub_.processRequest( pBuffer_, messageSize_, remoteEndpoint );

		delete pBuffer_;
		pBuffer_ = NULL;
		receivedSize_ = 0;
		messageSize_ = 0;
	}

	return true;
}


#endif /* ENABLE_WATCHERS */

// watcher_connection.cpp
