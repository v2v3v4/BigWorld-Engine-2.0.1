/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BILLING_SYSTEM_HPP
#define BILLING_SYSTEM_HPP

#include "entity_key.hpp"

#include "connection/log_on_status.hpp"

class EntityDefs;

/**
 *	This is the callback interface used by getEntityKeyForAccount(). One of its
 *	virtual methods is called when 
 */
class IGetEntityKeyForAccountHandler
{
public:
	/**
	 *	This method is called when getEntityKeyForAccount() completes
	 *	successfully.
	 *
	 *	@param	ekey	The entity key for the account.
	 */
	virtual void onGetEntityKeyForAccountSuccess( const EntityKey & ekey ) = 0;

	/**
	 *	This method is called when getEntityKeyForAccount() completes and it
	 *	wants to load the entity with the given username.
	 */
	virtual void onGetEntityKeyForAccountLoadFromUsername(
			EntityTypeID entityType, const std::string & username,
			bool shouldCreateNewOnLoadFailure ) = 0;

	/**
	 *	This method is called when getEntityKeyForAccount wants to create a new
	 *	entity for the account.
	 */
	virtual void onGetEntityKeyForAccountCreateNew(
		EntityTypeID entityType, bool shouldRemember ) = 0;

	/**
	 *	This method is called when getEntityKeyForAccount rejects the account.
	 */
	virtual void onGetEntityKeyForAccountFailure( LogOnStatus status,
			const std::string & errorMsg = "" ) = 0;
};


/**
 *	This class defines the interface for integrating with a billing system.
 */
class BillingSystem
{
public:
	BillingSystem( const EntityDefs & entityDefs );

	/**
	 *	This function turns user/pass login credentials into the EntityKey
	 *	associated with them.
	 *
	 *	This method provides the result of the operation by calling
	 *	IMapLoginToEntityKeyHandler::onMapLoginToEntityKeyComplete().
	 */
	virtual void getEntityKeyForAccount(
		const std::string & username, const std::string & password,
		const Mercury::Address & clientAddr,
		IGetEntityKeyForAccountHandler & handler ) = 0;

	/**
	 *	This function sets the mapping between user/pass to an entity.
	 */
	virtual void setEntityKeyForAccount( const std::string & username,
		const std::string & password, const EntityKey & ekey ) = 0;

	virtual bool isOkay() const
	{
		return (entityTypeIDForUnknownUsers_ != INVALID_ENTITY_TYPE_ID) ||
			(!shouldAcceptUnknownUsers_ && !authenticateViaBaseEntity_);
	}

protected:
	// Configuration from bw.xml
	bool shouldAcceptUnknownUsers_;
	bool shouldRememberUnknownUsers_;
	bool authenticateViaBaseEntity_;
	EntityTypeID entityTypeIDForUnknownUsers_;
	std::string entityTypeForUnknownUsers_;
};

#endif // BILLING_SYSTEM_HPP
