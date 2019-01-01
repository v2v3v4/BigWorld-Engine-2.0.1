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
#include "chunk_entity.hpp"

#include "chunk/chunk.hpp"
#include "chunk/chunk_space.hpp"

#include "cstdmf/memory_stream.hpp"

#include "entity_manager.hpp"
#include "entity_type.hpp"

#include "cstdmf/debug.hpp"

DECLARE_DEBUG_COMPONENT2( "Entity", 0 )


// -----------------------------------------------------------------------------
// Section: ChunkEntity
// -----------------------------------------------------------------------------


/// Static factory initialiser
ChunkItemFactory ChunkEntity::factory_( "entity", 0, ChunkEntity::create );
PROFILER_DECLARE( ChunkEntityCache_del, "ChunkEntityCache del" );

int ChunkEntity_token = 0;


/**
 *	Constructor.
 */
ChunkEntity::ChunkEntity() :
	pEntity_( NULL ),
	pType_( NULL ),
	pPropertiesDS_( NULL )
{
	BW_GUARD;	
}


/**
 *	Destructor.
 */
ChunkEntity::~ChunkEntity()
{
	BW_GUARD;
	if (pChunk_ != NULL) this->toss( NULL );

	if (pEntity_ != NULL)
	{
		Py_DECREF( pEntity_ );
		pEntity_ = NULL;
	}
}



/**
 *	This static method creates a chunk entity from the input section
 *	and adds it to the given chunk. It first checks whether or not the
 *	client should automatically instantiate the entity.
 */
ChunkItemFactory::Result ChunkEntity::create( Chunk * pChunk, DataSectionPtr pSection )
{
	BW_GUARD;
	// if this is a server-instantiated-only entity, then ignore it
	// here 'coz the server will tell us about it when we get there.
	if ( !pSection->readBool( "clientOnly",false ) )
		return ChunkItemFactory::SucceededWithoutItem();

	// now continue with the standard creation code
	ChunkEntity * pEntity = new ChunkEntity();

	if (!pEntity->load( pSection, pChunk ))
	{
		delete pEntity;
		return ChunkItemFactory::Result( NULL,
			"Failed to load entity of type " + pSection->readString( "type" ) );
	}
	else
	{
		pChunk->addStaticItem( pEntity );
		return ChunkItemFactory::Result( pEntity );
	}
}



/**
 *	Loads a chunk entity description from this section
 */
bool ChunkEntity::load( DataSectionPtr pSection, Chunk * pChunk )
{
	BW_GUARD;
	// our create method has already looked at 'instantiate',
	// and checked we have an id if we need one

	// id
	id_ = pSection->readInt( "id" );

	// type
	std::string type = pSection->readString( "type" );
	pType_ = EntityType::find( type );
	if (pType_ == NULL)
	{
		ERROR_MSG( "No such entity type '%s'\n", type.c_str() );
		return false;
	}

	DataSectionPtr pTransform = pSection->openSection( "transform" );
	if (pTransform)
	{
		Matrix m = pSection->readMatrix34( "transform", Matrix::identity );
		m.postMultiply( pChunk->transform() );
		position_ = m.applyToOrigin();
		yaw_ = m.yaw();
		pitch_ = m.pitch();
		roll_ = m.roll();		
	}
	else
	{
		ERROR_MSG( "Entity (of type '%s') has no position\n", type.c_str() );
		return false;
	}

	pPropertiesDS_ = pSection->openSection( "properties" );

	if (!pPropertiesDS_)
	{
		ERROR_MSG( "Entity has no 'properties' section\n" );
		return false;
	}

	// and that's how you load an entity description
	return true;
}


/**
 *	Puts a chunk entity description into the given chunk
 *
 *	We should consider having the entity entered only when our
 *	chunk is focussed. It may be less important once chunks unload
 *	themselves, but I think it should probably be done anyway.
 */
void ChunkEntity::toss( Chunk * pChunk )
{
	BW_GUARD;
	// short-circuit this method if we're adding to the same chunk.
	// a chunk can do this sometimes when its bindings change.
	if (pChunk == pChunk_) return;

	// now follow the standard toss schema
	// note that we only do this if our entity has been created.
	// this makes sure that we don't try to call the entity manager
	// or anything in python from the loading thread.

	if (pChunk_ != NULL)
	{
		if ( pEntity_ != NULL &&
			(pEntity_->isInWorld() || pEntity_->loadingPrerequisites()) )
		{
			// remove from entity manager
			EntityManager::instance().onEntityLeave( id_ );
		}

		ChunkEntityCache::instance( *pChunk_ ).del( this );
	}

	this->ChunkItem::toss( pChunk );

	if (pChunk_ != NULL)
	{
		ChunkEntityCache::instance( *pChunk_ ).add( this );

		if (pEntity_ != NULL)
		{
			// add to entity manager
			EntityManager::instance().onEntityEnter( id_,
				pChunk_->space()->id(), 0 );
			EntityManager::instance().onEntityMove( id_,
				pChunk_->space()->id(),
				0,
				pEntity_->position(),
				pEntity_->auxVolatile()[0],
				pEntity_->auxVolatile()[1],
				pEntity_->auxVolatile()[2],
				false );
		}
	}
}


/**
 *	This method is called when we are bound (or maybe unbound),
 *	anyway it is called early from the main thread.
 */
void ChunkEntity::bind()
{
	BW_GUARD;
	// get out now if we've already made the entity
	if (pEntity_ != NULL) return;

	// tell it the entity has entered the world
	if (id_ == 0) id_ = EntityManager::nextClientID();
	if (pChunk_ != NULL)
		EntityManager::instance().onEntityEnter( id_, pChunk_->space()->id(), 0 );

	// create the actual entity
	this->makeEntity();

	// now either send it an update, or take it out of the world,
	// depending on whether or not we are currently in a chunk
	if (pChunk_ != NULL)
	{
		EntityManager::instance().onEntityMove( id_,
			pChunk_->space()->id(),
			0,
			pEntity_->position(),
			pEntity_->auxVolatile()[0],
			pEntity_->auxVolatile()[1],
			pEntity_->auxVolatile()[2],
			false );
	}
	else
	{
		EntityManager::instance().onEntityLeave( id_ );
	}
}


/**
 *	This method makes the actual entity object in the entity manager
 */
void ChunkEntity::makeEntity()
{
	BW_GUARD;
	MemoryOStream propertiesStream( 2048 );
	// and all the properties
	uint8 * pNumProp = (uint8*)propertiesStream.reserve( sizeof(uint8) );
	*pNumProp = 0;
	const EntityDescription & edesc = pType_->description();
	for (uint8 i = 0; i < edesc.clientServerPropertyCount(); i++)
	{
		// make sure this is a property we want
		DataDescription * pDD = edesc.clientServerProperty( i );
		if (!pDD->isOtherClientData())
		{
			// If we have properties that will not be loaded, be verbose about
			// it.
			NOTICE_MSG( "ChunkEntity::makeEntity: Property %s.%s of type "
						"%s is not created for client-only entities.\n",
					pType_->name().c_str(), pDD->name().c_str(),
					pDD->getDataFlagsAsStr() );
			continue;
		}


		// open section and start with default value
		DataSectionPtr pPropSec = pPropertiesDS_->openSection( pDD->name() );
		SmartPointer<PyObject> pPyProp = pDD->pInitialValue();

		// change to created value if it parses ok
		if (pPropSec)
		{
			PyObjectPtr pTemp = pDD->createFromSection( pPropSec );
			if (pTemp) pPyProp = pTemp;
		}

		// and add whichever to the stream
		propertiesStream << i;
		pDD->addToStream( &*pPyProp, propertiesStream, false );
		(*pNumProp)++;
	}

	pPropertiesDS_ = NULL;

	propertiesStream.rewind();
	EntityManager::instance().onEntityCreate( id_, pType_->index(),
		pChunk_->space()->id(), 0,
		position_, yaw_, pitch_, roll_,
		propertiesStream );

	pEntity_ = EntityManager::instance().getEntity( id_, true );
	Py_INCREF( pEntity_ );
}



// -----------------------------------------------------------------------------
// Section: ChunkEntityCache
// -----------------------------------------------------------------------------

/**
 *	Constructor
 */
ChunkEntityCache::ChunkEntityCache( Chunk & chunk )
{
}

/**
 *	Destructor
 */
ChunkEntityCache::~ChunkEntityCache()
{
}


/**
 *	Bind (and unbind) method, lets us know when our chunk boundness changes
 */
void ChunkEntityCache::bind( bool isUnbind )
{
	BW_GUARD;
	for (ChunkEntities::iterator it = entities_.begin();
		it != entities_.end();
		it++)
	{
		(*it)->bind();
	}
}


/**
 *	Add an entity to our list
 */
void ChunkEntityCache::add( ChunkEntity * pEntity )
{
	BW_GUARD;
	entities_.push_back( pEntity );
}

/**
 *	Remove an entity from our list
 */
void ChunkEntityCache::del( ChunkEntity * pEntity )
{	
	BW_GUARD_PROFILER( ChunkEntityCache_del );
	ChunkEntities::iterator found =
		std::find( entities_.begin(), entities_.end(), pEntity );
	if (found != entities_.end()) entities_.erase( found );
}


/// Static instance object initialiser
ChunkCache::Instance<ChunkEntityCache> ChunkEntityCache::instance;

// chunk_entity.cpp
