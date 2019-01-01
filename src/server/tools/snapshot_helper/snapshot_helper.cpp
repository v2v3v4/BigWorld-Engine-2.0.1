/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "server/config_reader.hpp"

#include <mysql/mysql.h>

#include <errno.h>
#include <string.h>

#include <cstring>
#include <cstdlib>

#include "cstdmf/stdmf.hpp"

bool becomeRootUser();
bool validate_config( ConfigReader & config );
bool acquire_snapshot( ConfigReader & config,
	const char * dbUser, const char * dbPass );
bool release_snapshot( ConfigReader & config );

const char *CONFIG_SECTION = "snapshot";

/**
 *	This util executes privileged commands. The command arguments
 *	are read in from bigworld.conf which should be writable by
 *	root only.
 *
 *	To execute privileged commands the snapshot_helper binary needs
 *	to have it's setuid attribute set. This can be done by:
 *
 *	# chown root:root snapshot_helper
 *	# chmod 4511 snapshot_helper
 */
int main( int argc, char * argv[] )
{
	// If no arguments are provided, test if setuid attribute is set
	if (argc <= 1)
	{
		return becomeRootUser() ? 0 : 1;
	}

	// Read the configuration file and confirm all appropriate values exist
	ConfigReader config( "/etc/bigworld.conf" );
	if (!config.read())
	{
		printf( "Unable to read config file.\n" );
		return 1;
	}

	if (!validate_config( config ))
	{
		return 1;
	}

	// Execute commands
	if (std::strcmp( argv[ 1 ], "acquire-snapshot" ) == 0)
	{
		if (argc != 4)
		{
			printf( "Invalid argument list for 'acquire-snapshot'\n" );
		}

		if (!acquire_snapshot( config, argv[ 2 ], argv[ 3 ] ))
		{
			return 1;
		}
	}
	else if (std::strcmp( argv[ 1 ], "release-snapshot" ) == 0)
	{
		if (argc != 2)
		{
			printf( "Invalid argument list for 'release-snapshot'\n" );
		}

		if (!release_snapshot( config ))
		{
			return 1;
		}
	}
	else
	{
		printf( "Unknown arguments provided.\n" );
		return 1;
	}

	return 0;
}


bool validate_config( ConfigReader & config )
{
	bool status = true;

	const char *config_values[] = { "datadir",
		"lvgroup",
		"lvorigin",
		"lvsnapshot",
		"lvsizegb" };

	for (uint i=0; i < ARRAY_SIZE( config_values ); ++i)
	{
		const char *value = config_values[ i ];
		std::string testString;

		if (!config.getValue( CONFIG_SECTION, value, testString ))
		{
			printf( "Unable to read config value for '%s'\n", value );
			status = false;
		}
	}

	return status;
}


// TODO: sanitise the dbUser / dbPass, they are provided on the command line
bool acquire_snapshot( ConfigReader & config,
	const char * dbUser, const char * dbPass )
{
	// Elevate privileges for actual operations
	if (!becomeRootUser())
	{
		return false;
	}

	std::string dataDir;
	config.getValue( CONFIG_SECTION, "datadir", dataDir );

	std::string lvGroup;
	config.getValue( CONFIG_SECTION, "lvgroup", lvGroup );

	std::string lvOrigin;
	config.getValue( CONFIG_SECTION, "lvorigin", lvOrigin );

	std::string lvSnapshot;
	config.getValue( CONFIG_SECTION, "lvsnapshot", lvSnapshot );

	std::string lvSizeGB;
	config.getValue( CONFIG_SECTION, "lvsizegb", lvSizeGB );


	// Connect to the database and apply a read lock.
	std::string cmd;
	MYSQL sql;

	if (mysql_init( &sql ) == NULL)
	{
		printf( "Unable to initialise MySQL\n" );
		return false;
	}

	// TODO: assuming that the connection can be made on 'localhost'. it may
	//       need to use the hostname depending on the database user's GRANT
	//       privileges
	if (!mysql_real_connect( &sql, "localhost", dbUser, dbPass,
		NULL, 0, NULL, 0 ))
	{
		printf( "Failed to connect to MySQL: %s\n", mysql_error( &sql ) );
		return false;
	}

	cmd = "FLUSH TABLES WITH READ LOCK";
	if (mysql_real_query( &sql, cmd.c_str(), cmd.length() ) != 0)
	{
		printf( "Failed to apply read lock MySQL: %s\n", mysql_error( &sql ) );
		return false;
	}

	// Now create an LVM snapshot
	cmd = "lvcreate -L" + lvSizeGB + "G -s -n " + lvSnapshot +
		" /dev/" + lvGroup + "/" + lvOrigin;
	if (std::system( cmd.c_str() ) != 0)
	{
		printf( "Failed to 'lvcreate' LVM snapshot.\n" );
		mysql_close( &sql );
		return false;
	}

// TODO: if we fail to unlock or mount or chmod the snapshot we can't really
//       recover.

	cmd = "UNLOCK TABLES";
	if (mysql_real_query( &sql, cmd.c_str(), cmd.length() ) != 0)
	{
		printf( "Failed to unlock MySQL: %s\n", mysql_error( &sql ) );
		mysql_close( &sql );
		return false;
	}

	// The MySQL connection is no longer needed
	mysql_close( &sql );

	cmd = "mount /dev/" + lvGroup + "/" + lvSnapshot +
		" /mnt/" + lvSnapshot + "/";
	if (std::system( cmd.c_str() ) != 0)
	{
		printf( "Failed to mount newly created snapshot.\n" );
		return false;
	}

	std::string snapshotFiles( "/mnt/" + lvSnapshot + "/" + dataDir );

	// Relax permissions so we can take ownership of the backup files,
	// this makes sending and consolidating easier on the snapshot machine
	cmd = "chmod -R 755 " + snapshotFiles;
	if (std::system( cmd.c_str() ) != 0)
	{
		printf( "Failed to chmod the snapshot file.s\n" );
		return false;
	}

	printf( "%s\n", snapshotFiles.c_str() );

	return true;
}


bool release_snapshot( ConfigReader &config )
{
	// Elevate privileges for actual operations
	if (!becomeRootUser())
	{
		return false;
	}

	std::string lvGroup;
	config.getValue( CONFIG_SECTION, "lvgroup", lvGroup );

	std::string lvSnapshot;
	config.getValue( CONFIG_SECTION, "lvsnapshot", lvSnapshot );


	std::string cmd;
	bool isOK = true;

	cmd = "umount /mnt/" + lvSnapshot + "/";
	if (std::system( cmd.c_str() ) != 0)
	{
		printf( "Unable to unmount snapshot. '%s'\n", cmd.c_str() );
		isOK = false;
	}

	cmd = "lvremove -f /dev/" + lvGroup + "/" + lvSnapshot;
	if (std::system( cmd.c_str() ) != 0)
	{
		printf( "Unable to 'lvremove' snapshot. '%s'\n", cmd.c_str() );
		isOK = false;
	}

	return isOK;
}


bool becomeRootUser()
{
	if (setuid( 0 ) == -1)
	{
		printf( "%s\n", strerror( errno ) );
		return false;
	}

	return true;
}
