/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "result_set.hpp"

#include "cstdmf/debug.hpp"

DECLARE_DEBUG_COMPONENT( 0 );


// -----------------------------------------------------------------------------
// Section: ResultSet
// -----------------------------------------------------------------------------

/**
 *
 */
ResultSet::ResultSet() : pResultSet_( NULL )
{
}


/**
 *
 */
ResultSet::~ResultSet()
{
	this->setResults( NULL );
}


/**
 *	This method returns the number of result rows in this set.
 */
int ResultSet::numRows() const
{
	return pResultSet_ ? mysql_num_rows( pResultSet_ ) : 0;
}


/**
 *
 */
void ResultSet::setResults( MYSQL_RES * pResultSet )
{
	if (pResultSet_ != NULL)
	{
		mysql_free_result( pResultSet_ );
	}

	pResultSet_ = pResultSet;
}

// result_set.cpp
