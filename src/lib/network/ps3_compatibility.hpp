/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PS3_COMPATIBILITY_HPP
#define PS3_COMPATIBILITY_HPP

#include <netex/errno.h>
#include <sys/time.h>
#define unix
#undef EAGAIN
#define EAGAIN SYS_NET_EAGAIN
#define ECONNREFUSED SYS_NET_ECONNREFUSED
#define EHOSTUNREACH SYS_NET_EHOSTUNREACH
#define ENOBUFS SYS_NET_ENOBUFS
#define select socketselect
#undef errno
#define errno sys_net_errno

#endif // PS3_COMPATIBILITY_HPP
