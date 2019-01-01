/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef REMOTE_ENDPOINT_HPP
#define REMOTE_ENDPOINT_HPP

#include "endpoint.hpp"

/**
 *	This class represents either a TCP or UDP Endpoint.
 */
class RemoteEndpoint
{
public:
	RemoteEndpoint( Endpoint & endpoint ) :
		isTCP_( true ),
		endpoint_( endpoint )
	{
	}

	RemoteEndpoint( Endpoint & endpoint, sockaddr_in destAddr ) :
		isTCP_( false ),
		endpoint_( endpoint ),
		destAddr_( destAddr )
	{
	}

	int send( void * data, int32 size )
	{
		if (isTCP_)
		{
			if (endpoint_.send( &size, sizeof( size ) ) == -1)
			{
				return -1;
			}

			return endpoint_.send( data, size );
		}
		else
		{
			return endpoint_.sendto( data, size, destAddr_ );
		}
	}

	Mercury::Address remoteAddr() const
	{
		if (isTCP_)
		{
			Mercury::Address addr;
			endpoint_.getremoteaddress( (u_int16_t*)&addr.port,
					(u_int32_t*)&addr.ip );
			return addr;
		}
		else
		{
			return Mercury::Address( destAddr_.sin_addr.s_addr,
					destAddr_.sin_port );
		}
	}

	bool isTCP() const	{ return isTCP_; }

private:
	bool isTCP_;
	Endpoint & endpoint_;
	sockaddr_in destAddr_;
};

#endif // REMOTE_ENDPOINT_HPP
