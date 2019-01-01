/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

//#pragma warning (disable: 4503)
#pragma warning (disable: 4786)

#include "entity_manager.hpp"

#include "app.hpp"
#include "filter.hpp"
#include "player.hpp"

#include "common/space_data_types.hpp"

#include "camera/annal.hpp"

#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"

#include "connection/server_connection.hpp"

#include "cstdmf/binaryfile.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/memory_stream.hpp"

#include "network/basictypes.hpp"

#include "pyscript/personality.hpp"

#include "romp/time_of_day.hpp"

DECLARE_DEBUG_COMPONENT2( "Entity", 0 );


const uint16 SPACE_DATA_WEATHER = 8192;

#if ENABLE_ENVIRONMENT_SYNC

// flag indicating environmental sync state of demo client
static bool environmentSync = true;

void setEnvironmentSync( bool sync )
{
	environmentSync = sync;
}

PY_AUTO_MODULE_FUNCTION( RETVOID, setEnvironmentSync, ARG( bool, END ), BigWorld )

bool getEnvironmentSync()
{
	return environmentSync;
}

PY_AUTO_MODULE_FUNCTION( RETDATA, getEnvironmentSync, END, BigWorld )

#endif


// -----------------------------------------------------------------------------
// Section: EntityManager
// -----------------------------------------------------------------------------

EntityID EntityManager::nextClientID_ = EntityManager::FIRST_CLIENT_ID;

/**
 *	 Default constructor.
 */
EntityManager::EntityManager() :
	pServer_( NULL ),
	proxyEntityID_( 0 ),
	displayIDs_ ( false ),
	playbackLastMessageTime_( 0 )
{
	Player::init();

	PyObject * bigworldModule = PyImport_AddModule( "BigWorld" );
	entityIsDestroyedExceptionType_ = PyObjectPtr( PyErr_NewException( const_cast<char *>("BigWorld.EntityIsDestroyedException"), NULL, NULL ), true );

	Py_XINCREF(entityIsDestroyedExceptionType_.get());
	PyModule_AddObject( bigworldModule, "EntityIsDestroyedException", entityIsDestroyedExceptionType_.getObject() );
}

/**
 *	 Destructor.
 */
EntityManager::~EntityManager()
{}


/**
 *	 Instance accessor.
 */
EntityManager & EntityManager::instance()
{
	static EntityManager	ec;

	return ec;
}


/**
 *	This method tells the EntityManager class that a connection has
 *	been made and what its corresponding ServerConnection is.
 *
 *	@param	server	the server connection.
 */
void EntityManager::connected( ServerConnection & server )
{
	BW_GUARD;
	if (pServer_ != NULL)
	{
		WARNING_MSG( "EntityManager::connected: "
			"Already got a connection!\n" );

		this->disconnected();
	}

	TRACE_MSG( "EntityManager::connected\n" );

	pServer_ = &server;
}


/**
 *	This method tells the EntityManager class that the connection
 *	has been lost. Note that calls to this method need not be balanced
 *	with calls to the 'connected' method. You can disconnect multiple
 *	times if you are switching between offline worlds.
 */
void EntityManager::disconnected()
{
	BW_GUARD;
	TRACE_MSG( "EntityManager::disconnected\n" );

	// forget the server
	pServer_ = NULL;
}


/**
 *	This function is called when we get a new player entity.
 *	The data on the stream contains only properties provided by the base.
 *
 *	@param	id			entity id.
 *	@param	type		entity type id.
 *	@param	data		entity data.
 */
void EntityManager::onBasePlayerCreate( EntityID id, EntityTypeID type,
	BinaryIStream & data )
{
	BW_GUARD;
	// remember that this is who the server thinks player is
	proxyEntityID_ = id;

	EntityType * pEntityType = EntityType::find( type );
	if (!pEntityType)
	{
		ERROR_MSG( "EntityManager::onBasePlayerCreate: Bad type %d. id = %d\n",
			type, id );
		return;
	}

	// create the entity
	Entity * pSister =
		!enteredEntities_.empty() ? enteredEntities_.begin()->second :
		!cachedEntities_.empty()  ? cachedEntities_.begin()->second	: NULL;

	Position3D origin(0,0,0);
	float auxZero[3] = { 0, 0, 0 };
	int enterCount = 0;

	Entity * pNewEntity = pEntityType->newEntity( id, origin, auxZero,
		enterCount, data, EntityType::BASE_PLAYER_DATA, pSister );

	// put it into cached entities for now
	cachedEntities_[ id ] = pNewEntity;

	// and make it the player entity
	Player::instance().setPlayer( pNewEntity );

	// that is all we do for now
}

/**
 *	This function is called to create the call part of the player entity.
 *	The data on the stream contains only properties provided by the cell.
 *
 *	@param	id			entity id.
 *	@param	spaceID		id of space where to create the entity in.
 *	@param	vehicleID	id of an entity to use as vehicle.
 *	@param	position	position of entity.
 *	@param	yaw			yaw of entity.
 *	@param	pitch		pitch of entity.
 *	@param	roll		roll of entity.
 *	@param	data		entity's data.
 */
void EntityManager::onCellPlayerCreate( EntityID id,
		SpaceID spaceID, EntityID vehicleID, const Position3D & position,
		float yaw, float pitch, float roll, BinaryIStream & data )
{
	BW_GUARD;
	this->onEntityCreate( id,
		EntityTypeID( -1 ), // Dodgy indicator that it is the onCellPlayerCreate.
		spaceID, vehicleID, position, yaw, pitch, roll, data );

	this->onEntityEnter( id, spaceID, vehicleID );	// once for being the player
	this->onEntityEnter( id, spaceID, vehicleID );	// once for being controlled
}

/**
 *	This function is called to tell us that we may now control the given
 *	entity (or not as the case may be).
 *
 *	@param	id			entity id.
 *	@param	control		true if entity is now controlled, false otherwise.
 */
void EntityManager::onEntityControl( EntityID id, bool control )
{
	BW_GUARD;
	Entities::iterator found = enteredEntities_.find( id );
	if (found == enteredEntities_.end())
	{
		// TODO: handle this case
		ERROR_MSG( "EntityManager::onEntityControl: "
			"No entity id %d in the world\n", id );
		return;
	}

	// this will call through to the 'onPoseVolatile' method.
	// not entirely perfect, but not too bad.
	found->second->controlled( control );
}


/**
 *	This function is called when the server provides us with the details
 *	necessary to create an entity here. The minimum data it could send
 *	is the type of the entity, but for the moment it sends everything.
 *
 *	@param	id			entity id.
 *	@param	type		entity type id.
 *	@param	spaceID		id of space where to create the entity in.
 *	@param	vehicleID	id of an entity to use as vehicle.
 *	@param	position	position of entity.
 *	@param	yaw			yaw of entity.
 *	@param	pitch		pitch of entity.
 *	@param	roll		roll of entity.
 *	@param	data		entity's data.
 */
void EntityManager::onEntityCreate( EntityID id, EntityTypeID type,
	SpaceID spaceID, EntityID vehicleID, const Position3D & position,
	float yaw, float pitch, float roll,
	BinaryIStream & data )
{
	BW_GUARD;
	Entities::iterator iter;

	Vector3 pos( position.x, position.y, position.z );
	float auxFilter[3];
	auxFilter[0] = yaw;
	auxFilter[1] = pitch;
	auxFilter[2] = roll;

	Entity * pNewEntity;

	// If we already have the entity to create, then treat this as an update.
	if ((iter = cachedEntities_.find( id )) != cachedEntities_.end())
	{
		pNewEntity = iter->second;
		cachedEntities_.erase( iter );

		// Dodgy indicator that it is creating the cell part of a player.
		if (type != EntityTypeID( -1 ))
		{
			pNewEntity->updateProperties( data, /*callSetMethod:*/false );
		}
		else
		{
			pNewEntity->readCellPlayerData( data );
		}
		pNewEntity->pos( pos, auxFilter, 3 );
		// We remove it from the cached list without checking its enter count.
		// If that count was not > 0 then it'll be added back below.
		// And if we're loading prereqs, then our unknownEntities entry will get
		// clobbered below, but that's fine as we have newer info for it anyway.
	}
	// If it's already entered then treat it as an update also
	else if ((iter=enteredEntities_.find( id )) != enteredEntities_.end())
	{
		pNewEntity = iter->second;
		MF_ASSERT_DEV( pNewEntity->type().index() == type );
		MF_ASSERT_DEV( pNewEntity->enterCount() > 0 );
		pNewEntity->updateProperties( data, /*callSetMethod:*/true );
		// position information goes through to filter
	}
	// Otherwise we actually have to create an entity (yay!)
	else
	{
		// look up its type
		EntityType * pEntityType = EntityType::find( type );
		if (!pEntityType)
		{
			ERROR_MSG( "EntityManager::createEntity(%d): "
				"No such entity type %d\n", id, type );
			return;
		}

		// use the enterCount stored in this entity's unknown record
		int enterCount = unknownEntities_[id].count;
		//unknownEntities_.erase(id);	// done by clearQueuedMessages below

		if (enterCount == 0)
		{
			ERROR_MSG( "EntityManager::createEntity(%d): "
				"enterCount from unknown is zero (not 1) - "
				"didn't 'enter' before 'create'\n", id );
			// or possibly got the create message after the leave message,
			// in which case the entity will hang around in the cache below,
			// but that's not too bad (perfect if we start purging the cache)
		}
		else if (enterCount != 1)
		{
			WARNING_MSG( "EntityManager::createEntity(%d): "
				"enterCount from unknown is %d (not 1)\n", id, enterCount );
		}

		// find a sister entity
		Entity * pSister =
			!enteredEntities_.empty() ? enteredEntities_.begin()->second :
			!cachedEntities_.empty()  ? cachedEntities_.begin()->second	: NULL;

		// and instantiate the entity
		pNewEntity = pEntityType->newEntity( id, pos, auxFilter, enterCount,
			data, EntityType::TAGGED_CELL_ENTITY_DATA, pSister );
	}

	// If there are any messages queued for this entity then they are now stale
	// (Hmmm, what about out-of-order messages if the createEntity was dropped?)
	// Be sure to uncomment 'unknownEntities_.erase' above if this line changes
	this->clearQueuedMessages( id );

	// Now add it to our list of entered entities, as long as
	// it hasn't left the world again since we asked for the update
	if (pNewEntity->enterCount() > 0)
	{
		// Could already be in world if we got > 1 createEntity messages
		// after an enter-leave-enter sequence. Unlikely but possible.
		bool wantToEnter = !pNewEntity->isInWorld();
		if (wantToEnter)
		{
			// Note: it may already be loading its prerequisites, and in
			// fact it may complete that here. (Or if not then we'll add it
			// to the prereqs list twice). The code that checks the prereqs
			// list in tick handles all these cases: being in list twice,
			// being in list when already in world, etc.
			bool prereqsSatisfied = pNewEntity->checkPrerequisites();
			if (!prereqsSatisfied)
			{
				wantToEnter = false;

				cachedEntities_[ id ] = pNewEntity;
				prereqEntities_.push_back( pNewEntity );

				UnknownEntity & ue = unknownEntities_[ id ];
				ue.spaceID = spaceID;
				ue.vehicleID = vehicleID;
			}
		}

		if (wantToEnter && vehicleID != 0)
		{
			Entity * pVehicle = EntityManager::instance().getEntity( vehicleID, true );
			if (pVehicle == NULL || !pVehicle->isInWorld() )
			{
				wantToEnter = false;

				cachedEntities_[ id ] = pNewEntity;
				passengerEntities_.push_back( pNewEntity );

				UnknownEntity & ue = unknownEntities_[ id ];
				ue.spaceID = spaceID;
				ue.vehicleID = vehicleID;
			}
		}

		if (wantToEnter)
		{
			enteredEntities_[ id ] = pNewEntity;
			pNewEntity->enterWorld( spaceID, vehicleID, false );
		}
	}
	else
	{
		MF_ASSERT_DEV( !pNewEntity->isInWorld() );
		// cannot be still in world when enterCount is <= 0
		// that is always handled in leaveEntity.
		cachedEntities_[ id ] = pNewEntity;
	}

	// get time of message (or game time if offline)
	double time = this->lastMessageTime();
	if (time < 0.f) time = App::instance().getTime();

	// and pass the position update to the (mandatory) filter
	pNewEntity->filter().reset( time - 0.001 );
	pNewEntity->filter().input( time, spaceID, vehicleID,
		pos, Vector3::zero(), auxFilter );
}


/**
 *	This method is called to update an entity. This message is sent by the
 *	server when a LoD level changes and a number of property changes are sent in
 *	one message. (Or at the server's discretion, it may group multiple updates
 *	into one message at other times).
 *
 *	@param	id			entity id.
 *	@param	data		entity's data.
 */
void EntityManager::onEntityProperties( EntityID id,
	BinaryIStream & data )
{
	BW_GUARD;
	bool inWorld = true;

	// find the entity
	Entities::iterator iter = enteredEntities_.find( id );
	if (iter == enteredEntities_.end())
	{
		// complain if it wasn't in the world
		// (but accept the updates anyway if we can)
		inWorld = false;
		WARNING_MSG( "EntityManager::onEntityProperties: "
			"Got properties for %d who is not entered.\n", id );

		iter = cachedEntities_.find( id );
		if (iter == cachedEntities_.end())
		{
			// abort if it wasn't outside the world either
			WARNING_MSG( "EntityManager::onEntityProperties: "
				"Got properties for %d who does not exist!\n", id );
			return;
		}
	}

	// now stream off all the property updates
	iter->second->updateProperties( data, /*shouldCallSetMethod:*/inWorld );
}


/**
 *	This function is called when the server indicates that an entity has entered
 *	the player's AoI.
 *
 *	It is complicated because it may be called out of order.
 *
 *	@param	id			entity id.
 *	@param	spaceID		id of space where to create the entity in.
 *	@param	vehicleID	id of an entity to use as vehicle.
 *
 */
void EntityManager::onEntityEnter( EntityID id,
	SpaceID spaceID, EntityID vehicleID )
{
	BW_GUARD;
	Entity * pEntity = NULL;
	Entities::iterator iter;

	// look in the cache first
	if ((iter=cachedEntities_.find( id )) != cachedEntities_.end())
	{
		Entity * pDebutant = pEntity = iter->second;

		if (pDebutant->incrementEnter() > 0)
		{
			if (pDebutant->enterCount() != 1)
			{
				WARNING_MSG( "EntityManager::enterEntity(%d): "
					"enterCount is now %d.\n", id, pDebutant->enterCount() );
			}

			// store space and vehicle in case it reentered while
			// we were still loading its prerequisites, and we finish loading
			// the prereqs before the missing leave and future create
			// messages arrive
			Unknowns::iterator uiter = unknownEntities_.find( id );
			if (uiter != unknownEntities_.end())
			{
				uiter->second.spaceID = spaceID;
				uiter->second.vehicleID = vehicleID;
			}

			// if it's a client-side entity then the server isn't going to send
			// us a create message, so we should do all the processing here
			if (pDebutant == Player::entity() || // player gets create b4 enter
					pDebutant->isClientOnly())
			{
				if (pDebutant->isClientOnly() && vehicleID != 0)
				{
					// This seems a reasonable assumption but may need
					// to be changed in the future.
					MF_ASSERT_DEV( !"Client only entities should "
						"not start already aboard a vehicle." );
				}

				// if it has no prerequisites or has satisfied them already,
				// then we can immediately add it to the entered list
				if (pDebutant->checkPrerequisites())
				{
					cachedEntities_.erase( iter );
					enteredEntities_[ id ] = pDebutant;
					unknownEntities_.erase( id );

					pDebutant->enterWorld( spaceID, vehicleID, false );
					this->forwardQueuedMessages( pDebutant );
				}
				// otherwise it has just started loading its prerequisites,
				// so add it to a list of entities to check on
				else
				{
					prereqEntities_.push_back( pDebutant );

					UnknownEntity & ue = unknownEntities_[ id ];
					ue.spaceID = spaceID;
					ue.vehicleID = vehicleID;
				}
			}
		}
	}
	// ok, try the entered list
	else if ((iter=enteredEntities_.find( id )) != enteredEntities_.end())
	{
		pEntity = iter->second;

		pEntity->incrementEnter();

		if (pEntity != Player::entity())
		{
			// presumably there is a missing leaveEntity on its way
			WARNING_MSG( "EntityManager::enterEntity(%d): "
				"Entity is now entered %d times\n", id, pEntity->enterCount() );
			// .. since we ask for the update below, there's no problem with
			// ignoring the new space and vehice ids
		}
	}
	// it must be an unknown entity then
	else
	{
		// increment its count, possibly creating the record for it
		if (++unknownEntities_[id].count == 0)
		{
			// entity entered and left but packet order was swapped
			unknownEntities_.erase( id );
		}
	}

	// To keep things simple, always request an update from the server when
	// we get one of these, regardless of what state we think the entity is in.
	// The server will ignore the request if it is not in our AoI (or it has
	// already sent the 'create' message) at the time it receives the request.
	// (the player gets its 'create' messages via a different mechanism)
	// Don't request update if it we're not connected to the server or
	// if it's a client-only entity.
	if ((pServer_ != NULL) &&
		!EntityManager::isClientOnlyID( id ) &&
		// definitely request update if we don't know anything about entity
		((pEntity == NULL) ||
		// Otherwise request update for all entities except...
		// Must not request update for witness entity - server will complain.
		// Must not request update for self-controlled entity - this is mainly
		// a hack to get around the fact that we call this function from
		// Entity::controlled() but it is a good condition anyway.
		 ((id != proxyEntityID_) && !pEntity->selfControlled())))
	{
		pServer_->requestEntityUpdate( id,
			pEntity ? pEntity->cacheStamps() : CacheStamps() );
	}
}


/**
 *	This function is called when the server indicates that an entity has left
 *	the player's AoI. It is complicated because it may be called out of order.
 *
 *	@param	id				entity id.
 *	@param	cacheStamps		Unused parameter.
 */
void EntityManager::onEntityLeave( EntityID id, const CacheStamps & cacheStamps )
{
	BW_GUARD;
	Entities::iterator iter;

	// look in the entered list first
	if ((iter = enteredEntities_.find( id )) != enteredEntities_.end() )
	{
		// should not be in the entered list if its enterCount is <= 0
		MF_ASSERT_DEV( iter->second->enterCount() > 0 );

		// decrement its enterCount and see if that means it should go
		if (iter->second->decrementEnter() == 0)
		{
			Entity * pGeriatric = iter->second;

			pGeriatric->leaveWorld( false );

			enteredEntities_.erase( iter );
			/*
			cachedEntities_[ id ] = pGeriatric;
			pGeriatric->cacheStamps( cacheStamps );
			*/
			pGeriatric->destroy();
			Py_DECREF( pGeriatric );	// no caching for now
		}
		else
		{
			INFO_MSG( "EntityManager::leaveEntity(%d): "
					"enterCount is still %d.\n",
				id, iter->second->enterCount() );
		}
	}
	// ok, try the cached list
	else if ((iter = cachedEntities_.find( id )) != cachedEntities_.end())
	{
		// the only reason it will be in cachedEntities currently
		// (with the code above commented out) is if it was still loading
		// its prereqs when we got the leave, so the message below will
		// never appear
		if (iter->second->enterCount() <= 0)
		{
			WARNING_MSG( "EntityManager::leaveEntity(%d): "
				"Got leave for non-entered entity\n", id );
		}

		if (iter->second->decrementEnter() == 0)
		{
			// if we just made its count be zero, then dump this entity
			Entity * pGeriatric = iter->second;

			cachedEntities_.erase( iter );
			pGeriatric->destroy();
			Py_DECREF( pGeriatric );

			// it was loading its prereqs, so erase it from unknowns
			this->clearQueuedMessages( id );
			// the entity will be kept alive by the reference to it in
			// prereqEntities, which will go (and destruct the entity)
			// when its prerequisites have finised loading - we can't
			// touch it before then
		}
	}
	// ok it's an unknown entity then: an ordinary unknown entity that
	// hasn't received the onEntityCreate yet; not a 'loading-prereqs' one
	else
	{
		UnknownEntity & unknown = unknownEntities_[id];
		if (--unknown.count == 0)
		{
			// the count has got back to zero, so erase it from unknowns
			this->clearQueuedMessages( id );
		}
		else if (unknown.count < 0)
		{
			WARNING_MSG( "EntityManager::leaveEntity(%d): "
				"Got leave for unheard-of entity\n", id );
			// we intentionally made an unknowns entry for it above so when we
			// get the enter that has not yet arrived it will cancel it out
		}
	}
}

#include "cstdmf/profile.hpp"

/**
 *	This method is called when we receive a property update message
 *	for one of our client-side entities from the server.
 *
 *	@param	id				entity id.
 *	@param	messageID		message id.
 *	@param	data			message data.
 */
void EntityManager::onEntityProperty( EntityID id, int messageID,
	BinaryIStream & data )
{
	BW_GUARD;
	// And pass it on to the appropriate entity
	Entities::iterator iter = enteredEntities_.find( id );
	if (iter != enteredEntities_.end())
	{
		iter->second->handleProperty( messageID, data );
	}
	else
	{
		// or save it for later if it's not around
		int length = data.remainingLength();
		MemoryOStream	*pMos = new MemoryOStream( length );
		pMos->transfer( data, length );

		const int PROPERTY_FLAG = 0x40;

		queuedMessages_[ id ].push_back(
			std::make_pair( messageID | PROPERTY_FLAG, pMos ) );

		// TODO: Make sure this doesn't get too big. Options include:
		//	- delivering (some) messages to cached entities
		//	- cleaning up really old messages
		//	- definitely when an entity is cleared from the cache, get
		//		rid of its queue, 'coz you'll know there isn't an enter
		//		or create message for it in an out-of-order/dropped packet
	}
}

/**
 *	This method is called when we receive a script method call message
 *	for one of our client-side entities from the server.
 *
 *	@param	id				entity id.
 *	@param	messageID		message id.
 *	@param	data			message data.
 */
void EntityManager::onEntityMethod( EntityID id, int messageID,
	BinaryIStream & data )
{
	BW_GUARD;
	// And pass it on to the appropriate entity
	Entity * pEntity = NULL;
	Entities::iterator iter = enteredEntities_.find( id );
	if (iter != enteredEntities_.end())
		pEntity = iter->second;
	else if (Player::entity() != NULL && Player::entity()->id() == id)
		pEntity = Player::entity();

	if (pEntity != NULL)
	{
		pEntity->handleMethod( messageID, data );
	}
	else
	{
		// or save it for later if it's not around
		int length = data.remainingLength();
		MemoryOStream	*pMos = new MemoryOStream( length );
		pMos->transfer( data, length );

		queuedMessages_[ id ].push_back( std::make_pair( messageID, pMos ) );

		// TODO: Make sure this doesn't get too big. Options include:
		//	- delivering (some) messages to cached entities
		//	- cleaning up really old messages
		//	- definitely when an entity is cleared from the cache, get
		//		rid of its queue, 'coz you'll know there isn't an enter
		//		or create message for it in an out-of-order/dropped packet
	}
}

/**
 *	This method is called when we receive a fast-track avatar update
 *	from the server
 *
 *	@param	id			entity id.
 *	@param	spaceID		id of space where the entity is in.
 *	@param	vehicleID	id of vehicle the entity is on.
 *	@param	pos			new entity position.
 *	@param	yaw			new entity yaw.
 *	@param	pitch		new entity pitch.
 *	@param	roll		new entity roll.
 *	@param	isVolatile	true is new pose is volatile.
 */
void EntityManager::onEntityMoveWithError( EntityID id,
	SpaceID spaceID, EntityID vehicleID,
	const Position3D & pos, const Vector3 & posError,
	float yaw, float pitch, float roll, bool isVolatile )
{
	BW_GUARD;
	// Figure out what time it happened at
	double packetTime = this->lastMessageTime();
	if (packetTime < 0.f)
	{
		packetTime = App::instance().getTime();
	}

	// And tell the entity's filter, if it has one
	Entities::iterator	iter = enteredEntities_.find( id );
	if (iter != enteredEntities_.end())
	{
		Entity * pE = iter->second;

		PyObject * poseVolatileNotifier = NULL;
		if (pE->isPoseVolatile() != isVolatile)
			poseVolatileNotifier = pE->isPoseVolatile( isVolatile );

		if (pE->selfControlled()) // this is a forced position / physics correction
		{
			pE->filter().reset( packetTime );
		}
		float	auxVolatile[3] = { yaw, pitch, roll };
		pE->filter().input( packetTime, spaceID, vehicleID,
			pos, posError, auxVolatile );

		if (pE->selfControlled())	// make it take effect immediately
		{
			pE->filter().output( packetTime );

			if (!EntityManager::isClientOnlyID( pE->id() ))
			{
				pE->physicsCorrected( pServer_->lastSendTime() );

				if (pE->pPhysics())
				{
					pE->pPhysics()->cancelTeleport();
				}
			}
		}

		if (poseVolatileNotifier != NULL)
			Script::call( poseVolatileNotifier,
				Py_BuildValue( "(i)", int(isVolatile) ),
				"Entity::isPoseVolatile: ", false );
	}
	else
	{
		iter = cachedEntities_.find( id );
		bool storeInUnknowns = true;

		if (iter != cachedEntities_.end())
		{
			Entity * pEntity = iter->second;

			MF_ASSERT_DEV( pEntity != NULL );
			if (pEntity->enterCount() <= 0)
			{
				storeInUnknowns = false;
				WARNING_MSG( "EntityManager::avatarUpdate: "
					"Got update for %d who is cached\n", id );
			}
			// else store it in unknowns for when we put it in the world
		}

		if (storeInUnknowns)
		{
			// record it in unknowns.
			UnknownEntity & sleeper = unknownEntities_[ id ];
			sleeper.time = packetTime;
			sleeper.pos = pos;
			sleeper.spaceID = spaceID;
			sleeper.vehicleID = vehicleID;
			sleeper.auxFilter[0] = yaw;
			sleeper.auxFilter[1] = pitch;
			sleeper.auxFilter[2] = roll;

			/*
			WARNING_MSG( "EntityManager::avatarUpdate: "
				"Received update for %d who we do not know about.\n", id );
			*/
		}
	}
}


/**
 *	This method is called when the server tells us that the proxy has changed.
 *
 *	@param keepPlayerAround	true if the player should be kept.
 */
void EntityManager::onEntitiesReset( bool keepPlayerAround )
{
	BW_GUARD;
	TRACE_MSG( "EntityManager::onEntitiesReset\n" );
	if( Player::entity() && Player::entity()->pSpace() )
	{
		this->spaceGone( Player::entity()->pSpace()->id() );
	}

	this->clearAllEntities( keepPlayerAround, /*keepClientOnly:*/ true );
}


/**
 *	This method removes all entities from the world 
 *	expect, if requested, the player.
 *
 *	@param keepPlayerAround	true if the player should be kept around.
 *	@param keepClientOnly	Boolean indicating whether to retain client only entities.
 */
void EntityManager::clearAllEntities( bool keepPlayerAround, bool keepClientOnly )
{
	BW_GUARD;
	class DeleteCondition
	{
	public:
		DeleteCondition( bool keepPlayerAround, bool keepClientOnly ) :
			keepPlayerAround_( keepPlayerAround ),
			keepClientOnly_( keepClientOnly )
		{
		}

		bool operator()( const Entity * pEntity ) const
		{
			BW_GUARD;
			return (!keepPlayerAround_ || 
				(pEntity != Player::instance().entity())) &&
			(!keepClientOnly_ || !pEntity->isClientOnly());
		}

	private:
		bool keepPlayerAround_;
		bool keepClientOnly_;
	};
	DeleteCondition shouldDelete( keepPlayerAround, keepClientOnly );

	// Call leaveWorld() on all entities
	Entities::iterator iter = enteredEntities_.begin();
	while (iter != enteredEntities_.end())
	{
		/*
		if ((!keepPlayerAround ||
				(iter->second != Player::instance().entity())) &&
			(!keepClientOnly || !iter->second->isClientOnly()))
		*/
		if (shouldDelete( iter->second))
		{
			iter->second->leaveWorld( false );
		}

		iter++;
	}

	// get rid of any player things
	if (!keepPlayerAround)
	{
		// NOTE: Must be done after calling leaveWorld() on player entity,
		// otherwise leaveWorld() would be called on the wrong type.
		Player::instance().setPlayer( NULL );
		proxyEntityID_ = 0;

		if (!keepClientOnly)
		{
			// reset the next client ID
			nextClientID_ = FIRST_CLIENT_ID;
		}
	}

	// dispose of everything in our cache
	iter = cachedEntities_.begin();
	Entities::iterator iend = cachedEntities_.end();
	while (iter != iend)
	{
		/*
		if ((!keepPlayerAround ||
				(iter->second != Player::instance().entity())) &&
			(!keepClientOnly || !iter->second->isClientOnly()))
		*/
		if (shouldDelete( iter->second ))
		{
			iter->second->destroy();
			Py_DECREF( iter->second );
			iter = cachedEntities_.erase(iter);
		}
		else
		{
			++iter;
		}
	}

	// dispose of everything in our entered list
	iter = enteredEntities_.begin();
	iend = enteredEntities_.end();
	while (iter != iend)
	{
		/*
		if ((!keepPlayerAround ||
				(iter->second != Player::instance().entity())) &&
			(!keepClientOnly || !iter->second->isClientOnly()))
		*/
		if (shouldDelete( iter->second ))
		{
			iter->second->destroy();
			Py_DECREF( iter->second );
			iter = enteredEntities_.erase(iter);
		}
		else
		{
			++iter;
		}
	}

	// Destroy prereq entities - scoped so that new vector is destroyed
	{
		std::vector< EntityPtr> newPrereqEntities;
		Unknowns newUnknowns;

		// and even the unknown entities and prerequisite entity list
		for (std::vector<EntityPtr>::iterator i = prereqEntities_.begin();
			i != prereqEntities_.end();
			++i)
		{
			EntityPtr pEntity = *i;

			if (shouldDelete( pEntity.get() ))
			{
				pEntity->destroy();
			}
			else
			{
				newPrereqEntities.push_back( pEntity );
				newUnknowns[ pEntity->id() ] = unknownEntities_[ pEntity->id() ];
			}
		}

		prereqEntities_.swap( newPrereqEntities );
		unknownEntities_.swap( newUnknowns );
	}

	for (std::vector<EntityPtr>::iterator i = passengerEntities_.begin();
		i != passengerEntities_.end();
		++i)
	{
		(*i)->destroy();
	}
	passengerEntities_.clear();

	// don't leave the queued messages either
	for (MessageMap::iterator iter = queuedMessages_.begin();
		 iter != queuedMessages_.end();
		 iter++)
	{
		for (MessageQueue::iterator qIter = iter->second.begin();
			 qIter != iter->second.end();
			 qIter++)
		{
			delete qIter->second;
		}
	}
	queuedMessages_.clear();

	PyGC_Collect();
}


/**
 *	This function is called to call the tick functions of all the
 *	entities. Note: These functions do not do things like animation -
 *	that is left to Models that are actually in the scene graph.
 *
 *	@param	timeNow		current timestamp
 *	@param	timeLast	timestamp when this method was called last.
 */
void EntityManager::tick( double timeNow, double timeLast )
{
	BW_GUARD;
	// see if any of the entities waiting on prerequisites are now ready
	for (uint32 i = 0; i < prereqEntities_.size(); i++)
	{
		Entity * pEntity = &*prereqEntities_[i];
		if (pEntity->checkPrerequisites())
		{
			if (pEntity->enterCount() > 0 && !pEntity->isInWorld())
			{
				// can be already in world if in prereqs list twice
				EntityID id = pEntity->id();

				// get space and vehicle ID out of unknowns
				Unknowns::iterator uit = unknownEntities_.find( id );
				MF_ASSERT_DEV( uit != unknownEntities_.end() );

				Entity * pVehicle = NULL;

				if( uit->second.vehicleID == 0 ||
					( ( pVehicle = EntityManager::instance().getEntity(
											uit->second.vehicleID, true ) ) &&
					 pVehicle->isInWorld() ) )
				{
					cachedEntities_.erase( id );
					enteredEntities_[ id ] = pEntity;


					UnknownEntity ue = uit->second;
					unknownEntities_.erase( uit );

					// enter the entity
					pEntity->enterWorld( ue.spaceID, ue.vehicleID, false );

					// now send it any queued messages
					this->forwardQueuedMessages( pEntity );
				}
				else
				{
					passengerEntities_.push_back( prereqEntities_[i] );
				}
			}

			std::swap<>( prereqEntities_[i], prereqEntities_.back() );
			prereqEntities_.pop_back();

			i--;
		}
	}


	for (uint32 i = 0; i < passengerEntities_.size(); i++)
	{
		Entity * pEntity = &*passengerEntities_[i];

		if (pEntity->enterCount() > 0 && !pEntity->isInWorld())
		{
			Unknowns::iterator iUnknown = unknownEntities_.find( pEntity->id() );

			if( iUnknown != unknownEntities_.end() )
			{
				Entity * pVehicle = EntityManager::instance().getEntity(
					iUnknown->second.vehicleID, true );

				if ( pVehicle != NULL && pVehicle->isInWorld() )
				{
					cachedEntities_.erase( pEntity->id() );
					enteredEntities_[ pEntity->id() ] = pEntity;

					// enter the entity
					pEntity->enterWorld(	iUnknown->second.spaceID,
											iUnknown->second.vehicleID,
											false );

					// now send it any queued messages
					this->forwardQueuedMessages( pEntity );

					unknownEntities_.erase( iUnknown );
				}
				else
				{
					continue;
				}
			}
		}

		std::swap<>( passengerEntities_[i], passengerEntities_.back() );
		passengerEntities_.pop_back();

		i--;
	}


	// now tick all the entities that are in the world
	for (Entities::iterator iter = enteredEntities_.begin();
		iter != enteredEntities_.end();
		iter++)
	{
		iter->second->tick( timeNow, timeLast );
	}

	// Garbage collect any seenSpaces (server controlled spaces) which have been cleared. 
	// This is to avoid any empty ChunkSpaces from dangling around indefinately. If a 
	// spaceID does actually get reused in the future it can just be recreated.
	static ChunkSpaceSet spacesToErase;

	for(ChunkSpaceSet::iterator iter = seenSpaces_.begin();
		iter != seenSpaces_.end();
		iter++)
	{
		ChunkSpacePtr space = *iter;
		if (space->cleared())
		{
			spacesToErase.insert( space );
		}
	}

	for(ChunkSpaceSet::iterator iter = spacesToErase.begin();
		iter != spacesToErase.end();
		iter++)
	{
		//DEBUG_MSG( "EntityManager::tick: dereferencing cleared space %d.\n", (*iter)->id() );
		seenSpaces_.erase( *iter );
	}

	spacesToErase.clear();
}


/**
 *	This method gets the given entity ID if it exists in the world.
 *	Note that you should think twice before using this function,
 *		because the entity manager may be the best place to do
 *		whatever you were going to do with the Entity.
 *	This function does not increment the reference count of the
 *		entity it returns. Returns NULL if it can't be found.
 *
 *	@param	id			id of entity.
 *	@param	lookInCache	whether we should look in cached entities for this entity.
 *
 *	@return				pointer to entity, if any found. NULL otherwise.
 */
Entity * EntityManager::getEntity( EntityID id, bool lookInCache )
{
	BW_GUARD;
	Entities::iterator iter = enteredEntities_.find( id );
	if (iter != enteredEntities_.end()) return iter->second;

	if (lookInCache)
	{
		iter = cachedEntities_.find( id );
		if (iter != cachedEntities_.end()) return iter->second;
	}

	return NULL;
}


/**
 *	This is an internal method used to forward any messages queued
 *	for an entity to that entity, erasing them in the process.
 *
 *	@param	pEntity		pointer to entity.
 *
 *	@return				true if any message was forwarded, false otherwise.
 */
bool EntityManager::forwardQueuedMessages( Entity * pEntity )
{
	BW_GUARD;
	const int PROPERTY_FLAG = 0x40;
	bool hasForwardedMessages = false;
	MessageMap::iterator iter = queuedMessages_.find( pEntity->id() );

	if (iter != queuedMessages_.end())
	{
		hasForwardedMessages = true;

		for (MessageQueue::iterator qIter = iter->second.begin();
			qIter != iter->second.end();
			qIter++)
		{
			int msgID = qIter->first;
			if (msgID & PROPERTY_FLAG)
			{
				pEntity->handleProperty( msgID & ~PROPERTY_FLAG ,
					*qIter->second );
			}
			else
			{
				pEntity->handleMethod( msgID, *qIter->second );
			}
			delete qIter->second;
		}

		queuedMessages_.erase( iter );
	}

	return hasForwardedMessages;
}

/**
 *	This is an internal method used to clear any 
 *	messages queued for an entity.
 *
 *	@param	id	id of entity.
 */
void EntityManager::clearQueuedMessages( EntityID id )
{
	BW_GUARD;
	// clear messages
	MessageMap::iterator mit = queuedMessages_.find( id );
	if (mit != queuedMessages_.end())
	{
		for (MessageQueue::iterator qIter = mit->second.begin();
			qIter != mit->second.end();
			qIter++)
		{
			delete qIter->second;
		}
		queuedMessages_.erase( mit );
	}

	// and erase any unknown entity record
	unknownEntities_.erase( id );
}


/**
 *	This method sets the function which determines when the time
 *	of day from game time.
 *
 *	@param	pSpace					pointer to space whose time-of-day 
 *									should be set.
 *	@param	gameTime				current game time.
 *	@param	initialTimeOfDay		the initial time of day
 *	@param	gameSecondsPerSecond	number of game seconds ellapsed 
 *									per real world seconds.
 */
void EntityManager::setTimeInt( ChunkSpacePtr pSpace, GameTime gameTime,
	float initialTimeOfDay, float gameSecondsPerSecond )
{
	BW_GUARD;
	TimeOfDay & timeOfDay = *pSpace->enviro().timeOfDay();
	timeOfDay.updateNotifiersOn( false );

	TRACE_MSG( "EntityManager::setTimeInt: gameTime %d initial %f gsec/sec %f\n",
		gameTime, initialTimeOfDay, gameSecondsPerSecond );

	float tod;
	if ( gameSecondsPerSecond > 0.f )
	{
		timeOfDay.secondsPerGameHour((1.0f / gameSecondsPerSecond) * 3600.0f);
		DEBUG_MSG( "\tsec/ghour = %f\n", (1.0f / gameSecondsPerSecond) * 3600.0f );
		// This gives us the time of day as in game seconds, since the start of time.
		tod = initialTimeOfDay + ((float)gameTime /
			(float)ServerConnection::updateFrequency() *
			(float)gameSecondsPerSecond);
	}
	else
	{
		timeOfDay.secondsPerGameHour(0.f);
		DEBUG_MSG( "\tsec/ghour = 0.0\n" );
		tod = initialTimeOfDay;
	}

	DEBUG_MSG( "\ttherefore tod in seconds = %f\n", tod );

	// Set the time of day in hours.
	timeOfDay.gameTime(tod / 3600.0f);
	timeOfDay.tick(0.0f);

	timeOfDay.updateNotifiersOn( true );
}


/**
 *	Get accessor for the time that the last message occurred
 *
 *	@return		time of last message's arrival.
 */
double EntityManager::lastMessageTime() const
{
	return pServer_ != NULL ? pServer_->lastMessageTime() : -1.0;
}


/**
 *	Set accessor for the time that the last message occurred
 *	 (only used during server message playback)
 *
 *	@param	t	time of last message arrival.
 */
void EntityManager::lastMessageTime( double t )
{
	playbackLastMessageTime_ = t;
}


/*~ callback Entity.onGeometryMapped
 *
 *	This callback method tells the player entity about changes to
 *	the geometry in its current space.  It is called when geometry
 *	is mapped into the player's current space.  The name of the
 *	spaces geometry is passed in as a parameter
 *
 *	@param spaceName	name describing the space's geometry
 */
/*~ callback Personality.onGeometryMapped
 *
 *	This callback method tells the player entity about changes to
 *	the geometry in a space.  It is called when geometry is mapped 
 *	into any of the currently exisiting spaces on the client. The 
 *	space id and name of the space geometry is passed in as parameters.
 *
 *	@param spaceID		id of the space the geometry is being mapped in to
 *	@param spaceName	name describing the space's geometry
 */
/**
 *	We got some space data.
 *
 *	@param	spaceID		id of the space referred by the incomming data.
 *	@param	entryID		data entry id.
 *	@param	key			data key.
 *	@param	data		the data itself.
 */
void EntityManager::spaceData( SpaceID spaceID, SpaceEntryID entryID,
	uint16 key, const std::string & data )
{
	BW_GUARD;
	// Not at all sure that parsing of this data should be left up to the
	// space, but I certainly don't want to have a parallel set of classes
	// manging the data for each space - so the space can at least store it.
	ChunkSpacePtr pSpace = ChunkManager::instance().space( spaceID );
	seenSpaces_.insert( pSpace );
	uint16 okey = pSpace->dataEntry( entryID, key, data );

	// Acutally, for client friendliness, we'll parse the data ourselves here.
	if (okey == SPACE_DATA_TOD_KEY)
	{
		const std::string * pData =
			pSpace->dataRetrieveFirst( SPACE_DATA_TOD_KEY );
		if (pData != NULL && pData->size() >= sizeof(SpaceData_ToDData))
		{
			SpaceData_ToDData & todData = *(SpaceData_ToDData*)pData->data();
#if ENABLE_ENVIRONMENT_SYNC
			if( environmentSync )
			{
				this->setTimeInt( pSpace, pServer_->lastGameTime(),
					todData.initialTimeOfDay, todData.gameSecondsPerSecond );
			}
#else
			this->setTimeInt( pSpace, pServer_->lastGameTime(),
				todData.initialTimeOfDay, todData.gameSecondsPerSecond );
#endif
		}
	}
	else if( key == SPACE_DATA_WEATHER )
	{
		Script::call(
			PyObject_GetAttrString( Personality::instance(), "onWeatherChange" ),
			Py_BuildValue( "(is)", spaceID, data.c_str() ), "EntityManager::spaceData weather notifier: ", true );		
	}
	else if (okey == SPACE_DATA_MAPPING_KEY_CLIENT_SERVER ||
			okey == SPACE_DATA_MAPPING_KEY_CLIENT_ONLY)
	{
		// see if this mapping is being added
		if (key == okey)
		{
			MF_ASSERT_DEV( data.size() >= sizeof(SpaceData_MappingData) );
			SpaceData_MappingData & mappingData =
				*(SpaceData_MappingData*)data.data();
			std::string path( (char*)(&mappingData+1),
				data.length() - sizeof(SpaceData_MappingData) );
			pSpace->addMapping( entryID, &mappingData.matrix[0][0], path );

			// tell the player about this if it is relevant to it
			// this system probably wants to be expanded in future
			Entity * pPlayer = Player::entity();
			if (pPlayer != NULL && pPlayer->pSpace() == pSpace)
			{
				Script::call(
					PyObject_GetAttrString( pPlayer, "onGeometryMapped" ),
					Py_BuildValue( "(s)", path.c_str() ),
					"EntityManager::spaceData geometry notifier: ",
					true );
				Player::instance().updateWeatherParticleSystems(
					pSpace->enviro().playerAttachments() );
			}

			Script::call(
				PyObject_GetAttrString(
					Personality::instance(),
					"onGeometryMapped" ),
				Py_BuildValue( "(is)", spaceID, path.c_str() ),
				"EntityManager::spaceData geometry notifier: ",
				true );

		}
		// ok it's being removed then
		else
		{
			MF_ASSERT_DEV( key == uint16(-1) );
			pSpace->delMapping( entryID );
		}
	}
}

/**
 *	This space is no longer with us.
 *
 *	@param	spaceID		id of the space passing away.
 */
void EntityManager::spaceGone( SpaceID spaceID )
{
	BW_GUARD;
	ChunkSpacePtr pSpace = ChunkManager::instance().space( spaceID );
	if (pSpace)
	{
		pSpace->clear();
		seenSpaces_.erase( pSpace );
	}
}

/**
 *	Finalises the EntityManager.
 */
void EntityManager::fini()
{
	BW_GUARD;
	this->disconnected();
	this->clearAllEntities( false, false );
	seenSpaces_.clear();
	Player::fini();
}


/*~ callback Entity.onRestore
 *
 *	This callback method informs the player entity to restore
 *	itself to a previous state given the description provided by the server.

 *	@param position		position
 *	@param direction	direction
 *	@param spaceID		space
 *	@param vehicleID	vehicle
 *	@param dict			dictonary of attributes
 */
/**
 *	This method is called when the server provides us with the details
 *	necessary to restore the client to a previous state.
 *
 *	@param id			id of entity being restored
 *	@param spaceID		space the entity is currently in
 *	@param vehicleID	vehicle the entity is currently on (if any)
 *	@param pos			entity's current position
 *	@param dir			entity's current direction
 *	@param data			entity's attributes
 */
void EntityManager::onRestoreClient( EntityID id,
		SpaceID spaceID, EntityID vehicleID,
		const Position3D & pos, const Direction3D & dir,
		BinaryIStream & data )
{
	BW_GUARD;
	TRACE_MSG( "EntityManager::onRestoreClient: "
			"id = %d. vehicleID = %d.\n",
		id, vehicleID );
	Entity * pPlayer = Player::entity();

	if (pPlayer && id == pPlayer->id())
	{
		const EntityDescription & entityDesc = pPlayer->type().description();
		PyObjectPtr pDict( PyDict_New(), PyObjectPtr::STEAL_REFERENCE );

		entityDesc.readStreamToDict( data,
			EntityDescription::CLIENT_DATA, pDict.getObject() );

		PyObjectPtr pPos = Script::getData( pos );
		PyObjectPtr pDir = Script::getData( *(Vector3 *)&dir );

		Script::call( PyObject_GetAttrString( pPlayer, "onRestore" ),
				Py_BuildValue( "OOiiO",
					pPos.getObject(),
					pDir.getObject(),
					spaceID,
					vehicleID,
					pDict.getObject() ),
				"", true );
#if 0
		PyObjectPtr pCurrDict( PyObject_GetAttrString( pPlayer, "__dict__" ),
			PyObjectPtr::STEAL_REFERENCE );

		Physics * pPhysics = pPlayer->pPhysics();

		if (pPhysics)
		{
			pPhysics->teleport( pos, dir );
		}

		MF_ASSERT_DEV( pCurrDict );
		if (pCurrDict)
		{
			PyDict_Update( pCurrDict.getObject(), pDict.getObject() );
			Script::call( PyObject_GetAttrString( pPlayer, "onRestore" ),
				Py_BuildValue( "O", pDict.getObject() ), "", true );
		}
#endif

		typedef std::vector< EntityID > IDs;
		IDs ids;
		{
			Entities::iterator iter = enteredEntities_.begin();

			while (iter != enteredEntities_.end())
			{
				if (!iter->second->isClientOnly())
				{
					ids.push_back( iter->first );
				}

				++iter;
			}
		}

		{
			IDs::iterator iter = ids.begin();

			while (iter != ids.end())
			{
				if (*iter != pPlayer->id())
				{
					this->onEntityLeave( *iter );
				}

				++iter;
			}
		}


		if (vehicleID != 0)
		{
			pPlayer->waitingForVehicle( vehicleID );
			pPlayer->pVehicle( NULL );
		}

		INFO_MSG( "EntityManager::onRestoreClient: enteredEntities_.size() = %d\n",
			enteredEntities_.size() );
	}
	else
	{
		CRITICAL_MSG( "EntityManager::onRestoreClient: "
				"The entity to restore (%d) is not the player (%d)\n",
			id, (pPlayer ? pPlayer->id() : 0) );
	}
}


/**
*	This method gathers input from the stored ServerConnection,
*	if we are online and entities have been successfully enabled.
*/
void EntityManager::gatherInput()
{	
	BW_GUARD;
	if (pServer_ != NULL)
	{
		pServer_->processInput();
	}
}


/**
 *	This function returns the Python type object of the EntityIsDestroyedException.
 *
 *	@return	Returns a borrowed pointer to the EntityIsDestroyedException type object.
 */
PyObject * EntityManager::entityIsDestroyedExceptionType()
{
	return entityIsDestroyedExceptionType_.getObject();
}


// -----------------------------------------------------------------------------
// Section: Out-of-place handlers
// -----------------------------------------------------------------------------

/**
 *	This method handles voice data sent by another client.
 *
 *	@param	srcAddr		address of voice source.
 *	@param	data		voice data.
 */
void EntityManager::onVoiceData( const Mercury::Address & srcAddr,
							  BinaryIStream & data )
{
	BW_GUARD;
	int length = data.remainingLength();
	data.retrieve( length );
}

/**
 *	This method handles data sent from the baseapp and invokes the
 *	onStreamComplete() callback function in the personality script.
 */
void EntityManager::onStreamComplete( uint16 id,
	const std::string &desc, BinaryIStream & data )
{
	BW_GUARD;
	int len = data.remainingLength();

	if (len <= 0)
	{
		ERROR_MSG( "EntityManager::onStreamComplete: "
			"Received zero length data\n" );
		return;
	}

	const char *pData = (const char*)data.retrieve( len );

	PyObject * pFunc = PyObject_GetAttrString( Personality::instance(),
			"onStreamComplete" );

	Script::call( pFunc,
		Py_BuildValue( "(is#s#)", id, desc.c_str(), desc.size(), pData, len ),
		"EntityManager::onStreamComplete",
		/*okIfFunctionNull*/ true );

	// Try to call back into the old onProxyDataDownloadComplete() if it exists
	// and the new callback wasn't defined
	if (!pFunc && PyObject_HasAttrString( Personality::instance(),
			"onProxyDataDownloadComplete" ))
	{
		WARNING_MSG( "BWPersonality.onProxyDataDownloadComplete() is "
			"deprecated, use onStreamComplete() instead\n" );

		Script::call(
			PyObject_GetAttrString( Personality::instance(),
				"onProxyDataDownloadComplete" ),
			Py_BuildValue( "(is#)", id, pData, len ),
			"EntityManager::onStreamComplete",
			/*okIfFunctionNull*/ true );
	}
}


// -----------------------------------------------------------------------------
// Section: Python module functions
// -----------------------------------------------------------------------------

/*~ function BigWorld entity
 *  Returns the entity with the given id, or None if not found.
 *  This function can search all entities known to the client, or only entities
 *  which are not cached. An entity only becomes cached if in an online game
 *  the server indicates to the client that the entity has left the player's
 *  area of interest.
 *  @param id An integer representing the id of the entity to return.
 *  @param lookInCache An optional boolean which instructs the function to search
 *  for the entity amongst the entities which are cached. Otherwise, this
 *  function will only search amongst non-chached entities. This argument
 *  defaults to false.
 *  @return The entity corresponding to the id given, or None if no such
 *  entity is found.
 */
/**
 *	Returns the entity with the given id, or None if not found.
 *	The entity must be in the world (i.e. not cached).
 *	May want to rethink the decision stated in the line above.
 */
static PyObject * py_entity( PyObject * args )
{
	BW_GUARD;
	EntityID	id;
	int			lookInCache = 0;
	if (!PyArg_ParseTuple( args, "i|i", &id, &lookInCache ))
	{
		PyErr_SetString( PyExc_TypeError,
			"BigWorld.entity: Argument parsing error." );
		return NULL;
	}

	PyObject * pEntity = EntityManager::instance().getEntity(
		id, lookInCache != 0 );

	if (pEntity != NULL)
	{
		Py_INCREF( pEntity );
		return pEntity;
	}

	Py_Return;
}
PY_MODULE_FUNCTION( entity, BigWorld )

/*~ function BigWorld createEntity
 *  Creates a new entity on the client and places it in the world. The 
 *	resulting entity will have no base or cell part.
 *
 *  @param className	The string name of the entity to instantiate.
 *	@param spaceID		The id of the space in which to place the entity.
 *	@param vehicleID	The id of the vehicle on which to place the entity
 *						(0 for no vehicle).
 *  @param position		A Vector3 containing the position at which the new
 *						entity is to be spawned.
 *  @param direction	A Vector3 containing the initial orientation of the new
 *						entity (roll, pitch, yaw).
 *  @param state		A dictionary describing the initial states of the
 *						entity's properties (as described in the entity's .def
 *						file). A property will take on it's default value if it
 *						is not listed here.
 *  @return				The ID of the new entity, as an integer.
 *
 *  Example:
 *  @{
 *  # create an arrow style Info entity at the same position as the player
 *  p = BigWorld.player()
 *  direction = ( 0, 0, p.yaw )
 *  properties = { 'modelType':2, 'text':'Created Info Entity'}
 *  BigWorld.createEntity( 'Info', p.spaceID, 0, p.position, direction, properties )
 *  @}
 */
/**
 *	Creates a new client-side entity
 */
static PyObject * py_createEntity( PyObject * args )
{
	BW_GUARD;
	char * className = NULL;
	SpaceID spaceID = 0;
	EntityID vehicleID = 0;
	float x = 0; 
	float y = 0; 
	float z = 0;
	float roll = 0;
	float pitch = 0;
	float yaw = 0;
	PyObject * pDict = NULL;

	if (!PyArg_ParseTuple(args, "sii(fff)(fff)O!:BigWorld.createEntity",
			&className,
			&spaceID, &vehicleID, &x, &y, &z,
			&roll, &pitch, &yaw, &PyDict_Type, &pDict))
	{
		return NULL;
	}

	// Now find the index of this class
	EntityType * pType = EntityType::find( className );
	if (pType == NULL)
	{
		PyErr_Format( PyExc_ValueError, "BigWorld.createEntity: "
			"Class %s is not an entity class", className );
		return NULL;
	}

	// choose an ID for our entity
	EntityID id = EntityManager::nextClientID();

	// make an player entity if we have none
	bool makingPlayer = false;
	if (Player::entity() == NULL)
	{
		makingPlayer = true;

		// fake up a creation stream of base data (not so nice I know)
		MemoryOStream	stream( 4096 );
		pType->description().addDictionaryToStream( pDict, stream,
			EntityDescription::BASE_DATA |
			EntityDescription::CLIENT_DATA |
			EntityDescription::EXACT_MATCH );

		EntityManager::instance().onBasePlayerCreate( id, pType->index(),
			stream );
	}

	// fake up a creation stream of cell data (really sucks!)
	MemoryOStream	stream( 4096 );
	uint8 * pNumProps = (uint8*)stream.reserve( sizeof(uint8) );
	*pNumProps = 0;

	uint8 nprops = (uint8)pType->description().clientServerPropertyCount();
	for (uint8 i = 0; i < nprops; i++)
	{
		DataDescription * pDD = pType->description().clientServerProperty( i );
		if (!makingPlayer && !pDD->isOtherClientData()) continue;

		PyObjectPtr pVal = NULL;
		if (pDict != NULL)
			pVal = PyDict_GetItemString( pDict, (char*)pDD->name().c_str() );
		if (!pVal) pVal = pDD->pInitialValue();
		PyErr_Clear();

		stream << i;
		pDD->addToStream( &*pVal, stream, false );

		(*pNumProps)++;
	}

	if( makingPlayer )
	{	// In this case, the player entity has already been cached 
		// due to the onBasePlayerCreate call, so we should 
		// create the entity before entering it into the world.

		// create it / put in the cell data
		EntityManager::instance().onEntityCreate( id, pType->index(),
			spaceID, vehicleID, Position3D( x, y, z ), yaw, pitch, roll,
			stream );

		// and enter it unto the world
		EntityManager::instance().onEntityEnter( id, spaceID, vehicleID );
	}
	else
	{
		// In the case of all other entities, we should call onEntityEnter before
		// onEntityCreate, since the entities are not pre cached.

		// Enter it unto the world
		EntityManager::instance().onEntityEnter( id, spaceID, vehicleID );

		// create it / put in the cell data
		EntityManager::instance().onEntityCreate( id, pType->index(),
			spaceID, vehicleID, Position3D( x, y, z ), yaw, pitch, roll,
			stream );
	}

	return Script::getData( id );
}
PY_MODULE_FUNCTION( createEntity, BigWorld )



/*~ function BigWorld destroyEntity
 *  Destroys an exiting client-side entity.
 *  @param id The id of the entity to destroy.
 *
 *  Example:
 *  @{
 *  id = BigWorld.target().id # get current target ID
 *  BigWorld.destroyEntity( id )
 *  @}
 */
/**
 *	Destroys an existing client-side entity
 */
static PyObject * py_destroyEntity( PyObject * args )
{
	BW_GUARD;
	int id;

	if ( !PyArg_ParseTuple( args, "i", &id ) )
	{
		return NULL;
	}

	if (!EntityManager::isClientOnlyID( id ))
	{
		PyErr_Format( PyExc_ValueError, "BigWorld.destroyEntity: "
										"id is not a client only entity id" );
		return NULL;
	}

	EntityManager::instance().onEntityLeave( id );

	Py_Return;
}
PY_MODULE_FUNCTION( destroyEntity, BigWorld )




// entity_manager.cpp
