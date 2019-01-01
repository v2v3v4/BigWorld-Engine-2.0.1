/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PUT_IDS_TASK_HPP
#define PUT_IDS_TASK_HPP

#include "background_task.hpp"

#include "network/basictypes.hpp"

#include <vector>

/**
 *	This class encapsulates the MySqlDatabase::putIDs() operation
 *	so that it can be executed in a separate thread.
 */
class PutIDsTask : public MySqlBackgroundTask
{
public:
	PutIDsTask( int numIDs, const EntityID * ids );

	// MySqlBackgroundTask overrides
	virtual void performBackgroundTask( MySql & conn );
	virtual void performMainThreadTask( bool succeeded );

private:
	typedef std::vector< EntityID > Container;
	Container	ids_;
};

#endif // PUT_IDS_TASK_HPP
