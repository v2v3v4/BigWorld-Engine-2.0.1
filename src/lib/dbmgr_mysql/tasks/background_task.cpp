/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "background_task.hpp"

#include "dbmgr_mysql/database_exception.hpp"
#include "dbmgr_mysql/transaction.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/timestamp.hpp"

#include "dbmgr_mysql/thread_data.hpp"

DECLARE_DEBUG_COMPONENT( 0 );


/**
 *	Constructor.
 */
MySqlBackgroundTask::MySqlBackgroundTask( const char * taskName ) :
	succeeded_( false ),
	startTime_( timestamp() ),
	taskName_( taskName )
{
}


/**
 *	This method is called in a background thread to perform the task.
 */
void MySqlBackgroundTask::doBackgroundTask( BgTaskManager & mgr,
		BackgroundTaskThread * pThread )
{
	MySqlThreadData & threadData =
		*static_cast< MySqlThreadData * >( pThread->pData().get() );

	bool retry;

	do
	{
		retry = false;

		try
		{
			succeeded_ = true;
			MySqlTransaction transaction( threadData.connection() );
			this->performBackgroundTask( threadData.connection() );
			transaction.commit();
		}
		catch (DatabaseException & e)
		{
			if (e.isLostConnection())
			{
				INFO_MSG( "MySqlBackgroundTask::doBackgroundTask: "
						"Thread %p lost connection to database. Exception: %s. "
						"Attempting to reconnect.\n",
					&threadData,
					e.what() );

				int attempts = 1;

				while (!threadData.reconnect())
				{
					ERROR_MSG( "MySqlBackgroundTask::doBackgroundTask: "
									"Thread %p reconnect attempt %d failed.\n",
								&threadData,
								attempts );
					timespec t = { 1, 0 };
					nanosleep( &t, NULL );

					++attempts;
				}

				INFO_MSG( "MySqlBackgroundTask::doBackgroundTask: "
							"Thread %p reconnected. Attempts = %d\n",
						&threadData,
						attempts );

				retry = true;
				this->onRetry();
			}
			else if (e.shouldRetry())
			{
				WARNING_MSG(
						"MySqlBackgroundTask::doBackgroundTask: Retrying %s\n",
						this->taskName() );

				retry = true;
				this->onRetry();
			}
			else
			{
				WARNING_MSG( "MySqlBackgroundTask::doBackgroundTask: "
						"Exception: %s\n",
					e.what() );

				this->setFailure();
				this->onException( e );
			}
		}
	}
	while (retry);

	mgr.addMainThreadTask( this );
}


/**
 *	This method is called in the main thread to complete the task.
 */
void MySqlBackgroundTask::doMainThreadTask( BgTaskManager & mgr )
{
	uint64 duration = ::timestamp() - startTime_;

	if (duration > stampsPerSecond())
	{
		// Warn if greater than 1 second
		this->onTimeWarning( double( duration )/stampsPerSecondD() );
	}

	this->performMainThreadTask( succeeded_ );
}


/**
 *	This method is responsible for printing a warning if the task takes too long
 *	to execute.
 */
void MySqlBackgroundTask::onTimeWarning( double duration ) const
{
	WARNING_MSG( "%s took %.2f seconds\n", taskName_, duration );
}

// background_task.cpp
