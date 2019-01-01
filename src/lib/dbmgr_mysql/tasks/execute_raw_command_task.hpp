/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EXECUTE_RAW_COMMAND_TASK_HPP
#define EXECUTE_RAW_COMMAND_TASK_HPP

#include "dbmgr_lib/idatabase.hpp"
#include "background_task.hpp"

#include <string>

/**
 *	This class encapsulates the MySqlDatabase::executeRawCommand() operation
 *	so that it can be executed in a separate thread.
 */
class ExecuteRawCommandTask : public MySqlBackgroundTask
{
public:
	ExecuteRawCommandTask( const std::string & command,
			IDatabase::IExecuteRawCommandHandler & handler );

	virtual void performBackgroundTask( MySql & conn );
	virtual void performMainThreadTask( bool succeeded );

protected:
	void onRetry();
	void onException( const DatabaseException & e );

	std::string									command_;
	IDatabase::IExecuteRawCommandHandler &		handler_;
};

#endif // EXECUTE_RAW_COMMAND_TASK_HPP
