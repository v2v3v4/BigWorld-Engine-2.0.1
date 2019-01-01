/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BUFFERED_ENTITY_TASKS_HPP
#define BUFFERED_ENTITY_TASKS_HPP

#include "network/basictypes.hpp"

#include <map>

class BgTaskManager;
class EntityKey;
class EntityTask;


/**
 *	This class is responsible for ensuring there is only a single database
 *	operation outstanding for a specific entity. If there are two writeToDB
 *	calls for the same entity outstanding, these cannot be sent to different
 *	threads as the order these are applied to the database are not guaranteed.
 */
class BufferedEntityTasks
{
public:
	BufferedEntityTasks( BgTaskManager & bgTaskManager );

	void addBackgroundTask( EntityTask * pTask );

	void onFinished( EntityTask * pTask );

private:
	bool grabLock( EntityTask * pTask );
	void buffer( EntityTask * pTask );
	void doTask( EntityTask * pTask );

	void onFinishedNewEntity( EntityTask * pFinishedTask );

	template < class MAP, class ID >
	bool playNextTask( MAP & tasks, ID id );

	BgTaskManager & bgTaskManager_;

	typedef std::multimap< EntityKey, EntityTask * > EntityKeyMap;
	typedef std::multimap< EntityID, EntityTask * > EntityIDMap;

	EntityKeyMap tasks_;
	EntityIDMap priorToDBIDTasks_;

	typedef std::map< EntityID, DatabaseID > NewEntityMap;
	NewEntityMap newEntityMap_;
};

#endif // BUFFERED_ENTITY_TASKS_HPP
