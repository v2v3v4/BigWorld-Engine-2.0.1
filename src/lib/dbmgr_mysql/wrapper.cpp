/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "wrapper.hpp"

#include "connection_info.hpp"
#include "database_exception.hpp"
#include "result_set.hpp"
#include "string_conv.hpp"

#include "cstdmf/binary_stream.hpp"

#include <mysql/errmsg.h>

DECLARE_DEBUG_COMPONENT( 0 );

// -----------------------------------------------------------------------------
// Section: Utility functions
// -----------------------------------------------------------------------------

namespace // anonymous 
{

/**
 *	This function converts a MYSQL_TIME to Unix epoch time.
 */
time_t convertMySqlTimeToEpoch(  const MYSQL_TIME& mysqlTime )
{
	tm ctime;
	ctime.tm_year = mysqlTime.year - 1900;
	ctime.tm_mon = mysqlTime.month - 1;
	ctime.tm_mday = mysqlTime.day;
	ctime.tm_hour = mysqlTime.hour;
	ctime.tm_min = mysqlTime.minute;
	ctime.tm_hour = mysqlTime.hour;
	ctime.tm_sec = mysqlTime.second;

	// Init other fields to default
	ctime.tm_wday = -1;
	ctime.tm_yday = -1;
	ctime.tm_isdst = -1;

	return timegm( &ctime );
}

} // namespace (anonymous)


// -----------------------------------------------------------------------------
// Section: class MySql
// -----------------------------------------------------------------------------

bool MySql::s_hasInitedCollationLengths_ = false;
MySql::CollationLengths MySql::s_collationLengths_;

/**
 *	Constructor.
 *
 *	@param connectInfo 	The connection parameters.
 */
MySql::MySql( const DBConfig::ConnectionInfo & connectInfo ) :
	// initially set all pointers to 0 so that we can see where we got to
	// should an error occur
	sql_( NULL ),
	inTransaction_( false ),
	hasLostConnection_( false ),
	connectInfo_( connectInfo )
{
	this->connect( connectInfo );
}


/**
 *	Destructor.
 */
MySql::~MySql()
{
	MF_ASSERT( !inTransaction_ );
	this->close();
}


/**
 *	Close the MySQL connection, and clear the connection state.
 */
void MySql::close()
{
	if (sql_)
	{
		mysql_close( sql_ );
		sql_ = NULL;
	}
}


/**
 *	Close any current connection and reconnect to the given server.
 */
bool MySql::reconnectTo( const DBConfig::ConnectionInfo & connectInfo )
{
	this->close();

	try
	{
		this->connect( connectInfo );
	}
	catch (...)
	{
		return false;
	}

	return this->ping();
}


/**
 *	Connect to the given server.
 */
void MySql::connect( const DBConfig::ConnectionInfo & connectInfo )
{
	this->close();
	hasLostConnection_ = false;

	try
	{
		sql_ = mysql_init( 0 );

		if (!sql_)
		{
			this->throwError( sql_ );
		}

		if (!mysql_real_connect( sql_, connectInfo.host.c_str(),
				connectInfo.username.c_str(), connectInfo.password.c_str(),
				connectInfo.database.c_str(), connectInfo.port, NULL, 0 ))
		{
			this->throwError( sql_ );
		}

		// Set the client connection to be UTF8
		if (0 != mysql_set_character_set( sql_, "utf8" ))
		{
			ERROR_MSG( "MySql::MySql: "
				"Could not set client connection character set to UTF-8\n" );
			this->throwError( sql_ );
		}

		this->queryCharsetLengths();
	}
	catch (std::exception& e)
	{
		ERROR_MSG( "MySql::MySql: %s\n", e.what() );
		hasLostConnection_ = true;
		throw;
	}
}


/**
 *	Query the character set lengths.
 */
void MySql::queryCharsetLengths()
{
	if (s_hasInitedCollationLengths_)
	{
		return;
	}

	// We only need to collect char set information once over all
	// connections on the assumption that we are only ever connecting to
	// one database, and thus charset information shouldn't be different
	// for different connections.

	// Since character sets are identified in MYSQL_FIELD structures by a
	// collation ID, which uniquely identifies both the collation and the
	// character set used, we map each collation ID with the corresponding
	// character set's maximum length.

	const static std::string QUERY_COLLATIONS( "SHOW COLLATION" );
	const static std::string QUERY_CHARSETS( "SHOW CHARACTER SET" );
	const static unsigned long MAXLEN_WIDTH = 1;

	typedef std::map< std::string, uint > CharsetLengths;
	CharsetLengths charsetLengths;

	s_collationLengths_.clear();

	// Make a map of the character set names -> character lengths.
	this->realQuery( QUERY_CHARSETS );
	MYSQL_RES * res = mysql_store_result( sql_ );
	MYSQL_ROW row = NULL;
	while ((row = mysql_fetch_row( res )))
	{
		unsigned long * colLens = mysql_fetch_lengths( res );

		// The column order is: charset, description, default collation 
		// and maxlen.
		std::string charsetName( row[0], colLens[0] );
		if (colLens[3] != MAXLEN_WIDTH)
		{
			CRITICAL_MSG( "MySql::queryCharsetLength: Could not retrieve "
				"maxlen for charset, expecting length 1 column for "
				"maxlen\n" );
		}

		uint maxLen = 0;
		StringConv::toValue( maxLen, row[3] );
		charsetLengths[charsetName] = maxLen;

	}
	mysql_free_result( res );

	// Insert into the collation length map the collation ID and the
	// corresponding character set character length.
	this->realQuery( QUERY_COLLATIONS );
	res = mysql_store_result( sql_ );

	while ((row = mysql_fetch_row( res )))
	{
		unsigned long * colLens = mysql_fetch_lengths( res );
		// The column order is : collation name, charset, ID, default, 
		// compiled, sortlen.
		std::string charsetName( row[1], colLens[1] );
		uint collationID = 0;
		StringConv::toValue( collationID, row[2] );
		s_collationLengths_[collationID] = charsetLengths[charsetName];
	}
	mysql_free_result( res );

	s_hasInitedCollationLengths_ = true;
}


/**
 *	Return the maximum length of a character as represented in a charset
 *	identified by its charset/collation ID.
 *
 *	@param charsetnr 	The 'charsetnr' field value in the MYSQL_FIELD.
 */
uint MySql::charsetWidth( unsigned int charsetnr )
{
	MF_ASSERT( s_hasInitedCollationLengths_ );
	CollationLengths::const_iterator iLength =
		s_collationLengths_.find( charsetnr );

	if (iLength != s_collationLengths_.end())
	{
		return iLength->second;
	}

	// If unknown (or 0), assume it is 1-byte/character.
	return 1;
}


namespace MySqlUtils
{
	inline unsigned int getErrno( MYSQL* connection )
	{
		return mysql_errno( connection );
	}

	inline unsigned int getErrno( MYSQL_STMT* statement )
	{
		return mysql_stmt_errno( statement );
	}
}


/**
 *	This method throws an exeception based on MySQL's state.
 */
void MySql::throwError( MYSQL * failedObj )
{
	DatabaseException e( failedObj );

	if (e.isLostConnection())
	{
		this->hasLostConnection( true );
	}

	throw e;
}


/**
 *	This method executes mysql_real_query. If the initial attempt fails due to
 *	a lost connection. The connection is re-established and retried.
 */
int MySql::realQuery( const std::string & query )
{
	int result = mysql_real_query( sql_, query.c_str(), query.length() );

	if (result == 0)
	{
		// Success
		return 0;
	}

	unsigned int errNum = mysql_errno( sql_ );

	if (!inTransaction_ &&
			((errNum == CR_SERVER_LOST) || (errNum == CR_SERVER_GONE_ERROR)))
	{
		INFO_MSG( "MySql::realQuery: "
				"Connection lost. Attempting to reconnect.\n" );

		if (this->reconnect())
		{
			INFO_MSG( "MySql::realQuery: Reconnect succeeded.\n" );
			result = mysql_real_query( sql_, query.c_str(), query.length() );
		}
		else
		{
			WARNING_MSG( "MySql::realQuery: "
					"Reconnect failed when executing %s.\n", query.c_str() );
		}
	}

	return result;
}


/**
 *	This method executes the given string and adds the result to a stream. This
 *	is used by BigWorld.executeRawDatabaseCommand().
 *
 *	@param statement	The SQL string to execute.
 *	@param resdata If not NULL, the results from this query are put into this.
 */
void MySql::execute( const std::string & statement, BinaryOStream * resdata )
{
	int result = this->realQuery( statement );

	if (result != 0)
	{
		this->throwError( sql_ );
		return;
	}

	MYSQL_RES * pResult = mysql_store_result( sql_ );

	if (pResult)
	{
		if (resdata != NULL)
		{
			int nrows = mysql_num_rows( pResult );
			int nfields = mysql_num_fields( pResult );
			(*resdata) << nrows << nfields;
			MYSQL_ROW arow;

			while ((arow = mysql_fetch_row( pResult )) != NULL)
			{
				unsigned long *lengths = mysql_fetch_lengths( pResult );
				for (int i = 0; i < nfields; i++)
				{
					if (arow[i] == NULL)
					{
						(*resdata) << "NULL";
					}
					else
					{
						resdata->appendString(arow[i],lengths[i]);
					}
				}
			}
		}

		mysql_free_result( pResult );
	}
}


/**
 *	This method executes the given string.
 *
 *	@param queryStr	The SQL string to execute.
 *	@param pResultSet If not NULL, the results from this query are put into this
 *		object.
 *
 *	@note This method can throw DatabaseException exceptions.
 */
void MySql::execute( const std::string & queryStr, ResultSet * pResultSet )
{
	int result = this->realQuery( queryStr );

	MYSQL_RES * pMySqlResultSet = mysql_store_result( sql_ );

	if (pMySqlResultSet != NULL)
	{
		if (pResultSet != NULL)
		{
			pResultSet->setResults( pMySqlResultSet );
		}
		else
		{
			mysql_free_result( pMySqlResultSet );
		}
	}

	if (result != 0)
	{
		const size_t MAX_SIZE = 1000000;

		if (queryStr.length() < MAX_SIZE)
		{
			ERROR_MSG( "MySql::execute: Query failed (%d) '%s'\n",
					result, queryStr.c_str() );
		}
		else
		{
			ERROR_MSG( "MySql::execute: Query failed (%d): "
						"Size of query string is %"PRIzu"\n",
					result, queryStr.length() );
		}
		this->throwError( sql_ );
	}
}


/**
 *	This is the non-exception version of execute().
 */
int MySql::query( const std::string & statement )
{
	int errorNum = this->realQuery( statement );

	DatabaseException e( sql_ );

	if (e.isLostConnection())
	{
		this->hasLostConnection( true );
	}

	return errorNum;
}


/**
 * 	This function returns the list of table names that matches the specified
 * 	pattern.
 */
void MySql::getTableNames( std::vector<std::string>& tableNames,
							const char * pattern )
{
	tableNames.clear();

	MYSQL_RES * pResult = mysql_list_tables( sql_, pattern );
	if (pResult)
	{
		tableNames.reserve( mysql_num_rows( pResult ) );

		MYSQL_ROW row;
		while ((row = mysql_fetch_row( pResult )) != NULL)
		{
			unsigned long *lengths = mysql_fetch_lengths( pResult );
			tableNames.push_back( std::string( row[0], lengths[0] ) );
		}
		mysql_free_result( pResult );
	}
}

// mysql_wrapper.cpp
