/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// endpoint.ipp


#ifdef CODE_INLINE
    #define INLINE    inline
#else
	/// INLINE macro.
    #define INLINE
#endif

/**
 * 	This is the default constructor. The socket is not created until the socket
 * 	method is called.
 *
 * 	@see socket(int type)
 */
INLINE Endpoint::Endpoint() : socket_( NO_SOCKET )
{
}

/**
 *	This is the destructor.
 *	Note: use 'detach' if you don't want your socket to disappear when the
 *	Endpoint is destructed.
 *
 *	@see detach
 */
INLINE Endpoint::~Endpoint()
{
	this->close();
}

/**
 * 	This operator returns the file descriptor for this endpoint.
 */
INLINE Endpoint::operator int() const
{
	return socket_;
}

/**
 *	This method sets the file descriptor used by this endpoint.
 */
INLINE void Endpoint::setFileDescriptor( int fd )
{
	socket_ = fd;
}

/**
 * 	This method returns true if this endpoint is a valid socket.
 */
INLINE bool Endpoint::good() const
{
	return (socket_ != NO_SOCKET);
}

/**
 * 	This method creates a socket of the requested type.
 * 	It initialises Winsock if necessary.
 *
 * 	@param type		Normally SOCK_STREAM or SOCK_DGRAM
 */
INLINE void Endpoint::socket(int type)
{
	this->setFileDescriptor( ::socket( AF_INET, type, 0 ) );
#ifndef unix
#ifndef PLAYSTATION3
	if ((socket_ == INVALID_SOCKET) && (WSAGetLastError() == WSANOTINITIALISED))
	{
		// not initialised, so do so...
		initNetwork();

		// ... and try it again
		this->setFileDescriptor( ::socket( AF_INET, type, 0 ) );
	}
#endif
#endif
}

/**
 *	This method controls the blocking mode of the socket.
 *	When a socket is set to non-blocking mode, socket calls
 *	will return immediately.
 *
 *	@param nonblocking	The desired blocking mode.
 */
INLINE int Endpoint::setnonblocking(bool nonblocking)
{
#ifdef unix
	int val = nonblocking ? O_NONBLOCK : 0;
	return ::fcntl(socket_,F_SETFL,val);
#elif defined( PLAYSTATION3 )
	int val = nonblocking ? 1 : 0;
	return setsockopt( socket_, SOL_SOCKET, SO_NBIO, &val, sizeof(int) );
#else
	u_long val = nonblocking ? 1 : 0;
	return ::ioctlsocket(socket_,FIONBIO,&val);
#endif
}

/**
 *	This method toggles the broadcast mode of the socket.
 *
 *	@param broadcast	The desired broadcast mode.
 */
INLINE int Endpoint::setbroadcast(bool broadcast)
{
#ifdef unix
	int val;
	if (broadcast)
	{
		val = 2;
		::setsockopt( socket_, SOL_IP, IP_MULTICAST_TTL, &val, sizeof(int) );
	}
#else
	bool val;
#endif
	val = broadcast ? 1 : 0;
	return ::setsockopt(socket_,SOL_SOCKET,SO_BROADCAST,
		(char*)&val,sizeof(val));
}

/**
 *	This method toggles the reuse address mode of the socket.
 *
 *	@param reuseaddr	The desired reuse address mode.
 */
INLINE int Endpoint::setreuseaddr( bool reuseaddr )
{
#ifdef unix
	int val;
#else
	bool val;
#endif
	val = reuseaddr ? 1 : 0;
	return ::setsockopt( socket_, SOL_SOCKET, SO_REUSEADDR,
		(char*)&val, sizeof(val) );
}
INLINE int Endpoint::setkeepalive( bool keepalive )
{
#ifdef unix
	int val;
#else
	bool val;
#endif
	val = keepalive ? 1 : 0;
	return ::setsockopt( socket_, SOL_SOCKET, SO_KEEPALIVE,
		(char*)&val, sizeof(val) );
}

/**
 *	This method binds the socket to a given address and port.
 *
 *	@param networkPort	The port, in network byte order.
 *	@param networkAddr	The address, in network byte order.
 *
 *	@return	0 if successful, -1 otherwise.
 */
INLINE int Endpoint::bind( u_int16_t networkPort, u_int32_t networkAddr )
{
	sockaddr_in	sin;
	sin.sin_family = AF_INET;
	sin.sin_port = networkPort;
	sin.sin_addr.s_addr = networkAddr;
	return ::bind( socket_, (struct sockaddr*)&sin, sizeof(sin) );
}

/**
 *	This method joins the socket up to the given multicast group
 */
INLINE int Endpoint::joinMulticastGroup( u_int32_t networkAddr )
{
#ifdef unix
	struct ip_mreqn req;
	req.imr_multiaddr.s_addr = networkAddr;
	req.imr_address.s_addr = INADDR_ANY;
	req.imr_ifindex = 0;
	return ::setsockopt( socket_, SOL_IP, IP_ADD_MEMBERSHIP, &req, sizeof(req));
#else
	return -1;
#endif
}

/**
 *	This method gets the socket to quit the given multicast group
 */
INLINE int Endpoint::quitMulticastGroup( u_int32_t networkAddr )
{
#ifdef unix
	struct ip_mreqn req;
	req.imr_multiaddr.s_addr = networkAddr;
	req.imr_address.s_addr = INADDR_ANY;
	req.imr_ifindex = 0;
	return ::setsockopt( socket_, SOL_IP, IP_DROP_MEMBERSHIP,&req, sizeof(req));
#else
	return -1;
#endif
}

/**
 *	This method closes the socket associated with this endpoint.
 */
INLINE int Endpoint::close()
{
	if (socket_ == NO_SOCKET)
	{
		return 0;
	}

#ifdef unix
	int ret = ::close(socket_);
#elif defined( PLAYSTATION3 )
	int ret = ::socketclose(socket_);
#else
	int ret = ::closesocket(socket_);
#endif
	if (ret == 0)
	{
		this->setFileDescriptor( NO_SOCKET );
	}
	return ret;
}

/**
 * 	This method detaches the socket from this endpoint.
 * 	When the destructor is called, the socket will not be automatically closed.
 */
INLINE int Endpoint::detach()
{
	int ret = socket_;
	this->setFileDescriptor( NO_SOCKET );
	return ret;
}

/**
 *	This method returns the local address and port to which this endpoint
 *	is bound.
 *
 *	@param networkPort	The port is returned here in network byte order.
 *	@param networkAddr	The address is returned here in network byte order.
 *
 *	@return 0 if successful, or -1 if an error occurred.
 */
INLINE int Endpoint::getlocaladdress(
	u_int16_t * networkPort, u_int32_t * networkAddr ) const
{
	sockaddr_in		sin;
	socklen_t		sinLen = sizeof(sin);
	int ret = ::getsockname( socket_, (struct sockaddr*)&sin, &sinLen );
	if (ret == 0)
	{
		if (networkPort != NULL) *networkPort = sin.sin_port;
		if (networkAddr != NULL) *networkAddr = sin.sin_addr.s_addr;
	}
	return ret;
}


/**
 *	This method returns the remote address and port to which this endpoint
 *	is connected.
 *
 *	@param networkPort	The port is returned here in network byte order.
 *	@param networkAddr	The address is returned here in network byte order.
 *
 *	@return 0 if successful, or -1 if an error occurred.
 */
INLINE int Endpoint::getremoteaddress(
	u_int16_t * networkPort, u_int32_t * networkAddr ) const
{
	sockaddr_in		sin;
	socklen_t		sinLen = sizeof(sin);
	int ret = ::getpeername( socket_, (struct sockaddr*)&sin, &sinLen );
	if (ret == 0)
	{
		if (networkPort != NULL) *networkPort = sin.sin_port;
		if (networkAddr != NULL) *networkAddr = sin.sin_addr.s_addr;
	}
	return ret;
}


/**
 *  This method returns the string representation of the address this endpoint
 *  is bound to.
 */
INLINE const char * Endpoint::c_str() const
{
	Mercury::Address addr;
	this->getlocaladdress( &(u_int16_t&)addr.port, &(u_int32_t&)addr.ip );
	return addr.c_str();
}
/**
 * 	This method returns the hostname of the remote computer
 *
 * 	@param host			The string to return the hostname in
 *
 * 	@return 0 if successful, or -1 if an error occurred.
 */
INLINE int Endpoint::getremotehostname( std::string * host ) const
{
	sockaddr_in		sin;
	socklen_t		sinLen = sizeof(sin);
	int ret = ::getpeername( socket_, (struct sockaddr*)&sin, &sinLen );
	if (ret == 0)
	{
		hostent* h = gethostbyaddr( (char*) &sin.sin_addr,
				sizeof( sin.sin_addr ), AF_INET);

		if (h)
		{
			*host = h->h_name;
		}
		else
		{
			ret = -1;
		}
	}

	return ret;
}


/**
 * 	This method sends a packet to the given address.
 *
 * 	@param gramData		Pointer to a data buffer containing the packet.
 * 	@param gramSize		Number of bytes in the data buffer.
 * 	@param networkPort	Destination port, in network byte order.
 * 	@param networkAddr	Destination address, in network byte order.
 */
INLINE int Endpoint::sendto( void * gramData, int gramSize,
	u_int16_t networkPort, u_int32_t networkAddr )
{
	sockaddr_in	sin;
	sin.sin_family = AF_INET;
	sin.sin_port = networkPort;
	sin.sin_addr.s_addr = networkAddr;

	return this->sendto( gramData, gramSize, sin );
}

/**
 * 	This method sends a packet to the given address.
 *
 * 	@param gramData		Pointer to a data buffer containing the packet.
 * 	@param gramSize		Number of bytes in the data buffer.
 * 	@param sin			Destination address.
 */
INLINE int Endpoint::sendto( void * gramData, int gramSize,
	struct sockaddr_in & sin )
{
	return ::sendto( socket_, (char*)gramData, gramSize,
		0, (sockaddr*)&sin, sizeof(sin) );
}


/**
 *	This method attempts to receive a packet.
 *
 *	@param gramData		Pointer to a data buffer to receive the packet.
 *	@param gramSize		Number of bytes in the data buffer.
 *	@param networkPort	The port from which the packet originated is returned here,
 *	                    if this pointer is non-NULL.
 *	@param networkAddr	The address from which the packet originated is returned here,
 *						if this pointer is non-NULL.
 *
 *	@return The number of bytes received, or -1 if an error occurred.
 */
INLINE int Endpoint::recvfrom( void * gramData, int gramSize,
	u_int16_t * networkPort, u_int32_t * networkAddr )
{
	sockaddr_in sin;
	int result = this->recvfrom( gramData, gramSize, sin );

	if (result >= 0)
	{
		if (networkPort != NULL) *networkPort = sin.sin_port;
		if (networkAddr != NULL) *networkAddr = sin.sin_addr.s_addr;
	}

	return result;
}


/**
 *	This method attempts to receive a packet.
 *
 *	@param gramData		Pointer to a data buffer to receive the packet.
 *	@param gramSize		Number of bytes in the data buffer.
 *	@param sin			The address from which the packet originated is returned
 *						here.
 *
 *	@return The number of bytes received, or -1 if an error occurred.
 */
INLINE int Endpoint::recvfrom( void * gramData, int gramSize,
	struct sockaddr_in & sin )
{
	socklen_t		sinLen = sizeof(sin);
	int ret = ::recvfrom( socket_, (char*)gramData, gramSize,
		0, (sockaddr*)&sin, &sinLen );

	return ret;
}

/**
 *	This method instructs this endpoint to listen for incoming connections.
 */
INLINE int Endpoint::listen( int backlog )
{
	return ::listen( socket_, backlog );
}

/**
 *	This method connections this endpoint to a destination address
 */
INLINE int Endpoint::connect( u_int16_t networkPort, u_int32_t networkAddr )
{
	sockaddr_in	sin;
	sin.sin_family = AF_INET;
	sin.sin_port = networkPort;
	sin.sin_addr.s_addr = networkAddr;

	return ::connect( socket_, (sockaddr*)&sin, sizeof(sin) );
}

/**
 *	This method accepts a connection on the socket listen queue, returning
 *	a new endpoint if successful, or NULL if not. The remote port and
 *	address are set into the pointers passed in if not NULL.
 */
INLINE Endpoint * Endpoint::accept(
	u_int16_t * networkPort, u_int32_t * networkAddr )
{
	sockaddr_in		sin;
	socklen_t		sinLen = sizeof(sin);
	int ret = ::accept( socket_, (sockaddr*)&sin, &sinLen);
#if defined( unix ) || defined( PLAYSTATION3 )
	if (ret < 0) return NULL;
#else
	if (ret == INVALID_SOCKET) return NULL;
#endif

	Endpoint * pNew = new Endpoint();
	pNew->setFileDescriptor( ret );

	if (networkPort != NULL) *networkPort = sin.sin_port;
	if (networkAddr != NULL) *networkAddr = sin.sin_addr.s_addr;

	return pNew;
}


/**
 * 	This method sends some data to the given address.
 *
 * 	@param gramData		Pointer to a data buffer
 * 	@param gramSize		Number of bytes in the data buffer.
 */
INLINE int Endpoint::send( const void * gramData, int gramSize )
{
	return ::send( socket_, (char*)gramData, gramSize, 0 );
}


/**
 *	This method attempts to receive some data
 *
 *	@param gramData		Pointer to a data buffer
 *	@param gramSize		Number of bytes in the data buffer.
 *
 *	@return The number of bytes received, or -1 if an error occurred.
 */
INLINE int Endpoint::recv( void * gramData, int gramSize )
{
	return ::recv( socket_, (char*)gramData, gramSize, 0 );
}


#ifdef unix
/**
 *	This method returns the flags associated with the given interface.
 *
 *	@param name		Interface name for which flags are needed.
 *	@param flags	The flags are returned here.
 *
 *	@return	0 if successful, 1 otherwise.
 */
INLINE int Endpoint::getInterfaceFlags( char * name, int & flags )
{
	struct ifreq	request;

	strncpy( request.ifr_name, name, IFNAMSIZ );
	if (ioctl( socket_, SIOCGIFFLAGS, &request ) != 0)
	{
		return -1;
	}

	flags = request.ifr_flags;
	return 0;
}


/**
 * 	This method returns the address to which an interface is bound.
 *
 * 	@param name		Name of the interface.
 * 	@param address	The address is returned here.
 *
 * 	@return 0 if successful, 1 otherwise.
 */
INLINE int Endpoint::getInterfaceAddress( const char * name, u_int32_t & address )
{
	struct ifreq	request;

	strncpy( request.ifr_name, name, IFNAMSIZ );
	if (ioctl( socket_, SIOCGIFADDR, &request ) != 0)
	{
		return -1;
	}

	if (request.ifr_addr.sa_family == AF_INET)
	{
		address = ((sockaddr_in*)&request.ifr_addr)->sin_addr.s_addr;
		return 0;
	}
	else
	{
		return -1;
	}
}


/**
 * 	This method returns the netmask for a local interface.
 *
 * 	@param name		Name of the interface.
 * 	@param netmask	The netmask is returned here.
 *
 * 	@return 0 if successful, 1 otherwise.
 */
INLINE int Endpoint::getInterfaceNetmask( const char * name,
	u_int32_t & netmask )
{
	struct ifreq request;
	strncpy( request.ifr_name, name, IFNAMSIZ );

	if (ioctl( socket_, SIOCGIFNETMASK, &request ) != 0)
	{
		return -1;
	}

	netmask = ((sockaddr_in&)request.ifr_netmask).sin_addr.s_addr;

	return 0;
}

#else

/**
 *	This method returns the flags associated with the given interface.
 *
 *	@param name		Interface name for which flags are needed.
 *	@param flags	The flags are returned here.
 *
 *	@return	0 if successful, 1 otherwise.
 */
INLINE int Endpoint::getInterfaceFlags( char * name, int & flags )
{
	if (!strcmp(name,"eth0"))
	{
		flags = IFF_UP | IFF_BROADCAST | IFF_NOTRAILERS |
			IFF_RUNNING | IFF_MULTICAST;
		return 0;
	}
	else if (!strcmp(name,"lo"))
	{
		flags = IFF_UP | IFF_LOOPBACK | IFF_RUNNING;
		return 0;
	}
	return -1;
}

/**
 * 	This method returns the address to which an interface is bound.
 *
 * 	@param name		Name of the interface.
 * 	@param address	The address is returned here.
 *
 * 	@return 0 if successful, 1 otherwise.
 */
INLINE int Endpoint::getInterfaceAddress( const char * name, u_int32_t & address )
{
	if (!strcmp(name,"eth0"))
	{
#if defined( PLAYSTATION3 )
		CellNetCtlInfo netInfo;
		int ret = cellNetCtlGetInfo( CELL_NET_CTL_INFO_IP_ADDRESS, &netInfo );
		MF_ASSERT( ret == 0 );
		int ip0, ip1, ip2, ip3;
		sscanf( netInfo.ip_address, "%d.%d.%d.%d", &ip0, &ip1, &ip2, &ip3 );
		address = ( ip0 << 24 ) | ( ip1 << 16 ) | ( ip2 << 8 ) | ( ip3 << 0 );
#else
		char	myname[256];
		::gethostname(myname,sizeof(myname));

		struct hostent * myhost = gethostbyname(myname);
		if (!myhost)
		{
			return -1;
		}

		address = ((struct in_addr*)(myhost->h_addr_list[0]))->s_addr;
#endif
		return 0;
	}
	else if (!strcmp(name,"lo"))
	{
		address = htonl(0x7F000001);
		return 0;
	}

	return -1;
}
#endif

/**
 *	This method returns the current size of the transmit queue for this socket.
 *	It is only implemented on Unix.
 *
 *	@return	Current transmit queue size in bytes.
 */
INLINE int Endpoint::transmitQueueSize() const
{
	int tx=0, rx=0;
	this->getQueueSizes( tx, rx );
	return tx;
}

/**
 *	This method returns the current size of the receive queue for this socket.
 *	It is only implemented on Unix.
 *
 *	@return	Current receive queue size in bytes.
 */
INLINE int Endpoint::receiveQueueSize() const
{
	int tx=0, rx=0;
	this->getQueueSizes( tx, rx );
	return rx;
}

// endpoint.ipp
