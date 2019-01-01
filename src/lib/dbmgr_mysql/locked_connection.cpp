/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "locked_connection.hpp"

#include "dbmgr_mysql/wrapper.hpp"

/**
 * Constructor.
 */
MySqlLockedConnection::MySqlLockedConnection(
		const DBConfig::ConnectionInfo & connectionInfo ) :
	connectionInfo_( connectionInfo ),
	pConnection_( NULL ),
	dbLock_( connectionInfo.generateLockName() )
{
}


/**
 * Destructor.
 */
MySqlLockedConnection::~MySqlLockedConnection()
{
	if (pConnection_)
	{
		if (dbLock_.isLocked())
		{
			dbLock_.unlock();
		}

		pConnection_->close();
		delete pConnection_;
	}
}


/**
 * This method establishes a connection to the MySql server and acquires the
 * specified named lock.
 *
 * @returns true if both a connection and lock are established, false
 * 			otherwise.
 */
bool MySqlLockedConnection::connect( bool shouldLock )
{
	if (pConnection_)
	{
		ERROR_MSG( "MySqlLockedConnection::connect: "
			"A connection has already been established.\n" );
		return false;
	}

	bool isConnectedAndLocked = true;
	try
	{
		pConnection_ = new MySql( connectionInfo_ );

		if (shouldLock)
		{
			isConnectedAndLocked = this->lock();
			if (!isConnectedAndLocked)
			{
				ERROR_MSG( "MySqlLockedConnection::connect: "
						"Unable to obtain a named lock on MySql database "
						"%s:%d (%s). It may be in use by another process.\n",
					connectionInfo_.host.c_str(),
					connectionInfo_.port,
					connectionInfo_.database.c_str() );
			}
		}
	}
	catch (std::exception & e)
	{
		ERROR_MSG( "MySqlLockedConnection::connect: "
				"Unable to connect to MySql server %s:%d (%s): %s\n",
			connectionInfo_.host.c_str(),
			connectionInfo_.port,
			connectionInfo_.database.c_str(),
			e.what() );
		isConnectedAndLocked = false;
	}

	return isConnectedAndLocked;
}


/**
 * This method attempts to locks the connection if it is not already locked.
 *
 * @returns true if the connection was able to be locked, false otherwise.
 */
bool MySqlLockedConnection::lock()
{
	return pConnection_ && dbLock_.lock( *pConnection_ );
}


/**
 * This method attempts to locks the connection if it is not already locked.
 *
 * @returns true if the connection was able to be unlocked, false otherwise.
 */
bool MySqlLockedConnection::unlock()
{
	return pConnection_ && dbLock_.unlock();
}

// locked_connection.cpp
