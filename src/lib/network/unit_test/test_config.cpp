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

#include <stdio.h>
#include "network/network_interface.hpp"

/**
 *  This test verifies that the local machine is correctly configured for
 *  running unit tests.
 */
TEST( Config )
{
	Endpoint ep;
	ep.socket( SOCK_DGRAM );

	ASSERT_WITH_MESSAGE( ep.good(), "Couldn't bind endpoint" );

	ASSERT_WITH_MESSAGE( ep.setBufferSize( SO_RCVBUF,
			Mercury::NetworkInterface::RECV_BUFFER_SIZE ),
		"Insufficient recv buffer to run tests. \n"
		"Please consider runnning the following "
		"commands as root :\n"
		"echo 16777216 > /proc/sys/net/core/rmem_max\n"
		"echo 1048576 > /proc/sys/net/core/wmem_max\n"
		"echo 1048576 > /proc/sys/net/core/wmem_default"
		"Or to make permanent in /etc/sysctl.conf set: \n"
		"net.core.rmem_max = 16777216\n"
		"net.core.wmem_max = 1048576\n"
		"net.core.wmem_default = 1048576\n"
		);
}
