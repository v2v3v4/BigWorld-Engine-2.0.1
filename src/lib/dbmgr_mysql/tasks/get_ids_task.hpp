/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GET_IDS_TASK_HPP
#define GET_IDS_TASK_HPP

#include "background_task.hpp"

#include "dbmgr_lib/idatabase.hpp"

class MySql;


/**
 *	This class encapsulates the MySqlDatabase::getIDs() operation
 *	so that it can be executed in a separate thread.
 */
class GetIDsTask : public MySqlBackgroundTask
{
public:
	GetIDsTask( int numIDs, IDatabase::IGetIDsHandler & handler );

	// MySqlBackgroundTask overrides
	virtual void performBackgroundTask( MySql & conn );
	virtual void performMainThreadTask( bool succeeded );

protected:
	virtual void onRetry();

private:
	int getUsedIDs( MySql & conn, int numIDs, BinaryOStream & stream );
	void getNewIDs( MySql & conn, int numIDs, BinaryOStream & stream );

	int							numIDs_;
	IDatabase::IGetIDsHandler &	handler_;
};

#endif // GET_IDS_TASK_HPP
