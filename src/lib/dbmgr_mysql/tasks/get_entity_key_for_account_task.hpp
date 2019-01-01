/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GET_ENTITY_KEY_FOR_ACCOUNT_TASK_HPP
#define GET_ENTITY_KEY_FOR_ACCOUNT_TASK_HPP

#include "background_task.hpp"
#include "cstdmf/value_or_null.hpp"
#include "dbmgr_lib/billing_system.hpp"

/**
 *	This class encapsulates a mapLoginToEntityKey() operation so that it can
 *	be executed in a separate thread.
 */
class GetEntityKeyForAccountTask : public MySqlBackgroundTask
{
public:
	GetEntityKeyForAccountTask( const std::string & logOnName,
		const std::string & password,
		bool shouldAcceptUnknownUsers,
		bool shouldRememberUnknownUsers,
		EntityTypeID entityTypeIDForUnknownUsers,
		IGetEntityKeyForAccountHandler & handler );

	// MySqlBackgroundTask overrides
	virtual void performBackgroundTask( MySql & conn );
	virtual void performMainThreadTask( bool succeeded );

protected:
	void onException( const DatabaseException & e );

private:
	// Input values
	std::string			logOnName_;
	std::string			password_;

	// Output values
	LogOnStatus			loginStatus_;
	ValueOrNull< EntityTypeID >		entityTypeID_;
	EntityTypeID					defaultEntityTypeID_;
	ValueOrNull< DatabaseID >		databaseID_;

	bool shouldAcceptUnknownUsers_;
	bool shouldRememberUnknownUsers_;

	IGetEntityKeyForAccountHandler & handler_;
};

#endif // GET_ENTITY_KEY_FOR_ACCOUNT_TASK_HPP
