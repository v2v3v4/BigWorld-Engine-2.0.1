/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "billing_system.hpp"

#include "db_entitydefs.hpp"

#include "cstdmf/watcher.hpp"
#include "server/bwconfig.hpp"

/**
 *	Constructor.
 */
BillingSystem::BillingSystem( const EntityDefs & entityDefs ) :
	shouldAcceptUnknownUsers_(
		BWConfig::get( "billingSystem/shouldAcceptUnknownUsers", false ) ),
	shouldRememberUnknownUsers_(
		BWConfig::get( "billingSystem/shouldRememberUnknownUsers", false ) ),
	authenticateViaBaseEntity_(
			BWConfig::get( "billingSystem/authenticateViaBaseEntity", false ) ),
	entityTypeIDForUnknownUsers_( INVALID_ENTITY_TYPE_ID ),
	entityTypeForUnknownUsers_(
			BWConfig::get( "billingSystem/entityTypeForUnknownUsers", "" ) )
{
	INFO_MSG( "\tshouldAcceptUnknownUsers = %s\n",
							shouldAcceptUnknownUsers_ ? "True" : "False" );
	INFO_MSG( "\tshouldRememberUnknownUsers = %s\n",
							shouldRememberUnknownUsers_ ? "True" : "False" );
	INFO_MSG( "\tauthenticateViaBaseEntity = %s\n",
							authenticateViaBaseEntity_ ? "True" : "False" );
	INFO_MSG( "\tentityTypeForUnknownUsers = %s\n",
							entityTypeForUnknownUsers_.c_str() );

	entityTypeIDForUnknownUsers_ =
			entityDefs.getEntityType( entityTypeForUnknownUsers_ );

	if (shouldAcceptUnknownUsers_ || authenticateViaBaseEntity_)
	{
		if (entityTypeIDForUnknownUsers_ == INVALID_ENTITY_TYPE_ID)
		{
			ERROR_MSG( "BillingSystem::BillingSystem: "
						"Invalid entity type for unknown users = '%s'\n",
					entityTypeForUnknownUsers_.c_str() );
		}
	}
}

// billing_system.cpp
