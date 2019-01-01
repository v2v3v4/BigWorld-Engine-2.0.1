/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "entity_task.hpp"

#include "dbmgr_mysql/buffered_entity_tasks.hpp"
#include "dbmgr_mysql/mappings/entity_type_mapping.hpp"
#include "dbmgr_lib/idatabase.hpp" // For EntityKey


/**
 *	Constructor.
 */
EntityTask::EntityTask( const EntityTypeMapping & entityTypeMapping,
		DatabaseID dbID,
		const char * taskName ) :
	MySqlBackgroundTask( taskName ),
	entityTypeMapping_( entityTypeMapping ),
	dbID_( dbID ),
	pBufferedEntityTasks_( NULL )
{
}


/**
 *
 */
void EntityTask::performMainThreadTask( bool succeeded )
{
	this->performEntityMainThreadTask( succeeded );

	if (pBufferedEntityTasks_)
	{
		pBufferedEntityTasks_->onFinished( this );
	}
}


/**
 *	This method returns the EntityKey associated with this task.
 */
EntityKey EntityTask::entityKey() const
{
	return EntityKey( entityTypeMapping_.getTypeID(), dbID_ );
}

// entity_task.cpp
