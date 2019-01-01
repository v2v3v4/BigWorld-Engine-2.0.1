/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_TASK_WITH_ID_HPP
#define ENTITY_TASK_WITH_ID_HPP

#include "dbmgr_lib/idatabase.hpp"
#include "entity_task.hpp"

/**
 *	This class encapsulates the MySqlDatabase::putEntity() operation so that
 *	it can be executed in a separate thread.
 */
class EntityTaskWithID : public EntityTask
{
public:
	EntityTaskWithID( const EntityTypeMapping & entityTypeMapping,
							DatabaseID databaseID,
							EntityID entityID,
							const char * taskName );

	virtual EntityID entityID() const	{ return entityID_; }

private:
	EntityID						entityID_;
};

#endif // ENTITY_TASK_WITH_ID_HPP
