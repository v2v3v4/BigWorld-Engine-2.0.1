/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "query_runner.hpp"

#include "wrapper.hpp"

#include "cstdmf/debug.hpp"

DECLARE_DEBUG_COMPONENT( 0 );


// -----------------------------------------------------------------------------
// Section: QueryRunner
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
QueryRunner::QueryRunner( MySql & conn, const Query & query ) :
	query_( query ),
	stream_(),
	numArgsAdded_( 0 ),
	conn_( conn )
{
	stream_ << query.getPart( 0 );
}


/**
 *	This method executes the query.
 *
 *	@param pResults If not NULL, the results from the query are placed into this
 *		object.
 *
 *	@note This method will raise a DatabaseException if the query fails.
 */
void QueryRunner::execute( ResultSet * pResults ) const
{
	MF_ASSERT( numArgsAdded_ == query_.numArgs() );

	std::string queryStr = stream_.str();
	// DEBUG_MSG( "QueryRunner::execute: '%s'\n", queryStr.c_str() );

	conn_.execute( queryStr, pResults );
}


/**
 *
 */
void QueryRunner::pushArg( const char * arg, int length )
{
	if (length < 1024)
	{
		char buffer[ 2048 ];
		mysql_real_escape_string( conn_.get(), buffer, arg, length );
		stream_ << '\'' << buffer << '\'';
	}
	else
	{
		char * buffer = new char[ 1 + 2*length ];
		mysql_real_escape_string( conn_.get(), buffer, arg, length );
		stream_ << '\'' << buffer << '\'';
		delete [] buffer;
	}

	stream_ << query_.getPart( ++numArgsAdded_ );
}

// query_runner.cpp
