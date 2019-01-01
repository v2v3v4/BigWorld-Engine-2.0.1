/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "buffered_entity_tasks.hpp"

#include "dbmgr_lib/idatabase.hpp" // For EntityKey

#include "tasks/entity_task.hpp"
#include "cstdmf/debug.hpp"

// -----------------------------------------------------------------------------
// Section: Helper functions
// -----------------------------------------------------------------------------

namespace
{

bool isValidDBID( DatabaseID id )
{
	return (id != 0) && (id != PENDING_DATABASE_ID);
}


template < class MAP, class ID >
bool grabLockT( MAP & tasks, ID id )
{
	typedef typename MAP::iterator iterator;
	std::pair< iterator, iterator > range = tasks.equal_range( id );

	if (range.first != range.second)
	{
		return false;
	}

	tasks.insert( range.second, std::make_pair( id,
				static_cast< EntityTask * >( NULL ) ) );

	return true;
}


template < class MAP, class ID >
void bufferT( MAP & tasks, ID id, EntityTask * pTask )
{
	typedef typename MAP::iterator iterator;
	std::pair< iterator, iterator > range = tasks.equal_range( id );

	MF_ASSERT( range.first != range.second );
	MF_ASSERT( range.first->second == NULL );

	// Make sure that it is inserted at the end.
	tasks.insert( range.second, std::make_pair( id, pTask ) );
}

} // anonymous namespace


// -----------------------------------------------------------------------------
// Section: BufferedEntityTasks
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 *
 *	@param bgTaskManager The manager where the tasks will be run.
 */
BufferedEntityTasks::BufferedEntityTasks( BgTaskManager & bgTaskManager ) :
	bgTaskManager_( bgTaskManager )
{
}


/**
 *	This method attempts to "grab a lock" for writing an entity. This class
 *	stores a list of the outstanding tasks for an entity. If there are any
 *	outstanding tasks, this method will return false.
 *
 *	If there are no tasks, a new NULL task is added, blocking others from
 *	grabbing the lock until that task has finished.
 */
bool BufferedEntityTasks::grabLock( EntityTask * pTask )
{
	const EntityKey & entityKey = pTask->entityKey();
	EntityID entityID = pTask->entityID();

	if (entityKey.dbID == PENDING_DATABASE_ID)
	{
		NewEntityMap::const_iterator iter = newEntityMap_.find( entityID );

		if (iter != newEntityMap_.end())
		{
			pTask->dbID( iter->second );
		}
	}
	else if (entityKey.dbID != 0)
	{
		newEntityMap_.erase( entityID );
	}

	if (isValidDBID( entityKey.dbID ))
	{
		return grabLockT( tasks_, entityKey );
	}

	if (entityID != 0)
	{
		return grabLockT( priorToDBIDTasks_, entityID );
	}

	// Always allow a task with no databaseID or entityID to proceed.
	return true;
}


/**
 *	This method buffers a task to be performed once the other outstanding
 *	tasks for this entity have completed.
 */
void BufferedEntityTasks::addBackgroundTask( EntityTask * pTask )
{
	if (this->grabLock( pTask ))
	{
		this->doTask( pTask );
	}
	else
	{
		this->buffer( pTask );
	}
}


/**
 *	This method adds a the provided task to the collection of buffered tasks.
 */
void BufferedEntityTasks::buffer( EntityTask * pTask )
{
	const EntityKey & entityKey = pTask->entityKey();

	// INFO_MSG( "BufferedEntityTasks::buffer: "
	//			"Buffering task. dbID = %"FMT_DBID". entityID = %d\n",
	//		entityKey.dbID, pTask->entityID() );

	if (isValidDBID( entityKey.dbID ))
	{
		bufferT( tasks_, entityKey, pTask );
	}
	else
	{
		bufferT( priorToDBIDTasks_, pTask->entityID(), pTask );
	}
}


/**
 *	This method should be called when the currently performing task has
 *	completed. It will remove its entry from the collection and will perform
 *	the next task (if any).
 */
void BufferedEntityTasks::onFinished( EntityTask * pTask )
{
	const EntityKey & entityKey = pTask->entityKey();

	if (!this->playNextTask( tasks_, entityKey ))
	{
		if (entityKey.dbID != 0)
		{
			this->onFinishedNewEntity( pTask );
		}
		else
		{
			// The write failed. Play the next task even though we know it is
			// going to fail. This could be optimised but not worthwhile for
			// such a rare event.
			this->playNextTask( priorToDBIDTasks_, pTask->entityID() );
		}
	}
}


/**
 *	This method is called when a task has finished to check whether there are
 *	any tasks buffered by EntityID that should now be run.
 */
void BufferedEntityTasks::onFinishedNewEntity( EntityTask * pFinishedTask )
{
	const EntityKey & entityKey = pFinishedTask->entityKey();
	EntityID entityID = pFinishedTask->entityID();

	newEntityMap_[ entityID ] = entityKey.dbID;

	if (entityID == 0)
	{
		return;
	}

	typedef EntityIDMap Map;
	Map & tasks = priorToDBIDTasks_;

	std::pair< Map::iterator, Map::iterator > range =
			tasks.equal_range( entityID );

	MF_ASSERT( range.first != range.second );
	MF_ASSERT( range.first->second == NULL );

	Map::iterator iter = range.first;
	++iter;

	EntityTask * pTaskToPlay = NULL;

	while (iter != range.second)
	{
		EntityTask * pTask = iter->second;

		pTask->dbID( entityKey.dbID );

		if (grabLockT( tasks_, pTask->entityKey() ))
		{
			MF_ASSERT( pTaskToPlay == NULL );
			pTaskToPlay = pTask;
		}
		else
		{
			MF_ASSERT( pTaskToPlay != NULL );
			this->buffer( pTask );
		}

		++iter;
	}

	tasks.erase( range.first, range.second );

	if (pTaskToPlay)
	{
		this->doTask( pTaskToPlay );
	}
}


/**
 *	This method schedules a task to be performed by a background thread.
 */
void BufferedEntityTasks::doTask( EntityTask * pTask )
{
	// So that it can inform us when it is done.
	pTask->pBufferedEntityTasks( this );

	bgTaskManager_.addBackgroundTask( pTask );
}


/**
 *	This template method removes the lock associated with the current task and
 *	plays the next one, if any.
 *
 *	@return true on success, false if there was no lock to remove.
 */
template < class MAP, class ID >
bool BufferedEntityTasks::playNextTask( MAP & tasks, ID id )
{
	typedef typename MAP::iterator iterator;
	std::pair< iterator, iterator > range = tasks.equal_range( id );

	if ((range.first == range.second) ||
			(range.first->second != NULL))
	{
		// Not from this map.
		return false;
	}

	// There must be tasks and the first (the one being deleted) must be
	// NULL.
	MF_ASSERT( range.first != range.second );
	MF_ASSERT( range.first->second == NULL );

	iterator nextIter = range.first;
	++nextIter;
	EntityTask * pNextTask = NULL;

	if (nextIter != range.second)
	{
		pNextTask = nextIter->second;

		nextIter->second = NULL;
	}

	tasks.erase( range.first );

	if (pNextTask)
	{
		NOTICE_MSG( "Playing buffered task %s for dbID %"FMT_DBID", type %d\n",
				pNextTask->taskName(), pNextTask->dbID(),
				pNextTask->entityKey().typeID );
		this->doTask( pNextTask );
	}

	return true;
}

// buffered_entity_tasks.cpp
