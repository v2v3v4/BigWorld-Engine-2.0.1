/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_LOCKED_CONNECTION_HPP
#define MYSQL_LOCKED_CONNECTION_HPP

#include "db_config.hpp"
#include "named_lock.hpp"
#include "wrapper.hpp"

#include <mysql/mysql.h>


/**
 * This class wraps a MySql connection and a NamedLock. It is intended to
 * be used by the top level of an application to ensure no other processes
 * are using the same database prior to operating on it.
 *
 * This class does not throw any exceptions.
 */
class MySqlLockedConnection
{
public:
	MySqlLockedConnection( const DBConfig::ConnectionInfo & connectionInfo );
	~MySqlLockedConnection();

	bool connect( bool shouldLock );
	bool connectAndLock()				{ return this->connect( true ); }

	// TODO: This should be removed when the cleanup in mysql_database is done.
	bool reconnectTo( const DBConfig::ConnectionInfo & connectionInfo )
	{
		return pConnection_ && pConnection_->reconnectTo( connectionInfo );
	}

	bool lock();
	bool unlock();

	MySql * connection() { return pConnection_; }

private:
	DBConfig::ConnectionInfo connectionInfo_;

	MySql * pConnection_;
	MySQL::NamedLock dbLock_;
};

#endif // MYSQL_LOCKED_CONNECTION_HPP
