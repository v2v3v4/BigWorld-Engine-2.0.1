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
	#undef BASEAPP_EXT_INTERFACE_HPP
#endif

#ifndef BASEAPP_EXT_INTERFACE_HPP
#define BASEAPP_EXT_INTERFACE_HPP

#undef INTERFACE_NAME
#define INTERFACE_NAME BaseAppExtInterface
#include "common_baseapp_interface.hpp"


// -----------------------------------------------------------------------------
// Section: Includes
// -----------------------------------------------------------------------------

#include "network/msgtypes.hpp"

// -----------------------------------------------------------------------------
// Section: BaseApp External Interface
// -----------------------------------------------------------------------------

#pragma pack( push, 1 )
BEGIN_MERCURY_INTERFACE( BaseAppExtInterface )

	// let the proxy know who we really are
	BW_STREAM_MSG_EX( BaseApp, baseAppLogin )

	// let the proxy know who we really are
	BW_BEGIN_STRUCT_MSG_EX( BaseApp, authenticate )
		SessionKey		key;
	END_STRUCT_MESSAGE()
	MERCURY_ISTREAM( authenticate, x.key )
	MERCURY_OSTREAM( authenticate, x.key )

	// send an update for our position. seqNum is used to refer to this position
	// later as the base for relative positions.
	MF_BEGIN_BLOCKABLE_PROXY_MSG( avatarUpdateImplicit )
		Coord			pos;
		YawPitchRoll	dir;
		uint8			refNum;
	END_STRUCT_MESSAGE();
	MERCURY_ISTREAM( avatarUpdateImplicit, x.pos >> x.dir >> x.refNum )
	MERCURY_OSTREAM( avatarUpdateImplicit, x.pos << x.dir << x.refNum )

	MF_BEGIN_BLOCKABLE_PROXY_MSG( avatarUpdateExplicit )
		SpaceID			spaceID;
		EntityID		vehicleID;
		Coord			pos;
		YawPitchRoll	dir;
		bool			onGround;
		uint8			refNum;
	END_STRUCT_MESSAGE();
	MERCURY_ISTREAM( avatarUpdateExplicit,
		x.spaceID >> x.vehicleID >> x.pos >> x.dir >> x.onGround >> x.refNum );
	MERCURY_OSTREAM( avatarUpdateExplicit,
		x.spaceID << x.vehicleID << x.pos << x.dir << x.onGround << x.refNum );

	MF_BEGIN_BLOCKABLE_PROXY_MSG( avatarUpdateWardImplicit )
		EntityID		ward;
		Coord			pos;
		YawPitchRoll	dir;
	END_STRUCT_MESSAGE();
	MERCURY_ISTREAM( avatarUpdateWardImplicit, x.ward >> x.pos >> x.dir )
	MERCURY_OSTREAM( avatarUpdateWardImplicit, x.ward << x.pos << x.dir )

	MF_BEGIN_BLOCKABLE_PROXY_MSG( avatarUpdateWardExplicit )
		EntityID		ward;
		SpaceID			spaceID;
		EntityID		vehicleID;
		Coord			pos;
		YawPitchRoll	dir;
		bool			onGround;
	END_STRUCT_MESSAGE();
	MERCURY_ISTREAM( avatarUpdateWardExplicit,
		x.ward >> x.spaceID >> x.vehicleID >> x.pos >> x.dir >> x.onGround );
	MERCURY_OSTREAM( avatarUpdateWardExplicit,
		x.ward << x.spaceID << x.vehicleID << x.pos << x.dir << x.onGround );

	MF_BEGIN_BLOCKABLE_PROXY_MSG( ackPhysicsCorrection )
		uint8 dummy;
	END_STRUCT_MESSAGE()
	MERCURY_ISTREAM( ackPhysicsCorrection, x.dummy )
	MERCURY_OSTREAM( ackPhysicsCorrection, x.dummy )

	MF_BEGIN_BLOCKABLE_PROXY_MSG( ackWardPhysicsCorrection )
		EntityID ward;
		uint8 dummy;
	END_STRUCT_MESSAGE()
	MERCURY_ISTREAM( ackWardPhysicsCorrection, x.ward >> x.dummy )
	MERCURY_OSTREAM( ackWardPhysicsCorrection, x.ward << x.dummy )

	// TODO: I'm pretty sure this is not used. We should remove it.
	// forward all further messages (over 128) in this bundle to the cell
	MERCURY_FIXED_MESSAGE( switchInterface, 0, NULL )

	// requestEntityUpdate:
	MF_VARLEN_BLOCKABLE_PROXY_MSG( requestEntityUpdate )
	//	EntityID		id;
	//	EventNumber[]	lastEventNumbers;

	MF_BEGIN_BLOCKABLE_PROXY_MSG( enableEntities )
		uint8				dummy;
	END_STRUCT_MESSAGE();
	MERCURY_ISTREAM( enableEntities, x.dummy )
	MERCURY_OSTREAM( enableEntities, x.dummy )

	MF_BEGIN_UNBLOCKABLE_PROXY_MSG( restoreClientAck )
		int					id;
	END_STRUCT_MESSAGE();
	MERCURY_ISTREAM( restoreClientAck, x.id )
	MERCURY_OSTREAM( restoreClientAck, x.id )

	MF_BEGIN_UNBLOCKABLE_PROXY_MSG( disconnectClient )
		uint8 reason;
	END_STRUCT_MESSAGE()
	MERCURY_ISTREAM( disconnectClient, x.reason )
	MERCURY_OSTREAM( disconnectClient, x.reason )

	// 128 to 254 are messages destined either for our entities
	// or for the cell's. They all look like this:
	MERCURY_VARIABLE_MESSAGE( entityMessage, 2, NULL )

END_MERCURY_INTERFACE()

#pragma pack( pop )

#endif // BASEAPP_EXT_INTERFACE_HPP
