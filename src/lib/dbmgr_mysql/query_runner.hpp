/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_QUERY_RUNNER_HPP
#define MYSQL_QUERY_RUNNER_HPP

#include <string.h>

#include <sstream>

class MySql;
class Query;
class ResultSet;


/**
 *
 */
class QueryRunner
{
public:
	QueryRunner( MySql & conn, const Query & query );

	template <class ARG>
	void pushArg( const ARG & arg );

	void pushArg( const char * arg, int length );

	void pushArg( const std::string & arg )
	{
		this->pushArg( arg.c_str(), arg.size() );
	}

	void pushArg( const char * arg )
	{
		this->pushArg( arg, strlen( arg ) );
	}

	void execute( ResultSet * pResults ) const;

	MySql & connection() const		{ return conn_; }

private:
	const Query & query_;
	std::ostringstream stream_;
	int numArgsAdded_;
	MySql & conn_;
};


#include "query.hpp"
#include "string_conv.hpp"

template <class ARG>
void QueryRunner::pushArg( const ARG & arg )
{
	StringConv::addToStream( stream_, arg );
	stream_ << query_.getPart( ++numArgsAdded_ );
}

#endif // MYSQL_QUERY_RUNNER_HPP
