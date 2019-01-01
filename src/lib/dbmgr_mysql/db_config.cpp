/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "db_config.hpp"

#include "server/bwconfig.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/watcher.hpp"

DECLARE_DEBUG_COMPONENT( 0 );

namespace DBConfig
{

/**
 *	This method sets the serverInfo from the given data section.
 */
void readConnectionInfo( ConnectionInfo & connectionInfo )
{
	BWConfig::update( "dbMgr/host", connectionInfo.host );
	BWConfig::update( "dbMgr/port", connectionInfo.port );
	BWConfig::update( "dbMgr/username", connectionInfo.username );
	BWConfig::update( "dbMgr/password", connectionInfo.password );

	if (!BWConfig::update( "dbMgr/databaseName",
			connectionInfo.database ))
	{
		ERROR_MSG( "Server::readConnectionInfo: dbMgr/databaseName "
				   "has not been set.\n" );
	}
}


const ConnectionInfo & connectionInfo()
{
	static ConnectionInfo s_connectionInfo;
	static bool isFirst = true;

	if (isFirst)
	{

		s_connectionInfo.host = "localhost";
		s_connectionInfo.port = 0;
		s_connectionInfo.username = "bigworld";
		s_connectionInfo.password = "bigworld";
		s_connectionInfo.database = "";

		isFirst = false;
		readConnectionInfo( s_connectionInfo );
	}

	return s_connectionInfo;
}

}	// namespace DBConfig
