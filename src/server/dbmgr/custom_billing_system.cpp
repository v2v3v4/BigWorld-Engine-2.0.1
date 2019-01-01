/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "custom_billing_system.hpp"

#include "dbmgr_lib/db_entitydefs.hpp"

// -----------------------------------------------------------------------------
// Section: CustomBillingSystem
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
CustomBillingSystem::CustomBillingSystem( const EntityDefs & entityDefs ) :
	BillingSystem( entityDefs ),
	entityTypeID_( entityDefs.getEntityType( "Account" ) )
{
}


/**
 *	Destructor.
 */
CustomBillingSystem::~CustomBillingSystem()
{
}


/**
 *	This method validates a login attempt and returns the details of the entity
 *	to use via the handler.
 */
void CustomBillingSystem::getEntityKeyForAccount(
	const std::string & username, const std::string & password,
	const Mercury::Address & clientAddr,
	IGetEntityKeyForAccountHandler & handler )
{
	// TODO: Look up the entity and call onGetEntityKeyForAccountSuccess.

	// As a fallback, allow the player to log in and create the default entity
	// type. Do not remember the entity since setEntityKeyForAccount is not yet
	// implemented by this class.
	handler.onGetEntityKeyForAccountCreateNew( entityTypeID_, false );
}


/**
 *	This method is used to inform the billing system that a new entity has been
 *	associated with a login.
 */
void CustomBillingSystem::setEntityKeyForAccount( const std::string & username,
	const std::string & password, const EntityKey & ekey )
{
	// TODO: Code to store the new account in the billing system. This is
	// required is shouldRemember is set to true in
	// onGetEntityKeyForAccountCreateNew.
}


/**
 *	This method returns whether the billing system has been set up correctly.
 */
bool CustomBillingSystem::isOkay() const
{
	return entityTypeID_ != INVALID_ENTITY_TYPE_ID;
}

// py_billing_system.cpp
