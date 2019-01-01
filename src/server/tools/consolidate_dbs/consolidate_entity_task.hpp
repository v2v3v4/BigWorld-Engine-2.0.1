/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONSOLIDATE_ENTITY_TASK_HPP
#define CONSOLIDATE_ENTITY_TASK_HPP

#include "dbmgr_mysql/tasks/put_entity_task.hpp"

/**
 *	This class extends PutEntityTask to ensure that the entity is only written
 *	if the existing gameTime value is small enough.
 */
class ConsolidateEntityTask : public PutEntityTask
{
public:
	ConsolidateEntityTask( const EntityTypeMapping * pEntityTypeMapping,
							DatabaseID databaseID,
							BinaryIStream & stream,
							GameTime time,
							IDatabase::IPutEntityHandler & handler );

	virtual void performBackgroundTask( MySql & conn );

private:
	GameTime time_;
};

#endif // CONSOLIDATE_ENTITY_TASK_HPP
