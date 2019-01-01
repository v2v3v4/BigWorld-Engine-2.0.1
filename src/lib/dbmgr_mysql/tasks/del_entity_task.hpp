/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DEL_ENTITY_TASK_HPP
#define DEL_ENTITY_TASK_HPP

#include "dbmgr_lib/idatabase.hpp"
#include "entity_task_with_id.hpp"

class EntityDBKey;
class EntityTypeMapping;
class MySqlDatabase;

/**
 *	This class encapsulates the MySqlDatabase::delEntity() operation so that
 *	it can be executed in a separate thread.
 */
class DelEntityTask : public EntityTaskWithID
{
public:
	DelEntityTask( const EntityTypeMapping * pEntityMapping,
					const EntityDBKey & ekey,
					EntityID entityID,
					IDatabase::IDelEntityHandler & handler );

	virtual void performBackgroundTask( MySql & conn );
	virtual void performEntityMainThreadTask( bool succeeded );

private:
	const EntityDBKey entityKey_;

	IDatabase::IDelEntityHandler &	handler_;
};

#endif // DEL_ENTITY_TASK_HPP
