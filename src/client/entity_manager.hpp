/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_MANAGER_HPP
#define ENTITY_MANAGER_HPP

#pragma warning (disable: 4786)

#include "entity.hpp"

#include "connection/server_message_handler.hpp"

#include "network/basictypes.hpp"

#include <map>
#include <set>
#include <string>
#include <vector>

class MemoryOStream;
class ServerConnection;
class SubSpace;
class ChunkSpace;

typedef std::map< EntityID, Entity * >	Entities;
typedef SmartPointer<ChunkSpace> ChunkSpacePtr;



/**
 *	This class stores all the entities that exist on the client,
 *	and controls which ones should be displayed in the world.
 */
class EntityManager : public ServerMessageHandler
{
public:
	EntityManager();
	~EntityManager();

	static EntityManager & instance();

	void connected( ServerConnection & server );
	ServerConnection * pServer()			{ return pServer_; }
	void disconnected();
	void fini();

	virtual void onBasePlayerCreate( EntityID id, EntityTypeID type,
		BinaryIStream & data );

	virtual void onCellPlayerCreate( EntityID id,
		SpaceID spaceID, EntityID vehicleID, const Position3D & position,
		float yaw, float pitch, float roll, BinaryIStream & data );

	virtual void onEntityControl( EntityID id, bool control );

	virtual void onEntityCreate( EntityID id, EntityTypeID type,
		SpaceID spaceID, EntityID vehicleID, const Position3D & position,
		float yaw, float pitch, float roll, BinaryIStream & data );

	virtual void onEntityProperties( EntityID id, BinaryIStream & data );

	virtual void onEntityEnter( EntityID id,
		SpaceID spaceID, EntityID vehicleID );
	virtual void onEntityLeave( EntityID id,
		const CacheStamps & stamps = CacheStamps() );

	virtual void onEntityProperty( EntityID id, int messageID,
		BinaryIStream & data );
	virtual void onEntityMethod( EntityID id, int messageID,
		BinaryIStream & data );

	virtual void onEntityMoveWithError( EntityID id,
		SpaceID spaceID, EntityID vehicleID,
		const Position3D & pos, const Vector3 & posError,
		float yaw, float pitch, float roll, bool isVolatile );

	virtual void onEntitiesReset( bool keepPlayerAround );
	void clearAllEntities( bool keepPlayerAround, bool keepClientOnly = false );

	virtual void spaceData( SpaceID spaceID, SpaceEntryID entryID,
		uint16 key, const std::string & data );

	virtual void spaceGone( SpaceID spaceID );

	virtual void onVoiceData( const Mercury::Address & srcAddr,
		BinaryIStream & data );

	virtual void onStreamComplete( uint16 proxyDataID, const std::string &desc,
		BinaryIStream & data );

	virtual void onRestoreClient( EntityID id,
		SpaceID spaceID, EntityID vehicleID,
		const Position3D & pos, const Direction3D & dir,
		BinaryIStream & data );

	void tick( double timeNow, double timeLast );

	EntityID	proxyEntityID()				{ return proxyEntityID_; }

	bool displayIDs() const					{ return displayIDs_; }
	void displayIDs( bool data ) 			{ displayIDs_ = data; }

	Entity * getEntity( EntityID id, bool lookInCache = false );

	Entities& entities() 					{ return enteredEntities_; }
	const Entities & cachedEntities() const { return cachedEntities_; }

	static const EntityID FIRST_CLIENT_ID = (1L << 30) + 1;
	static EntityID nextClientID()				{ return nextClientID_++; }
	static bool isClientOnlyID( EntityID id )	{ return id >= FIRST_CLIENT_ID; }

	double lastMessageTime() const;
	void lastMessageTime( double t );

	void record();
	void playback();

	void gatherInput();

	PyObject * entityIsDestroyedExceptionType();

private:
	bool forwardQueuedMessages( Entity * pEntity );
	void clearQueuedMessages( EntityID id );
	void setTimeInt( ChunkSpacePtr pSpace, GameTime gameTime,
		float initialTimeOfDay, float gameSecondsPerSecond );


	ServerConnection	* pServer_;
	EntityID	proxyEntityID_;

	Entities	enteredEntities_;
	Entities	cachedEntities_;

	PyObjectPtr entityIsDestroyedExceptionType_;

	class UnknownEntity
	{
	public:
		UnknownEntity() :
			count( 0 ),
			time(-1000),
			spaceID(0),
			vehicleID(0),
			pos(0,0,0)
		{
			auxFilter[0] = 0;
			auxFilter[1] = 0;
			auxFilter[2] = 0;
		}

		int			count;

		double		time;
		SpaceID		spaceID;
		EntityID	vehicleID;
		Position3D	pos;
		float		auxFilter[3];

		// something for the queue of messages
	};

	typedef std::map<EntityID,UnknownEntity>	Unknowns;
	Unknowns	unknownEntities_;

	typedef std::vector< std::pair< uint8, MemoryOStream * > > MessageQueue;
	typedef std::map< EntityID, MessageQueue > MessageMap;
	MessageMap	queuedMessages_;

	std::vector< EntityPtr >	prereqEntities_;
	std::vector< EntityPtr >	passengerEntities_;

	bool		displayIDs_;

	static EntityID	nextClientID_;

	double		playbackLastMessageTime_;

	typedef std::set< ChunkSpacePtr > ChunkSpaceSet;
	ChunkSpaceSet seenSpaces_;

public:
	static SubSpace		* defaultEntitySubSpace_;
};

#endif // ENTITY_MANAGER_HPP
