/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "transaction.hpp"

#include "database_exception.hpp"
#include "wrapper.hpp"

/**
 *	Constructor.
 */
MySqlTransaction::MySqlTransaction( MySql& sql ) :
	sql_( sql ),
	committed_( false )
{
	sql_.execute( "START TRANSACTION" );

	// Note: Important that this is last as execute can cause an exception
	sql_.inTransaction( true );
}


/**
 *	Destructor.
 */
MySqlTransaction::~MySqlTransaction()
{
	if (!committed_ && !sql_.hasLostConnection())
	{
		// Can't let exception escape from destructor otherwise terminate()
		// will be called.
		try
		{
			WARNING_MSG( "MySqlTransaction::~MySqlTransaction: "
					"Rolling back\n" );
			sql_.execute( "ROLLBACK" );
		}
		catch (DatabaseException & e)
		{
			if (e.isLostConnection())
			{
				sql_.hasLostConnection( true );
			}
		}
	}

	sql_.inTransaction( false );
}


/**
 *
 */
bool MySqlTransaction::shouldRetry() const
{
	return (sql_.getLastErrorNum() == ER_LOCK_DEADLOCK);
}


/**
 *
 */
void MySqlTransaction::commit()
{
	MF_ASSERT( !committed_ );
	sql_.execute( "COMMIT" );
	committed_ = true;
}

// transaction.cpp
