/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#if defined(DEFINE_INTERFACE_HERE) || defined(DEFINE_SERVER_HERE)
	#undef CELLAPP_INTERFACE_HPP
	#define SECOND_PASS
#endif


#ifndef CELLAPP_INTERFACE_HPP
#define CELLAPP_INTERFACE_HPP


#include "entitydef/volatile_info.hpp"

#undef INTERFACE_NAME
#define INTERFACE_NAME CellAppInterface
#include "network/common_interface_macros.hpp"
#include "network/msgtypes.hpp"

#include "server/common.hpp"
#include "server/anonymous_channel_client.hpp"

typedef uint16 NumTimesRealOffloadedType;
typedef uint8 AoIUpdateSchemeID;

// -----------------------------------------------------------------------------
// Section: Helper macros
// -----------------------------------------------------------------------------

#ifndef SECOND_PASS
enum EntityReality
{
	GHOST_ONLY,
	REAL_ONLY,
	WITNESS_ONLY
};

#endif


// For Cell referenced by Entity
#define MF_CREATE_ENTITY_MSG( NAME, METHOD_NAME ) 							\
	MERCURY_HANDLED_VARIABLE_MESSAGE( NAME, 2,								\
			CreateEntityNearEntityHandler, &Cell::METHOD_NAME )

// For Entity
#define MF_BEGIN_ENTITY_MSG( NAME, IS_REAL_ONLY ) 							\
	BEGIN_HANDLED_PREFIXED_MESSAGE( NAME, EntityID,							\
			CellEntityMessageHandler< CellAppInterface::NAME##Args >, 		\
			std::make_pair( &Entity::NAME, IS_REAL_ONLY ) )

#define MF_VARLEN_ENTITY_MSG( NAME, IS_REAL_ONLY )							\
	MERCURY_HANDLED_VARIABLE_MESSAGE( NAME, 2, EntityVarLenMessageHandler,	\
			std::make_pair( &Entity::NAME, IS_REAL_ONLY) )

#define MF_RAW_VARLEN_ENTITY_MSG( NAME, IS_REAL_ONLY )						\
	MERCURY_HANDLED_VARIABLE_MESSAGE( NAME, 2, 								\
			RawEntityVarLenMessageHandler,									\
			std::make_pair( &Entity::NAME, IS_REAL_ONLY) )

#define MF_VARLEN_ENTITY_REQUEST( NAME, IS_REAL_ONLY )						\
	MERCURY_HANDLED_VARIABLE_MESSAGE( NAME, 2, EntityVarLenRequestHandler,	\
			std::make_pair( &Entity::NAME, IS_REAL_ONLY) )



// -----------------------------------------------------------------------------
// Section: Cell interface
// -----------------------------------------------------------------------------

#pragma pack( push, 1 )
BEGIN_MERCURY_INTERFACE( CellAppInterface )

	BW_ANONYMOUS_CHANNEL_CLIENT_MSG( DBInterface )

	// -------------------------------------------------------------------------
	// CellApp messages
	// -------------------------------------------------------------------------
	BW_STREAM_MSG_EX( CellApp, addCell )
		// SpaceID spaceID;

	BW_BEGIN_STRUCT_MSG( CellApp, startup )
		Mercury::Address baseAppAddr;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( CellApp, setGameTime )
		GameTime		gameTime;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( CellApp, handleCellAppMgrBirth )
		Mercury::Address addr;
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( CellApp, handleCellAppDeath )
		Mercury::Address addr;
	END_STRUCT_MESSAGE()

	BW_STREAM_MSG( CellApp, handleBaseAppDeath )

	BW_BEGIN_STRUCT_MSG( CellApp, shutDown )
		bool isSigInt; // Not used.
	END_STRUCT_MESSAGE()

	BW_BEGIN_STRUCT_MSG( CellApp, controlledShutDown )
		ShutDownStage	stage;
		GameTime		shutDownTime;
	END_STRUCT_MESSAGE()

	MERCURY_HANDLED_VARIABLE_MESSAGE( sendEntityPositions, 2, \
			EntityPositionSender, NULL )

	BW_STREAM_MSG( CellApp, setSharedData );

	BW_STREAM_MSG( CellApp, delSharedData );

	BW_BEGIN_STRUCT_MSG( CellApp, setBaseApp )
		Mercury::Address baseAppAddr;
	END_STRUCT_MESSAGE()

	BW_STREAM_MSG_EX( CellApp, onloadTeleportedEntity )

	BW_BEGIN_STRUCT_MSG( CellApp, cellAppMgrInfo )
		float maxCellAppLoad;
	END_STRUCT_MESSAGE()

	// -------------------------------------------------------------------------
	// Space messages
	// -------------------------------------------------------------------------

	// The arguments are as follows:
	//  EntityID			The id of the entity
	//  Position3D			The position of the entity
	//  EntityTypeID		The type of the entity
	//  Mercury::Address	The address of the entity's owner
	//  Variable script state data
	// MF_CREATE_GHOST_MSG( createGhost )
	BW_STREAM_MSG_EX( Space, createGhost )

	BW_STREAM_MSG( Space, spaceData )
	//	SpaceEntryID	entryID
	//	uint16			key;
	//	char[]			value;		// rest of message

	BW_STREAM_MSG( Space, allSpaceData )
	//	int				numEntries;
	//	numEntries of:
	//	SpaceEntryID	entryID;
	//	uint16			key;
	//	std::string		value;

	BW_STREAM_MSG( Space, updateGeometry )

	#define SPACE_GEOMETRY_LOADED_BOOTSTRAP_FLAG	0x01
	BW_STREAM_MSG( Space, spaceGeometryLoaded )
	//	uint8 flags;
	//	std::string lastGeometryPath

	BW_STREAM_MSG( Space, shutDownSpace )

	// -------------------------------------------------------------------------
	// Cell messages
	// -------------------------------------------------------------------------

	// Entity creation

	// The arguments are as follows:
	//	ChannelVersion 	The channel version
	//	bool			IsRestore flag
	// 	EntityID		The id for the new entity
	// 	Position3D		The position of the new entity
	//	bool			IsOnGround flag
	// 	EntityTypeID	The type for the new entity
	// 	Variable script initialisation data
	// 	Variable real entity data
	BW_STREAM_MSG_EX( Cell, createEntity )
	MF_CREATE_ENTITY_MSG( createEntityNearEntity, createEntity )

	// Miscellaneous
	BW_STREAM_MSG( Cell, shouldOffload )

	// Called from CellAppMgr
	BW_STREAM_MSG( Cell, retireCell )
	BW_STREAM_MSG( Cell, removeCell )

	BW_STREAM_MSG( Cell, notifyOfCellRemoval )

	BW_STREAM_MSG_EX( Cell, ackCellRemoval )

	// -------------------------------------------------------------------------
	// Entity messages
	// -------------------------------------------------------------------------

	// Destined for real entity only

	// Fast-track avatar update
	MF_BEGIN_ENTITY_MSG( avatarUpdateImplicit, REAL_ONLY )
		Coord			pos;
		YawPitchRoll	dir;
		uint8			refNum;
	END_STRUCT_MESSAGE()

	// Brisk-track avatar update
	MF_BEGIN_ENTITY_MSG( avatarUpdateExplicit, REAL_ONLY )
		SpaceID			spaceID;
		EntityID		vehicleID;
		Coord			pos;
		YawPitchRoll	dir;
		bool			onGround;
		uint8			refNum;
	END_STRUCT_MESSAGE()

	MF_BEGIN_ENTITY_MSG( ackPhysicsCorrection, REAL_ONLY )
	END_STRUCT_MESSAGE()

	MF_VARLEN_ENTITY_REQUEST( enableWitness, REAL_ONLY )

	MF_BEGIN_ENTITY_MSG( witnessCapacity, WITNESS_ONLY )
		EntityID			witness;
		uint32				bps;
	END_STRUCT_MESSAGE()

	// requestEntityUpdate:
	//	EntityID	id;
	//	Array of event numbers;
	MF_VARLEN_ENTITY_MSG( requestEntityUpdate, WITNESS_ONLY )

	// This is used by ghost entities to inform the real entity that it is being
	// witnessed.
	MF_BEGIN_ENTITY_MSG( witnessed, REAL_ONLY )
	END_STRUCT_MESSAGE()

	MF_VARLEN_ENTITY_REQUEST( writeToDBRequest, REAL_ONLY )

	MF_BEGIN_ENTITY_MSG( destroyEntity, REAL_ONLY )
		int					flags; // Currently not used.
	END_STRUCT_MESSAGE()

	// Destined for ghost entity only
	MF_RAW_VARLEN_ENTITY_MSG( onload, GHOST_ONLY )

	MF_BEGIN_ENTITY_MSG( ghostAvatarUpdate, GHOST_ONLY )
		Coord				pos;
		YawPitchRoll		dir;
		bool				isOnGround;
		VolatileNumber		updateNumber;
	END_STRUCT_MESSAGE()

	MF_VARLEN_ENTITY_MSG( ghostHistoryEvent, GHOST_ONLY )

	MF_BEGIN_ENTITY_MSG( ghostSetReal, GHOST_ONLY )
		NumTimesRealOffloadedType numTimesRealOffloaded;
		Mercury::Address	owner;
	END_STRUCT_MESSAGE()

	MF_BEGIN_ENTITY_MSG( ghostSetNextReal, GHOST_ONLY )
		Mercury::Address	nextRealAddr;
	END_STRUCT_MESSAGE()

	MF_BEGIN_ENTITY_MSG( delGhost, GHOST_ONLY )
	END_STRUCT_MESSAGE()

	MF_BEGIN_ENTITY_MSG( ghostVolatileInfo, GHOST_ONLY )
		VolatileInfo	volatileInfo;
	END_STRUCT_MESSAGE()

	MF_VARLEN_ENTITY_MSG( ghostControllerCreate, GHOST_ONLY )
	MF_VARLEN_ENTITY_MSG( ghostControllerDelete, GHOST_ONLY )
	MF_VARLEN_ENTITY_MSG( ghostControllerUpdate, GHOST_ONLY )

	MF_VARLEN_ENTITY_MSG( ghostedDataUpdate, GHOST_ONLY )
		// EventNumber (int32) eventNumber
		// data for ghostDataUpdate

	// The real entity uses this to query whether there are any entities
	// witnessing its ghost entities.
	MF_BEGIN_ENTITY_MSG( checkGhostWitnessed, GHOST_ONLY )
	END_STRUCT_MESSAGE()

	MF_BEGIN_ENTITY_MSG( aoiUpdateSchemeChange, GHOST_ONLY )
		AoIUpdateSchemeID scheme;
	END_STRUCT_MESSAGE()

	// Message to run cell script.
	MF_VARLEN_ENTITY_MSG( runScriptMethod, REAL_ONLY )

	// Message to run base method via a cell mailbox
	MF_VARLEN_ENTITY_MSG( callBaseMethod, REAL_ONLY )

	// Message to run client method via a cell mailbox
	MF_VARLEN_ENTITY_MSG( callClientMethod, REAL_ONLY )

	MF_BEGIN_ENTITY_MSG( delControlledBy, REAL_ONLY )
		EntityID deadController;
	END_STRUCT_MESSAGE()

	// CellApp's EntityChannelFinder uses this to forward base entity packets
	// from the ghost to the real
	MF_VARLEN_ENTITY_MSG( forwardedBaseEntityPacket, REAL_ONLY )

	MF_RAW_VARLEN_ENTITY_MSG( onBaseOffloaded, REAL_ONLY )

	MF_BEGIN_ENTITY_MSG( teleport, REAL_ONLY )
		EntityMailBoxRef dstMailBoxRef;
	END_STRUCT_MESSAGE()

	// 128 to 254 are messages destined for our entities.
	// They all look like this:
	MERCURY_VARIABLE_MESSAGE( runExposedMethod, 2, NULL )


	// -------------------------------------------------------------------------
	// Watcher messages
	// -------------------------------------------------------------------------

	// Message to forward watcher requests via
	BW_STREAM_MSG_EX( CellApp, callWatcher )


END_MERCURY_INTERFACE()

#pragma pack( pop )

#undef SECOND_PASS

#endif // CELLAPP_INTERFACE_HPP
