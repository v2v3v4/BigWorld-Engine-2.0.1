/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SQLITE__SQLITE_UTIL_HPP
#define SQLITE__SQLITE_UTIL_HPP

#include "cstdmf/debug.hpp"

#include "third_party/sqlite/sqlite3.h"

#include <string>

/**
 * 	Wrapper for a sqlite3 connection.
 */
class SqliteConnection
{
public:
	SqliteConnection( const std::string & filePath, int & result ) :
		pConnection_( NULL )
	{
		result = sqlite3_open( filePath.c_str(), &pConnection_ );
	}

	~SqliteConnection()
	{
		if (pConnection_)
		{
			// __kyl__(1/5/2008) Hmmm... Isn't there an OK return code?
			MF_VERIFY( sqlite3_close( pConnection_ ) != SQLITE_BUSY );
		}
	}

	sqlite3 * get()
		{ return pConnection_; }

	const char * lastError()
		{ return sqlite3_errmsg( pConnection_ ); }

private:
	sqlite3 * pConnection_;
};


/**
 * 	Wrapper for a sqlite3_stmt.
 */
class SqliteStatement
{
public:
	SqliteStatement( SqliteConnection & connection,
			const std::string & statement, int & result ) :
		pStmt_( NULL )
	{
		result = sqlite3_prepare_v2( connection.get(), statement.c_str(), 
			-1, &pStmt_, NULL );
	}

	~SqliteStatement()
	{
		MF_VERIFY( sqlite3_finalize( pStmt_ ) == SQLITE_OK );
	}

	sqlite3_stmt * get()
		{ return pStmt_; }

	int reset()
		{ return sqlite3_reset( pStmt_ ); }

	int step()
		{ return sqlite3_step( pStmt_ ); }

	const unsigned char* textColumn( int column )
		{ return sqlite3_column_text( pStmt_, column ); }

	int intColumn( int column )
		{ return sqlite3_column_int( pStmt_, column ); }

	int64 int64Column( int column )
		{ return sqlite3_column_int64( pStmt_, column ); }

	const void * blobColumn( int column, int * pSize ) 
	{ 
		if (pSize)
		{
			*pSize = this->columnBytes( column );
		}
		return sqlite3_column_blob( pStmt_, column );
	}

	int columnBytes( int column )
		{ return sqlite3_column_bytes( pStmt_, column ); }

private:
	sqlite3_stmt * pStmt_;
};

#endif // SQLITE__SQLITE_UTIL_HPP
