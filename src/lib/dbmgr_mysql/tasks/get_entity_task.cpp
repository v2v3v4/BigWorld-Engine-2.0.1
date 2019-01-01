/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "get_entity_task.hpp"

#include "dbmgr_mysql/mappings/entity_type_mapping.hpp"

// -----------------------------------------------------------------------------
// Section: GetEntityTask
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
GetEntityTask::GetEntityTask( const EntityTypeMapping * pEntityTypeMapping,
			const EntityDBKey & entityKey,
			BinaryOStream * pStream,
			bool shouldGetBaseEntityLocation,
			const char * pPasswordOverride,
			IDatabase::IGetEntityHandler & handler ) :
	EntityTask( *pEntityTypeMapping, entityKey.dbID, "GetEntityTask" ),
	entityKey_( entityKey ),
	pStream_( pStream ),
	handler_( handler ),
	passwordOverride_( (pPasswordOverride != NULL) ? pPasswordOverride : "" ),
	baseEntityLocation_(),

	shouldGetBaseEntityLocation_( shouldGetBaseEntityLocation ),
	hasBaseLocation_( false ),
	hasPasswordOverride_( pPasswordOverride != NULL )
{
}


/**
 *	This method performs this task in a background thread.
 */
void GetEntityTask::performBackgroundTask( MySql & conn )
{
	bool isOkay = true;

	bool definitelyExists = false;

	if (pStream_ != NULL)
	{
		const std::string * pPasswordOverride =
					hasPasswordOverride_ ? &passwordOverride_ : NULL;

		if (entityKey_.dbID != 0)
		{
			definitelyExists = entityTypeMapping_.getStreamByID(
					conn, entityKey_.dbID, *pStream_, pPasswordOverride );
		}
		else
		{
			entityKey_.dbID = entityTypeMapping_.getStreamByName(
					conn, entityKey_.name, *pStream_, pPasswordOverride );
			definitelyExists = (entityKey_.dbID != 0);
		}

		isOkay = definitelyExists;
	}

	if (isOkay && shouldGetBaseEntityLocation_)
	{
		// Need to get base mail box
		if (!definitelyExists)
		{
			isOkay = this->fillKey( conn, entityKey_ );
		}

		if (isOkay)
		{
			// Try to get base mailbox
			definitelyExists = true;

			hasBaseLocation_ = entityTypeMapping_.getLogOnRecord( conn,
							entityKey_.dbID, baseEntityLocation_ );
		}
	}

	if (isOkay && !definitelyExists)
	{
		// Caller hasn't asked for anything except the missing member of
		// entityKey_
		isOkay = this->fillKey( conn, entityKey_ );
	}

	if (!isOkay)
	{
		this->setFailure();
	}
}


/**
 *	This method is called in the main thread after run() is complete.
 */
void GetEntityTask::performEntityMainThreadTask( bool succeeded )
{
	handler_.onGetEntityComplete( succeeded, entityKey_,
			hasBaseLocation_ ? &baseEntityLocation_ : NULL );
}


/**
 *	This method set the missing member of the EntityDBKey. If entity doesn't
 *	have a name property then ekey.name is set to empty.
 *
 *	This method may be called from another thread.
 *
 *	@return	True if successful. False if entity doesn't exist.
 */
bool GetEntityTask::fillKey( MySql & connection, EntityDBKey & ekey )
{
	bool isOkay;

	if (entityTypeMapping_.hasIdentifier())
	{
		if (ekey.dbID)
		{
			isOkay = entityTypeMapping_.getName( connection,
				ekey.dbID, ekey.name );
		}
		else
		{
			ekey.dbID = entityTypeMapping_.getDbID( connection, ekey.name );
			isOkay = ekey.dbID != 0;
		}
	}
	else
	{	// Entity doesn't have a name property. Check for entity existence if
		// DBID is provided
		if (ekey.dbID)
		{
			isOkay = entityTypeMapping_.checkExists( connection,
				ekey.dbID );
			ekey.name.clear();
		}
		else
		{
			isOkay = false;
		}
	}

	return isOkay;
}

// get_entity_task.cpp
