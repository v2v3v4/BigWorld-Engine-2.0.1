/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SERVER_MESSAGE_HANDLER_HPP
#define SERVER_MESSAGE_HANDLER_HPP

#include "network/basictypes.hpp"

class BinaryIStream;

#include <string>

/**
 *	This abstract class defines the interface that is called by
 *	ServerConnection::processInput.
 *
 *	@ingroup network
 */
class ServerMessageHandler
{
public:
	/// This method is called to create a new player as far as required to
	/// talk to the base entity. Only data shared between the base and the
	/// client is provided in this method - the cell data will be provided by
	/// onCellPlayerCreate later if the player is put on the cell also.
	virtual void onBasePlayerCreate( EntityID id, EntityTypeID type,
		BinaryIStream & data ) = 0;

	/// This method is called to create a new player as far as required to
	/// talk to the cell entity. Only data shared between the cell and the
	/// client is provided in this method - the base data will have been
	/// previously provided by onBasePlayerCreate.
	virtual void onCellPlayerCreate( EntityID id,
		SpaceID spaceID, EntityID vehicleID, const Position3D & pos,
		float yaw, float pitch, float roll, BinaryIStream & data ) = 0;

	/// This method is called to indicate that the given entity is controlled
	/// by this client, at least as far as pose updates go. i.e. You may
	/// move the entity around with calls to addMove. Currently, control is
	/// implicit for the player entity and may not be withdrawn.
	virtual void onEntityControl( EntityID id, bool control ) { }

	/// This method is called when an entity enters the client's AoI.
	/// If the entity has not been seen before then spaceID need not be
	/// recorded, since it will be provided again in onEntityCreate.
	virtual void onEntityEnter( EntityID id, SpaceID spaceID,
		EntityID vehicleID ) = 0;

	/// This method is called when an entity leaves the client's AoI.
	/// The CacheStamps are a record of the data received about the entity to
	/// this point, that should be reported to the server if the entity is
	/// seen again via the requestEntityUpdate message.
	virtual void onEntityLeave( EntityID id, const CacheStamps & stamps ) = 0;

	/// This method is called in response to a requestEntityUpdate message to
	/// provide the bulk of the information about this entity. If the client has
	/// seen this entity before then data it already has is not resent
	/// (as determined by the CacheStamps sent in requestEntityUpdate).
	virtual void onEntityCreate( EntityID id, EntityTypeID type,
		SpaceID spaceID, EntityID vehicleID, const Position3D & pos,
		float yaw, float pitch, float roll, BinaryIStream & data ) = 0;

	/// This method is called by the server when it wants to provide multiple
	/// properties at once to the client for an entity in its AoI, such as when
	/// a detail level boundary is crossed.
	virtual void onEntityProperties( EntityID id, BinaryIStream & data ) = 0;

	/// This method is called when the server sets a property on an entity.
	virtual void onEntityProperty( EntityID objectID, int messageID,
		BinaryIStream & data ) = 0;

	/// This method is called when the server calls a method on an entity.
	virtual void onEntityMethod( EntityID objectID, int messageID,
		BinaryIStream & data ) = 0;

	/// This method is called when the position of an entity changes.
	/// This will only be received for a controlled entity if the server
	/// overrides the position directly (physics correction, teleport, etc.)
	virtual void onEntityMove( EntityID id, SpaceID spaceID, EntityID vehicleID,
		const Position3D & pos, float yaw, float pitch, float roll,
		bool isVolatile ) {}

	/// This method is called when the position of an entity changes.
	/// This will only be received for a controlled entity if the server
	/// overrides the position directly (physics correction, teleport, etc.)
	/// This method is like onEntityMove but also receives an indication of how
	/// much error could be in the position that is caused by compression.
	virtual void onEntityMoveWithError( EntityID id,
					SpaceID spaceID, EntityID vehicleID,
					const Position3D & pos, const Vector3 & posError,
					float yaw, float pitch, float roll,
					bool isVolatile )
	{
		this->onEntityMove( id, spaceID, vehicleID, pos, yaw, pitch, roll,
				isVolatile );
	}

	/// This method is called when data associated with a space is received.
	virtual void spaceData( SpaceID spaceID, SpaceEntryID entryID,
		uint16 key, const std::string & data ) = 0;

	/// This method is called when the given space is no longer visible
	/// to the client.
	virtual void spaceGone( SpaceID spaceID ) = 0;

	/// This method is called to deliver peer-to-peer data.
	virtual void onVoiceData( const Mercury::Address & srcAddr,
		BinaryIStream & data ) {}

	/// This method is called to deliver downloads when they are complete.  This
	/// was called onProxyData() in BW1.8.x and earlier, and didn't have the
	/// description parameter.
	virtual void onStreamComplete( uint16 id, const std::string &desc,
		BinaryIStream & data ) {}

	/// This method is called when the server tells us to reset all our
	/// entities. The player entity may optionally be saved (but still
	/// should not be considered to be in the world (i.e. no cell part yet))
	virtual void onEntitiesReset( bool keepPlayerOnBase ) {}

	/// This method is called to indicate that the client entity has been
	/// restored from a (recent) backup due to a failure on the server.
	virtual void onRestoreClient( EntityID id,
		SpaceID spaceID, EntityID vehicleID,
		const Position3D & pos, const Direction3D & dir,
		BinaryIStream & data ) {}

	/// This method is called when an enableEntities request was rejected.
	/// Usually it is because the server is in the middle of an update.
	/// The handler should check latest and impending versions and try later.
	/// (also the impending version may need to have been fully downloaded)
	virtual void onEnableEntitiesRejected() {}
};

#endif // SERVER_MESSAGE_HANDLER_HPP
