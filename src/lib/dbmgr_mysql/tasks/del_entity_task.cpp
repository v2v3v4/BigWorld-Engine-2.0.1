/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "del_entity_task.hpp"

#include "dbmgr_mysql/mappings/entity_type_mapping.hpp"

/**
 *	Constructor.
 */
DelEntityTask::DelEntityTask( const EntityTypeMapping * pEntityMapping,
		const EntityDBKey & entityKey,
		EntityID entityID,
		IDatabase::IDelEntityHandler & handler ) :
	EntityTaskWithID( *pEntityMapping, entityKey.dbID , entityID, "DelEntityTask" ),
	entityKey_( entityKey ),
	handler_( handler )
{
}


/**
 *	This method deletes an entity from the database. May be executed in a
 *	separate thread.
 */
void DelEntityTask::performBackgroundTask( MySql & conn )
{
	DatabaseID dbID = entityKey_.dbID;

	MF_ASSERT( dbID != PENDING_DATABASE_ID );

	if (dbID == 0)
	{
		dbID = entityTypeMapping_.getDbID( conn, entityKey_.name );
	}

	if (dbID == 0)
	{
		this->setFailure();
		return;
	}

	if (!entityTypeMapping_.deleteWithID( conn, dbID ))
	{
		this->setFailure();
		return;
	}

	entityTypeMapping_.removeLogOnRecord( conn, dbID );
}


/**
 *	This method is called in the main thread after run() completes.
 */
void DelEntityTask::performEntityMainThreadTask( bool succeeded )
{
	handler_.onDelEntityComplete( succeeded );
}

// del_entity_task.cpp
