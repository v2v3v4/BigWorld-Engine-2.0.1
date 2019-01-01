/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SET_ENTITY_KEY_FOR_ACCOUNT_HPP
#define SET_ENTITY_KEY_FOR_ACCOUNT_HPP

#include "background_task.hpp"

#include "dbmgr_lib/idatabase.hpp"

class EntityKey;

// -----------------------------------------------------------------------------
// Section: SetEntityKeyForAccountTask
// -----------------------------------------------------------------------------

/**
 *	This class encapsulates the setLoginMapping() operation so that it can be
 *	executed in a separate thread.
 */
class SetEntityKeyForAccountTask : public MySqlBackgroundTask
{
public:
	SetEntityKeyForAccountTask( const std::string & username,
			const std::string & password, const EntityKey & ekey );

	// MySqlBackgroundTask overrides
	virtual void performBackgroundTask( MySql & conn );
	virtual void performMainThreadTask( bool succeeded );

private:
	std::string username_;
	std::string password_;
	EntityTypeID typeID_;
	DatabaseID entityDatabaseID_;
};

#endif // SET_ENTITY_KEY_FOR_ACCOUNT_HPP
