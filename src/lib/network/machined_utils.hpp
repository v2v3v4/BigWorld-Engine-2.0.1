/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MACHINED_UTIL_HPP
#define MACHINED_UTIL_HPP

#include "misc.hpp"
#include "endpoint.hpp"
#include <string>

namespace Mercury
{

namespace MachineDaemon
{
	Reason registerWithMachined( const Address & srcAddr,
			const std::string & name, int id, bool isRegister = true );

	Reason deregisterWithMachined( const Address & srcAddr,
			const std::string & name, int id );

	Reason registerBirthListener( const Address & srcAddr,
			Bundle & bundle, int addrStart, const char * ifname );

	Reason registerDeathListener( const Address & srcAddr,
			Bundle & bundle, int addrStart, const char * ifname );

	Reason registerBirthListener( const Address & srcAddr,
			const InterfaceElement & ie, const char * ifname );

	Reason registerDeathListener( const Address & srcAddr,
			const InterfaceElement & ie, const char * ifname );

	Reason findInterface( const char * name, int id, Address & address,
			int retries = 0, bool verboseRetry = true );

	bool queryForInternalInterface( u_int32_t & addr );

} // namespace MachineDaemon

} // namespace Mercury

#endif // MACHINED_UTIL_HPP
