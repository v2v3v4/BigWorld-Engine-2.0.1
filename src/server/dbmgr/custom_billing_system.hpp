/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CUSTOM_BILLING_SYSTEM_HPP
#define CUSTOM_BILLING_SYSTEM_HPP

#include "dbmgr_lib/billing_system.hpp"

class EntityDefs;


/**
 *	This class is used to integrate with a billing system via Python.
 */
class CustomBillingSystem : public BillingSystem
{
public:
	CustomBillingSystem( const EntityDefs & entityDefs );
	~CustomBillingSystem();

	virtual void getEntityKeyForAccount(
		const std::string & username, const std::string & password,
		const Mercury::Address & clientAddr,
		IGetEntityKeyForAccountHandler & handler );

	virtual void setEntityKeyForAccount( const std::string & username,
		const std::string & password, const EntityKey & ekey );

	virtual bool isOkay() const;

private:
	EntityTypeID entityTypeID_;
};

#endif // CUSTOM_BILLING_SYSTEM_HPP
