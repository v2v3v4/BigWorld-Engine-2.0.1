/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DB_INTERFACE_UTILS_HPP
#define DB_INTERFACE_UTILS_HPP

#include "pyscript/script.hpp"
#include "network/channel.hpp"

namespace Mercury
{
	class NetworkInterface;
}
class BinaryIStream;
class BinaryOStream;

namespace DBInterfaceUtils
{
	bool executeRawDatabaseCommand( const std::string & command,
		PyObjectPtr pResultHandler, Mercury::Channel & channel );

	bool executeRawDatabaseCommand( const std::string & command,
		PyObjectPtr pResultHandler, Mercury::NetworkInterface & interface,
		const Mercury::Address & dbMgrAddr );
}

#endif // DB_INTERFACE_UTILS_HPP
