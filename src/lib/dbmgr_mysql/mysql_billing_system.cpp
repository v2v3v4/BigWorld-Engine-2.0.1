/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "mysql_billing_system.hpp"

#include "tasks/get_entity_key_for_account_task.hpp"
#include "tasks/set_entity_key_for_account_task.hpp"

/**
 *	Constructor.
 */
MySqlBillingSystem::MySqlBillingSystem( BgTaskManager & bgTaskManager,
	   const EntityDefs & entityDefs ) :
	BillingSystem( entityDefs ),
	bgTaskManager_( bgTaskManager )
{
}


/**
 *	BillingSystem override
 */
void MySqlBillingSystem::getEntityKeyForAccount( const std::string & logOnName,
		const std::string & password,
		const Mercury::Address & clientAddr,
		IGetEntityKeyForAccountHandler & handler )
{
	if (authenticateViaBaseEntity_)
	{
		handler.onGetEntityKeyForAccountLoadFromUsername(
				entityTypeIDForUnknownUsers_, logOnName, true );
	}
	else
	{
		bgTaskManager_.addBackgroundTask(
			new GetEntityKeyForAccountTask( logOnName, password,
				shouldAcceptUnknownUsers_,
				shouldRememberUnknownUsers_,
				entityTypeIDForUnknownUsers_,
				handler ) );
	}
}


/**
 *	BillingSystem override
 */
void MySqlBillingSystem::setEntityKeyForAccount( const std::string & username,
	const std::string & password, const EntityKey & ekey )
{
	bgTaskManager_.addBackgroundTask(
		new SetEntityKeyForAccountTask( username, password, ekey ) );
}

// mysql_billing_system.cpp
