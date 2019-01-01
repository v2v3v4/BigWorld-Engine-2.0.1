/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "mysql_synchronise.hpp"

#include "mysql_table_initialiser.hpp"
#include "mysql_upgrade_database.hpp"

#include "dbmgr_mysql/database_exception.hpp"
#include "dbmgr_mysql/locked_connection.hpp"
#include "dbmgr_mysql/query.hpp"
#include "dbmgr_mysql/table_inspector.hpp"
#include "dbmgr_mysql/transaction.hpp"
#include "dbmgr_mysql/versions.hpp"
#include "dbmgr_mysql/wrapper.hpp"

#include "dbmgr_mysql/mappings/property_mappings_per_type.hpp"

#include "network/basictypes.hpp"
#include "resmgr/bwresource.hpp"
#include "server/bwconfig.hpp"

extern int UserDataObjectLinkDataType_token;
extern int ResMgr_token;
static int s_moduleTokens = ResMgr_token | UserDataObjectLinkDataType_token;


/**
 *	Constructor.
 */
MySqlSynchronise::MySqlSynchronise():
		DatabaseToolApp()
{}


/**
 *	Destructor.
 */
MySqlSynchronise::~MySqlSynchronise()
{}


/**
 *	Initialise the instance.
 */
bool MySqlSynchronise::init( bool isVerbose, bool shouldLock )
{
	return this->DatabaseToolApp::init( "SyncDB", isVerbose, shouldLock );
}


/**
 *	Synchronise the database.
 *
 *	This method wraps exception handling around doSynchronise().
 */
bool MySqlSynchronise::run()
{
	bool syncSuccess = true;

	try
	{
		syncSuccess = this->doSynchronise();
	}
	catch (DatabaseException & e)
	{
		ERROR_MSG( "%s\n", e.what() );
		syncSuccess = false;
	}
	return syncSuccess;
}


/**
 *	Synchronise the database with the entity definitions.
 */
bool MySqlSynchronise::doSynchronise()
{
	MySql & connection = this->connection();

	this->createSpecialBigWorldTables();

	// Don't drop tables or columns if there are unconsolidated databases
	if (numSecondaryDBs( connection ) > 0)
	{
		ERROR_MSG( "Cannot sync database while there are "
					"unconsolidated secondary databases.\n"
					"Please revert any changes to entity definitions "
					"and run the data consolidation tool.\n"
					"Alternatively, run \"consolidate_dbs --clear\" "
					"to clear secondary database information, "
					"unconsolidated data will be lost.\n" );
		return false;
	}

	uint32 version = getBigWorldDBVersion( connection );

	if (version > DBMGR_CURRENT_VERSION)
	{
		ERROR_MSG( "Unable to upgrade the database to version %u, a newer "
					"DB version (%u) is already in use.\n",
					DBMGR_CURRENT_VERSION, version );
		return false;
	}

	static const char * DEFAULT_CHARACTER_SET = "utf8";
	static const char * DEFAULT_COLLATION = "utf8_bin";

	// TODO: I am considering having an empty string for characterSet to mean
	// do not modify and empty for collation to mean use default collation.
	// There's currently a bug in XMLSection that prevents setting a config
	// option to empty string if there's a default value.
	const std::string characterSet =
		BWConfig::get( "dbMgr/unicodeString/characterSet",
				DEFAULT_CHARACTER_SET );
	const std::string collation =
		BWConfig::get( "dbMgr/unicodeString/collation", DEFAULT_COLLATION );

	if (characterSet != DEFAULT_CHARACTER_SET)
	{
		NOTICE_MSG( "Character set is '%s'\n", characterSet.c_str() );
	}

	if (collation != DEFAULT_COLLATION)
	{
		NOTICE_MSG( "Collation is '%s'\n", collation.c_str() );
	}

	if (!this->alterDBCharSet( characterSet, collation ))
	{
		return false;
	}

	if (version != DBMGR_CURRENT_VERSION)
	{
		MySqlUpgradeDatabase upgradeDatabase( *this );
		upgradeDatabase.run( version );
	}

	bool wasPasswordHashed = getIsPasswordHashed( connection );

	return this->updatePasswordHash( wasPasswordHashed ) &&
		this->synchroniseEntityDefinitions( true, characterSet, collation );
}


/**
 * This method alters the database default character set and collation.
 *
 * @param characterSet 	The name of the character set encoding.
 * @param collation		The name of the character set encoding's collation to
 * 						use.
 */
bool MySqlSynchronise::alterDBCharSet( const std::string & characterSet,
	   const std::string & collation )
{
	MySql & connection = this->connection();

	if (characterSet.empty())
	{
		NOTICE_MSG( "Empty character set. Leaving at existing setting\n" );
		return true;
	}

	try
	{
		if (collation.empty())
		{
			const Query query( "ALTER DATABASE CHARACTER SET ?" );
			query.execute( connection, characterSet, NULL );
		}
		else
		{
			const Query query( "ALTER DATABASE CHARACTER SET ? COLLATE ?" );
			query.execute( connection, characterSet, collation, NULL );
		}
	}
	catch (DatabaseException & e)
	{
		ERROR_MSG( "Invalid characterSet or collation setting in 'bw.xml'\n" );
		ERROR_MSG( "%s\n", e.what() );
		return false;
	}

	return true;
}


/**
 * 	This method creates all the tables that DBMgr uses to store non-entity
 * 	data e.g. logons, meta data etc.
 */
void MySqlSynchronise::createSpecialBigWorldTables()
{
	MySql & connection = this->connection();

	char buffer[ 512 ];

	// Version table
	connection.execute( "CREATE TABLE IF NOT EXISTS bigworldInfo "
			"(version INT UNSIGNED NOT NULL, "
				"snapshotTime TIMESTAMP NULL, "
				"isPasswordHashed BOOL NOT NULL DEFAULT FALSE) "
			"ENGINE="MYSQL_ENGINE_TYPE );

	// Metadata tables.
	connection.execute( "CREATE TABLE IF NOT EXISTS bigworldEntityTypes "
			 "(typeID INT NOT NULL AUTO_INCREMENT, bigworldID INT, "
			 "name CHAR(255) CHARACTER SET latin1 NOT NULL UNIQUE, "
			 " PRIMARY KEY(typeID), "
			 "KEY(bigworldID)) ENGINE="MYSQL_ENGINE_TYPE );

	// Logon/checkout tables
	connection.execute( "CREATE TABLE IF NOT EXISTS bigworldLogOns "
			 "(databaseID BIGINT NOT NULL, typeID INT NOT NULL, "
			 "objectID INT, ip INT UNSIGNED, port SMALLINT UNSIGNED, "
			 "salt SMALLINT UNSIGNED, "
			 "shouldAutoLoad BOOL NOT NULL DEFAULT FALSE, "
			 "PRIMARY KEY(typeID, databaseID)) "
			 "ENGINE="MYSQL_ENGINE_TYPE );

	bw_snprintf( buffer, sizeof( buffer ),
				"CREATE TABLE IF NOT EXISTS bigworldLogOnMapping "
				 "(logOnName VARBINARY(%d) NOT NULL,"
				 " password VARBINARY(%d) NOT NULL,"
				 " entityType INT, entityID BIGINT,"
				 " PRIMARY KEY(logOnName)) ENGINE="MYSQL_ENGINE_TYPE,
			 BWMySQLMaxLogOnNameLen, BWMySQLMaxLogOnPasswordLen );
	connection.execute( buffer );

	// Entity ID tables.
	connection.execute( "CREATE TABLE IF NOT EXISTS bigworldNewID "
					 "(id INT NOT NULL) ENGINE="MYSQL_ENGINE_TYPE );
	connection.execute( "CREATE TABLE IF NOT EXISTS bigworldUsedIDs "
					 "(id INT NOT NULL) ENGINE="MYSQL_ENGINE_TYPE );

	// Game time
	connection.execute( "CREATE TABLE IF NOT EXISTS bigworldGameTime "
					"(time INT NOT NULL) ENGINE="MYSQL_ENGINE_TYPE );

	// Space data tables.
	int maxSpaceDataSize = std::max( BWConfig::get( "dbMgr/maxSpaceDataSize",
										maxSpaceDataSize ), 1 );
	const std::string& blobTypeName =
		MySqlTypeTraits< std::string >::colTypeStr( maxSpaceDataSize );
	connection.execute( "CREATE TABLE IF NOT EXISTS bigworldSpaces "
			"(id INT NOT NULL UNIQUE) ENGINE="MYSQL_ENGINE_TYPE );
	bw_snprintf( buffer, sizeof( buffer ),
			"CREATE TABLE IF NOT EXISTS bigworldSpaceData "
			"(id INT NOT NULL, INDEX (id), "
			"spaceEntryID BIGINT NOT NULL, "
			"entryKey SMALLINT UNSIGNED NOT NULL, "
			"data %s NOT NULL ) ENGINE="MYSQL_ENGINE_TYPE,
			blobTypeName.c_str() );
	connection.execute( buffer );

	// Just in case the table already exists and have a different BLOB
	// type for the data column. maxSpaceDataSize is configurable.
	bw_snprintf( buffer, sizeof( buffer ),
			"ALTER TABLE bigworldSpaceData MODIFY data %s",
			 blobTypeName.c_str() );
	connection.execute( buffer );

	// Must fit in BLOB column
	MF_ASSERT( MAX_SECONDARY_DB_LOCATION_LENGTH < (1<<16) );
	connection.execute( "CREATE TABLE IF NOT EXISTS "
			"bigworldSecondaryDatabases (ip INT UNSIGNED NOT NULL, "
			"port SMALLINT UNSIGNED NOT NULL, appID INT NOT NULL, "
			"location BLOB NOT NULL, INDEX addr (ip, port, appID)) "
			"ENGINE="MYSQL_ENGINE_TYPE );

	// SQLite checksum table.
	connection.execute( "CREATE TABLE IF NOT EXISTS bigworldEntityDefsChecksum "
			 "(checksum CHAR(255) CHARACTER SET latin1) "
			 "ENGINE="MYSQL_ENGINE_TYPE );
}


bool MySqlSynchronise::updatePasswordHash( bool wasHashed )
{
	MySql & connection = this->connection();

	bool shouldHash = BWConfig::get( "dbmgr/isPasswordHashed", true );

	if (wasHashed && !shouldHash)
	{
		ERROR_MSG( "MySqlSynchronise::updatePasswordHash: "
			"Cannot remove hashing. Manually update "
			"bigworldInfo.isPasswordHashed and bigworldLogOnMapping.password "
			"fields.\n" );

		return false;
	}

	if (!wasHashed && shouldHash)
	{
		MySqlTransaction transaction( connection );

		// hash password
		connection.execute( "UPDATE bigworldInfo SET isPasswordHashed = TRUE" );
		connection.execute( "UPDATE bigworldLogOnMapping "
				"SET password = MD5( CONCAT( password, logOnName ) )" );

		transaction.commit();
	}

	return true;
}


/**
 *	This method synchronises the entity definitions.
 */
bool MySqlSynchronise::synchroniseEntityDefinitions( bool allowNew,
		const std::string & characterSet,
		const std::string & collation )
{
	MySql & connection = this->connection();

	bool isOkay = true;
	try
	{
		PropertyMappingsPerType propertyMappingPerType;

		if (!propertyMappingPerType.init( this->entityDefs() ))
		{
			return false;
		}

		TableInitialiser tableInitialiser( connection, allowNew,
			   characterSet, collation );
		isOkay &= propertyMappingPerType.visit( this->entityDefs(), 
			tableInitialiser );
	}
	catch (std::exception& e )
	{
		ERROR_MSG( "MySqlSynchronise::synchroniseEntityDefinitions: %s\n", 
			e.what() );
		isOkay = false;
	}

	return isOkay;
}

// mysql_synchronise.cpp
