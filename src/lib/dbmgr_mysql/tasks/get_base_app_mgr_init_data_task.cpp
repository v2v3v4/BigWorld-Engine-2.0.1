/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "get_base_app_mgr_init_data_task.hpp"

#include "../query.hpp"
#include "../result_set.hpp"

/**
 *	Constructor.
 */
GetBaseAppMgrInitDataTask::GetBaseAppMgrInitDataTask(
		IDatabase::IGetBaseAppMgrInitDataHandler & handler ) :
	MySqlBackgroundTask( "GetBaseAppMgrInitDataTask" ),
	handler_( handler ),
	gameTime_( 0 ),
	maxAppID_( 0 )
{
}


/**
 *
 */
void GetBaseAppMgrInitDataTask::performBackgroundTask( MySql & connection )
{
	GetBaseAppMgrInitDataTask::getGameTime( connection, gameTime_ );
	GetBaseAppMgrInitDataTask::getMaxSecondaryDBAppID( connection, maxAppID_ );
}


/**
 *
 */
void GetBaseAppMgrInitDataTask::performMainThreadTask( bool succeeded )
{
	handler_.onGetBaseAppMgrInitDataComplete( gameTime_, maxAppID_ );
}


/**
 *	This method returns the game time stored in the database. Returns result
 *	via the gameTime parameter.
 */
bool GetBaseAppMgrInitDataTask::getGameTime( MySql & connection,
		GameTime & gameTime )
{
	static const Query query( "SELECT * FROM bigworldGameTime" );

	ResultSet resultSet;
	query.execute( connection, &resultSet );

	return resultSet.getResult( gameTime );
}


/**
 * 	This method returns the maximum app ID stored in the list of secondary
 * 	database entries.
 */
bool GetBaseAppMgrInitDataTask::getMaxSecondaryDBAppID( MySql & connection,
		int32 & maxAppID )
{
	static const Query query(
			"SELECT MAX( appID ) FROM bigworldSecondaryDatabases" );

	ResultSet resultSet;
	query.execute( connection, &resultSet );

	ValueOrNull< int32 > maxAppIDBuf;
	bool isOkay = resultSet.getResult( maxAppIDBuf );

	if (isOkay && maxAppIDBuf.get())
	{
		maxAppID = *maxAppIDBuf.get();
	}

	return isOkay;
}

// get_base_app_mgr_init_data_task.cpp
