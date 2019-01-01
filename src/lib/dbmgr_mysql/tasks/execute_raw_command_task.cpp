/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "execute_raw_command_task.hpp"

#include "cstdmf/blob_or_null.hpp"

#include "../database_exception.hpp"
#include "../query.hpp"
#include "../result_set.hpp"
#include "../wrapper.hpp"


// -----------------------------------------------------------------------------
// Section: ExecuteRawCommandTask
// -----------------------------------------------------------------------------

ExecuteRawCommandTask::ExecuteRawCommandTask( const std::string & command,
		IDatabase::IExecuteRawCommandHandler & handler ) :
	MySqlBackgroundTask( "ExecuteRawCommandTask" ),
	command_( command ),
	handler_( handler )
{
}


/**
 *	This method executes a raw database command.
 */
void ExecuteRawCommandTask::performBackgroundTask( MySql & conn )
{
	const Query query( command_, /* shouldPartition */false );

	ResultSet resultSet;
	query.execute( conn, &resultSet );

	BinaryOStream & stream = handler_.response();

	if (resultSet.hasResult())
	{
		stream << ""; // no error.
		uint32 numFields =  uint32( resultSet.numFields() );
		stream << numFields;
		stream << uint32( resultSet.numRows() );

		ResultRow row;

		while (row.fetchNextFrom( resultSet ))
		{
			for (uint32 i = 0; i < numFields; ++i)
			{
				BlobOrNull value;
				row.getField( i, value );
				stream << value;
			}
		}
	}
	else
	{
		stream << std::string();	// no error.
		stream << int32( 0 ); 		// no fields.
		stream << uint64( conn.affectedRows() );
	}
}


void ExecuteRawCommandTask::onRetry()
{
	// Should rewind stream, if possible
}


void ExecuteRawCommandTask::onException( const DatabaseException & e )
{
	handler_.response() << e.what();
}


/**
 *	This method is called in the main thread after run() completes.
 */
void ExecuteRawCommandTask::performMainThreadTask( bool succeeded )
{
	handler_.onExecuteRawCommandComplete();
}

// execute_raw_command_task.cpp
