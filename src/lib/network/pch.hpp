/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef NETWORK_PCH_HPP
#define NETWORK_PCH_HPP

#ifdef _WIN32

#include "cstdmf/pch.hpp"

#include "basictypes.hpp"
#include "endpoint.hpp"
#include "interface_macros.hpp"
#include "machine_guard.hpp"
#include "msgtypes.hpp"
#include "portmap.hpp"
#include "watcher_nub.hpp"
#include "watcher_packet_handler.hpp"

#ifdef _XBOX360
#include "net_360.hpp"
#endif

#endif // _WIN32

#endif // NETWORK_PCH_HPP
