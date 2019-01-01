/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_HPP
#define ENTITY_HPP

#include "Python.h"

#include "pyscript/pyobject_plus.hpp"

#include "cellapp_interface.hpp"
#include "controller.hpp"
#include "entity_type.hpp"
#include "history_event.hpp"

#include "cstdmf/memory_stream.hpp"
#include "cstdmf/time_queue.hpp"

#include "entitydef/mailbox_base.hpp"
#include "entitydef/property_event_stamps.hpp"
#include "entitydef/property_owner.hpp"
#include "entitydef/py_volatile_info.hpp"

#include "physics2/worldtri.hpp" // For WorldTriangle::Flags
#include "pyscript/script.hpp"
#include "server/backup_hash.hpp"

#include <set>
#include <vector>
#include <map>

// Forward declarations
class Cell;
class CellApp;
class CellAppChannel;
class Chunk;
class ChunkSpace;
class Controllers;
class Entity;
class EntityCache;
class EntityExtra;
class EntityExtraInfo;
class EntityRangeListNode;
class EntityType;
class MemoryOStream;
class RangeTrigger;
class RealEntity;
class PropertyChange;
class Space;
class Watcher;

typedef SmartPointer< Watcher > WatcherPtr;

typedef uint8 AoIUpdateSchemeID;

typedef SmartPointer< Entity >                   EntityPtr;
typedef SmartPointer< PyObject >                 PyObjectPtr;
typedef std::set< EntityPtr >                    EntitySet;
typedef std::map< EntityID, EntityPtr >          EntityMap;
class EntityPopulation;

// From "space.hpp"
typedef std::vector< EntityPtr >				SpaceEntities;
typedef SpaceEntities::size_type				SpaceRemovalHandle;
const SpaceRemovalHandle NO_SPACE_REMOVAL_HANDLE = SpaceRemovalHandle( -1 );

enum ClientMethodCallingFlags
{
	MSG_FOR_OWN_CLIENT		= 0x01,		///< Send to own client
	MSG_FOR_OTHER_CLIENTS	= 0x02		///< Send to other clients
};

/**
 *	This interface is used to implement an object that wants to visit a set of
 *	entities.
 */
class EntityVisitor
{
public:
	virtual ~EntityVisitor() {};
	virtual void visit( Entity * pEntity ) = 0;
};


/*~ class BigWorld.Entity
 *  @components{ cell }
 *  Instances of the Entity class represent generic game objects on the cell.
 *  An Entity may be "real" or "ghosted", where a "ghost" Entity is a copy of
 *  of a "real" Entity that lives on an adjacent cell. For each entity there is
 *  one authoritative "real" Entity instance, and 0 or more "ghost" Entity
 *  instances which copy it.
 *
 *  An Entity instance handles the entity's positional data, including its
 *  space and rotation. It also controls how frequently (if ever) this data is
 *  sent to clients. Positional data can be altered by updates from an
 *  authoritative client, by controller objects, and by the teleport member
 *  function. Controllers are non-python objects that can be applied to
 *  cell entities to change their positional data over time. They are added
 *  to an Entity via member functions like "trackEntity" and "turnToYaw", and can
 *  be removed via "cancel".
 *
 *  Area of Interest, or "AoI" is an important concept for all BigWorld
 *  entities which belong to clients. The AoI of an entity is the area around
 *  it which the entity's client (if it has one) is aware of. This is used for
 *  culling the amount of data that the client is sent. The actual shape of an
 *  AoI is defined by a range of equal length on both the x and z axis, with a
 *  hysteresis area of similar shape extending out further. An Entity enters
 *  another Entity's AoI when it enters the AoI area, but doesn't leave it
 *  until it has moved outside the hysteresis area. An Entity can change its
 *  AoI size via "setAoIRadius".
 *  Entities within a particular distance can be found via "entitiesInRange",
 *  and changes to the set of entities within a given range can be observed via
 *  "addProximity".
 *
 *  The basis for a stealth system resides in Entity with noise event
 *  being handled by "makeNoise", and a movement noise handling system
 *  being accessible through "getStealthFactor" and "setStealthFactor".
 *
 *	A new Entity on a cellApp can be created using BigWorld.createEntity or
 *	BigWorld.createEntityFromFile function. An entity could also be spawned
 *	from remote baseApp BigWorld.createCellEntity function call.
 *
 *  An Entity can access the equivalent entity on base and client applications
 *  via a BaseEntityMailbox and a series of PyClient objects. These allow
 *  a set of function calls (which are specified in the entity's .def file) to
 *  be called remotely.
 */
/**
 *	Instances of this class are used to represent a generic game object on the
 *	cell. An entity may be <i>real</i> or <i>ghosted</i>. A <i>ghost</i> entity
 *	is an entity that is a copy of a <i>real</i> entity that lives on an
 *	adjacent cell.
 */
class Entity : public PyInstancePlus
{
	Py_InstanceHeader( Entity )

public:
	static const EntityPopulation & population()	{ return population_; }
	static void addWatchers();

	// Preventing NaN's getting through, hopefully
	static bool isValidPosition( const Position3D &c )
	{
		const float MAX_ENTITY_POS = 1000000000.f;
		return (-MAX_ENTITY_POS < c.x && c.x < MAX_ENTITY_POS &&
			-MAX_ENTITY_POS < c.y && c.y < MAX_ENTITY_POS &&
			-MAX_ENTITY_POS < c.z && c.z < MAX_ENTITY_POS);
	}

	// Preventing NaN's getting through, hopefully
	static bool isValidPosition( const Coord &c )
	{
		const float MAX_ENTITY_POS = 1000000000.f;
		return (-MAX_ENTITY_POS < c.x_ && c.x_ < MAX_ENTITY_POS &&
			-MAX_ENTITY_POS < c.y_ && c.y_ < MAX_ENTITY_POS &&
			-MAX_ENTITY_POS < c.z_ && c.z_ < MAX_ENTITY_POS);
	}

	/// @name Construction and Destruction
	//@{
	Entity( EntityTypePtr pEntityType );
	void setToInitialState( EntityID id, Space * pSpace );
	~Entity();

	bool initReal( BinaryIStream & data, PyObject * pDict,
		bool isRestore,
		Mercury::ChannelVersion channelVersion,
	   	EntityPtr pNearbyEntity );

	void initGhost( BinaryIStream & data );

	void offload( CellAppChannel * pChannel, bool isTeleport );

	void onload( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data );

	void createGhost( Mercury::Bundle & bundle );
	//@}

	void callback( const char * methodName );

	/// @name Accessors
	//@{
	EntityID id() const;
	void setShouldReturnID( bool shouldReturnID );
	const Position3D & position() const;
	const Direction3D & direction() const;

	const VolatileInfo & volatileInfo() const;

	bool isReal() const;
	bool isRealToScript() const;
	RealEntity * pReal() const;

	const Mercury::Address & realAddr() const;
	const Mercury::Address & nextRealAddr() const { return nextRealAddr_; }

	CellAppChannel * pRealChannel() { return pRealChannel_; }

	Space & space();
	const Space & space() const;

	Cell & cell();
	const Cell & cell() const;

	EventHistory & eventHistory();
	const EventHistory & eventHistory() const;

	bool isDestroyed() const;
	bool inDestroy() const		{ return inDestroy_; }
	void destroy();

	EntityTypeID entityTypeID() const;
	EntityTypeID clientTypeID() const;

	VolatileNumber volatileUpdateNumber() const
											{ return volatileUpdateNumber_; }

	float topSpeed() const				{ return topSpeed_; }
	float topSpeedY() const				{ return topSpeedY_; }

	uint8 physicsCorrections() const	{ return physicsCorrections_; }

	EntityRangeListNode * pRangeListNode() const;

	ChunkSpace * pChunkSpace() const;

	AoIUpdateSchemeID aoiUpdateSchemeID() const	{ return aoiUpdateSchemeID_; }

	//@}

	void incRef() const;
	void decRef() const;

	HistoryEvent * addHistoryEventLocally( uint8 type,
		MemoryOStream & stream, HistoryEvent::Level level,
		EntityMemberStats * pChangedDescription,
		const std::string * pName = NULL );

	void writeClientUpdateDataToBundle( Mercury::Bundle & bundle,
			const Vector3 & basePos,
			EntityCache & cache,
			float lodPriority ) const;

	void writeVehicleChangeToBundle( Mercury::Bundle & bundle,
		EntityCache & cache ) const;

	static void forwardMessageToReal( CellAppChannel & realChannel,
		EntityID entityID,
		uint8 messageID,
		BinaryIStream & data,
		const Mercury::Address & srcAddr, Mercury::ReplyID replyID );

	bool sendMessageToReal( const MethodDescription * pDescription,
			PyObject * args );

	const Mercury::Address & addrForMessagesFromReal() const;

	void trimEventHistory( GameTime cleanUpTime );

	void setPositionAndDirection( const Position3D & position,
		const Direction3D & direction );

	// DEBUG
	int numHaunts() const;

	INLINE EntityTypePtr pType() const;

	void reloadScript();	///< deprecated
	bool migrate();
	void migratedAll();

	/// @name Message handlers
	//@{
	void avatarUpdateImplicit(
		const CellAppInterface::avatarUpdateImplicitArgs & args );
	void avatarUpdateExplicit(
		const CellAppInterface::avatarUpdateExplicitArgs & args );
	void ackPhysicsCorrection(
		const CellAppInterface::ackPhysicsCorrectionArgs & args );

	void ghostAvatarUpdate(
		const CellAppInterface::ghostAvatarUpdateArgs & args );
	void ghostHistoryEvent( BinaryIStream & data );
	void ghostedDataUpdate( BinaryIStream & data );
	void ghostSetReal( const CellAppInterface::ghostSetRealArgs & args );
	void ghostSetNextReal(
		const CellAppInterface::ghostSetNextRealArgs & args );
	void delGhost( const CellAppInterface::delGhostArgs & args );

	void ghostVolatileInfo(
		const CellAppInterface::ghostVolatileInfoArgs & args );
	void ghostControllerCreate( BinaryIStream & data );
	void ghostControllerDelete( BinaryIStream & data );
	void ghostControllerUpdate( BinaryIStream & data );

	void witnessed( const CellAppInterface::witnessedArgs & args );
	void checkGhostWitnessed(
			const CellAppInterface::checkGhostWitnessedArgs & args );


	void aoiUpdateSchemeChange(
			const CellAppInterface::aoiUpdateSchemeChangeArgs & args );

	void delControlledBy( const CellAppInterface::delControlledByArgs & args );

	void forwardedBaseEntityPacket( BinaryIStream & data );

	void onBaseOffloaded( const Mercury::Address & srcAddr,
		const Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data );

	void teleport( const CellAppInterface::teleportArgs & args );

	void onTeleportSuccess( Entity * pNearbyEntity );

	void enableWitness( const Mercury::Address & srcAddr,
		Mercury::UnpackedMessageHeader & header, BinaryIStream & data );

	void witnessCapacity( const CellAppInterface::witnessCapacityArgs & args );

	void requestEntityUpdate( BinaryIStream & data );

	void writeToDBRequest( const Mercury::Address & srcAddr,
			Mercury::UnpackedMessageHeader & header, BinaryIStream & stream );

	void destroyEntity( const CellAppInterface::destroyEntityArgs & args );

	void runScriptMethod( BinaryIStream & data );

	void callBaseMethod( BinaryIStream & data );
	void callClientMethod( BinaryIStream & data );
	// General (script) message handler
	void runExposedMethod( int type, BinaryIStream & data );
	//@}

	/// @name Script related methods
	//@{
	PY_METHOD_DECLARE( py_destroy )

	PY_METHOD_DECLARE( py_cancel )
	PY_METHOD_DECLARE( py_isReal )
	PY_METHOD_DECLARE( py_isRealToScript )
	PY_METHOD_DECLARE( py_clientEntity )
	PY_METHOD_DECLARE( py_debug )

	PY_PICKLING_METHOD_DECLARE( MailBox )

	PY_AUTO_METHOD_DECLARE( RETOK, destroySpace, END );
	bool destroySpace();

	PY_AUTO_METHOD_DECLARE( RETOK, writeToDB, END );
	bool writeToDB();

	PY_AUTO_METHOD_DECLARE( RETOWN, entitiesInRange, ARG( float,
								OPTARG( PyObjectPtr, NULL ,
								OPTARG( PyObjectPtr, NULL, END ) ) ) )

	PyObject * entitiesInRange( float range,
			PyObjectPtr pClass = NULL, PyObjectPtr pActualPos = NULL );


	bool outdoorPropagateNoise( float range,
								int event,
							   	int info);

	PY_AUTO_METHOD_DECLARE( RETOK,
			makeNoise, ARG( float, ARG( int, OPTARG( int, 0, END ) ) ) )
	bool makeNoise( float noiseLevel, int event, int info=0);

	PY_AUTO_METHOD_DECLARE( RETOWN, getGroundPosition, END )
	PyObject * getGroundPosition( ) const;

	PY_AUTO_METHOD_DECLARE( RETOWN,
			bounceGrenade, ARG( Vector3, ARG( Vector3, ARG( float, ARG( float,
					ARG( float, ARG( int, OPTARG( int, -1, END ) ) ) ) ) ) ) )
	PyObject *
		bounceGrenade( const Vector3 & sourcePos, const Vector3 & velocity,
									float elasticity,
									float radius, float timeSample,
									int maxSamples, int maxBounces ) const;

	PY_RO_ATTRIBUTE_DECLARE( periodsWithoutWitness_, periodsWithoutWitness )
	PY_RO_ATTRIBUTE_DECLARE( pType()->name(), className );
	PY_RO_ATTRIBUTE_DECLARE( id_, id )

	PY_RO_ATTRIBUTE_DECLARE( isDestroyed_, isDestroyed )

	PyObject * pyGet_spaceID();
	PY_RO_ATTRIBUTE_SET( spaceID )

	SpaceID spaceID() const;

	// PY_READABLE_ATTRIBUTE_GET( globalPosition_, position )
	PyObject * pyGet_position();

	int pySet_position( PyObject * value );

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( Vector3, directionPy, direction )
	const Vector3 & directionPy() const;
	void directionPy( const Vector3 & newDir );

	PY_RO_ATTRIBUTE_DECLARE( globalDirection_.yaw, yaw )
	PY_RO_ATTRIBUTE_DECLARE( globalDirection_.pitch, pitch )
	PY_RO_ATTRIBUTE_DECLARE( globalDirection_.roll, roll )

	PY_RO_ATTRIBUTE_DECLARE( (Vector3 &)localPosition_, localPosition );
	PY_RO_ATTRIBUTE_DECLARE( localDirection_.yaw, localYaw );
	PY_RO_ATTRIBUTE_DECLARE( localDirection_.pitch, localPitch );
	PY_RO_ATTRIBUTE_DECLARE( localDirection_.roll, localRoll );

	PY_RO_ATTRIBUTE_DECLARE( pVehicle_, vehicle );

	bool isOutdoors() const;
	bool isIndoors() const;

	PY_RO_ATTRIBUTE_DECLARE( isOutdoors(), isOutdoors )
	PY_RO_ATTRIBUTE_DECLARE( isIndoors(), isIndoors )

	PY_READABLE_ATTRIBUTE_GET( volatileInfo_, volatileInfo )
	int pySet_volatileInfo( PyObject * value );

	PY_RW_ACCESSOR_ATTRIBUTE_DECLARE( bool, isOnGround, isOnGround )

	PyObject * pyGet_velocity();
	PY_RO_ATTRIBUTE_SET( velocity )

	PY_RW_ATTRIBUTE_DECLARE( topSpeed_, topSpeed )
	PY_RW_ATTRIBUTE_DECLARE( topSpeedY_, topSpeedY )

	PyObject * pyGet_aoiUpdateScheme();
	int pySet_aoiUpdateScheme( PyObject * value );

	PyObject * trackEntity( int entityId, float velocity = 2*MATH_PI,
			   int period = 10, int userArg = 0 );
	PY_AUTO_METHOD_DECLARE( RETOWN, trackEntity,
		ARG( int, OPTARG( float, 2*MATH_PI,
				OPTARG( int, 5, OPTARG( int, 0,  END ) ) ) ) )

	bool setPortalState( bool isOpen, WorldTriangle::Flags collisionFlags );
	PY_AUTO_METHOD_DECLARE( RETOK, setPortalState,
			ARG( bool, OPTARG( WorldTriangle::Flags, 0, END ) ) )

	PyObject * getDict();
	PY_AUTO_METHOD_DECLARE( RETOWN, getDict, END )

	bool sendToClient( const MethodDescription & description,
			MemoryOStream & argStream, bool isForOwn = true, bool isForOthers = false );
	bool sendToClientViaReal( const MethodDescription & description,
			MemoryOStream & argStream, bool isForOwn = true, bool isForOthers = false );
	//@}

	// The following is used by:
	//	void Space::addEntity( Entity * );
	//	void Space::removeEntity( Entity * );
	// It makes removing entities efficient.
	SpaceRemovalHandle removalHandle() const		{ return removalHandle_; }
	void removalHandle( SpaceRemovalHandle handle )	{ removalHandle_ = handle; }

	// This is just used in the Witness constructor.
	bool isInAoIOffload() const;
	void isInAoIOffload( bool isInAoIOffload );

	INLINE bool isOnGround() const;
	void isOnGround( bool isOnGround );

	static WatcherPtr pWatcher();

	static const Vector3 INVALID_POSITION;

	const Position3D & localPosition() const		{ return localPosition_; }
	const Direction3D & localDirection() const		{ return localDirection_; }

	void setLocalPositionAndDirection( const Position3D & localPosition,
			const Direction3D & localDirection );

	void setGlobalPositionAndDirection( const Position3D & globalPosition,
			const Direction3D & globalDirection );

	Entity * pVehicle() const;
	uint8 vehicleChangeNum() const;
	EntityID vehicleID() const	{ return pVehicle_ ? pVehicle_->id() : 0; }

	typedef uintptr SetVehicleParam;
	enum SetVehicleParamEnum
	{
		KEEP_LOCAL_POSITION,
		KEEP_GLOBAL_POSITION,
		IN_LIMBO
	};

	void setVehicle( Entity * pVehicle, SetVehicleParam keepWho );
	void onVehicleMove();

	NumTimesRealOffloadedType numTimesRealOffloaded() const
									{ return numTimesRealOffloaded_; }

	// void setClass( PyObject * pClass );

	EventNumber lastEventNumber() const;
	EventNumber getNextEventNumber();

	const PropertyEventStamps & propertyEventStamps() const;

	void debugDump();

	void fakeID( EntityID id );

	void addTrigger( RangeTrigger * pTrigger );
	void modTrigger( RangeTrigger * pTrigger );
	void delTrigger( RangeTrigger * pTrigger );

	bool hasBase() const { return baseAddr_.ip != 0; }
	const Mercury::Address & baseAddr() const { return baseAddr_; }

	void adjustForDeadBaseApp( const BackupHash & backupHash );

	void informBaseOfAddress( const Mercury::Address & addr, SpaceID spaceID,
		   bool shouldSendNow );

	/// @name PropertyOwnerLink method implementations
	//@{
	void onOwnedPropertyChanged( PropertyChange & change );

	bool getTopLevelOwner( PropertyChange & change,
			PropertyOwnerBase *& rpTopLevelOwner );

	int getNumOwnedProperties() const;
	PropertyOwnerBase * getChildPropertyOwner( int ref ) const;
	PyObjectPtr setOwnedProperty( int ref, BinaryIStream & data );
	//@}

	PyObjectPtr propertyByLocalIndex( int index ) const;

	Chunk * pChunk() const { return pChunk_; };
	Entity * prevInChunk() const { return pPrevInChunk_;}
	Entity * nextInChunk() const { return pNextInChunk_;}
	void prevInChunk( Entity* pEntity ) { pPrevInChunk_ = pEntity; }
	void nextInChunk( Entity* pEntity ) { pNextInChunk_ = pEntity; }
	void removedFromChunk();

	void heardNoise( const Entity * who, float propRange, float distance,
						int event, int info );

	ControllerID 	addController( ControllerPtr pController, int userArg );
	void			modController( ControllerPtr pController );
	bool			delController( ControllerID controllerID,
						bool warnOnFailure = true );

	static int registerEntityExtra(
		EntityExtra * (*touchFn)( Entity & e ) = NULL,
		PyDirInfo * pTouchDir = NULL );
	EntityExtra * & entityExtra( int eeid )		{ return extras_[eeid]; }
	EntityExtra * entityExtra( int eeid ) const	{ return extras_[eeid]; }

	void checkChunkCrossing();

	bool callback( const char * funcName, PyObject * args,
		const char * errorPrefix, bool okIfFunctionNull );

	static void callbacksPermitted( bool permitted );
	static bool callbacksPermitted()
		{ return s_callbacksPrevented_ <= s_allowCallbacksOverride; }

	static void nominateRealEntity( Entity & e );
	static void nominateRealEntityPop();

	static void s_init();

private:
	// Private methods
	Entity( const Entity & );

	void updateLocalPosition();
	bool updateGlobalPosition( bool shouldUpdateGhosts = true );

	void updateInternalsForNewPositionOfReal( const Vector3 & oldPos );
	void updateInternalsForNewPosition( const Vector3 & oldPosition );

	bool getEntitiesInRange( EntityVisitor & visitor, float range,
			PyObjectPtr pClass = NULL, PyObjectPtr pActualPos = NULL );
	void findEntitiesInSquare( float range, EntityVisitor & visitor ) const;

	void callScriptInit( bool isRestore, EntityPtr pNearbyEntity );

	void clearPythonProperties();

	bool readRealDataInEntityFromStreamForInitOrRestore( BinaryIStream & data,
		PyObject * pDict );

	void readGhostDataFromStream( BinaryIStream & data );
	void writeGhostDataToStream( BinaryOStream & stream ) const;

	void readGhostDataFromStreamInternal( BinaryIStream & data );
	void writeGhostDataToStreamInternal( BinaryOStream & stream ) const;

	void convertRealToGhost( BinaryOStream * pStream = NULL,
			CellAppChannel * pChannel = NULL,
			bool isTeleport = false );

	// Used by egextra/debug_extension.cpp
	friend PyObject * calcOffloadData( PyObjectPtr pEnt );
	void writeRealDataToStream( BinaryOStream & data,
			const Mercury::Address & dstAddr,
			bool isTeleport ) const;

	void convertGhostToReal( BinaryIStream & data,
		const Mercury::Address * pBadHauntAddr = NULL );

	void readRealDataFromStreamForOnload( BinaryIStream & data,
		const Mercury::Address * pBadHauntAddr = NULL );

	void readRealDataFromStreamForOnloadInternal( BinaryIStream & data,
			const Mercury::Address * pBadHauntAddr );
	void writeRealDataToStreamInternal( BinaryOStream & data,
					const Mercury::Address & dstAddr, bool isTeleport ) const;

	void setGlobalPosition( const Vector3 & v );

	void avatarUpdateCommon( const Coord & pos, const YawPitchRoll & dir,
		bool onGround, uint8 refNum );

	void setVolatileInfo( const VolatileInfo & newInfo );

	void writeVolatileDataToStream( Mercury::Bundle & bundle,
			const Vector3 & basePos, IDAlias idAlias,
			float priorityThreshold ) const;

	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	PyObject * pyAdditionalMembers( PyObject * pBaseSeq );
	PyObject * pyAdditionalMethods( PyObject * pBaseSeq );

	bool writeCellMessageToBundle( Mercury::Bundle & bundle,
		const MethodDescription * pDescription,
		PyObject * args ) const;

	bool writeClientMessageToBundle( Mercury::Bundle & bundle,
		const MethodDescription & description,
		MemoryOStream & argstream, int callingMode ) const;

	bool physicallyPossible( const Coord & newPosition, Entity * pVehicle,
		float propMove = 1.f );

	bool traverseChunks( Chunk * pCurChunk,
		const Chunk * pDstChunk,
		Vector3 cSrcPos, Vector3 cDstPos,
		std::vector< Chunk * > & visitedChunks );

	bool validateAvatarVehicleUpdate( Entity * pNewVehicle );

	void readGhostControllersFromStream( BinaryIStream & data );
	void writeGhostControllersToStream( BinaryOStream & stream ) const;

	void readRealControllersFromStream( BinaryIStream & data );
	void writeRealControllersToStream( BinaryOStream & stream ) const;

	void startRealControllers();
	void stopRealControllers();

	void runMethodHelper( BinaryIStream & data, int methodID, bool isExposed );

	bool sendDBDataToBase( const Mercury::Address * pReplyAddr = NULL,
		Mercury::ReplyID replyID = 0 );

	bool sendCellEntityLostToBase();

	// Private data
	Space *				pSpace_;

	// This handle is used to help the speed of Space::removeEntity.
	SpaceRemovalHandle	removalHandle_;

	EntityID		id_;
	EntityTypePtr	pEntityType_;
	Position3D		globalPosition_;
	Direction3D		globalDirection_;

	Position3D		localPosition_;
	Direction3D		localDirection_;

	Mercury::Address baseAddr_;

	Entity *		pVehicle_;
	uint8			vehicleChangeNum_;

	// Default scheme to use
	AoIUpdateSchemeID aoiUpdateSchemeID_; // uint8

	NumTimesRealOffloadedType numTimesRealOffloaded_; // uint16

	CellAppChannel * pRealChannel_;
	Mercury::Address nextRealAddr_;

	RealEntity * pReal_;

	typedef std::vector<PyObjectPtr>	Properties;
	Properties	properties_;

	PropertyOwnerLink<Entity>	propertyOwner_;

	EventHistory eventHistory_;

	bool		isDestroyed_;
	bool		inDestroy_;
	bool		isInAoIOffload_;
	bool		isOnGround_;

	VolatileInfo	volatileInfo_;
	VolatileNumber	volatileUpdateNumber_;

	float		topSpeed_;
	float		topSpeedY_;
	uint8		physicsCorrections_;
	uint64		physicsLastValidated_;
	float		physicsNetworkJitterDebt_;
	static float	s_maxPhysicsNetworkJitter_;

	PropertyEventStamps	propertyEventStamps_;

	EventNumber lastEventNumber_;

	EntityRangeListNode * pRangeListNode_;

	Controllers *			pControllers_;

	bool shouldReturnID_;

	EntityExtra ** extras_;
	static std::vector<EntityExtraInfo*> & s_entityExtraInfo();

	typedef std::vector<RangeTrigger*> Triggers;
	Triggers triggers_;

	// Used when deciding to call onWitnessed.
	// If periodsWithoutWitness_ is this value, the entity is considered not to
	// be witnessed. If periodsWithoutWitness_ gets to 2, the real entity is not
	// witnessed. If it gets to 3, neither the real or its ghosts are being
	// witnessed.
	enum { NOT_WITNESSED_THRESHOLD = 3 };
	mutable int		periodsWithoutWitness_;

	Chunk* pChunk_;
	Entity* pPrevInChunk_;
	Entity* pNextInChunk_;

	static EntityPopulation population_;

	friend class RealEntity;
	// friend void EntityRangeListNode::remove();
	friend class EntityRangeListNode;

	struct BufferedScriptCall
	{
		EntityPtr		entity;
		PyObject *		callable;
		PyObject *		args;
		const char *	errorPrefix;
	};
	static std::vector<BufferedScriptCall>	s_callbacksBuffer_;
	static int s_callbacksPrevented_;
	static int s_allowCallbacksOverride;
};


typedef bool (*CustomPhysicsValidator)( Entity * pEntity,
	const Vector3 & newLocalPos, Entity * pNewVehicle,
	double physValidateTimeDelta );
/**
 *	This function pointer can be set to a function to do further validation
 *	of physical movement than that provided by the BigWorld core code.
 *	The function is called after the speed has been validated, but before
 *	chunk portal traversals have been examined.
 *	It should return true if the move is valid. If it returns false, then
 *	a physics correction will be sent to the client controlling the entity.
 *	(note that even if it returns true, the move may still be disallowed
 *	by the following chunk portal traversal checks)
 */
extern CustomPhysicsValidator g_customPhysicsValidator;


typedef void (*EntityMovementCallback)( const Vector3 & oldPosition,
		Entity * pEntity );
/**
 *	This function pointer can be set to a function that is called whenever an
 *	entity moves. This can be useful when implementing things like custom range
 *	triggers or velocity properties via EntityExtra and Controller classes.
 */
extern EntityMovementCallback g_entityMovementCallback;

#ifdef CODE_INLINE
#include "entity.ipp"
#endif

#endif // ENTITY_HPP
