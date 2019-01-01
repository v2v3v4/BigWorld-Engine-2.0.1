/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONSOLIDATE_DBS__PRIMARY_DATABASE_UPDATE_QUEUE_HPP
#define CONSOLIDATE_DBS__PRIMARY_DATABASE_UPDATE_QUEUE_HPP

#include "cstdmf/bgtask_manager.hpp"

#include "dbmgr_mysql/mappings/entity_type_mappings.hpp"
#include "dbmgr_mysql/wrapper.hpp"

#include "dbmgr_lib/idatabase.hpp"

#include <set>

class BufferedEntityTasks;
class EntityDefs;
class EntityKey;
namespace DBConfig
{
	class ConnectionInfo;
}


/**
 *	This class implements job queue for entity update operations. Entity
 *	update operations will be serviced by multiple threads.
 */
class PrimaryDatabaseUpdateQueue : public IDatabase::IPutEntityHandler
{
public:
	PrimaryDatabaseUpdateQueue( const DBConfig::ConnectionInfo & connectionInfo,
		const EntityDefs & entityDefs, int numConnections );
	~PrimaryDatabaseUpdateQueue();

	void addUpdate( const EntityKey & key, BinaryIStream & data,
			GameTime time );
	void waitForUpdatesCompletion();

	bool hasError() const				{ return hasError_; }

private:
	// Called by ConsolidateEntityTask
	virtual void onPutEntityComplete( bool isOkay, DatabaseID dbID );

// Member data

	BgTaskManager 			bgTaskMgr_;
	BufferedEntityTasks * 	pBufferedEntityTasks_;
	EntityTypeMappings 		entityTypeMappings_;

	bool					hasError_;
	int						numOutstanding_;

	typedef std::map< EntityKey, GameTime > ConsolidatedTimes;
	ConsolidatedTimes		consolidatedTimes_;
};

#endif // CONSOLIDATE_DBS__PRIMARY_DATABASE_UPDATE_QUEUE_HPP
