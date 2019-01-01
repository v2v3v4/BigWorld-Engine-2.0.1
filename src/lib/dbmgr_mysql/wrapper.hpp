/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_WRAPPER_HPP
#define MYSQL_WRAPPER_HPP

#include "connection_info.hpp"

#include "cstdmf/stdmf.hpp"
#include "cstdmf/debug.hpp"

#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>

#include <map>
#include <string.h>
#include <stdexcept>

// Forward declarations.
class BinaryOStream;
class BinaryIStream;
class ResultSet;

namespace DBConfig
{
	class ConnectionInfo;
}

// Constants
#define MYSQL_ENGINE_TYPE "InnoDB"

time_t convertMySqlTimeToEpoch(  const MYSQL_TIME& mysqlTime );





// represents a MySQL server connection
class MySql
{
public:
	MySql( const DBConfig::ConnectionInfo & connectInfo );
	~MySql();

	MYSQL * get() { return sql_; }

	void execute( const std::string & statement,
		BinaryOStream * pResData = NULL );
	void execute( const std::string & queryStr, ResultSet * pResultSet );
	int query( const std::string & statement );

	void close();
	bool reconnectTo( const DBConfig::ConnectionInfo & connectInfo );
	bool reconnect()
	{
		return this->reconnectTo( connectInfo_ );
	}

	bool ping()					{ return mysql_ping( sql_ ) == 0; }
	void getTableNames( std::vector<std::string>& tableNames,
						const char * pattern );
	my_ulonglong insertID()		{ return mysql_insert_id( sql_ ); }
	my_ulonglong affectedRows()	{ return mysql_affected_rows( sql_ ); }
	const char* info()			{ return mysql_info( sql_ ); }
	const char* getLastError()	{ return mysql_error( sql_ ); }
	unsigned int getLastErrorNum() { return mysql_errno( sql_ ); }

	void inTransaction( bool value )
	{
		MF_ASSERT( inTransaction_ != value );
		inTransaction_ = value;
	}

	bool hasLostConnection() const		{ return hasLostConnection_; }
	void hasLostConnection( bool v )	{ hasLostConnection_ = v; }

	static uint charsetWidth( unsigned int charsetnr );

private:
	int realQuery( const std::string & query );
	void throwError( MYSQL * pConnection );

	void connect( const DBConfig::ConnectionInfo & connectInfo );

	void queryCharsetLengths();
	typedef std::map<uint, uint> CollationLengths;

	MySql( const MySql& );
	void operator=( const MySql& );

	MYSQL * sql_;
	bool inTransaction_;
	bool hasLostConnection_;

	DBConfig::ConnectionInfo connectInfo_;

	static bool s_hasInitedCollationLengths_;
	static CollationLengths s_collationLengths_;
};


#endif // MYSQL_WRAPPER_HPP
