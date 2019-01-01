/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_BACKGROUND_TASK_HPP
#define MYSQL_BACKGROUND_TASK_HPP

#include "cstdmf/bgtask_manager.hpp"

class DatabaseException;
class MySql;
class MySqlDatabase;

class MySqlBackgroundTask : public BackgroundTask
{
public:
	MySqlBackgroundTask( const char * taskName );

	void doBackgroundTask( BgTaskManager & mgr ) {};
	void doBackgroundTask( BgTaskManager & mgr,
			BackgroundTaskThread * pThread );

	void doMainThreadTask( BgTaskManager & mgr );

	void setFailure()		{ succeeded_ = false; }

	const char * taskName() const	{ return taskName_; }

protected:
	virtual void performBackgroundTask( MySql & conn ) = 0;
	virtual void performMainThreadTask( bool succeeded ) = 0;

	virtual void onRetry() {}
	virtual void onException( const DatabaseException & e ) {}

	virtual void onTimeWarning( double duration ) const;

	bool succeeded_;

	uint64 startTime_;

	const char * taskName_;
};

#endif // MYSQL_BACKGROUND_TASK_HPP
