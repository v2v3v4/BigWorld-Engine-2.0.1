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

#include "endpoint.hpp"
#include "cstdmf/binary_stream.hpp"

#ifdef unix
extern "C" {
	/// 32 bit unsigned integer.
	#define __u32 uint32
	/// 8 bit unsigned integer.
	#define __u8 uint8
	#include <linux/errqueue.h>
}
#include <signal.h>
#include <sys/uio.h>
#include <netinet/ip.h>
#else	// not unix
	// Need to implement if_nameindex functions on Windows
	/** @internal */
	struct if_nameindex
	{

		unsigned int if_index;	/* 1, 2, ... */

		char *if_name;			/* null terminated name: "eth0", ... */

	};

	/** @internal */
	struct if_nameindex *if_nameindex(void)
	{
		static struct if_nameindex staticIfList[3] =
		{ { 1, "eth0" }, { 2, "lo" }, { 0, 0 } };

		return staticIfList;
	}

	/** @internal */
	inline void if_freenameindex(struct if_nameindex *)
	{}
#endif	// not unix

#ifndef CODE_INLINE
#include "endpoint.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "Network", 0 )

/**
 *	This method gets the address of the UDP port that has closed.
 *
 *	@param closedPort	A reference to the address that is to receive the
 *						address of the closed port.
 *
 *	@return True if closedPort was set, otherwise false.
 */
bool Endpoint::getClosedPort( Mercury::Address & closedPort )
{
	bool isResultSet = false;

#ifdef unix
//	MF_ASSERT( errno == ECONNREFUSED );

	struct sockaddr_in	offender;
	offender.sin_family = 0;
	offender.sin_port = 0;
	offender.sin_addr.s_addr = 0;

	struct msghdr	errHeader;
	struct iovec	errPacket;

	char data[ 256 ];
	char control[ 256 ];

	errHeader.msg_name = &offender;
	errHeader.msg_namelen = sizeof( offender );
	errHeader.msg_iov = &errPacket;
	errHeader.msg_iovlen = 1;
	errHeader.msg_control = control;
	errHeader.msg_controllen = sizeof( control );
	errHeader.msg_flags = 0;	// result only

	errPacket.iov_base = data;
	errPacket.iov_len = sizeof( data );

	int errMsgErr = recvmsg( *this, &errHeader, MSG_ERRQUEUE );
	if (errMsgErr < 0)
	{
		return false;
	}

	struct cmsghdr * ctlHeader;

	for (ctlHeader = CMSG_FIRSTHDR(&errHeader);
		ctlHeader != NULL;
		ctlHeader = CMSG_NXTHDR(&errHeader,ctlHeader))
	{
		if (ctlHeader->cmsg_level == SOL_IP &&
			ctlHeader->cmsg_type == IP_RECVERR) break;
	}

	// Was there an IP_RECVERR error.

	if (ctlHeader != NULL)
	{
		struct sock_extended_err * extError =
			(struct sock_extended_err*)CMSG_DATA(ctlHeader);

		// Only use this address if the kernel has the bug where it does not
		// report the packet details.

		if (errHeader.msg_namelen == 0)
		{
			// Finally we figure out whose fault it is except that this is the
			// generator of the error (possibly a machine on the path to the
			// destination), and we are interested in the actual destination.
			offender = *(sockaddr_in*)SO_EE_OFFENDER( extError );
			offender.sin_port = 0;

			ERROR_MSG( "Endpoint::getClosedPort: "
				"Kernel has a bug: recv_msg did not set msg_name.\n" );
		}

		closedPort.ip = offender.sin_addr.s_addr;
		closedPort.port = offender.sin_port;

		isResultSet = true;
	}
#endif // unix

	return isResultSet;
}

/**
 * Generate a address/name map of all network interfaces.
 *
 * @param	interfaces	The map to populate with the interface list.
 *
 * @returns true on success, false on error.
 */
bool Endpoint::getInterfaces( std::map< u_int32_t, std::string > &interfaces )
{
	// Find a list of all of our interfaces
	struct if_nameindex* pIfInfo = if_nameindex();
	if (!pIfInfo)
	{
		ERROR_MSG( "Unable to discover network interfaces.\n" );
		return false;
	}

	int		flags = 0;
	struct if_nameindex* pIfInfoCur = pIfInfo;
	while (pIfInfoCur->if_name)
	{
		flags = 0;
		this->getInterfaceFlags( pIfInfoCur->if_name, flags );

		if ((flags & IFF_UP) && (flags & IFF_RUNNING))
		{
			u_int32_t	addr;
			if (this->getInterfaceAddress( pIfInfoCur->if_name, addr ) == 0)
			{
				interfaces[ addr ] = pIfInfoCur->if_name;
			}
		}
		++pIfInfoCur;
	}
	if_freenameindex(pIfInfo);

	return true;
}


/**
 *  This function finds the default interface, i.e. the one to use if
 *	an IP address is required for a socket that is bound to all interfaces.
 *
 *	Currently, the first valid (non-loopback) interface is used, but this
 *	should really be changed to be whatever interface is used by a local
 *	network broadcast - i.e. the interface that the default route goes over
 */
int Endpoint::findDefaultInterface( char * name )
{
#ifndef unix
	strcpy( name, "eth0" );
	return 0;
#else
	int		ret = -1;

	struct if_nameindex* pIfInfo = if_nameindex();
	if (pIfInfo)
	{
		int		flags = 0;
		struct if_nameindex* pIfInfoCur = pIfInfo;
		while (pIfInfoCur->if_name)
		{
			flags = 0;
			this->getInterfaceFlags( pIfInfoCur->if_name, flags );

			if ((flags & IFF_UP) && (flags & IFF_RUNNING))
			{
				u_int32_t	addr;
				if (this->getInterfaceAddress( pIfInfoCur->if_name, addr ) == 0)
				{
					strcpy( name, pIfInfoCur->if_name );
					ret = 0;

					// we only stop if it's not a loopback address,
					// otherwise we continue, hoping to find a better one
					if (!(flags & IFF_LOOPBACK)) break;
				}
			}
			++pIfInfoCur;
		}
		if_freenameindex(pIfInfo);
	}
	else
	{
		ERROR_MSG( "Endpoint::findDefaultInterface: "
							"if_nameindex returned NULL (%s)\n",
						strerror( errno ) );
	}

	return ret;
#endif // unix
}

/**
 *	This function finds the interfaced specified by a string. The
 *	specification may take the form of a straight interface name,
 *	a IP address (name/dotted decimal), or a netmask (IP/bits).
 */
int Endpoint::findIndicatedInterface( const char * spec, char * name )
{
	// start with it cleared
	name[0] = 0;

	// make sure there's something there
	if (spec == NULL || spec[0] == 0) return -1;

	// set up some working vars
	char * slash;
	int netmaskbits = 32;
	char iftemp[IFNAMSIZ+16];
	strncpy( iftemp, spec, IFNAMSIZ ); iftemp[IFNAMSIZ] = 0;
	u_int32_t addr = 0;

	// see if it's a netmask
	if ((slash = const_cast< char * >( strchr( spec, '/' ) )) && slash-spec <= 16)
	{
		// specified a netmask
		MF_ASSERT( IFNAMSIZ >= 16 );
		iftemp[slash-spec] = 0;
		bool ok = Endpoint::convertAddress( iftemp, addr ) == 0;

		netmaskbits = atoi( slash+1 );
		ok &= netmaskbits > 0 && netmaskbits <= 32;

		if (!ok)
		{
			ERROR_MSG("Endpoint::findIndicatedInterface: "
				"netmask match %s length %s is not valid.\n", iftemp, slash+1 );
			return -1;
		}
	}
	else if (this->getInterfaceAddress( iftemp, addr ) == 0)
	{
		// specified name of interface
		strncpy( name, iftemp, IFNAMSIZ );
	}
	else if (Endpoint::convertAddress( spec, addr ) == 0)
	{
		// specified ip address
		netmaskbits = 32; // redundant but instructive
	}
	else
	{
		ERROR_MSG( "Endpoint::findIndicatedInterface: "
			"No interface matching interface spec '%s' found\n", spec );
		return -1;
	}

	// if we haven't set a name yet then we're supposed to
	// look up the ip address
	if (name[0] == 0)
	{
		int netmaskshift = 32-netmaskbits;
		u_int32_t netmaskmatch = ntohl(addr);

		std::vector< std::string > interfaceNames;

		struct if_nameindex* pIfInfo = if_nameindex();
		if (pIfInfo)
		{
			struct if_nameindex* pIfInfoCur = pIfInfo;
			while (pIfInfoCur->if_name)
			{
				interfaceNames.push_back( pIfInfoCur->if_name );
				++pIfInfoCur;
			}
			if_freenameindex(pIfInfo);
		}

		std::vector< std::string >::iterator iter = interfaceNames.begin();

		while (iter != interfaceNames.end())
		{
			u_int32_t tip = 0;
			char * currName = (char *)iter->c_str();

			if (this->getInterfaceAddress( currName, tip ) == 0)
			{
				u_int32_t htip = ntohl(tip);

				if ((htip >> netmaskshift) == (netmaskmatch >> netmaskshift))
				{
					//DEBUG_MSG("Endpoint::bind(): found a match\n");
					strncpy( name, currName, IFNAMSIZ );
					break;
				}
			}

			++iter;
		}

		if (name[0] == 0)
		{
			uint8 * qik = (uint8*)&addr;
			ERROR_MSG( "Endpoint::findIndicatedInterface: "
				"No interface matching netmask spec '%s' found "
				"(evals to %d.%d.%d.%d/%d)\n", spec,
				qik[0], qik[1], qik[2], qik[3], netmaskbits );

			return -2; // parsing ok, just didn't match
		}
	}

	return 0;
}


/**
 * 	This method converts a string containing an IP address into
 * 	a 32 integer in network byte order. It handles both numeric
 * 	and named addresses.
 *
 *	@param string	The address as a string
 *	@param address	The address is returned here as an integer.
 *
 *	@return 0 if successful, -1 otherwise.
 */
int Endpoint::convertAddress(const char * string, u_int32_t & address)
{
	u_int32_t	trial;

	// first try it as numbers and dots
	#ifdef unix
	if (inet_aton( string, (struct in_addr*)&trial ) != 0)
	#else
	if ( (trial = inet_addr( string )) != INADDR_NONE )
	#endif
		{
			address = trial;
			return 0;
		}

	// ok, try looking it up then
	struct hostent * hosts = gethostbyname( string );
	if (hosts != NULL)
	{
		address = *(u_int32_t*)(hosts->h_addr_list[0]);
		return 0;
	}

	// that didn't work either - I give up then
	return -1;
}


/**
 *	This method returns the current size of the transmit and receive queues
 *	for this socket. This method is only implemented on Unix.
 *
 *	@param tx	The current size of the transmit queue is returned here.
 *	@param rx 	The current size of the receive queue is returned here.
 *
 *	@return 0 if successful, -1 otherwise.
 */
#ifdef unix
int Endpoint::getQueueSizes( int & tx, int & rx ) const
{
	int	ret = -1;

	u_int16_t	nport = 0;
	this->getlocaladdress(&nport,NULL);

	char		match[16];
	bw_snprintf( match, sizeof(match), "%04X", (int)ntohs(nport) );

	FILE * f = fopen( "/proc/net/udp", "r" );

	if (!f)
	{
		ERROR_MSG( "Endpoint::getQueueSizes: "
				"could not open /proc/net/udp: %s\n",
			strerror( errno ) );
		return -1;
	}

	char	aline[256];
	fgets( aline, 256, f );

	while (fgets( aline, 256, f) != NULL)
	{	// it goes "iiii: hhhhhhhh:pppp" (could check ip too 'tho)
		if(!strncmp( aline+4+1+ 1 +8+1, match, 4 ))
		{	// then goes " hhhhhhhh:pppp ss tttttttt:rrrrrrrr"
			char * start = aline+4+1+ 1 +8+1+4+ 1 +8+1+4+ 1 +2+ 1;
			start[8] = 0;
			tx = strtol( start, NULL, 16 );

			start += 8+1;
			start[8] = 0;
			rx = strtol( start, NULL, 16 );

			ret = 0;

			break;
		}
	}

	fclose(f);

	return ret;
}
#else
int Endpoint::getQueueSizes( int &, int & ) const
{
	return -1;
}
#endif


/**
 *  This method returns either the send or receive buffer size for this socket,
 *  or -1 on error.  You should pass either SO_RCVBUF or SO_SNDBUF as the
 *  argument to this method.
 */
int Endpoint::getBufferSize( int optname ) const
{
#ifdef unix
	MF_ASSERT( optname == SO_SNDBUF || optname == SO_RCVBUF );

	int recvbuf = -1;
	socklen_t rbargsize = sizeof( int );
	int rberr = getsockopt( socket_, SOL_SOCKET, optname,
		(char*)&recvbuf, &rbargsize );

	if (rberr == 0 && rbargsize == sizeof( int ))
	{
		return recvbuf;
	}
	else
	{
		ERROR_MSG( "Endpoint::getBufferSize: "
			"Failed to read option %s: %s\n",
			optname == SO_SNDBUF ? "SO_SNDBUF" : "SO_RCVBUF",
			strerror( errno ) );

		return -1;
	}

#else
	return -1;
#endif
}


/**
 *  This method sets either the send or receive buffer size for this socket.
 *  You should pass either SO_RCVBUF or SO_SNDBUF as the optname argument to
 *  this method.
 */
bool Endpoint::setBufferSize( int optname, int size )
{
#ifdef unix
	setsockopt( socket_, SOL_SOCKET, optname, (const char*)&size,
		sizeof( size ) );
#endif

	return this->getBufferSize( optname ) >= size;
}


/**
 *	This helper method wait until exactly gramSize data has been read.
 *
 *	True if gramSize was read, otherwise false (usually indicating the
 *	connection was lost.
 */
bool Endpoint::recvAll( void * gramData, int gramSize )
{
	while (gramSize > 0)
	{
		int len = this->recv( gramData, gramSize );

		if (len <= 0)
		{
			if (len == 0)
			{
				WARNING_MSG( "Endpoint::recvAll: Connection lost\n" );
			}
			else
			{
				WARNING_MSG( "Endpoint::recvAll: Got error '%s'\n",
					strerror( errno ) );
			}

			return false;
		}
		gramSize -= len;
		gramData = ((char *)gramData) + len;
	}

	return true;
}


/**
 *	This method returns the address that this endpoint is bound to.
 */
Mercury::Address Endpoint::getLocalAddress() const
{
	Mercury::Address addr( 0, 0 );

	if (this->getlocaladdress( (u_int16_t*)&addr.port,
				(u_int32_t*)&addr.ip ) == -1)
	{
		ERROR_MSG( "Endpoint::getLocalAddress: Failed\n" );
	}

	return addr;
}


/**
 *	This method returns the address that this endpoint is bound to.
 */
Mercury::Address Endpoint::getRemoteAddress() const
{
	Mercury::Address addr( 0, 0 );

	if (this->getremoteaddress( (u_int16_t*)&addr.port,
				(u_int32_t*)&addr.ip ) == -1)
	{
		ERROR_MSG( "Endpoint::getRemoteAddress: Failed\n" );
	}

	return addr;
}


/**
 *	Global function to initialise the network
 */
static bool s_networkInitted = false;

void initNetwork()
{
	if (s_networkInitted) return;
	s_networkInitted = true;

#if !defined( PLAYSTATION3 )

#ifndef unix

	WSAData wsdata;
	WSAStartup( 0x202, &wsdata );
#endif // !unix

#endif
}


#ifdef MF_SERVER
namespace
{

class StaticIniter
{
public:
	StaticIniter()
	{
		struct sigaction ignore;
		ignore.sa_handler = SIG_IGN;
		sigaction( SIGPIPE, &ignore, NULL );
	}
};

StaticIniter g_staticIniter;

} // anonymous namespace
#endif

// endpoint.cpp
