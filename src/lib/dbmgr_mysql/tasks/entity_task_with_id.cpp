/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "entity_task_with_id.hpp"

#include "dbmgr_mysql/query.hpp"
#include "dbmgr_mysql/result_set.hpp"

/**
 *	Constructor.
 */
EntityTaskWithID::EntityTaskWithID( const EntityTypeMapping & entityTypeMapping,
							DatabaseID databaseID,
							EntityID entityID,
							const char * taskName ) :
	EntityTask( entityTypeMapping, databaseID, taskName ),
	entityID_( entityID )
{
}

// entity_task_with_id.cpp
