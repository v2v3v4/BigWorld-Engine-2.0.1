/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_UPGRADE_DATABASE_HPP
#define MYSQL_UPGRADE_DATABASE_HPP

#include "cstdmf/stdmf.hpp"

class MySqlSynchronise;

class MySqlUpgradeDatabase
{
public:
	MySqlUpgradeDatabase( MySqlSynchronise & synchronise );

	bool run( uint32 version );

private:
	bool upgradeDatabaseLogOnMapping2();
	bool upgradeDatabaseShouldAutoLoad();
	bool upgradeDatabaseLogOnMapping();
	bool upgradeDatabaseBinaryStrings();
	void upgradeDatabase1_9NonNull();
	void upgradeDatabase1_9Snapshot();
	void upgradeDatabase1_8();
	
	void upgradeVersionNumber( const uint32 newVersion );
	bool convertRecordNameToDBID( int bwEntityTypeID, int dbEntityTypeID );

// Member data

	MySqlSynchronise & synchronise_;
};


#endif /* MYSQL_UPGRADE_DATABASE_HPP */
