/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_TASK_HPP
#define ENTITY_TASK_HPP

#include "background_task.hpp"

#include "network/basictypes.hpp"

class BufferedEntityTasks;
class EntityKey;
class EntityTypeMapping;


/**
 *	This class encapsulates the MySqlDatabase::putIDs() operation
 *	so that it can be executed in a separate thread.
 */
class EntityTask : public MySqlBackgroundTask
{
public:
	EntityTask( const EntityTypeMapping & entityTypeMapping,
			DatabaseID dbID, const char * taskName );

	virtual void performMainThreadTask( bool succeeded );
	virtual EntityID entityID() const	{ return 0; }

	DatabaseID dbID() const			{ return dbID_; }
	void dbID( DatabaseID dbID )	{ dbID_ = dbID; }

	EntityKey entityKey() const;

	void pBufferedEntityTasks( BufferedEntityTasks * pBufferedEntityTasks )
	{
		pBufferedEntityTasks_ = pBufferedEntityTasks;
	}

protected:
	virtual void performEntityMainThreadTask( bool succeeded ) = 0;

	const EntityTypeMapping & entityTypeMapping_;
	DatabaseID	dbID_;

private:
	BufferedEntityTasks * pBufferedEntityTasks_;
};

#endif // ENTITY_TASK_HPP
