/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PUT_ENTITY_TASK_HPP
#define PUT_ENTITY_TASK_HPP

#include "dbmgr_lib/idatabase.hpp"
#include "entity_task_with_id.hpp"

#include "cstdmf/memory_stream.hpp"

#include "server/common.hpp"

class BufferedEntityTasks;
class EntityTypeMapping;

/**
 *	This class encapsulates the MySqlDatabase::putEntity() operation so that
 *	it can be executed in a separate thread.
 */
class PutEntityTask : public EntityTaskWithID
{
public:
	PutEntityTask( const EntityTypeMapping * pEntityTypeMapping,
							DatabaseID databaseID,
							EntityID entityID,
							BinaryIStream * pStream,
							const EntityMailBoxRef * pBaseMailbox,
							bool removeBaseMailbox,
							UpdateAutoLoad updateAutoLoad,
							IDatabase::IPutEntityHandler & handler,
							GameTime * pGameTime = NULL );

	virtual void performBackgroundTask( MySql & conn );
	virtual void performEntityMainThreadTask( bool succeeded );

private:
	bool							writeEntityData_;
	bool							writeBaseMailbox_;
	bool							removeBaseMailbox_;
	UpdateAutoLoad 					updateAutoLoad_;

	MemoryOStream					stream_;
	EntityMailBoxRef				baseMailbox_;

	IDatabase::IPutEntityHandler &	handler_;

	GameTime * pGameTime_;
};

#endif // PUT_ENTITY_TASK_HPP
