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
	#undef CLIENT_INTERFACE_HPP
#endif


#ifndef CLIENT_INTERFACE_HPP
#define CLIENT_INTERFACE_HPP

#include "network/interface_macros.hpp"
#include "network/msgtypes.hpp"
#include "network/network_interface.hpp"

// -----------------------------------------------------------------------------
// Section: Helper macros
// -----------------------------------------------------------------------------


#define MF_BEGIN_CLIENT_MSG( NAME )											\
	BEGIN_HANDLED_STRUCT_MESSAGE( NAME,												\
			ClientMessageHandler< ClientInterface::NAME##Args >,			\
			&ServerConnection::NAME )										\

#define MF_VARLEN_CLIENT_MSG( NAME )										\
	MERCURY_HANDLED_VARIABLE_MESSAGE( NAME, 2,								\
			ClientVarLenMessageHandler,	&ServerConnection::NAME )

#define MF_VARLEN_WITH_ADDR_CLIENT_MSG( NAME )								\
	MERCURY_HANDLED_VARIABLE_MESSAGE( NAME, 2,								\
			ClientVarLenWithAddrMessageHandler,	&ServerConnection::NAME )



// -----------------------------------------------------------------------------
// Section: Client interface
// -----------------------------------------------------------------------------

#pragma pack(push, 1)
BEGIN_MERCURY_INTERFACE( ClientInterface )

	MF_BEGIN_CLIENT_MSG( authenticate )
		uint32				key;
	END_STRUCT_MESSAGE()
	MERCURY_ISTREAM( authenticate, x.key )
	MERCURY_OSTREAM( authenticate, x.key )

	MF_BEGIN_CLIENT_MSG( bandwidthNotification )
		uint32				bps;
	END_STRUCT_MESSAGE()
	MERCURY_ISTREAM( bandwidthNotification, x.bps )
	MERCURY_OSTREAM( bandwidthNotification, x.bps )

	MF_BEGIN_CLIENT_MSG( updateFrequencyNotification )
		uint8				hertz;
	END_STRUCT_MESSAGE()
	MERCURY_ISTREAM( updateFrequencyNotification, x.hertz )
	MERCURY_OSTREAM( updateFrequencyNotification, x.hertz )

	MF_BEGIN_CLIENT_MSG( setGameTime )
		GameTime			gameTime;
	END_STRUCT_MESSAGE()
	MERCURY_ISTREAM( setGameTime, x.gameTime )
	MERCURY_OSTREAM( setGameTime, x.gameTime )

	MF_BEGIN_CLIENT_MSG( resetEntities )
		bool			keepPlayerOnBase;
	END_STRUCT_MESSAGE()
	MERCURY_ISTREAM( resetEntities, x.keepPlayerOnBase )
	MERCURY_OSTREAM( resetEntities, x.keepPlayerOnBase )

	MF_VARLEN_CLIENT_MSG( createBasePlayer )
	MF_VARLEN_CLIENT_MSG( createCellPlayer )

	MF_VARLEN_CLIENT_MSG( spaceData )
	//	EntityID		spaceID
	//	SpaceEntryID	entryID
	//	uint16			key;
	//	char[]			value;		// rest of message

	MF_VARLEN_CLIENT_MSG( createEntity )
	MF_VARLEN_CLIENT_MSG( updateEntity )

	MF_BEGIN_CLIENT_MSG( enterAoI )
		EntityID			id;
		IDAlias				idAlias;
	END_STRUCT_MESSAGE()
	MERCURY_ISTREAM( enterAoI, x.id >> x.idAlias )
	MERCURY_OSTREAM( enterAoI, x.id << x.idAlias )

	MF_BEGIN_CLIENT_MSG( enterAoIOnVehicle )
		EntityID			id;
		EntityID			vehicleID;
		IDAlias				idAlias;
	END_STRUCT_MESSAGE()
	MERCURY_ISTREAM( enterAoIOnVehicle,
		x.id >> x.vehicleID >> x.idAlias )
	MERCURY_OSTREAM( enterAoIOnVehicle,
		x.id << x.vehicleID << x.idAlias )

	MF_VARLEN_CLIENT_MSG( leaveAoI )
	//	EntityID		id;
	//	EventNumber[]	lastEventNumbers;	// rest


	// The interface that is shared with the base app.
#define MF_BEGIN_COMMON_UNRELIABLE_MSG MF_BEGIN_CLIENT_MSG
#define MF_BEGIN_COMMON_PASSENGER_MSG MF_BEGIN_CLIENT_MSG
#define MF_BEGIN_COMMON_RELIABLE_MSG MF_BEGIN_CLIENT_MSG
#include "common_client_interface.hpp"

	// This message is used to send an accurate position of an entity down to
	// the client. It is usually sent when the volatile information of an entity
	// becomes less volatile.
	MF_BEGIN_CLIENT_MSG( detailedPosition )
		EntityID		id;
		Vector3			position;
		Direction3D		direction;
	END_STRUCT_MESSAGE()
	MERCURY_ISTREAM( detailedPosition, x.id >> x.position >> x.direction )
	MERCURY_OSTREAM( detailedPosition, x.id << x.position << x.direction )

	// This is used to send a physics correction or other server-set position
	// to the client for an entity that it is controlling
	MF_BEGIN_CLIENT_MSG( forcedPosition )
		EntityID		id;
		SpaceID			spaceID;
		EntityID		vehicleID;
		Position3D		position;
		Direction3D		direction;
	END_STRUCT_MESSAGE()
	MERCURY_ISTREAM( forcedPosition,
		x.id >> x.spaceID >> x.vehicleID >> x.position >> x.direction )
	MERCURY_OSTREAM( forcedPosition,
		x.id << x.spaceID << x.vehicleID << x.position << x.direction )

	MF_BEGIN_CLIENT_MSG( controlEntity )
		EntityID		id;
		bool			on;
	END_STRUCT_MESSAGE()
	MERCURY_ISTREAM( controlEntity, x.id >> x.on )
	MERCURY_OSTREAM( controlEntity, x.id << x.on )


	MF_VARLEN_WITH_ADDR_CLIENT_MSG( voiceData )

	MF_VARLEN_CLIENT_MSG( restoreClient )
	MF_VARLEN_CLIENT_MSG( switchBaseApp )

	MF_VARLEN_CLIENT_MSG( resourceHeader )

	MF_VARLEN_CLIENT_MSG( resourceFragment )
	// ResourceFragmentArgs
#ifndef CLIENT_INTERFACE_HPP_ONCE
	struct ResourceFragmentArgs {
		uint16			rid;
		uint8			seq;
		uint8			flags; };	// 1 = first (method in seq), 2 = final
	//	uint8			data[];		// rest
#endif

	MF_BEGIN_CLIENT_MSG( loggedOff )
		uint8	reason;
	END_STRUCT_MESSAGE()
	MERCURY_ISTREAM( loggedOff, x.reason )
	MERCURY_OSTREAM( loggedOff, x.reason )

	// 128 to 254 are messages destined for our entities.
	// They all look like this:
	MERCURY_VARIABLE_MESSAGE( entityMessage, 2, NULL )

END_MERCURY_INTERFACE()
#pragma pack( pop )

#define CLIENT_INTERFACE_HPP_ONCE

#endif // CLIENT_INTERFACE_HPP
