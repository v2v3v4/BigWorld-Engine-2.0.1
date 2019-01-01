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
	#undef BASE_APP_MGR_INTERFACE_HPP
#endif

#ifndef BASE_APP_MGR_INTERFACE_HPP
#define BASE_APP_MGR_INTERFACE_HPP

#define BASE_APP_MGR_INTERFACE_HPP_FIRSTTIME

#ifndef BASE_APP_MGR_INTERFACE_HPP_ONCE
#define BASE_APP_MGR_INTERFACE_HPP_ONCE

#include "network/basictypes.hpp"

/**
 * Data to use when initialising a BaseApp.
 */
#pragma pack( push, 1 )
struct BaseAppInitData
{
	int32 id;			//!< ID of the new BaseApp
	GameTime time;		//!< Current game time
	bool isReady;		//!< Flag indicating whether the server is ready
};
#pragma pack( pop )

#else
#undef BASE_APP_MGR_INTERFACE_HPP_FIRSTTIME
#endif // BASE_APP_MGR_INTERFACE_HPP_ONCE

#undef INTERFACE_NAME
#define INTERFACE_NAME BaseAppMgrInterface
#include "network/common_interface_macros.hpp"

// -----------------------------------------------------------------------------
// Section: Includes
// -----------------------------------------------------------------------------

#include "server/common.hpp"
#include "server/anonymous_channel_client.hpp"
#include "server/reviver_subject.hpp"


// -----------------------------------------------------------------------------
// Section: Base App Manager Interface
// -----------------------------------------------------------------------------

BEGIN_MERCURY_INTERFACE( BaseAppMgrInterface )

	BW_ANONYMOUS_CHANNEL_CLIENT_MSG( DBInterface )

#ifdef BASE_APP_MGR_INTERFACE_HPP_FIRSTTIME
	enum CreateEntityError
	{
		CREATE_ENTITY_ERROR_NO_BASEAPPS = 1,
		CREATE_ENTITY_ERROR_BASEAPPS_OVERLOADED
	};
#endif

	BW_STREAM_MSG_EX( BaseAppMgr, createEntity )

	BW_BEGIN_STRUCT_MSG_EX( BaseAppMgr, add )
		Mercury::Address	addrForCells;
		Mercury::Address	addrForClients;
	END_STRUCT_MESSAGE()

	BW_STREAM_MSG_EX( BaseAppMgr, recoverBaseApp )
		// Mercury::Address		addrForCells;
		// Mercury::Address		addrForClients;
		// Mercury::Address		backupAddress;
		// BaseAppID			id;
		// float				maxLoad;
		// string, MailBoxRef	globalBases; (0 to many)

	BW_BEGIN_STRUCT_MSG_EX( BaseAppMgr, del )
		BaseAppID		id;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG_EX( BaseAppMgr, informOfLoad )
		float load;
		int numBases;
		int numProxies;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( BaseAppMgr, shutDown )
		bool		shouldShutDownOthers;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( BaseAppMgr, controlledShutDown )
		ShutDownStage stage;
		GameTime shutDownTime;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( BaseAppMgr, handleBaseAppDeath )
		Mercury::Address addr;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( BaseAppMgr, handleCellAppMgrBirth )
		Mercury::Address addr;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( BaseAppMgr, handleBaseAppMgrBirth )
		Mercury::Address addr;
	END_STRUCT_MESSAGE()

	BW_STREAM_MSG_EX( BaseAppMgr, handleCellAppDeath )

	BW_STREAM_MSG_EX( BaseAppMgr, registerBaseGlobally )
	BW_STREAM_MSG_EX( BaseAppMgr, deregisterBaseGlobally )

	BW_STREAM_MSG_EX( BaseAppMgr, requestHasStarted )

	// Sent by DBMgr to initialise game time etc.
	BW_STREAM_MSG_EX( BaseAppMgr, initData )

	// This is called by the DBMgr when it is ready to start the server.
	BW_STREAM_MSG_EX( BaseAppMgr, startup )

	BW_STREAM_MSG_EX( BaseAppMgr, checkStatus )

	// This is forwarded to the CellAppMgr.
	BW_STREAM_MSG_EX( BaseAppMgr, spaceDataRestore )

	BW_STREAM_MSG( BaseAppMgr, setSharedData )
	BW_STREAM_MSG( BaseAppMgr, delSharedData )

	BW_STREAM_MSG_EX( BaseAppMgr, useNewBackupHash )

	BW_STREAM_MSG_EX( BaseAppMgr, informOfArchiveComplete )

	BW_STREAM_MSG_EX( BaseApp, retireApp )

	BW_STREAM_MSG_EX( BaseAppMgr, requestBackupHashChain )

	MF_REVIVER_PING_MSG()

END_MERCURY_INTERFACE()

#endif // BASE_APP_MGR_INTERFACE_HPP
