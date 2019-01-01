/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ADD_SECONDARY_DB_ENTRY_TASK_HPP
#define ADD_SECONDARY_DB_ENTRY_TASK_HPP

#include "dbmgr_lib/idatabase.hpp"
#include "background_task.hpp"


/**
 * 	This thread task adds a secondary db entry.
 */
class AddSecondaryDBEntryTask : public MySqlBackgroundTask
{
public:
	AddSecondaryDBEntryTask( const IDatabase::SecondaryDBEntry & entry );

	virtual void performBackgroundTask( MySql & conn );
	virtual void performMainThreadTask( bool succeeded );

private:
	IDatabase::SecondaryDBEntry entry_;
};

#endif // ADD_SECONDARY_DB_ENTRY_TASK_HPP
