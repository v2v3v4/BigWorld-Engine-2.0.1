/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GET_ENTITY_TASK_HPP
#define GET_ENTITY_TASK_HPP

#include "dbmgr_lib/idatabase.hpp"
#include "entity_task.hpp"

class EntityTypeMapping;

/**
 *	This class encapsulates the MySqlDatabase::getEntity() operation so that
 *	it can be executed in a separate thread.
 */
class GetEntityTask : public EntityTask
{
public:
	GetEntityTask( const EntityTypeMapping * pEntityTypeMapping,
			const EntityDBKey & entityKey,
			BinaryOStream * pStream,
			bool shouldGetBaseEntityLocation,
			const char * pPasswordOverride,
			IDatabase::IGetEntityHandler & handler );

	virtual void performBackgroundTask( MySql & conn );
	virtual void performEntityMainThreadTask( bool succeeded );

private:
	bool fillKey( MySql & connection, EntityDBKey & ekey );

	EntityDBKey entityKey_;

	BinaryOStream * pStream_;

	IDatabase::IGetEntityHandler & handler_;

	std::string passwordOverride_;

	EntityMailBoxRef baseEntityLocation_;

	// Put at the end for better packing
	bool shouldGetBaseEntityLocation_;
	bool hasBaseLocation_;
	bool hasPasswordOverride_;
};

#endif // GET_ENTITY_TASK_HPP
