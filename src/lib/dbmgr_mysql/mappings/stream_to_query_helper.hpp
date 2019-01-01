/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef STREAM_TO_QUERY_HELPER_HPP
#define STREAM_TO_QUERY_HELPER_HPP

#include "network/basictypes.hpp"

#include <list>

class ChildQuery;
class MySql;
class Query;
class QueryRunner;


/**
 *	This class is used as a helper when running PropertyMapping::fromStreamToDatabase.
 */
class StreamToQueryHelper
{
public:
	StreamToQueryHelper( MySql & connection, DatabaseID parentID );
	~StreamToQueryHelper();

	void executeBufferedQueries( DatabaseID parentID );

	ChildQuery & createChildQuery( const Query & query );

	MySql & connection()				{ return connection_; }

	DatabaseID parentID() const			{ return parentID_; }
	void parentID( DatabaseID id )		{ parentID_ = id; }

	bool hasBufferedQueries() const		{ return !bufferedQueries_.empty(); }

private:
	MySql & connection_;
	DatabaseID parentID_;

	typedef std::list< ChildQuery * > BufferedQueries;
	BufferedQueries bufferedQueries_;
};


#include "../query_runner.hpp"

/**
 *	This class represents a query for a child table that needs to be queued
 *	for execution after it's parent query has been executed. This is required
 *	for situations such as writing out sub-tables of entities that require
 *	the databaseID of the entity record in order to maintain data integrity.
 */
class ChildQuery
{
public:
	ChildQuery( MySql & connection, const Query & query );

	void execute( DatabaseID parentID );

	QueryRunner & queryRunner()			{ return queryRunner_; }
	StreamToQueryHelper & helper()		{ return helper_; }

private:
	QueryRunner queryRunner_;
	StreamToQueryHelper helper_;
};

#endif // STREAM_TO_QUERY_HELPER_HPP
