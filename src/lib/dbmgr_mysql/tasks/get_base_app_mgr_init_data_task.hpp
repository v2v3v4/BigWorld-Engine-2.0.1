/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GET_BASE_APP_MGR_INIT_DATA_TASK_HPP
#define GET_BASE_APP_MGR_INIT_DATA_TASK_HPP

#include "dbmgr_lib/idatabase.hpp"
#include "background_task.hpp"

/**
 *	This class implements the getBaseAppMgrInitData() function as a thread task.
 */
class GetBaseAppMgrInitDataTask : public MySqlBackgroundTask
{
public:
	GetBaseAppMgrInitDataTask(
			IDatabase::IGetBaseAppMgrInitDataHandler & handler );

	virtual void performBackgroundTask( MySql & conn );
	virtual void performMainThreadTask( bool succeeded );

private:
	static bool getGameTime( MySql & connection, GameTime & gameTime );
	static bool getMaxSecondaryDBAppID( MySql & connection, int32 & maxAppID );

	IDatabase::IGetBaseAppMgrInitDataHandler & 	handler_;
	GameTime	gameTime_;
	int32		maxAppID_;
};

#endif // GET_BASE_APP_MGR_INIT_DATA_TASK_HPP
