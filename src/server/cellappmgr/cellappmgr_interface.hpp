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
	#undef CELL_APP_MGR_INTERFACE_HPP
#endif

#ifndef CELL_APP_MGR_INTERFACE_HPP
#define CELL_APP_MGR_INTERFACE_HPP


#include "network/basictypes.hpp"

#undef INTERFACE_NAME
#define INTERFACE_NAME CellAppMgrInterface
#include "network/common_interface_macros.hpp"

#include "server/anonymous_channel_client.hpp"
#include "server/common.hpp"
#include "server/reviver_subject.hpp"

// -----------------------------------------------------------------------------
// Section: Cell App Manager interface
// -----------------------------------------------------------------------------

#pragma pack(push, 1)
BEGIN_MERCURY_INTERFACE( CellAppMgrInterface )

	BW_ANONYMOUS_CHANNEL_CLIENT_MSG( DBInterface )

	// The arguments are the same as for Cell::createEntity.
	// It assumes that the first two arguments are:
	// 	EntityID		- The id of the new entity
	// 	Position3D		- The position of the new entity
	//
	BW_STREAM_MSG_EX( CellAppMgr, createEntity )
	BW_STREAM_MSG_EX( CellAppMgr, createEntityInNewSpace )

	BW_STREAM_MSG_EX( CellAppMgr, prepareForRestoreFromDB )

	BW_STREAM_MSG_EX( CellAppMgr, startup )

	BW_BEGIN_STRUCT_MSG( CellAppMgr, shutDown )
		bool isSigInt;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( CellAppMgr, controlledShutDown )
		ShutDownStage stage;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( CellAppMgr, shouldOffload )
		bool enable;
	END_STRUCT_MESSAGE()

	BW_STREAM_MSG_EX( CellAppMgr, addApp );

	BW_STREAM_MSG( CellAppMgr, recoverCellApp );

	BW_BEGIN_STRUCT_MSG( CellAppMgr, delApp )
		Mercury::Address addr;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( CellAppMgr, setBaseApp )
		Mercury::Address addr;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( CellAppMgr, handleCellAppMgrBirth )
		Mercury::Address addr;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( CellAppMgr, handleBaseAppMgrBirth )
		Mercury::Address addr;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG_EX( CellAppMgr, handleCellAppDeath )
		Mercury::Address addr;
	END_STRUCT_MESSAGE()

	BW_STREAM_MSG( CellAppMgr, handleBaseAppDeath )

	BW_BEGIN_STRUCT_MSG_EX( CellAppMgr, ackCellAppDeath )
		Mercury::Address deadAddr;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG_EX( CellAppMgr, gameTimeReading )
		double				gameTimeReadingContribution;
	END_STRUCT_MESSAGE()	// double is good for ~100 000 years

	// These could be a space messages
	BW_STREAM_MSG_EX( CellAppMgr, updateSpaceData )

	BW_BEGIN_STRUCT_MSG( CellAppMgr, shutDownSpace )
		SpaceID spaceID;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( CellAppMgr, ackBaseAppsShutDown )
		ShutDownStage stage;
	END_STRUCT_MESSAGE()

	BW_STREAM_MSG_EX( CellAppMgr, checkStatus )

	// ---- Cell App messages ----
	BW_BEGIN_STRUCT_MSG( CellApp, informOfLoad )
		float load;
		int	numEntities;
	END_STRUCT_MESSAGE()

	BW_STREAM_MSG( CellApp, updateBounds );

	BW_BEGIN_STRUCT_MSG( CellApp, retireApp )
		int8 dummy;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( CellApp, ackCellAppShutDown )
		ShutDownStage stage;
	END_STRUCT_MESSAGE()

	MF_REVIVER_PING_MSG()

	BW_STREAM_MSG( CellAppMgr, setSharedData )
	BW_STREAM_MSG( CellAppMgr, delSharedData )

END_MERCURY_INTERFACE()
#pragma pack(pop)

#endif // CELL_APP_MGR_INTERFACE_HPP
