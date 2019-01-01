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

// Disable the 'truncated-identifier' warning for templates.
#pragma warning(disable: 4786)	// browse info too long
#pragma warning(disable: 4503)	// class name too long
#pragma warning(disable: 4355)	// 'this' used in initialiser

#include "entity.hpp"

#include "action_matcher.hpp"
#include "app.hpp"
#include "connection_control.hpp"
#include "dumb_filter.hpp"
#include "entity_manager.hpp"
#include "entity_picker.hpp"
#include "entity_type.hpp"
#include "filter.hpp"
#include "player.hpp"
#include "portal_state_changer.hpp"
#include "py_server.hpp"

#include "common/simple_client_entity.hpp"

#include "camera/base_camera.hpp"

#include "chunk/chunk_loader.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"

#include "connection/server_connection.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/diary.hpp"
#include "cstdmf/dogwatch.hpp"

#include "duplo/pymodel.hpp"

#include "pyscript/pyobject_base.hpp"
#include "pyscript/pywatcher.hpp"
#include "pyscript/script.hpp"

#include "romp/console_manager.hpp"
#include "romp/enviro_minder.hpp"
#include "romp/py_resource_refs.hpp"
#include "romp/resource_ref.hpp"
#include "romp/xconsole.hpp"

DECLARE_DEBUG_COMPONENT2( "Entity", 0 )


#ifndef CODE_INLINE
#include "entity.ipp"
#endif

// -----------------------------------------------------------------------------
// Section: Entity
// -----------------------------------------------------------------------------

// Static variable initialisation
//ModelManager & Entity::modelManager_ = ModelManager::instance();
Entity::Census Entity::census_;

#define ENTITY_COUNTERS

#ifdef ENTITY_COUNTERS
static uint32 s_entitiesCurrent = 0;
static uint32 s_entitiesEver = 0;
typedef std::map<std::string, std::pair<uint32, uint32> > ETypes;
static const std::auto_ptr<ETypes> s_entitiesOfType( new ETypes );
#endif

///	Constructor
Entity::Entity( EntityType & type, EntityID id, Vector3 & pos,
		float * pAuxVolatile, int enterCount, PyObject * pInitDict,
		Entity * pSister ) :
	PyInstancePlus( type.pClass(), true ),
	loadingPrerequisites_( false ),
	id_( id ),
	position_( pos ),
	type_( type ),
	pPyCell_( NULL ),
	pPyBase_( NULL ),
	pFilter_( new DumbFilter(), true ),
	isPoseVolatile_( false ),
	isDestroyed_( false ),
	primaryEmbodiment_( NULL ),
	auxiliaryEmbodiments_(),
	auxModelsHolder_( auxiliaryEmbodiments_, this, true ),
	pSpace_( NULL ),
	pVehicle_( NULL ),
	pPhysics_( EntityManager::isClientOnlyID( id ) ? NULL : (Physics*)-1 ),
	physicsCorrected_( 0.0 ),
	waitingForVehicle_( 0 ),
	tickAdvanced_( false ),
	lastInvoked_ ( 0 ),
	enterCount_( enterCount ),
	nextTrapID_( 0 ),
	nextLeft_( pSister != NULL ? pSister->nextLeft_ : NULL ),
	nextRight_( pSister ),
	nextUp_( pSister != NULL ? pSister->nextUp_ : NULL ),
	nextDown_( pSister ),
	prerequisitesOrder_( NULL ),
	invisible_( false ),
	visiblityTime_( 0 ),
	targetFullBounds_( false )
{
	BW_GUARD;
	pFilter_->owner( this );

	auxVolatile_[0] = pAuxVolatile[0];
	auxVolatile_[1] = pAuxVolatile[1];
	auxVolatile_[2] = pAuxVolatile[2];

	if (nextLeft_ != NULL)	nextLeft_->nextRight_ = this;
	if (nextRight_ != NULL)	nextRight_->nextLeft_ = this;
	if (nextUp_ != NULL)	nextUp_->nextDown_ = this;
	if (nextDown_ != NULL)	nextDown_->nextUp_ = this;

	this->shuffle();

	if (!&type_)
	{
		CRITICAL_MSG( "Entity::Entity: "
			"Type for type id %d, entity id %d is NULL!\n",
			type, id );
	}

	// Update our dictionary with the one passed in
	PyObject * pCurrDict = this->pyGetAttribute( "__dict__" );

	if (pInitDict == NULL || pCurrDict == NULL ||
		PyDict_Update( pCurrDict, pInitDict ) < 0)
	{
		PY_ERROR_CHECK()
	}

	Py_XDECREF( pInitDict );
	Py_XDECREF( pCurrDict );

	// Now that everything is in working order, create the script object

	Script::call( PyObject_GetAttrString( this, "__init__" ),
		PyTuple_New(0), "Entity::Entity: ", true );

	census_.insert( this );

#ifdef ENTITY_COUNTERS
	if (s_entitiesEver == 0)
	{
		MF_WATCH( "PyObjCounts/Entities",
			s_entitiesCurrent,
			Watcher::WT_READ_WRITE,
			"Current number of entities constructed and not yet destructed." );

		MF_WATCH( "PyObjCounts/EntitiesEver",
			s_entitiesEver,
			Watcher::WT_READ_WRITE,
			"Total number of entities constructed." );
	}
	s_entitiesCurrent++;
	s_entitiesEver++;

	ETypes::iterator it = s_entitiesOfType->find( type_.name() );
	if (it == s_entitiesOfType->end())
	{
		(*s_entitiesOfType)[type_.name()] = std::make_pair( 1,  1 );
		it = s_entitiesOfType->find( type_.name() );
#ifdef ENABLE_WATCHERS
		// Having the string built in the watch call broke VS2003 with watches disabled.
		std::string watchName = std::string("Entities/Counts/") + type_.name();
		std::string watchName2 = std::string("Entities/Totals/") + type_.name();
		MF_WATCH( watchName.c_str(), it->second.first );
		MF_WATCH( watchName2.c_str(), it->second.second );
#endif
	}
	else
	{
		it->second.first++;
		it->second.second++;
	}
#endif
}


/// Destructor
Entity::~Entity()
{
	BW_GUARD;
	census_.erase( this );

	if (pSpace_) this->leaveWorld( false );

	if (pPyCell_ != NULL)
	{
		((PyServer*)pPyCell_)->disown();
		Py_DECREF( pPyCell_ );
		pPyCell_ = NULL;
	}

	if (pPyBase_ != NULL)
	{
		((PyServer*)pPyBase_)->disown();
		Py_DECREF( pPyBase_ );
		pPyBase_ = NULL;
	}

	if (pFilter_)
	{
		pFilter_->owner( NULL );
		pFilter_ = NULL;
	}


	if (nextLeft_	!= NULL)	nextLeft_	->nextRight_	= nextRight_;
	if (nextRight_	!= NULL)	nextRight_	->nextLeft_		= nextLeft_;
	if (nextUp_		!= NULL)	nextUp_		->nextDown_		= nextDown_;
	if (nextDown_	!= NULL)	nextDown_	->nextUp_		= nextUp_;

#ifdef ENTITY_COUNTERS
	s_entitiesCurrent--;
	ETypes::iterator it = s_entitiesOfType->find( type_.name() );
	MF_ASSERT_DEV(it != s_entitiesOfType->end());
	it->second.first--;
#endif
}


// -----------------------------------------------------------------------------
// Section: Entry and exit
// -----------------------------------------------------------------------------


/**
 *	Constructor.
 */
PrerequisitesOrder::PrerequisitesOrder( Entity * pEntity,
		const std::vector<std::string> & resourceIDs,
		const ResourceRefs & resourceRefs ) :
	loaded_( false ),
	pEntity_( pEntity ),
	resourceIDs_( resourceIDs ),
	resourceRefs_( resourceRefs )
{
	BW_GUARD;	
}


/**
 *	This method loads all the prerequisites described in the given order.
 *
 *	Note: This method runs in a loading thread, and thus may not modify
 *	the entity at all (not even its reference count)
 */
void PrerequisitesOrder::doBackgroundTask( BgTaskManager & mgr )
{
	BW_GUARD;
	for (uint i = 0; i < resourceIDs_.size(); i++)
		resourceRefs_.push_back( ResourceRef::getOrLoad( resourceIDs_[i] ) );

	loaded_ = true;
}

/*~ callback Entity.prerequisites
 *
 *	This callback method is called before the entity enters the world.  It asks
 *	the entity for a list of resources that are required by the entity.  The
 *	entity is not added to the world until its prerequisite resources are
 *	loaded.
 *	The loading of prerequisite resources is done in a background thread,
 *	meaning that entities can thus enter the world without causing a loading
 *	pause to the main rendering thread.
 *
 *	The entity should return a tuple of resource IDs that represent its list
 *	of prerequisites.
 */
/**
 *	This function checks whether the prerequisites for this entity entering the
 *	world are satisfied. It retrieves this list from a Python callback
 *	Entity.prerequisites. If they are, it returns true. Otherwise, it starts
 *	the process of satisfying them (if not yet started) and returns false.
 *
 *	Note: This method assumes that if it returns true and it has a positive
 *	enterCount, then it will definitely be entered into the world very soon.
 */
bool Entity::checkPrerequisites()
{
	BW_GUARD;
	// if we already have some resources then we are fine
	if (!resourceRefs_.empty())
	{
		WARNING_MSG( "Entity::checkPrerequisites: "
			"Unexpectedly called for %d which already has resource refs\n",
			id_ );
		loadingPrerequisites_ = false;
		return true;
	}

	// if there's one in progress see if it's done
	if (prerequisitesOrder_ != NULL)
	{
		if (!prerequisitesOrder_->loaded())
			return false;

		// keep the resources around if we will enter the world
		if (enterCount_ > 0)
			resourceRefs_ = prerequisitesOrder_->resourceRefs();

		// it's done! now never call us again...
		prerequisitesOrder_ = NULL;
		loadingPrerequisites_ = false;
		return true;
	}

	// otherwise ask it for its prerequisites
	PyObject * res = Script::ask(
		PyObject_GetAttrString( this, "prerequisites" ),
		PyTuple_New(0),
		"Entity::prerequisites: ",
		true );

	// and find out who is there
	ResourceRefs presentPrereqs;
	std::vector<std::string> missingPrereqs;

	bool good = false;
	if (res != NULL && PySequence_Check( res ))
	{
		std::string prereq;

		int sz = PySequence_Size( res );
		for (int i = 0; i < sz; i++)
		{
			PyObject * pItem = PySequence_GetItem( res, i );
			int itemOk = Script::setData( pItem, prereq ) == 0;
			Py_DECREF( pItem );
			if (!itemOk) break;

			ResourceRef rr = ResourceRef::getIfLoaded( prereq );
			if (rr)
				presentPrereqs.push_back( rr );
			else
				missingPrereqs.push_back( prereq );
		}
		good = true;
	}
	if (!good)
	{
		if (res != NULL)
		{		// if not good and have res, then error in res
			PyErr_SetString( PyExc_TypeError, "prerequisites response: "
				"expected a sequence of strings" );
		}
		// only have an error here if one was
		// set if res is NULL and no error set
		// then just have no prereqs
		if (PyErr_Occurred())
		{
			PyErr_PrintEx(0);
			PyErr_Clear();

			presentPrereqs.clear();
			missingPrereqs.clear();
		}
	}
	Py_XDECREF(res);

	// see if we have them all
	if (missingPrereqs.empty())
	{
		// keep the resources around if we will enter the world
		if (enterCount_ > 0)
			resourceRefs_ = presentPrereqs;

		loadingPrerequisites_ = false;
		return true;
	}

	// otherwise start loading them
	prerequisitesOrder_ =
		new PrerequisitesOrder( this, missingPrereqs, presentPrereqs );

	BgTaskManager::instance().addBackgroundTask( prerequisitesOrder_ );

	loadingPrerequisites_ = true;
	return false;
}



/*~ callback Entity.onEnterWorld
 *
 *	This method is called when the entity enters into the player's AoI. On the
 *	client, this means that the entity has just been placed in the world.
 *	Most of the entity's initialisation code should be placed in this method
 *	instead of the entity's __init__ method because at the time that the __init__
 *	method is called the entity has not been placed in the world. Furthermore,
 *	the enterWorld method can be called multiple times for the same entity
 *	because the entity is reused if it exits the player's AoI and re-enters
 *	it within a short period of time.
 *
 *	It has a PyResourceRefs object passed in as the first parameter.  This
 *	object holds onto the prerequisites that were loaded from the prerequisites
 *	method.  It is up to the script programmer to manage the lifetime of the
 *	prerequisites.
 *
 *	The leaveWorld method is called when the entity leaves the AoI.
 */
/*~ callback Entity.enterWorld
 *
 *	This method is called when the entity enters into the player's AoI. On the
 *	client, this means that the entity has just been placed in the world.
 *	Most of the entity's initialisation code should be placed in this method
 *	instead of the entity's __init__ method because at the time that the __init__
 *	method is called the entity has not been placed in the world. Furthermore,
 *	the enterWorld method can be called multiple times for the same entity
 *	because the entity is reused if it exits the player's AoI and re-enters
 *	it within a short period of time.
 *
 *	This callback has been deprecated, instead use onEnterWorld.
 *
 *	The leaveWorld method is called when the entity leaves the AoI.
 */
/*~ callback Entity.onEnterSpace
 *
 *	This function is called when the entity changes space. This can occur, for
 *	example, when other entities teleport between spaces with the player
 *	entity.
 */
/**
 *	This function is called by the entity manager to place this entity
 *	physically in the world.
 *
 *	Note that at the time that an entity's script's __init__ function is
 *	called, the entity will not be in the world.
 *
 *	The transient parameter is set to true if the entity is returning to
 *	the world after a transient absence, i.e. usually from another space.
 */
void Entity::enterWorld( SpaceID spaceID, EntityID vehicleID, bool transient )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( !pSpace_ )
	{
		leaveWorld( true );
	}

	IF_NOT_MF_ASSERT_DEV( spaceID != NULL_CHUNK_SPACE )
	{
		return;
	}

	pSpace_ = ChunkManager::instance().space( spaceID );
	pVehicle_ = NULL;
	if (vehicleID != 0)
	{
		pVehicle_ = EntityManager::instance().getEntity( vehicleID, true );
		if (pVehicle_ == NULL || !pVehicle_->isInWorld() )
		{
			ERROR_MSG( "Entity::enterWorld: "
				"Got entity %d before carrying vehicle %d "
				"(going into limbo)\n", id_, vehicleID );
			// but do nothing, the filter will keep trying to put us in
			// the vehicle every frame until we can find it here
		}
	}

	if (primaryEmbodiment_)
	{
		primaryEmbodiment_->enterSpace( pSpace_, transient );
	}

	for (ChunkEmbodiments::iterator iter = auxiliaryEmbodiments_.begin();
		iter != auxiliaryEmbodiments_.end();
		iter++)
	{
		(*iter)->enterSpace( pSpace_, transient );
	}

	if (!transient )
	{
		std::string desc ("Entity ");
		desc += this->type_.name();
		DiaryScribe ds( Diary::instance(), desc );

		if (this == Player::entity())
			pPhysics_ = NULL;

		//Entity now prefers onEnterWorld( self, prerequisites ) to
		//enterWorld( self )
		if (PyObject_HasAttrString( this, "onEnterWorld" ))
		{
			PyObject * args = PyTuple_New(1);
			PyTuple_SET_ITEM( args, 0, new PyResourceRefs(resourceRefs_) );
			Script::call( PyObject_GetAttrString( this, "onEnterWorld" ),
				args,
				"Entity::onEnterWorld: ",
				true );

			//If onEnterWorld is used, then the C++ entity no longer manages
			//the lifetime of its prerequisites, it is up to the script (they
			//can hold onto the PyResourceRefs as long as they like.)
			resourceRefs_.clear();
		}
		else
		{
			Script::call( PyObject_GetAttrString( this, "enterWorld" ),
				PyTuple_New(0),
				"Entity::enterWorld: ", true );

			//If the deprecated enterWorld is used, then the C++ entity
			//manages the lifetime of its prerequisites, they exist as long
			//as the entity remains in the world.
			WARNING_MSG("(%s) The use of enterWorld is deprecated.  Please use"
				" onEnterWorld( prerequisites ) instead, where prerequisites "
				"is an instance of PyResourceRefs.\n", type_.name().c_str() );
		}

		if (this == Player::entity())
			Player::instance().updateWeatherParticleSystems(
				pSpace_->enviro().playerAttachments() );

	}
	else
	{
		if (this->isATarget())
		{
			EntityPicker::instance().check();
		}

		Script::call( PyObject_GetAttrString( this, "onEnterSpace" ),
			PyTuple_New(0),
			"Entity::onEnterSpace: ", true );
	}
}


/*~ callback Entity.onLeaveWorld
 *
 *	This function is called when the entity leaves the player's AoI. On the
 *	client, this means that the entity is about to be removed from the world.
 *	The entity may not be destroyed immediately after this call. The same
 *	entity may be re-used if it re-enters the player's AoI a short time after
 *	leaving.
 */
/*~ callback Entity.leaveWorld
 *
 *	This function is called when the entity leaves the player's AoI. On the
 *	client, this means that the entity is about to be removed from the world.
 *	The entity may not be destroyed immediately after this call. The same
 *	entity may be re-used if it re-enters the player's AoI a short time after
 *	leaving.
 *
 *	This callback has been deprecated, instead use onLeaveWorld.
 */
/*~ callback Entity.onLeaveSpace
 *
 *	This function is called when the entity changes space. This can occur, for
 *	example, when other entities teleport between spaces with the player
 *	entity.
 */
/**
 *	This function is called by the entity manager to remove this entity
 *	physically from the world. Tick will no longer be called.
 *
 *	The transient parameter is set if the entity is only leaving the world
 *	for a short time and will return very soon (before any other functions
 *	on the entity are called). This happens when an entity changes spaces.
 */
void Entity::leaveWorld( bool transient )
{
	BW_GUARD;
	if (!transient)
	{
		// first tell the entity about it
		if (PyObject_HasAttrString( this, "onLeaveWorld" ))
		{
			Script::call( PyObject_GetAttrString( this, "onLeaveWorld" ),
				PyTuple_New(0),
				"Entity::onLeaveWorld: ", true );
		}
		else
		{
			Script::call( PyObject_GetAttrString( this, "leaveWorld" ),
				PyTuple_New(0),
				"Entity::leaveWorld: ", true );

			WARNING_MSG("(%s) The use of leaveWorld is deprecated.  Please use"
				" onLeaveWorld instead.\n", type_.name().c_str() );
		}

		if (this == Player::entity() && pPhysics_ != (Physics*)-1)
		{
			if (pPhysics_ != NULL)
			{
				pPhysics_->disown();
				Py_DECREF( pPhysics_ );
			}
			pPhysics_ = (Physics*)-1;
		}

		// let the picker know we're no longer available
		if (this->isATarget())
		{
			EntityPicker::instance().clear();
		}

		// release all our resource refs
		resourceRefs_.clear();
	}
	else
	{
		Script::call( PyObject_GetAttrString( this, "onLeaveSpace" ),
			PyTuple_New(0),
			"Entity::onLeaveSpace: ", true );
	}

	// remove auxiliary models
	for (ChunkEmbodiments::iterator iter = auxiliaryEmbodiments_.begin();
		iter != auxiliaryEmbodiments_.end();
		iter++)
	{
		(*iter)->leaveSpace( transient );
	}

	// remove primary model
	if (primaryEmbodiment_)
	{
		primaryEmbodiment_->leaveSpace( transient );
	}

	if (!transient)
	{
		// clear all traps
		for (uint i=0; i < traps_.size(); i++)
		{
			Py_DECREF( traps_[i].function );
		}
		traps_.clear();

		// get rid of physics if we have any
		if (this->selfControlled() && pPhysics_ != NULL)
		{
			pPhysics_->disown();
			Py_DECREF( pPhysics_ );
			pPhysics_ = NULL;
		}
	}

	MF_ASSERT_DEV( pSpace_ );

	pSpace_ = NULL;
}


/**
 * This method clears the dictionary of the entity and
 * marks it as destroyed. Subsequent attempts to access
 * script attributes other than 'id', 'isDestroyed',
 * 'inWorld' or language defined attributes will raise
 * an EntityIsDestroyedException.
 */
void Entity::destroy()
{
	BW_GUARD;
	PyObject * entityDict = PyObject_GetAttrString( this, "__dict__" );
	if (entityDict != NULL)
	{
		PyDict_Clear(entityDict);
		Py_DECREF(entityDict);
	}
	else
	{
		PyErr_Clear();
	}

	isDestroyed_ = true;
}


// -----------------------------------------------------------------------------
// Section: Regular timekeeping
// -----------------------------------------------------------------------------

typedef VectorNoDestructor<PyObject*> StolenPyObjects;
template <>
void PySTLSequenceHolder<StolenPyObjects>::commit()
{
	// this method doesn't compile 'coz there's no erase
	// using iterators in VectorNoDestructor
}

/**
 *	This function is called periodically by the EntityManager to give
 *	time to this entity.
 */
void Entity::tick( double timeNow, double timeLast )
{
	BW_GUARD;
	if (tickAdvanced_)
	{
		// tick is already called by a passenger
		// clear the flag.
		tickAdvanced_ = false;
		return;
	}

	if ( pVehicle_ && !pVehicle_->tickCalled()) 
 	{ 
 		pVehicle_->tick( timeNow, timeLast ); 
 		pVehicle_->setTickAdvanced(); 
	}

	static DogWatch dwTick("Tick");
	dwTick.start();

	// update our position (and other data) if it needs updating
	if (pFilter_)
	{
		pFilter_->output( timeNow );
	}

	// go through all our traps and call any that have gone off
	for (uint i = 0; i < traps_.size(); i++)
	{
		TrapRecord & tr = traps_[i];

		int	numIn = 0;

		static StolenPyObjects trapped;
		static PySTLSequenceHolder<StolenPyObjects>
			trappedHolder( trapped, NULL, false );
		trapped.clear();

		// find out how many are inside
		Entities & ens = EntityManager::instance().entities();
		for (Entities::iterator iter = ens.begin(); iter != ens.end(); iter++)
		{
			if (iter->second == this) continue;

			float distSq = (iter->second->position() -
				position_).lengthSquared();
			if (distSq <= tr.radiusSquared)
			{
				trapped.push_back( iter->second );
				numIn++;
			}

			// we do this loop around this way for pure convenience of
			//  coding. multiple traps are not expected (or very useful)
		}

		// is its state different?
		if (numIn != tr.num)
		{
			tr.num = numIn;

			Py_INCREF( tr.function );
			PyObject * pTuple = PyTuple_New(1);
			PyTuple_SetItem( pTuple, 0, Script::getData( trappedHolder ) );
			Script::call( tr.function, pTuple, "Entity::tick.trap: " );

			// Note: anything might have happened to traps_ now.
			// We attempt to continue nevertheless, but some might be missed
			// It should be rare for an entity to have a trap anyway...
			// and very rare for it to have more than one, so this is OK.
		}
	}

	// rev the motors of our models
	float dTime = float(timeNow - timeLast);

	if (primaryEmbodiment_)
	{
		primaryEmbodiment_->move( dTime );
	}

	for (ChunkEmbodiments::iterator iter = auxiliaryEmbodiments_.begin();
		iter != auxiliaryEmbodiments_.end();
		iter++)
	{
		(*iter)->move( dTime );
	}


	// If we're the player call this to send our position to the server.
	// At some near time in the future this'll be tied to a 100ms
	// Mercury timer so it gets sent as fast as it's supposed to
	// instead of every frame as it does here.
	if (this->selfControlled())
		Player::instance().poseUpdateNotification( this );

	dwTick.stop();
}


// -----------------------------------------------------------------------------
// Section: Script stuff
// -----------------------------------------------------------------------------

PY_BASETYPEOBJECT( Entity )

PY_BEGIN_METHODS( Entity )
	PY_METHOD( prints )
	PY_METHOD( addModel )
	PY_METHOD( delModel )
	PY_METHOD( addTrap )
	PY_METHOD( delTrap )
	PY_METHOD( hasTargetCap )
	PY_METHOD( setInvisible )
	PY_METHOD( setPortalState )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( Entity )

	/*~ attribute Entity.id
	 *	Unique Entity identifier, shared across Cell, Base and Client
	 *	@type Read-Only float
	 */
	PY_ATTRIBUTE( id )

	/*~ attribute Entity.isClientOnly
	 *	This property is True if it is a client-only entity.
	 *	@type Read-Only Boolean
	 */
	PY_ATTRIBUTE( isClientOnly )

	/**	attribute Entity.isDestroyed
	 *	This property will be True if the entity has been destroyed.
	 *	For example when an entity leave the AoI. Accessing properties
	 *	other than 'Entity.id' or 'Entity.isDestroyed' will raise an
	 *	EntityIsDestroyedException.
	 *	@type Read-Only Boolean
	 */
	PY_ATTRIBUTE( isDestroyed )

	/*~ attribute Entity.position
	 *	Current position of Entity in world
	 *	@type Read-Only Vector3
	 */
	PY_ATTRIBUTE( position )

	/*~ attribute Entity.yaw
	 *	Current yaw of Entity
	 *	@type Read-Only float
	 */
	PY_ATTRIBUTE( yaw )

	/*~ attribute Entity.pitch
	 *	Current pitch of Entity
	 *	@type Read-Only float
	 */
	PY_ATTRIBUTE( pitch )

	/*~ attribute Entity.roll
	 *	Current roll of Entity
	 *	@type Read-Only float
	 */
	PY_ATTRIBUTE( roll )

	// TODO: Remove this.
	/*~ attribute Entity.server
	 *	Communicates with Cell component of this Entity.
	 *	Deprecated:  The cell attribute should be used instead.
	 *	@type Read-Only PyServer
	 */
	PY_ATTRIBUTE( server )

	/*~ attribute Entity.cell
	 *	Communicates with Cell component of this Entity.
	 *	@type Read-Only PyServer
	 */
	PY_ATTRIBUTE( cell )

	/*~ attribute Entity.base
	 *	Communicates with Base component if Base is Proxy, otherwise this attribute is not available.
	 *	@type Read-Only PyBase
	 */
	PY_ATTRIBUTE( base )

	/*~ attribute Entity.model
	 *	Current Model visible to user
	 *	@type Read-Only PyModel
	 */
	PY_ATTRIBUTE( model )

	/*~ attribute Entity.models
	 *	List of PyModels attached to Entity
	 *	@type list of PyModels
	 */
	PY_ATTRIBUTE( models )

	/*~ attribute Entity.filter
	 *	Movement interpolation filter for this Entity.
	 *	@type Filter
	 */
	PY_ATTRIBUTE( filter )

	/*~ attribute Entity.isPoseVolatile
	 *	On initialisation, if the definition for this Entity includes a volatile clause,
	 *	this attribute will be true.  If the components of the volatileInfo attribute on
	 *	the Cell entity are cleared, then this value will become false and the onPoseVolatile
	 *	notification method will be triggered.  This is defined as follows:
	 *	@{
	 *		def onPoseVolatile( self, isVolatile ):
	 *	@}
	 *	where isVolatile will be false (0).  The notfication method will also be called when the
	 *	volatileInfo attribute is set after being cleared.  In this case, isVolatile will be true.
	 *	This method is also called when control of the entity's position by this client is
	 *	granted or withdrawn, for both server-side entities and client-side entities.
	 *	They can be distinguished by checking if the 'physics' attribute is available.
	 *	(It is not called in this case for the Player entity as the client always controls it)
	 *	@type Read-Only bool
	 */
	PY_ATTRIBUTE( isPoseVolatile )

	/*~ attribute Entity.targetCaps
	 *	Stores the current targeting capabilities for this Entity.  These will be compared against
	 *	another set of targeting capabilities to determine whether this Entity should be included
	 *	in a list of potential targets.
	 *	@type list of integers
	 */
	PY_ATTRIBUTE( targetCaps )

	/*~ attribute Entity.inWorld
	 *	True when Entity is in the world (ie, added to a BigWorld Space), false when it is not
	 *	@type Read-Only bool
	 */
	PY_ATTRIBUTE( inWorld )

	/*~ attribute Entity.spaceID
	 *	Stores ID of BigWorld Space in which this Entity resides
	 *	@type Read-Only int
	 */
	PY_ATTRIBUTE( spaceID )

	/*~ attribute Entity.vehicle
	 *	Stores the vehicle Entity to which this Entity is attached.  None if not on vehicle.
	 *	@type Read-Only Entity
	 */
	PY_ATTRIBUTE( vehicle )

	/*~ attribute Entity.physics
	 *	When this Entity becomes controlled, this attribute becomes available to allow a Physics
	 *	object to be attached to this Entity.  The physics attribute is not available if this
	 *	Entity is not controlled.  See BigWorld.controlEntity().
	 *
	 *  Assigning an int representing one of the supported physics styles to the
	 *  physics property of an entity creates and attaches a Physics object to
	 *  the entity.  The constants for the different physics styles are
	 *  accessible in the BigWorld module.
	 *
	 *	Current physics styles are as follows:
	 *
	 *		STANDARD_PHYSICS:	Suitable for player, other controlled animals, etc.
	 *		HOVER_PHYSICS:		Hovering vehicle physics.
	 *		CHASE_PHYSICS:		Causes controlled Entity to follow exact movements
	 *							of Entity being chased. Has no effect unless
	 *							Physics::chase() activated.
	 *		TURRET_PHYSICS:		Useful for controlled turrets, guns, etc.
	 *
	 *	Other custom physics can be integrated into BigWorld and assigned their own integer
	 *	creation flag.  Once set, the appropriate physics module will be created and assigned to
	 *	this attribute.  The appropriate Physics style can then be accessed through this attribute.
	 *
	 *	@type Write-Only int/Physics
	 */
	PY_ATTRIBUTE( physics )

	/*~ attribute Entity.matrix
	 *	Current matrix representation of this Entity in the world.
	 *	@type Read-Only EntityMProv
	 */
	PY_ATTRIBUTE( matrix )

	/*~	attribute Entity.targetFullBounds
	 *	If true, targeting uses the full bounding box of this Entity. This, however,
	 *	will allow the player to target an entity using the extreme corners of the
	 *	bounding box (which may be out in the middle of open space). Setting this to
	 *	false will shrink the bounding box used for testing, and will also only select 
	 *	the entity if two points on the bounding box (one at the top and one at the 
	 *	bottom) are visible.
	 *
	 *	See the watcher values "Client Settings/Entity Picker/Lower Pct." and
	 *	"Client Settings/Entity Picker/Sides Pct." for the shrink amount control 
	 *	factors.
	 *
	 *	@type bool
	 */
	PY_ATTRIBUTE( targetFullBounds )

PY_END_ATTRIBUTES()

PY_SCRIPT_CONVERTERS( Entity )


	/*~ function Entity.prints
	*
	*	This function writes the Entity's ID, followed by the given string to the BigWorld console
	*
	*	@param	string			(string)			The string to display in the console
	*
	*	@return					(none)
	*/
/**
 * This method prints the message appended to the entity's id
 */
PyObject * Entity::py_prints( PyObject * args )
{
	BW_GUARD;
	char * msg;
	if (!PyArg_ParseTuple( args, "s", &msg ))
	{
		ERROR_MSG( "Entity::py_print: Could not parse the arguments\n" );
		PyErr_PrintEx(0);

		PyErr_SetString( PyExc_TypeError, "prints expects 1 string argument" );
		return NULL;
	}

	char fullMessage[BUFSIZ];
	bw_snprintf( fullMessage, sizeof(fullMessage), "%d> ", id_ );

	char * restMessage = fullMessage + strlen(fullMessage);
	strncpy( restMessage, msg, BUFSIZ-16 );
	restMessage[ BUFSIZ-16 ] = 0;

	strcat( fullMessage, "\n" );

	INFO_MSG( "%s", fullMessage );
	ConsoleManager::instance().find( "Python" )->print( fullMessage );

	Py_Return;
}


	/*~ function Entity.addModel
	*
	*	This function adds a PyModel to this Entity's list of usable Models
	*
	*	@param	model			(PyModel)			The PyModel to add to the list
	*
	*	@return					(none)
	*/
/**
 *	This method adds the given model to our list of models
 */
PyObject * Entity::addModel( ChunkEmbodimentPtr pCA )
{
	BW_GUARD;
	auxiliaryEmbodiments_.push_back( pCA );
	if (pSpace_) pCA->enterSpace( pSpace_ );

	Py_Return;
}


	/*~ function Entity.delModel
	*
	*	This function removes a PyModel to this Entity's list of usable Models
	*
	*	@param	model			(PyModel)			The PyModel to remove from the list
	*
	*	@return					(none)
	*/
/**
 *	This method removes the given model from our list of models
 */
PyObject * Entity::delModel( PyObjectPtr pEmbodimentPyObject )
{
	BW_GUARD;
	ChunkEmbodiments::iterator iter;
	for (iter = auxiliaryEmbodiments_.begin();
		iter != auxiliaryEmbodiments_.end();
		iter++)
	{
		if ((*iter)->pPyObject() == pEmbodimentPyObject) break;
	}

	if (iter == auxiliaryEmbodiments_.end())
	{
		PyErr_SetString( PyExc_ValueError, "Entity.delModel: "
			"Embodiment not added to this Entity." );
		return NULL;
	}

	if (pSpace_) (*iter)->leaveSpace();
	auxiliaryEmbodiments_.erase( iter );

	Py_Return;
}


	/*~ function Entity.addTrap
	*
	*	This function adds a sphere of awareness around this Entity, invoking a callback method whenever the
	*	contents of the Trap changes.  The callback method is provided upon creation of the Trap and should have
	*	the following prototype;
	*
	*		def trapCallback( self, entitiesInTrap ):
	*
	*	The "entitiesInTrap" argument is a list containing references to the entities contained inside the trap.
	*
	*	BigWorld.addPot is a cheaper method that only works for players.
	*
	*	@see BigWorld.addPot
	*
	*	@param	radius			(float)				The radius of the sphere around this Entity
	*	@param	function		(float)				The callback function
	*
	*	@return					(integer)			The ID of the newly created Trap
	*/
/**
 *	This method registers a trap function to be called when an entity
 *	(not us) enters it
 */
PyObject * Entity::py_addTrap( PyObject * args )
{
	BW_GUARD;
	if (!pSpace_)
	{
		PyErr_SetString( PyExc_EnvironmentError, "py_addTrap: "
			"Traps not allowed when not in world" );
		return NULL;
	}

	float		radius = 0.f;
	PyObject *	function = NULL;

	if (!PyArg_ParseTuple( args, "fO", &radius, &function ) ||
			!PyCallable_Check( function ) )
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.addTrap: "
			"Argument parsing error." );
		return NULL;
	}

	Py_INCREF( function );

	TrapRecord	newTR = { nextTrapID_++, 0, radius*radius, function };
	traps_.push_back( newTR );

	return PyInt_FromLong( newTR.id );
}


	/*~ function Entity.delTrap
	*
	*	This function removes a Trap (sphere of Entity awareness) from around the player.
	*
	*	@param	trapID			(float)				The ID of the Trap to remove
	*
	*	@return					(none)
	*/
/**
 *	This method deregisters a trap function
 */
PyObject * Entity::py_delTrap( PyObject * args )
{
	BW_GUARD;
	int		id;
	if (!PyArg_ParseTuple( args, "i", &id ))
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.delTrap: "
			"Argument parsing error." );
		return NULL;
	}

	for (uint i=0; i < traps_.size(); i++)
	{
		if (traps_[i].id == id)
		{
			Py_DECREF( traps_[i].function );
			traps_.erase( traps_.begin() + i );
			Py_Return;
		}
	}

	PyErr_SetString( PyExc_ValueError, "py_delTrap: "
		"No such trap." );
	return NULL;
}

/**
 *	This method resets the filter of this entity. It also inputs the current
 *	position 100ms in the past, so that the next update will be interpolated
 *	to rather than snapped to.
 */
/*
PyObject * Entity::py_resetFilter( PyObject * args )
{
	if(pFilter_)
	{
		pFilter_->reset( getGameTotalTime() );
		Vector3 v = position_;
		pFilter_->input( getGameTotalTime() - 0.1, v, auxVolatile_);
	}

	Py_Return;
}
*/

/**
 *	This method returns an attribute associated with this object.
 */
PyObject * Entity::pyGetAttribute( const char * attr )
{
	BW_GUARD;

	if (this->isDestroyed_ &&
		!(attr[0] == '_' && attr[1] == '_') &&
		strcmp( attr, "isDestroyed" ) != 0 &&
		strcmp( attr, "inWorld" ) != 0 &&
		strcmp( attr, "id" ) != 0)
	{
		char errorMessage[256];
		bw_snprintf( errorMessage, ARRAY_SIZE(errorMessage)-1, "%s %d has been destroyed", type_.name().c_str(), id_ );
		errorMessage[ARRAY_SIZE(errorMessage)-1] = '\0';

		PyErr_SetString( EntityManager::instance().entityIsDestroyedExceptionType(), errorMessage );
		return NULL;
	}

	// See if it's one of our methods or attributes
	PY_GETATTR_STD();

	if (!strcmp( attr, ".chunkSpace" ))
		return Script::getData( (int)(&*pSpace_) );

	// Do the default then
	return PyInstancePlus::pyGetAttribute( attr );
}


/**
 *	This method sets an attribute associated with this object.
 */
int Entity::pySetAttribute( const char * attr, PyObject * value )
{
	BW_GUARD;

	if (this->isDestroyed_ &&
		!(attr[0] == '_' && attr[1] == '_') &&
		strcmp( attr, "isDestroyed" ) != 0 &&
		strcmp( attr, "inWorld" ) != 0 &&
		strcmp( attr, "id" ) != 0)
	{
		char errorMessage[256];
		bw_snprintf( errorMessage, ARRAY_SIZE(errorMessage)-1, "%s %d has been destroyed", type_.name().c_str(), id_ );
		errorMessage[ARRAY_SIZE(errorMessage)-1] = '\0';

		PyErr_SetString( EntityManager::instance().entityIsDestroyedExceptionType(), errorMessage );
		return -1;
	}

	// See if it's one of our standard attributes
	PY_SETATTR_STD();

	// Do the default then
	return PyInstancePlus::pySetAttribute( attr, value );
}


/**
 *	Specialised get method for the 'model' attribute
 */
PyObject * Entity::pyGet_model()
{
	BW_GUARD;
	if (primaryEmbodiment_)
		return Script::getData( &**this->pPrimaryEmbodiment() );

	Py_Return;
}

/*~ callback Entity.targetModelChanged
 *
 *	This callback method is invoked on the player entity when the primary
 *	model of currently targeted entity has changed.  This allows the player
 *	script to reset any trackers, gui components or special effects refering to
 *	the targets old model.
 *
 *	The currently targetted entity is passed as a parameter.
 */
/**
 *	Specialised set method for the 'model' attribute
 */
int Entity::pySet_model( PyObject * value )
{
	BW_GUARD;
	// use setData to do most of our work
	ChunkEmbodimentPtr oldBody = primaryEmbodiment_;
	int ret = Script::setData( value, primaryEmbodiment_, "Entity.model" );
	if (ret != 0) return ret;

	// take the old one out of the space if it was in
	if (oldBody && pSpace_)
		oldBody->leaveSpace();

	// fix up the new primary embodiment
	if (primaryEmbodiment_)
	{
		if (pSpace_)
			primaryEmbodiment_->enterSpace( pSpace_ );

		// give primary models their own motors.
		// we don't attempt to retrieve the ActionMatcher from the
		//  old model, so scripts will have to watch out for this,
		//  (as any settings made in it will be lost)
		PyModel * pModel = this->pPrimaryModel();
		if (pModel != NULL)
		{
			PyObject * pTuple = PyTuple_New(1);
			PyTuple_SetItem( pTuple, 0, new ActionMatcher( this ) );
			pModel->pySet_motors( pTuple );		// I'm too lazy to write
			Py_DECREF( pTuple );				// a C++ function for this :)

			// re-attach rain particles if this model belongs to the player
			if (this == Player::instance().entity())
			{
				MF_ASSERT_DEV( pSpace_ );

				if( pSpace_ )
					Player::instance().updateWeatherParticleSystems(
						pSpace_->enviro().playerAttachments() );
			}
		}
	}

	// and let the player know if it has us targeted
	if (this->isATarget() && Player::entity())
	{
		Script::call(
			PyObject_GetAttrString( Player::entity(), "targetModelChanged" ),
			Py_BuildValue( "(O)", (PyObject*)this ),
			"Entity::targetModelChanged: ",
			true );
	}

	return 0;
}


/**
 *	Get method for the 'cell' attribute
 */
PyObject * Entity::getPyCell()
{
	BW_GUARD;
	if (pPyCell_ == NULL)
	{
		pPyCell_ =
			new PyServer( this, this->type().description().cell(), false );
	}

	return pPyCell_;
}


/**
 *	Get method for the 'base' attribute
 */
PyObject * Entity::pyGet_base()
{
	BW_GUARD;
	if (EntityManager::instance().pServer() &&
		EntityManager::instance().pServer()->connectedID() != id_)
	{
		PyErr_SetString( PyExc_TypeError,
			"Entity.base is only available on the connected entity" );
		return NULL;
	}
	if (pPyBase_ == NULL)
	{
		pPyBase_ =
			new PyServer( this, this->type().description().base(), true );
	}
	return Script::getData( pPyBase_ );
}

/**
 *	Get method for the 'filter' attribute
 *	(not in header 'coz script converters not declared there)
 */
PyObject * Entity::pyGet_filter()
{
	BW_GUARD;
	return Script::getData( pFilter_ );
}

/**
 *	Specialised set method for the 'filter' attribute
 */
int Entity::pySet_filter( PyObject * value )
{
	BW_GUARD;
	// make sure it's a filter
	if (!Filter::Check( value ))
	{
		PyErr_SetString( PyExc_TypeError,
			"Entity.filter must be set to a Filter" );
		return -1;
	}

	FilterPtr pNewFilter = static_cast< Filter * >( value );

	// and change to using it if it's different
	if (pNewFilter != pFilter_)
	{
		double time = App::instance().getTime();
		SpaceID topSID;
		EntityID topVID;
		Vector3 topPos;
		Vector3 topErr( Vector3::zero() );
		float topAuxVol[3] = { 0.0f, 0.0f, 0.0f };
		const bool hadInput = pFilter_->getLastInput( time, topSID, topVID, topPos, topErr, topAuxVol );
		pFilter_->owner( NULL );
		pFilter_ = pNewFilter;
		pFilter_->owner( this );

		pFilter_->reset( time );
		if ( hadInput )
		{
			pFilter_->input( time, topSID, topVID, topPos, topErr, topAuxVol );
		}
	}

	return 0;
}


/**
 *	Specialised set method for the 'targetCaps' attribute
 */
int Entity::pySet_targetCaps( PyObject * value )
{
	BW_GUARD;
	int ret = Script::setData( value, targetCapabilities_, "targetCaps" );
	if (ret == 0 && this->isATarget())
	{
		EntityPicker::instance().check();
	}
	return ret;
}


/**
 *	Specialised set method for the 'matchCoupled' attribute
 */
/*
int Entity::pySet_matcherCoupled( PyObject * value )
{
	bool isPlayer = (Player::entity() == this);

	bool	oldMC = matcherCoupled_;
	int ret = Script::setData( value, matcherCoupled_, "matcherCoupled" );
	if (isPlayer && ret == 0)
	{
		if (primaryModel_ != NULL && inheritOnRecouple_ &&
			matcherCoupled_ && !oldMC)
		{
			// If we're the player and we have a primary model
			//  and we just turned ON the matcher coupling from being
			//  off, copy the model's world position into ours.
			// Scripts should really call teleport on physics before
			//  this time to give other clients the chance to have the
			//  right position by the time they've finished the action.
			this->pos( primaryModel_->worldTransform().applyToOrigin() );
		}

		inheritOnRecouple_ = true;
	}
	return ret;
}
*/

/**
 *	Specialised get method for the 'spaceID' attribute
 */
PyObject * Entity::pyGet_spaceID()
{
	BW_GUARD;
	if (pSpace_)
		return Script::getData( pSpace_->id() );

	Py_Return;
}


/**
 *	Specialised get method for the 'physics' attribute
 */
PyObject * Entity::pyGet_physics()
{
	BW_GUARD;
	if (this->selfControlled())
	{
		return Script::getData( this->pPhysics() );
	}
	else
	{
		PyErr_SetString( PyExc_AttributeError,
			"Only controlled entities have a 'physics' attribute" );
		return NULL;
	}
}

/**
 *	Specialised set method for the 'physics' attribute
 */
int Entity::pySet_physics( PyObject * value )
{
	BW_GUARD;
	if(	!( this->selfControlled() ||
		( ConnectionControl::instance().server() != NULL &&
		strlen( ConnectionControl::instance().server() ) == 0 ) ) )
	{
		PyErr_SetString( PyExc_AttributeError,
			"Only controlled entities have a 'physics' attribute" );
		return -1;
	}


	if(	value == Py_None &&
		ConnectionControl::instance().server() != NULL &&
		strlen( ConnectionControl::instance().server() ) == 0 )
	{
		if ( this->pPhysics() != NULL)
		{
			pPhysics_->disown();
			Py_DECREF( pPhysics_ );
		}

		pPhysics_ = NULL;
		return 0;
	}


	int style;
	int ret = Script::setData( value, style, "physics" );
	if (ret == 0)
	{
		if ( this->pPhysics() != NULL)
		{
			pPhysics_->disown();
			Py_DECREF( pPhysics_ );
		}

		pPhysics_ = new Physics( this, style );
	}

	return ret;
}

typedef WeakPyPtr<Entity> EntityWPtr;

/**
 *	Entity matrix provider class
 */
class EntityMProv : public MatrixProvider
{
	Py_Header( EntityMProv, MatrixProvider )

public:
	EntityMProv( Entity * pEntity, PyTypePlus * pType = &s_type_ ) :
		MatrixProvider( false, pType ),
		pEntity_( pEntity ),
		notModel_( false )
	{
		BW_GUARD;	
	}

	~EntityMProv()
	{
		BW_GUARD;	
	}

	virtual void matrix( Matrix & m ) const
	{
		BW_GUARD;
		if (!pEntity_)
		{
			m.setIdentity();
		}
		else if (!notModel_)
		{
			m = pEntity_->fallbackTransform();
		}
		else
		{
			m.setRotate(	pEntity_->auxVolatile()[0],
							-pEntity_->auxVolatile()[1],
							pEntity_->auxVolatile()[2] );
			m.translation( pEntity_->position() );
		}
	}

	PY_RW_ATTRIBUTE_DECLARE( notModel_, notModel )

	PyObject * pyGetAttribute( const char * attr )
	{
		BW_GUARD;
		PY_GETATTR_STD();
		return MatrixProvider::pyGetAttribute( attr );
	}

	int pySetAttribute( const char * attr, PyObject * value )
	{
		BW_GUARD;
		PY_SETATTR_STD();
		return MatrixProvider::pySetAttribute( attr, value );
	}

private:
	EntityWPtr	pEntity_;
	bool		notModel_;
};

/*~	class BigWorld.EntityMProv
 *
 *	This is a special MatrixProvider for an Entity.
 *	It is useful if the Entity and Model matrix differ.
 */
PY_TYPEOBJECT( EntityMProv )

PY_BEGIN_METHODS( EntityMProv )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( EntityMProv )

	/*~	attribute EntityMProv.notModel
	 *
	 *	If the Model and entity position are expected to be different throughout the course of their respective
	 *	lifetimes, then this attribute can be used to flag which should be regarded as the authoritative in-game
	 *	position.  If true, this MatrixProvider will track the Entity's matrix;  If false, it will always provide
	 *	the Model's.  It can be changed at any time to switch between the two and defaults to false, since the
	 *	Model is the visual representation of the Entity on the Client.
	 *
	 *	@type bool
	 */
	PY_ATTRIBUTE( notModel )

PY_END_ATTRIBUTES()


/**
 *	Specialised get method for the 'matrix' attribute
 */
PyObject * Entity::pyGet_matrix()
{
	BW_GUARD;
	return new EntityMProv( this );
}


/**
 *
 */
bool Entity::isPhysicsAllowed() const
{
	BW_GUARD;
	if (physicsCorrected_ != 0.f)
	{
		return false;
	}

	if (waitingForVehicle_)
	{
		Entity * pVehicle =
			EntityManager::instance().getEntity( waitingForVehicle_ );

		if (pVehicle)
		{
			waitingForVehicle_ = 0;

			// const_cast< Entity * >( this )->pVehicle( pVehicle );
			const_cast< Entity * >( this )->filter().output( App::instance().getTime() );
		}
	}

	return waitingForVehicle_ == 0;
}


/**
 * This method is called to get the visiblity level of the player.
 */
float Entity::transparency() const
{
	BW_GUARD;
	float tdiff = float( max( 0.0, App::instance().getTime() - visiblityTime_ ) );
	return tdiff >= 2.0 ? (invisible_ ? 0.8f : -1.f)
						: (invisible_ ? 0.0f + tdiff / 2.5f : 0.8f - tdiff / 2.5f);
}

/*~ function Entity.setInvisible
 *
 *	This function fades the visibility of the player in and out. It can only be
 *	called on a Player entity.
 *
 *	Raises a TypeError if this method is not called on the player entity.
 *
 *	@param	invisible	(bool)	True to make Entity invisible, False to
 *								make it reappear
 */
/**
 * This method is called when the player becomes invisible.
 */
bool Entity::setInvisible( bool invis )
{
	BW_GUARD;
	if ( this != Player::entity() )
	{
		PyErr_SetString( PyExc_TypeError,
			"You can only call this method on a player" );
		return false;
	}
	else if (invisible_ != invis)
	{
		invisible_ = invis;

		// If the previous setting hadn't finished fading in, we need to
		// account for it in the visibility time so the reverse fade is smooth.
		double timeNow = App::instance().getTime();
		double tdiff = min( 2.0, max( 0.0, timeNow - visiblityTime_ ) );
		visiblityTime_ = timeNow - (2.0 - tdiff);
	}

	return true;
}

/*~ function Entity.setPortalState
 *  @components{ client }
 *  The setPortalState function allows an entity to control the state of a
 *	portal. If the portal is not permissive, it is added to the collision scene.
 *
 *	The position of the entity must be within 1 metre of the portal that is
 *	to be configured, otherwise the portal will probably not be found. No
 *	other portal may be closer.
 *
 *	The cell entity has an identical method. This also adds an object into the
 *	collision scene and disables navigation through the doorway when the portal
 *	is closed.
 *
 *	@param permissive	Whether the portal is open or not.
 *  @param triFlags Optional argument that sets the flags on the portal's
 *		triangles as returned from collision tests.
 */
/**
 *	This method implements the Python method to allow changing the state of a
 *	nearby portal.
 */
void Entity::setPortalState( bool isOpen, WorldTriangle::Flags collisionFlags )
{
	PortalStateChanger::changePortalCollisionState( this,
		isOpen, collisionFlags );
}


/**
 *	This method handles a change to a property of the entity sent
 *  from the server.
 */
void Entity::handleProperty( uint type, BinaryIStream & data )
{
	BW_GUARD;
	SimpleClientEntity::propertyEvent( this, this->type().description(),
		type, data, /*callSetForTopLevel:*/true );

	/*
	if (index < this->type().description().clientServerPropertyCount())
	{
		const DataDescription * pDataDescription = this->type().
			description().clientServerProperty( index );

		if (pDataDescription != NULL)
		{
			PyObjectPtr pValue = pDataDescription->createFromStream( data );

			MF_ASSERT_DEV( pValue );

			if (pValue != NULL)
			{
				this->setProperty( pDataDescription, pValue );
			}
		}
	}
	else
	{
		ERROR_MSG( "Entity::handleProperty: "
				"property index %d is out of range for type %s\n",
			index, this->type().name().c_str() );
	}
	*/
}

/**
 *	This method handles a call to a method of the entity sent
 *  from the server.
 */
void Entity::handleMethod( uint index, BinaryIStream & data )
{
	BW_GUARD;
	SimpleClientEntity::methodEvent( this, this->type().description(),
		index, data );
}


/**
 *	This method sets the described property of the script.
 */
void Entity::setProperty( const DataDescription * pDataDescription,
	PyObjectPtr pValue,
	bool shouldCallSetMethod )
{
	BW_GUARD;
	std::string	propName = pDataDescription->name();

//	DEBUG_MSG( "Entity::setProperty( '%s' )\n", propName.c_str() );

	// remember the old value
	PyObject * pOldValue = PyObject_GetAttrString(
		this, const_cast<char*>( propName.c_str() ) );

	// make it none (should only happen for OWN_CLIENT) properties
	// the first time they are set
	if (pOldValue == NULL)
	{
		// Actually, this can also happen when using LoDs. If an entity enters,
		// it may not have properties at higher LoDs set yet.
		//MF_ASSERT_DEV( this == Player::entity() );

		PyErr_Clear();
		pOldValue = Py_None;
		Py_INCREF( pOldValue );
	}


	// now set the value
	PyObject_SetAttrString( this, const_cast<char*>( propName.c_str() ),
		&*pValue );

	// if this is yaw, put it into the filter. (TODO: not this!)
	if (propName == "yaw")
	{
		float yaw = float(PyInt_AsLong( &*pValue )) * MATH_PI / 128.f;
		if (pFilter_)
		{
			double time = App::instance().getTime();
			SpaceID topSID;
			EntityID topVID;
			Vector3 topPos = position_;
			Vector3 topErr( Vector3::zero() );
			float topAuxVol[3] =
				{ auxVolatile_[0], auxVolatile_[1], auxVolatile_[2] };
			pFilter_->getLastInput( time, topSID, topVID, topPos, topErr, topAuxVol );
			topAuxVol[0] = yaw;
			pFilter_->input( time+0.00001, topSID, topVID, topPos, topErr, topAuxVol );
		}
		else
		{
			auxVolatile_[0] = yaw;
		}
	}

	if (shouldCallSetMethod)
	{
		// then see if there's a set handler for it
		std::string methodName = "set_" + propName;
		PyObject * pMethod = PyObject_GetAttrString( this,
			const_cast< char * >( methodName.c_str() ) );

		// and call it if there is
		if (pMethod != NULL)
		{
			Script::call( pMethod, Py_BuildValue( "(O)", pOldValue ),
				"Entity::setProperty: " );
		}
		else
		{
			PyErr_Clear();
		}
	}

	Py_DECREF( pOldValue );
}


/**
 *	This method sets the Python class that is associated with this object.
 */
void Entity::setClass( PyTypeObject * pClass )
{
	BW_GUARD;
	MF_ASSERT_DEV( PyObject_IsSubclass( (PyObject *)pClass,
		(PyObject *)&Entity::s_type_ ) );
	MF_VERIFY( this->pySetAttribute( "__class__", (PyObject *)pClass ) != -1 );
}


/*~ callback Entity.onBecomePlayer
 *
 *	This callback method is called in response to a call to BigWorld.player()
 *	It signals that an entity has become the player, and its script has become
 *	an instance of the player class, instead of its original avatar class.
 *
 *	@see BigWorld.player
 *	@see Entity.onBecomeNonPlayer
 */
/**
 *	This method changes the Python class of this object to be the player class.
 */
bool Entity::makePlayer()
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( this->ob_type == type_.pClass() )
	{
		// most likely already a player - presume we've already been called
		return true;
	}

	if (type_.pPlayerClass() == NULL)
	{
		ERROR_MSG( "Entity::makePlayer: "
			"Entity of type %s cannot be made a player.\n",
			this->typeName() );
		return false;
	}

	this->setClass( type_.pPlayerClass() );
	Script::call( PyObject_GetAttrString( this, "onBecomePlayer" ),
		PyTuple_New(0), "Player::onBecomePlayer: ", true );

	return true;
}


/*~ callback Entity.onBecomeNonPlayer
 *
 *	This callback method is called in response to a call to BigWorld.player()
 *	It signals that an entity is no longer the player, and its script is just
 *	about to become an instance its original class, instead of the player
 *	avatar class.
 *
 *	@see BigWorld.player
 *	@see Entity.onBecomePlayer
 */
/**
 *	This method changes the Python class of this object to be the non-player
 *	class.
 */
bool Entity::makeNonPlayer()
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( this->ob_type == type_.pPlayerClass() )
	{
		// most likely no longer a player - presume we've already been called
		return true;
	}

	Script::call( PyObject_GetAttrString( this, "onBecomeNonPlayer" ),
		PyTuple_New(0), "Player::onBecomeNonPlayer: ", true );
	this->setClass( type_.pClass() );

	return true;
}


// -----------------------------------------------------------------------------
// Section: Targetting methods
// -----------------------------------------------------------------------------

/**
 *	This method returns whether or not this entity is a target.
 */
bool Entity::isATarget() const
{
	BW_GUARD;
	return EntityPicker::instance().pGeneralTarget() == this;
}


/*~ callback Entity.targetFocus
 *
 *	This callback method is called in response to a change in the targeted
 *	entity.  It notifies the player that an entity has just become the current
 *	target.
 *
 *	@param The newly targetted entity.
 */
/**
 *	Called when we become a target
 */
void Entity::targetFocus()
{
	BW_GUARD;
	// If the Player Entity exists, inform this Entity that it has been targeted by the Player.
	// The Player may not exist if logging off whilst targeting an Entity, however, this does then
	// provide a warning that the Entity Picker is being destructed with a non-NULL current entity...
	if ( Player::entity() )
	{
		Script::call( PyObject_GetAttrString( Player::entity(), "targetFocus" ),
			Py_BuildValue( "(O)", (PyObject*)this ), "Entity::targetFocus: ", true );
	}

	// possibly call our own script here too
}


/*~ callback Entity.targetBlur
 *
 *	This callback method is called in response to a change in the targeted
 *	entity.  It notifies the player that an entity has is no longer the current
 *	target.
 *
 *	@param The orginially targetted entity.
 */
/**
 *	Called when we stop being a target
 */
void Entity::targetBlur()
{
	BW_GUARD;
	if (Player::entity() != NULL)
	{
		Script::call( PyObject_GetAttrString( Player::entity(), "targetBlur" ),
			Py_BuildValue( "(O)", (PyObject*)this ),
			"Entity::targetBlur: ", true );
	}

	// possibly call our own script here too
}


// -----------------------------------------------------------------------------
// Section: Miscellaneous
// -----------------------------------------------------------------------------

/**
 *	This method returns whether this entity is client-only. That is, it has no
 *	representation on the server.
 */
bool Entity::isClientOnly() const
{
	return EntityManager::isClientOnlyID( id_ );
}


/**
 *	Notifier for our pose becoming volatile or non-volatile
 */
PyObject * Entity::isPoseVolatile( bool v )
{
	BW_GUARD;
	isPoseVolatile_ = v;

	PyObject * ret = PyObject_GetAttrString( this, "onPoseVolatile" );
	if (!ret) PyErr_Clear();
	return ret;
}


/**
 *	This function returns a transform for things that absolutely MUST
 *	know a transform for every entity. Its use is discouraged.
 */
const Matrix & Entity::fallbackTransform() const
{
	BW_GUARD;
	static Matrix		fakeTransform;

	PyModel * pModel = this->pPrimaryModel();
	if (pModel != NULL)
	{
		return pModel->worldTransform();
	}
	else
	{
		fakeTransform.setRotate(	auxVolatile_[0],
									auxVolatile_[1],
									auxVolatile_[2] );
		fakeTransform.translation( position_ );
		return fakeTransform;
	}
}


#if ENABLE_WATCHERS
/**
 *	Static function to return a watcher for entities
 */
Watcher & Entity::watcher()
{
	BW_GUARD;
	static DirectoryWatcherPtr	watchMe = NULL;

	if (!watchMe)
	{
		watchMe = new DirectoryWatcher;

		Entity * pNull = NULL;

		watchMe->addChild( "pos", new DataWatcher<Vector3>(
			pNull->position_ ) );

		//Watcher * pDictWatcher = new DataWatcher<PyObject*>( *(PyObject**)NULL );

#if 0
		watchMe->addChild( "dict", &PyMapping_Watcher(), &pNull->in_dict );
#endif
	}

	return *watchMe;
}
#endif


/*~ callback Entity.reload
 *
 *	This callback method is called in response to reloading scripts.
 *	It allows the entity to reload itself, and attach itself to its
 *	associated objects again.
 */
/**
 *	Method to swizzle instances of pOldClass into pNewClass when
 *	reloading script.
 */
void Entity::swizzleClass( PyTypeObject * pOldClass, PyTypeObject * pNewClass )
{
	BW_GUARD;
	if (this->ob_type == pOldClass)
	{
		this->setClass( pNewClass );

		Script::call( PyObject_GetAttrString( this, "reload" ), PyTuple_New(0),
			"Entity::swizzleClass (reload): ", true );
	}
}



/**
 *	This method makes sure that this entity is in the correct
 *	place in the global sorted position lists.
 */
void Entity::shuffle()
{
	BW_GUARD;
	// Shuffle to the left...
	while (nextLeft_ != NULL && position_[0] < nextLeft_->position_[0])
	{
		// unlink us
		nextLeft_->nextRight_ = nextRight_;
		if (nextRight_ != NULL) nextRight_->nextLeft_ = nextLeft_;
		// fix our pointers
		nextRight_ = nextLeft_;
		nextLeft_ = nextLeft_->nextLeft_;
		// relink us
		if (nextLeft_ != NULL) nextLeft_->nextRight_ = this;
		nextRight_->nextLeft_ = this;
	}

	// Shuffle to the right...
	while (nextRight_ != NULL && position_[0] > nextRight_->position_[0])
	{
		// unlink us
		if (nextLeft_ != NULL) nextLeft_->nextRight_ = nextRight_;
		nextRight_->nextLeft_ = nextLeft_;
		// fix our pointers
		nextLeft_ = nextRight_;
		nextRight_ = nextRight_->nextRight_;
		// relink us
		nextLeft_->nextRight_ = this;
		if (nextRight_ != NULL) nextRight_->nextLeft_ = this;
	}

	// Shuffle to the front...
	while (nextUp_ != NULL && position_[2] < nextUp_->position_[2])
	{
		// unlink us
		nextUp_->nextDown_ = nextDown_;
		if (nextDown_ != NULL) nextDown_->nextUp_ = nextUp_;
		// fix our pointers
		nextDown_ = nextUp_;
		nextUp_ = nextUp_->nextUp_;
		// relink us
		if (nextUp_ != NULL) nextUp_->nextDown_ = this;
		nextDown_->nextUp_ = this;
	}

	// Shuffle to the back...
	while (nextDown_ != NULL && position_[2] > nextDown_->position_[2])
	{
		// unlink us
		if (nextUp_ != NULL) nextUp_->nextDown_ = nextDown_;
		nextDown_->nextUp_ = nextUp_;
		// fix our pointers
		nextUp_ = nextDown_;
		nextDown_ = nextDown_->nextDown_;
		// relink us
		nextUp_->nextDown_ = this;
		if (nextDown_ != NULL) nextDown_->nextUp_ = this;
	}

	// Ayeeeeee, Macarena!
}


/**
 *	Get the next entity of the specified chain
 */
Entity * Entity::nextOfChain( int chain )
{
	BW_GUARD;
	switch (chain)
	{
		case 0:		return nextLeft_;	break;
		case 1:		return nextRight_;	break;
		case 2:		return nextUp_;		break;
		case 3:		return nextDown_;	break;
	}

	return NULL;
}


/**
 *	This method sets a set of properties from the input stream.
 */
void Entity::updateProperties( BinaryIStream & stream,
	bool shouldCallSetMethod )
{
	BW_GUARD;
	// it's easy if we don't call the set method
	if (!shouldCallSetMethod)
	{
		PyObject * pMoreDict = this->type().newDictionary( stream,
			(this==Player::entity()) ?
				EntityType::TAGGED_CELL_PLAYER_DATA :
				EntityType::TAGGED_CELL_ENTITY_DATA );

		TRACE_MSG( "Entity::updateProperties(under): "
			"%d props for id %d\n", PyDict_Size( pMoreDict ), id_ );

		PyObject * pCurrDict = this->pyGetAttribute( "__dict__" );
		PyDict_Update( pCurrDict, pMoreDict );
		Py_DECREF( pCurrDict );
		Py_DECREF( pMoreDict );
	}
	// otherwise set them one by one
	else
	{
		// it's a big pity we have to do this but order must be preserved
		uint8 size;
		stream >> size;

		TRACE_MSG( "Entity::updateProperties(callSet): "
			"%d props for id %d\n", size, id_ );

		for (uint8 i = 0; i < size; i++)
		{
			uint8 index;
			stream >> index;

			DataDescription * pDD =
				this->type().description().clientServerProperty( index );

			MF_ASSERT_DEV( pDD && pDD->isOtherClientData() );

			if (pDD != NULL && pDD->isOtherClientData())
			{
				PyObjectPtr pValue = pDD->createFromStream( stream, false );
				MF_ASSERT_DEV( pValue );

				this->setProperty( pDD, pValue, shouldCallSetMethod );
			}
		}
	}
}


/**
 *	This method read in the cell data that is associated with the player.
 */
void Entity::readCellPlayerData( BinaryIStream & stream )
{
	BW_GUARD;
	PyObject * pCurrDict = this->pyGetAttribute( "__dict__" );

	if (pCurrDict != NULL)
	{
		PyObject * pCellData = type_.newDictionary( stream,
			EntityType::CELL_PLAYER_DATA );

		PyDict_Update( pCurrDict, pCellData );
		Py_DECREF( pCellData );
		Py_DECREF( pCurrDict );
	}
	else
	{
		ERROR_MSG( "Entity::readCellPlayerData: Could not get __dict__\n" );
		PyErr_PrintEx(0);
	}
}


/*~ callback Entity.onControlled
 *
 *	This callback method is called when the local entity control by the client
 *	has been enabled or disabled. See the Entity.controlledBy() method in the
 *	CellApp Python API for more information.
 *
 *	@param isControlled	Whether the entity is now controlled locally.
 */
/**
 *	This method sets whether or not this entity should be controlled
 */
bool Entity::controlled( bool shouldBeControlled )
{
	BW_GUARD;
	if (!pSpace_)
	{
		ERROR_MSG( "Entity::controlled: "
			"Can only be called on entities in the world\n" );
		return false;
	}

	if (shouldBeControlled == this->selfControlled())
	{
		return true;
	}

	if (shouldBeControlled)
	{
		pPhysics_ = NULL;
		EntityManager::instance().onEntityEnter( id_,
			pSpace_->id(), pVehicle_ ? pVehicle_->id() : 0 );
		Script::call(
			this->isPoseVolatile( false ),
			Py_BuildValue( "(O)", Py_False ),
			"Entity.onPoseVolatile",
			true );

		Script::call( PyObject_GetAttrString( this, "onControlled" ),
			Py_BuildValue( "(O)", Py_True ),
			"Entity.onControlled",
			/*okIfFunctionNull:*/ true );
	}
	else
	{
		if (pPhysics_ != NULL)
		{
			pPhysics_->disown();
			Py_DECREF( pPhysics_ );
		}
		pPhysics_ = (Physics*)-1;

		Script::call( PyObject_GetAttrString( this, "onControlled" ),
			Py_BuildValue( "(O)", Py_False ),
			"Entity.onControlled",
			/*okIfFunctionNull:*/ true );

		EntityManager::instance().onEntityLeave( id_ );
	}

	return true;
}


/*~ function BigWorld controlEntity
 *	Sets whether the movement of an entity should be controlled locally by
 *	physics.
 *
 *	When shouldBeControlled is set to True, the entity's physics attribute
 *	becomes accessible. Each time this is called with its shouldBeControlled
 *	attribute as True, the entity's physics attribute is set to None. As this
 *	is also made accessible, it can then be set to a different value.
 *
 *	When shouldBeControlled is set to False, attempts to access the entity's
 *	physics object raise an AttributeError.
 *
 *	This function only works for client-side entities. The server decides
 *	who controls server-side entities. A TypeError is raised if the given
 *	entity is not a client-side entity.
 *
 *	@param entity The entity to control/uncontrol.
 *	@param beControlled Whether the entity should be controlled.
 */
/**
 *	Controls or uncontrols this entity
 */
static bool controlEntity( SmartPointer<Entity> pEntity, bool shouldBeControlled )
{
	BW_GUARD;
	if (!pEntity->isClientOnly())
	{
		PyErr_SetString( PyExc_TypeError, "BigWorld.controlEntity: "
			"This function only works on client-side entities" );
		return false;
	}

	if (!pEntity->controlled( shouldBeControlled ))
	{
		PyErr_Format( PyExc_TypeError, "BigWorld.controlEntity: "
			"Entity %d could not be controlled (probably because "
			"it is not in the world)", pEntity->id() );
		return false;
	}

	return true;
}
PY_AUTO_MODULE_FUNCTION( RETOK, controlEntity,
	NZARG( SmartPointer<Entity>, ARG( bool, END ) ), BigWorld )




// -----------------------------------------------------------------------------
// Section: Neighbour
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
Entity::Neighbour::Neighbour( Entity * pSource, float radius, bool end ) :
	pSource_( pSource ),
	radius_( radius ),
	chain_( end ? 4 : 0),
	steps_( 0 ),
	pNext_( NULL )
{
	BW_GUARD;
	this->nextChain();
}


/**
 *	Private method to get the first element of the next chain.
 */
void Entity::Neighbour::nextChain()
{
	BW_GUARD;
	pNext_ = NULL;
	while (chain_ < 4)
	{
		pNext_ = pSource_->nextOfChain( chain_ );
		if (pNext_ != NULL) break;
		chain_++;
	}
}


/**
 *	Get the begin iterator for this entity of the specified radius.
 */
Entity::Neighbour Entity::begin( float radius )
{
	BW_GUARD;
	return Neighbour( this, radius, false );
}


/**
 *	Get the end iterator for this entity of the specified radius.
 */
Entity::Neighbour Entity::end( float radius )
{
	BW_GUARD;
	return Neighbour( this, radius, true );
}

/* entity.cpp */
