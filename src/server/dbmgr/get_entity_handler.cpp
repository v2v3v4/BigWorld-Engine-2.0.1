/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "get_entity_handler.hpp"

#include "database.hpp"

/**
 *	This method intercepts the result of IDatabase::getEntity() operations and
 *	mucks around with it before passing it to onGetEntityCompleted().
 */
void GetEntityHandler::onGetEntityComplete( bool isOK,
					const EntityDBKey & entityKey,
					const EntityMailBoxRef * pBaseEntityLocation )
{
	// Update mailbox for dead BaseApps.
	if (Database::instance().hasMailboxRemapping() &&
			pBaseEntityLocation)
	{
		EntityMailBoxRef remappedLocation = *pBaseEntityLocation;

		Database::instance().remapMailbox( remappedLocation );

		this->onGetEntityCompleted( isOK, entityKey, &remappedLocation );
	}
	else
	{
		this->onGetEntityCompleted( isOK, entityKey, pBaseEntityLocation );
	}
}


/**
 *	This method checks Checks that pBaseRef is fully checked out i.e. not in "pending base
 *	creation" state.
 */
bool GetEntityHandler::isActiveMailBox( const EntityMailBoxRef * pBaseRef )
{
	if (pBaseRef && pBaseRef->id != 0)
	{
		MF_ASSERT( pBaseRef->addr.ip != 0 );

		return true;
	}

	return false;
}

// get_entity_handler.cpp
