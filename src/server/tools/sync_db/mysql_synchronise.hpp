/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MYSQL_SYNCHRONISE_HPP
#define MYSQL_SYNCHRONISE_HPP

#include "dbmgr_mysql/database_tool_app.hpp"

#include <string>

class MySql;

class MySqlSynchronise : public DatabaseToolApp
{
public:
	MySqlSynchronise();
	virtual ~MySqlSynchronise();

	bool init( bool isVerbose, bool shouldLock );

	virtual bool run();

	MySql & connection()
		{ return this->DatabaseToolApp::connection(); }

	const EntityDefs & entityDefs()
		{ return this->DatabaseToolApp::entityDefs(); }

	bool synchroniseEntityDefinitions( bool allowNew,
		const std::string & characterSet = "",
		const std::string & collation = "" );
private:
	bool doSynchronise();
	void createSpecialBigWorldTables();
	bool updatePasswordHash( bool wasHashed );
	bool alterDBCharSet( const std::string & characterSet, 
		const std::string & collation );


};

#endif // MYSQL_SYNCHRONISE_HPP
