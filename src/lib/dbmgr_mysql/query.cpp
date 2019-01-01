/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "query.hpp"

#include "cstdmf/debug.hpp"

DECLARE_DEBUG_COMPONENT( 0 );


// -----------------------------------------------------------------------------
// Section: Query
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
Query::Query( const std::string & stmt, bool shouldPartitionArgs )
{
	if (!stmt.empty())
	{
		this->init( stmt, shouldPartitionArgs );
	}
}


/**
 *	This method initialises this query from a query string. The query string
 *	should contain '?' characters to indicate the arguments of the query.
 */
bool Query::init( const std::string & stmt, bool shouldPartitionArgs )
{
	IF_NOT_MF_ASSERT_DEV( queryParts_.empty() )
	{
		return false;
	}

	if (!shouldPartitionArgs)
	{
		queryParts_.push_back( stmt );

		return true;
	}

	// TODO: Need better query parsing to handle '?' characters in strings that
	// should not be treated as argument placeholders. Only really an issue for
	// executeRawDatabaseCommand since everything else should not hardcode
	// strings in the query template.

	std::string::const_iterator partStart = stmt.begin();
	std::string::const_iterator partEnd = stmt.begin();

	while (partEnd != stmt.end())
	{
		if (*partEnd == '?')
		{
			queryParts_.push_back( std::string( partStart, partEnd ) );
			partStart = partEnd + 1;
		}

		++partEnd;
	}

	queryParts_.push_back( std::string( partStart, partEnd ) );

	return true;
}



// statement.cpp
