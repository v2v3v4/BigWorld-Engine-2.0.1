/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "primary_database_update_queue.hpp"

#include "consolidate_entity_task.hpp"

#include "dbmgr_mysql/mappings/entity_type_mapping.hpp"

#include "dbmgr_mysql/buffered_entity_tasks.hpp"
#include "dbmgr_mysql/thread_data.hpp"

#include <time.h>

DECLARE_DEBUG_COMPONENT( 0 )

/**
 * 	Constructor.
 */
PrimaryDatabaseUpdateQueue::PrimaryDatabaseUpdateQueue(
		const DBConfig::ConnectionInfo & connectionInfo,
		const EntityDefs & entityDefs,
		int numConnections ) :
	bgTaskMgr_(),
	pBufferedEntityTasks_( new BufferedEntityTasks( bgTaskMgr_ ) ),
	entityTypeMappings_(),
	hasError_( false ),
	numOutstanding_( 0 ),
	consolidatedTimes_()
{
	for (int i = 0; i < numConnections; ++i)
	{
		bgTaskMgr_.startThreads( 1,
				new MySqlThreadData( connectionInfo ) );
	}

	MySql connection( connectionInfo );
	entityTypeMappings_.init( entityDefs, connection );
}


/**
 * 	Destructor.
 */
PrimaryDatabaseUpdateQueue::~PrimaryDatabaseUpdateQueue()
{
	bgTaskMgr_.stopAll( false );
}


/**
 * 	Adds an entity update into our queue.
 */
void PrimaryDatabaseUpdateQueue::addUpdate( const EntityKey & key,
		BinaryIStream & data, GameTime time )
{
	// Check if we've already written a newer version of this entity.

	ConsolidatedTimes::const_iterator iEntityGameTime = 
		consolidatedTimes_.find( key );

	if (iEntityGameTime != consolidatedTimes_.end() &&
			time < iEntityGameTime->second)
	{
		return;
	}

	EntityTask * pTask =
		new ConsolidateEntityTask( entityTypeMappings_[ key.typeID ],
				key.dbID, data, time, *this );

	++numOutstanding_;
	pBufferedEntityTasks_->addBackgroundTask( pTask );

	consolidatedTimes_[key] = time;
}


/**
 * 	Waits for all outstanding entity updates to complete.
 */
void PrimaryDatabaseUpdateQueue::waitForUpdatesCompletion()
{
	bgTaskMgr_.tick();

	while (numOutstanding_ > 0)
	{
		usleep( 1000 );
		bgTaskMgr_.tick();
	}
}


/**
 * 	Called by UpdateTask when it completes.
 */
void PrimaryDatabaseUpdateQueue::onPutEntityComplete( bool isOkay,
		DatabaseID dbID )
{
	if (!isOkay)
	{
		ERROR_MSG( "PrimaryDatabaseUpdateQueue::onPutEntity: "
				"could not write entity with DBID=%"FMT_DBID" to database\n",
			dbID );
		hasError_ = true;
	}

	--numOutstanding_;
}

// primary_database_update_queue.cpp
