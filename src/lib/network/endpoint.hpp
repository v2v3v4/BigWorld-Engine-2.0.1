/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENDPOINT_HPP
#define ENDPOINT_HPP

#include "cstdmf/debug.hpp"
#include "basictypes.hpp" // For Mercury::Address

#include <map>

#include <sys/types.h>
#if defined( unix ) || defined( PLAYSTATION3 )
	#include <sys/time.h>
	#include <sys/socket.h>
#ifndef PLAYSTATION3
	#include <sys/fcntl.h>
	#include <sys/ioctl.h>
	#include <net/if.h>
#endif
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
#else
	#include <WINSOCK.H>
#endif
#include <errno.h>
#include <stdlib.h>

#ifdef PLAYSTATION3
#include <netex/libnetctl.h>
#endif

#include "cstdmf/memory_stream.hpp"

#ifndef unix

#ifdef PLAYSTATION3

typedef uint8_t 	u_int8_t;
typedef uint16_t 	u_int16_t;
typedef uint32_t 	u_int32_t;

#else

#ifndef socklen_t
	/// The length of a socket address. It is for Windows only.
	typedef int socklen_t;
#endif
	/// A 16 bit unsigned integer.
	typedef u_short u_int16_t;
	/// A 32 bit unsigned integer.
	typedef u_long u_int32_t;

#endif

	/// The maximum length of a network interface name.
	#define IFNAMSIZ 16

#ifndef IFF_UP
	enum
	{
		IFF_UP = 0x1,
		IFF_BROADCAST = 0x2,
		IFF_DEBUG = 0x4,
		IFF_LOOPBACK = 0x8,
		IFF_POINTOPOINT = 0x10,
		IFF_NOTRAILERS = 0x20,
		IFF_RUNNING = 0x40,
		IFF_NOARP = 0x80,
		IFF_PROMISC = 0x100,
		IFF_MULTICAST = 0x1000
	};
#endif
#endif

class BinaryIStream;

/**
 *	This class provides a wrapper around a socket.
 *
 *	@ingroup network
 */
class Endpoint
{
public:
	/// @name Construction/Destruction
	//@{
	Endpoint();
	~Endpoint();
	//@}

	static const int NO_SOCKET = -1;

	/// @name File descriptor access
	//@{
	operator int() const;
	void setFileDescriptor(int fd);
	bool good() const;
	//@}

	/// @name General Socket Methods
	//@{
	void socket( int type );

	int setnonblocking( bool nonblocking );
	int setbroadcast( bool broadcast );
	int setreuseaddr( bool reuseaddr );
	int setkeepalive( bool keepalive );

	int bind( u_int16_t networkPort = 0, u_int32_t networkAddr = INADDR_ANY );

	int joinMulticastGroup( u_int32_t networkAddr );
	int quitMulticastGroup( u_int32_t networkAddr );

	INLINE int close();
	INLINE int detach();

	int getlocaladdress(
		u_int16_t * networkPort, u_int32_t * networkAddr ) const;
	int getremoteaddress(
		u_int16_t * networkPort, u_int32_t * networkAddr ) const;

	Mercury::Address getLocalAddress() const;
	Mercury::Address getRemoteAddress() const;

	const char * c_str() const;
	int getremotehostname( std::string * name ) const;

	bool getClosedPort( Mercury::Address & closedPort );
	//@}

	/// @name Connectionless Socket Methods
	//@{
	int sendto( void * gramData, int gramSize,
		u_int16_t networkPort, u_int32_t networkAddr = BROADCAST);
	INLINE int sendto( void * gramData, int gramSize, struct sockaddr_in & sin );
	INLINE int recvfrom( void * gramData, int gramSize,
		u_int16_t * networkPort, u_int32_t * networkAddr );
	INLINE int recvfrom( void * gramData, int gramSize,
		struct sockaddr_in & sin );
	//@}

	/// @name Connecting Socket Methods
	//@{
	int listen( int backlog = 5 );
	int connect( u_int16_t networkPort, u_int32_t networkAddr = BROADCAST );
	Endpoint * accept(
		u_int16_t * networkPort = NULL, u_int32_t * networkAddr = NULL );

	INLINE int send( const void * gramData, int gramSize );
	int recv( void * gramData, int gramSize );
	bool recvAll( void * gramData, int gramSize );
	//@}

	/// @name Network Interface Methods
	//@{
	int getInterfaceFlags( char * name, int & flags );
	int getInterfaceAddress( const char * name, u_int32_t & address );
	int getInterfaceNetmask( const char * name, u_int32_t & netmask );
	bool getInterfaces( std::map< u_int32_t, std::string > &interfaces );
	int findDefaultInterface( char * name );
	int findIndicatedInterface( const char * spec, char * name );
	static int convertAddress( const char * string, u_int32_t & address );
	//@}

	/// @name Queue Size Methods
	//@{
	int transmitQueueSize() const;
	int receiveQueueSize() const;
	int getQueueSizes( int & tx, int & rx ) const;
	//@}

	/// @name Buffer Size Methods
	//@{
	int getBufferSize( int optname ) const;
	bool setBufferSize( int optname, int size );
	//@}

private:

	/// This is internal socket representation of the Endpoint.
#if defined( unix ) || defined( PLAYSTATION3 )
	int	socket_;
#else //ifdef unix
	SOCKET	socket_;
#endif //def _WIN32
};

extern void initNetwork();


#ifdef CODE_INLINE
#include "endpoint.ipp"
#endif

#endif // ENDPOINT_HPP
