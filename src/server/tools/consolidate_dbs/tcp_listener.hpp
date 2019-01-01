/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TCP_LISTENER_HPP
#define TCP_LISTENER_HPP

#include "network/interfaces.hpp"

// -----------------------------------------------------------------------------
// Section: TcpListener
// -----------------------------------------------------------------------------
/**
 *	TCP listener socket that just accepts all connections to it.
 */
template <class CONNECTION_MGR>
class TcpListener : public Mercury::InputNotificationHandler
{
public:
	TcpListener( CONNECTION_MGR& connectionMgr ) : 
		connectionMgr_( connectionMgr ), endPoint_()
	{}
	~TcpListener();

	bool init( uint16 port = 0, uint32 ip = INADDR_ANY, int backLog = 5 );

	void getBoundAddr( Mercury::Address& addr );

	// InputNotificationHandler override
	virtual int handleInputNotification( int fd );

private:
	CONNECTION_MGR& 	connectionMgr_;
	Endpoint 			endPoint_;
};

/**
 * 	Destructor
 */
template <class CONNECTION_MGR>
TcpListener< CONNECTION_MGR >::~TcpListener()
{
	connectionMgr_.dispatcher().deregisterFileDescriptor( endPoint_ );
}

/**
 * 	Binds this listener to a specific IP adress and port.
 */
template <class CONNECTION_MGR>
bool TcpListener< CONNECTION_MGR >::init( uint16 port, uint32 ip, 
		int backLog )
{
	endPoint_.socket( SOCK_STREAM );
	endPoint_.setnonblocking( true );
	if (endPoint_.bind( port, ip ) == -1)
	{
		connectionMgr_.onFailedBind( ip, port );
		return false;
	}
	else
	{
		// Make socket into listener socket.
		endPoint_.listen( std::min( SOMAXCONN, backLog ) );
		connectionMgr_.dispatcher().registerFileDescriptor( endPoint_, this );
	}
	return true;
}

/**
 * 	Gets the bound address and port.
 */
template <class CONNECTION_MGR>
void TcpListener< CONNECTION_MGR >::getBoundAddr( Mercury::Address& addr )
{
	uint32 ip;
	endPoint_.getlocaladdress( &addr.port, &ip );
	addr.ip = ip;
}

/**
 *	Handles an incoming connection
 */
template <class CONNECTION_MGR>
int TcpListener< CONNECTION_MGR >::handleInputNotification( int fd )
{
	sockaddr_in addr;
	socklen_t size = sizeof(addr);

	int socket = accept( endPoint_, (sockaddr *)&addr, &size );

	if (socket == -1)
	{
		connectionMgr_.onFailedAccept( addr.sin_addr.s_addr, addr.sin_port );
	}
	else
	{
		connectionMgr_.onAcceptedConnection( socket, addr.sin_addr.s_addr,
				addr.sin_port );
	}

	return 0;
}


#endif /*TCP_LISTENER_HPP*/
