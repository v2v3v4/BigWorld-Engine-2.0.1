/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GET_SECONDARY_DBS_TASK_HPP
#define GET_SECONDARY_DBS_TASK_HPP

#include "dbmgr_lib/idatabase.hpp"
#include "background_task.hpp"


void getSecondaryDBEntries( MySql & connection,
		IDatabase::SecondaryDBEntries & entries,
		const std::string & condition = std::string() );

/**
 *	This class implements the getSecondaryDBs() function as a thread task.
 */
class GetSecondaryDBsTask : public MySqlBackgroundTask
{
public:
	typedef IDatabase::IGetSecondaryDBsHandler Handler;

	GetSecondaryDBsTask( Handler & handler );

	virtual void performBackgroundTask( MySql & conn );
	virtual void performMainThreadTask( bool succeeded );

private:
	Handler & handler_;
	IDatabase::SecondaryDBEntries entries_;
};

#endif // GET_SECONDARY_DBS_TASK_HPP
