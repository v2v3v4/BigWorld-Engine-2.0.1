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

// Disable the 'truncated-identifier' warning for templates.
#pragma warning(disable: 4786)

#include <vector>
#include <set>

#include "moo/moo_math.hpp"

#include "network/basictypes.hpp"
#include "cstdmf/binary_stream.hpp"
#include "cstdmf/bgtask_manager.hpp"

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "pyscript/pickler.hpp"

#include "entity_type.hpp"	// for the pClass() accessor

#include "duplo/pymodel.hpp"
#include "duplo/py_attachment.hpp"
#include "duplo/chunk_embodiment.hpp"

#include "chunk/chunk_space.hpp"

class Filter;
class Matrix;
class DataDescription;
class ChunkSpace;
class Physics;
typedef SmartPointer<ChunkSpace> ChunkSpacePtr;

typedef SmartPointer<Filter> FilterPtr;

#ifndef PyModel_CONVERTERS
	PY_SCRIPT_CONVERTERS_DECLARE( PyModel )
	#define PyModel_CONVERTERS
#endif

class ResourceRef;
typedef std::vector<ResourceRef> ResourceRefs;

class Entity;
typedef SmartPointer<Entity> EntityPtr;


/**
 *	This helper class encapsulates the prerequisites of an entity instance.
 */
class PrerequisitesOrder : public BackgroundTask
{
public:
	PrerequisitesOrder( Entity * pEntity,
			const std::vector<std::string> & resourceIDs,
			const ResourceRefs & resourceRefs );

	bool loaded() const	{ return loaded_; };

	const ResourceRefs & resourceRefs() const { return resourceRefs_; }


protected:
	virtual void doBackgroundTask( BgTaskManager & mgr );

private:
	bool		loaded_;
	EntityPtr	pEntity_;	// to keep a reference

	std::vector<std::string>	resourceIDs_;
	ResourceRefs				resourceRefs_;
};

typedef SmartPointer< PrerequisitesOrder > PrerequisitesOrderPtr;


/*~ class BigWorld.Entity
 *
 *	This is the Client component of the Distributed Entity Model. 
 *	It is responsible for handling the visual aspects of the Entity,
 *	updates from the server, etc...
 *
 *	It is possible to create a client-only Entity using
 *	BigWorld.createEntity function.
 */
/**
 *	Instances of this class are client representation of entities,
 *	the basic addressable object in the BigWorld system.
 */
class Entity : public PyInstancePlus
{
	Py_InstanceHeader( Entity )

public:
	Entity( EntityType & pType, EntityID id, Vector3 & pos, float * pAuxVolatile,
		int enterCount, PyObject * pInitDict, Entity * pSister );

	~Entity();

	/* Entry and exit */
	bool checkPrerequisites();
	void enterWorld( SpaceID spaceID, EntityID vehicleID, bool transient );
	void leaveWorld( bool transient );
	void destroy();

	bool loadingPrerequisites() const { return loadingPrerequisites_; }

	/* Regular timekeeping */
	void tick( double timeNow, double timeLast );

	/* Script stuff */
	PY_METHOD_DECLARE( py_prints )

	PyObject * addModel( ChunkEmbodimentPtr pCA );
	PyObject * delModel( PyObjectPtr pEmbodimentPyObject );
	PY_AUTO_METHOD_DECLARE( RETOWN, addModel, NZARG( ChunkEmbodimentPtr, END ) )
	PY_AUTO_METHOD_DECLARE( RETOWN, delModel, NZARG( PyObjectPtr, END ) )

	PY_METHOD_DECLARE( py_addTrap )
	PY_METHOD_DECLARE( py_delTrap )
	//PY_METHOD_DECLARE( py_resetFilter )
	PY_AUTO_METHOD_DECLARE( RETDATA, hasTargetCap, ARG( uint, END ) )

	PY_AUTO_METHOD_DECLARE( RETOK, setInvisible, OPTARG( bool, true, END ) )
	bool setInvisible( bool invis = true );

	void setPortalState( bool isOpen, WorldTriangle::Flags collisionFlags );
	PY_AUTO_METHOD_DECLARE( RETVOID, setPortalState,
		ARG( bool, OPTARG( WorldTriangle::Flags, 0, END ) ) )

	void handleProperty( uint type, BinaryIStream & data );
	void handleMethod( uint type, BinaryIStream & data );

	void setProperty( const DataDescription * pDataDescription,
		PyObjectPtr pValue,
		bool shouldCallSetMethod = true );

	bool makePlayer();
	bool makeNonPlayer();

	void setClass( PyTypeObject * pClass );

	/* Targetting methods */
	void targetFocus();
	void targetBlur();

	/* Accessors */
	EntityID id() const					{ return id_; }

	Position3D & position()				{ return position_; }
	const Position3D & position() const	{ return position_; }
	void position( const Position3D & pos );

	Vector3 & velocity()				{ return velocity_; }
	const Vector3 & velocity() const	{ return velocity_; }
	void velocity( const Vector3 & pos );

	const float * auxVolatile() const	{ return auxVolatile_; }
	void pos( const Position3D & localPos,
		const float * auxVolatile, int numAux, const Vector3 & velocity = Vector3( 0.f, 0.f, 0.f) );
	EntityType & type()					{ return type_; }

	Filter & filter()					{ return *pFilter_; }

	bool isClientOnly() const;

	bool isPoseVolatile() const			{ return isPoseVolatile_; }
	PyObject * isPoseVolatile( bool v );

	ChunkEmbodiment * pPrimaryEmbodiment() const{ return &*primaryEmbodiment_; }
	PyModel * pPrimaryModel() const;

	ChunkEmbodiments & auxiliaryEmbodiments()	{ return auxiliaryEmbodiments_;}

	bool isDestroyed() const			{ return isDestroyed_; }

	bool isInWorld() const				{ return pSpace_; }
	ChunkSpacePtr pSpace() const		{ return pSpace_; }

	bool isOnVehicle() const			{ return !!pVehicle_; }
	Entity * pVehicle() const			{ return pVehicle_.getObject(); }
	void pVehicle( Entity * v )			{ pVehicle_ = v; }
	EntityID vehicleID() const		{ return pVehicle_ ? pVehicle_->id() : 0; }

	bool selfControlled() const			{ return pPhysics_ != (Physics*)-1; }
	Physics * pPhysics() const			{ return pPhysics_ != (Physics*)-1 ?
											pPhysics_ : NULL; }

	uint64 lastInvoked()				{ return lastInvoked_; }
	void lastInvoked( uint64 now )		{ lastInvoked_ = now; }

	int incrementEnter()				{ return ++enterCount_; }
	int decrementEnter()				{ return --enterCount_; }
	int enterCount() const				{ return enterCount_; }

	/*~ function Entity.hasTargetCap
	*
	*	This function checks whether this Entity has the given target capability.
	*
	*	@param	targetCap		(int)				The targeting capability to check against
	*
	*	@return					(bool)				Returns true if capability matches, false otherwise
	*/
	bool hasTargetCap( uint i ) const	{ return targetCapabilities_.has( i ); }
	const Capabilities & targetCaps() const		{ return targetCapabilities_; }

	const CacheStamps & cacheStamps() const		{ return cacheStamps_; }
	void cacheStamps( const CacheStamps & s )	{ cacheStamps_ = s; }

	/* Python Attributes */
	PY_RO_ATTRIBUTE_DECLARE( id_, id )
	PY_RO_ATTRIBUTE_DECLARE( isClientOnly(), isClientOnly )
	PY_RO_ATTRIBUTE_DECLARE( isDestroyed_, isDestroyed )
	PY_RO_ATTRIBUTE_DECLARE( position_, position )
	PY_RO_ATTRIBUTE_DECLARE( velocity_, velocity )
	PY_RO_ATTRIBUTE_DECLARE( auxVolatile_[0], yaw )
	PY_RO_ATTRIBUTE_DECLARE( auxVolatile_[1], pitch )
	PY_RO_ATTRIBUTE_DECLARE( auxVolatile_[2], roll )
	PY_RO_ATTRIBUTE_DECLARE( getPyCell(), server ) // TODO: Remove this.
	PY_RO_ATTRIBUTE_DECLARE( getPyCell(), cell )
	PyObject * pyGet_base();
	PY_RO_ATTRIBUTE_SET( base );

	PyObject * getPyCell();

	PY_RW_ATTRIBUTE_DECLARE( targetFullBounds_, targetFullBounds );
	bool	targetFullBounds() const { return targetFullBounds_; }

	PyObject * pyGet_filter();
	int pySet_filter( PyObject * value );

	PY_RO_ATTRIBUTE_DECLARE( isPoseVolatile_, isPoseVolatile )

	PyObject * pyGet_model();
	int pySet_model( PyObject * value );

	PY_RW_ATTRIBUTE_DECLARE( auxModelsHolder_, models );

	PY_READABLE_ATTRIBUTE_GET( targetCapabilities_, targetCaps )
	int pySet_targetCaps( PyObject * value );

	PY_RO_ATTRIBUTE_DECLARE( !!pSpace_, inWorld )
	PyObject * pyGet_spaceID();
	PY_RO_ATTRIBUTE_SET( spaceID );

	PY_RO_ATTRIBUTE_DECLARE( pVehicle_, vehicle );

	PyObject * pyGet_physics();
	int pySet_physics( PyObject * value );

	PyObject * pyGet_matrix();
	PY_RO_ATTRIBUTE_SET( matrix );

	/* Miscellaneous */
	const Matrix & fallbackTransform() const;

#if ENABLE_WATCHERS
	static Watcher & watcher();
#endif

	typedef std::set<Entity*> Census;
	static Census	census_;

	void swizzleClass( PyTypeObject * pOldClass, PyTypeObject * pNewClass );

	void updateProperties( BinaryIStream & stream,
		bool shouldCallSetMethod = true );

	void readCellPlayerData( BinaryIStream & stream );

	bool controlled( bool shouldBeControlled );
	double	physicsCorrected() const			{ return physicsCorrected_; }
	void	physicsCorrected( double atTime )	{ physicsCorrected_ = atTime; }

	void waitingForVehicle( EntityID vehicleID )
		{ waitingForVehicle_ = vehicleID; }

	bool isPhysicsAllowed() const;

	float transparency() const;
	bool  isInvisible() const { return invisible_; }

	bool tickCalled() const { return tickAdvanced_; }
	void setTickAdvanced() { tickAdvanced_ = true; }

	/**
	 *	Single entry in positional-sorted-linked list of entities.
	 */
	class Neighbour
	{
	public:
		void operator++(int)
		{
			BW_GUARD;
			pNext_ = pNext_->nextOfChain( chain_ );
			while (chain_ < 4 && (pNext_ == NULL ||
				((chain_ & 2) ?
					fabs(pNext_->position_[2] - pSource_->position_[2]):
					fabs(pNext_->position_[0] - pSource_->position_[0]))
				> radius_ ))
			{
				chain_++;
				this->nextChain();
			}
		}

		Entity * operator->()
		{
			return pNext_;
		}

		bool operator ==( const Neighbour & n )
		{
			return n.pSource_ == pSource_ && n.radius_ == radius_ &&
				n.chain_ == chain_ && (chain_ == 4 || n.steps_ == steps_);
		}

		bool operator !=( const Neighbour & n )
		{
			return !this->operator==( n );
		}

	private:
		Neighbour( Entity * pSource, float radius, bool end );

		void nextChain();

		Entity		* pSource_;
		float		radius_;

		int			chain_;
		int			steps_;
		Entity		*pNext_;

		friend class Entity;
	};

	Neighbour	begin( float radius );
	Neighbour	end( float radius );

	friend class Neighbour;

	void transformVehicleToCommon( Vector3 & pos, Vector3 & dir ) const;
	void transformCommonToVehicle( Vector3 & pos, Vector3 & dir ) const;

private:
	PyObject * pyGetAttribute( const char * attr );
	int pySetAttribute( const char * attr, PyObject * value );

	bool isATarget() const;

	bool loadingPrerequisites_;

	EntityID	id_;
	Position3D	position_;
	Vector3		velocity_;
	float		auxVolatile_[3];		// 0 - [Head] Yaw of Entity.
										// 1 - Head Pitch of Entity.
	EntityType & type_;

	PyObject	* pPyCell_;
	PyObject	* pPyBase_;

	FilterPtr	pFilter_;				// never NULL
	bool		isPoseVolatile_;

	bool		isDestroyed_;

	ChunkEmbodimentPtr	primaryEmbodiment_;

	ChunkEmbodiments	auxiliaryEmbodiments_;
	PySTLSequenceHolder<ChunkEmbodiments>	auxModelsHolder_;

	ChunkSpacePtr		pSpace_;
	EntityPtr			pVehicle_;

	Physics		* pPhysics_;
	double		physicsCorrected_;
	mutable EntityID	waitingForVehicle_;
	bool		tickAdvanced_;

	uint64		lastInvoked_;
	int			enterCount_;

	Capabilities	targetCapabilities_;
	//Capabilities	matchCapabilities_;

	int			nextTrapID_;
	struct TrapRecord
	{
		int			id;
		int			num;
		float		radiusSquared;
		PyObject	* function;
	};
	std::vector<TrapRecord>	traps_;

	Entity		* nextLeft_;
	Entity		* nextRight_;
	Entity		* nextUp_;
	Entity		* nextDown_;
	void		shuffle();
	Entity		* nextOfChain( int chain );

	CacheStamps	cacheStamps_;

	PrerequisitesOrderPtr		prerequisitesOrder_;
	ResourceRefs				resourceRefs_;

	bool		targetFullBounds_;

	bool invisible_;
	double visiblityTime_;
};

#ifndef Entity_CONVERTERS
#define Entity_CONVERTERS
PY_SCRIPT_CONVERTERS_DECLARE( Entity )
#endif



#ifdef CODE_INLINE
#include "entity.ipp"
#endif

#endif

/* entity.hpp */
