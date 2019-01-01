/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef UPDATE_SECONDARY_DBS_TASK_HPP
#define UPDATE_SECONDARY_DBS_TASK_HPP

#include "dbmgr_lib/idatabase.hpp"
#include "background_task.hpp"


/**
 *	This class implements the updateSecondaryDBs() function as a thread task.
 */
class UpdateSecondaryDBsTask : public MySqlBackgroundTask
{
public:
	UpdateSecondaryDBsTask( const BaseAppIDs & ids,
			IDatabase::IUpdateSecondaryDBshandler & handler );

	virtual void performBackgroundTask( MySql & conn );
	virtual void performMainThreadTask( bool succeeded );

private:
	IDatabase::IUpdateSecondaryDBshandler &			handler_;
	std::string 									condition_;
	IDatabase::SecondaryDBEntries					entries_;
};

#endif // UPDATE_SECONDARY_DBS_TASK_HPP
