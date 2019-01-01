/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#if defined( DEFINE_INTERFACE_HERE ) || defined( DEFINE_SERVER_HERE )
	#undef DB_INTERFACE_HPP
#endif

#ifndef DB_INTERFACE_HPP
#define DB_INTERFACE_HPP

// -----------------------------------------------------------------------------
// Section: Includes
// -----------------------------------------------------------------------------
#undef INTERFACE_NAME
#define INTERFACE_NAME DBInterface
#include "network/common_interface_macros.hpp"

#include "server/common.hpp"
#include "server/reviver_subject.hpp"

// -----------------------------------------------------------------------------
// Section: Database Interface
// -----------------------------------------------------------------------------

BEGIN_MERCURY_INTERFACE( DBInterface )

	MF_REVIVER_PING_MSG()

	BW_BEGIN_STRUCT_MSG( Database, handleBaseAppMgrBirth )
		Mercury::Address	addr;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( Database, shutDown )
		// none
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( Database, controlledShutDown )
		ShutDownStage stage;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( Database, cellAppOverloadStatus )
		bool anyOverloaded;
	END_STRUCT_MESSAGE()


	BW_STREAM_MSG_EX( Database, logOn )
		// std::string logOnName
		// std::string password
		// Mercury::Address addrForProxy
		// bool offChannel
		// MD5::Digest digest

	BW_STREAM_MSG_EX( Database, loadEntity )
		// EntityTypeID	entityTypeID;
		// EntityID entityID;
		// bool nameNotID;
		// nameNotID ? (std::string name) : (DatabaseID id );

	BW_BIG_STREAM_MSG_EX( Database, writeEntity )
		// int8 flags; (cell? base? log off?)
		// EntityTypeID entityTypeID;
		// DatabaseID	databaseID;
		// properties

	BW_BEGIN_STRUCT_MSG_EX( Database, deleteEntity )
		EntityTypeID	entityTypeID;
		DatabaseID		dbid;
	END_STRUCT_MESSAGE()

	BW_STREAM_MSG_EX( Database, deleteEntityByName )
		// EntityTypeID	entityTypeID;
		// std::string name;

	BW_BEGIN_STRUCT_MSG_EX( Database, lookupEntity )
		EntityTypeID	entityTypeID;
		DatabaseID		dbid;
		bool			offChannel;
	END_STRUCT_MESSAGE()

	BW_STREAM_MSG_EX( Database, lookupEntityByName )
		// EntityTypeID		entityTypeID;
		// std::string 		name;
		// bool				offChannel;

	BW_STREAM_MSG_EX( Database, lookupDBIDByName )
		// EntityTypeID	entityTypeID;
		// std::string name;

	BW_BIG_STREAM_MSG_EX( Database, executeRawCommand )
		// char[] command;

	BW_STREAM_MSG_EX( Database, putIDs )
		// EntityID ids[];

	BW_STREAM_MSG_EX( Database, getIDs )
		// int numIDs;

	BW_BIG_STREAM_MSG_EX( Database, writeSpaces )

	BW_BEGIN_STRUCT_MSG( Database, writeGameTime )
		GameTime gameTime;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( Database, handleDatabaseBirth )
		Mercury::Address	addr;
	END_STRUCT_MESSAGE()

	BW_STREAM_MSG_EX( Database, handleBaseAppDeath )

	BW_STREAM_MSG_EX( Database, checkStatus )

	BW_STREAM_MSG_EX( Database, secondaryDBRegistration );
	BW_STREAM_MSG_EX( Database, updateSecondaryDBs );
	BW_STREAM_MSG_EX( Database, getSecondaryDBDetails );

END_MERCURY_INTERFACE()

#endif // DB_INTERFACE_HPP
