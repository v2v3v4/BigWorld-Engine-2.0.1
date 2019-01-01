/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_BILLING_SYSTEM_HPP
#define MYSQL_BILLING_SYSTEM_HPP

#include "cstdmf/bgtask_manager.hpp"
#include "dbmgr_lib/billing_system.hpp"

class EntityDefs;

/**
 *	This class implements the default billing system integration when the
 *	MySQL database is used.
 */
class MySqlBillingSystem : public BillingSystem
{
public:
	MySqlBillingSystem( BgTaskManager & bgTaskManager,
			const EntityDefs & entityDefs );

	virtual void getEntityKeyForAccount(
		const std::string & logOnName, const std::string & password,
		const Mercury::Address & clientAddr,
		IGetEntityKeyForAccountHandler & handler );
	virtual void setEntityKeyForAccount( const std::string & username,
		const std::string & password, const EntityKey & ekey );

private:
	BgTaskManager & bgTaskManager_;
};

#endif // MYSQL_BILLING_SYSTEM_HPP
