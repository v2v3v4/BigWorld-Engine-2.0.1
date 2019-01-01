/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "consolidate_entity_task.hpp"

#include "dbmgr_mysql/mappings/entity_type_mapping.hpp"


/**
 *	Constructor.
 */
ConsolidateEntityTask::ConsolidateEntityTask(
						const EntityTypeMapping * pEntityTypeMapping,
						DatabaseID databaseID,
						BinaryIStream & stream,
						GameTime time,
						IDatabase::IPutEntityHandler & handler ) :
	PutEntityTask( pEntityTypeMapping,
			databaseID, 0, &stream, NULL, false, UPDATE_AUTO_LOAD_RETAIN, 
			handler, &time_ ),
	time_( time )
{
}


/**
 *
 */
void ConsolidateEntityTask::performBackgroundTask( MySql & conn )
{
	if (!entityTypeMapping_.hasNewerRecord( conn, dbID_, time_ ))
	{
		this->PutEntityTask::performBackgroundTask( conn );
	}
}


// consolidate_entity_task.cpp
