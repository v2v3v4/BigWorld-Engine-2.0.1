/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONNECTION_INFO_HPP
#define CONNECTION_INFO_HPP

#include <string>

namespace DBConfig
{

/**
 *	This struct contains the information to connect to a database server.
 */
class ConnectionInfo
{
public:
	ConnectionInfo(): port( 0 ) {}

	std::string	host;
	unsigned int port;
	std::string	username;
	std::string	password;
	std::string	database;

	/**
	 *	Generates a name used by all BigWorld processes to lock the database.
	 *	Only one connection can successfully obtain a lock with this name
	 *	at any one time.
	 */
	std::string generateLockName() const
	{
		std::string lockName( "BigWorld ");
		lockName += database;

		return lockName;
	}
};

} // namespace DBConfig

#endif // CONNECTION_INFO_HPP
