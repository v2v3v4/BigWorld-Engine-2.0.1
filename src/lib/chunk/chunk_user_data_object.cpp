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

#include "Python.h"		// See http://docs.python.org/api/includes.html

#include "chunk_user_data_object.hpp"
#include "resmgr/bwresource.hpp"
#include "cstdmf/unique_id.hpp"
#include "cstdmf/guard.hpp"
#include "chunk.hpp"
#include "chunk_item.hpp"
#include "chunk_space.hpp"
#include "math/matrix.hpp"
#include "user_data_object_type.hpp"
#include "user_data_object.hpp"
#include "pyscript/script.hpp"
#include <string>


int ChunkUserDataObject_token=1;


// force UserDataObjectLinkDataType token as well
extern int UserDataObjectLinkDataType_token;
static int s_tokenSet = UserDataObjectLinkDataType_token;


DECLARE_DEBUG_COMPONENT2( "ChunkUserDataObject", 0 )
PROFILER_DECLARE( ChunkUserDataObjectCache_del, "ChunkUserDataObjectCache del" );


// -----------------------------------------------------------------------------
// Section: ChunkUserDataObject
// -----------------------------------------------------------------------------


/**
 *	Constructor.
 */

ChunkUserDataObject::ChunkUserDataObject() :
	ChunkItem( WANTS_NOTHING ), // TODO: what wants flags do i need?
	pType_( NULL ),
	pUserDataObject_( NULL )
{

}

/**
 *	Destructor.
 */
ChunkUserDataObject::~ChunkUserDataObject()
{
	BW_GUARD;

	if (pUserDataObject_)
	{
		pUserDataObject_->decChunkItemRefCount();
	}
}


/**
 *  This static method creates a chunk entity from the input section
 *  and adds it to the given chunk, according to the domain of the UDO.
 */
ChunkItemFactory::Result ChunkUserDataObject::create( Chunk * pChunk, DataSectionPtr pSection )
{
	BW_GUARD;
	int domain = pSection->readInt("Domain");
#ifdef MF_SERVER
	if ((domain & UserDataObjectDescription::CELL) == 0)
	{
	    return ChunkItemFactory::SucceededWithoutItem();
	}
#else
	#ifndef EDITOR_ENABLED
	if ( (domain & UserDataObjectDescription::CLIENT) == 0)
	{
	    return ChunkItemFactory::SucceededWithoutItem();
	}
	#endif
#endif

	ChunkUserDataObject* pItem = new ChunkUserDataObject();

	std::string errorString;
	if ( pItem->load(pSection, pChunk, &errorString) )
	{
		pChunk->addStaticItem( pItem );
		return ChunkItemFactory::Result( pItem );
	}

	delete pItem;
	return ChunkItemFactory::Result( NULL, errorString );
}
ChunkItemFactory ChunkUserDataObject::factory_( "UserDataObject", 0, ChunkUserDataObject::create );


/**
 *	Load method
 */
bool ChunkUserDataObject::load( DataSectionPtr pSection,Chunk* pChunk,std::string *errorString)
{
	BW_GUARD;
	std::string type = pSection->readString( "type" );
	pType_ = UserDataObjectType::getType( type.c_str() );
	if (pType_ == NULL)
	{
		ERROR_MSG( "ChunkUserDataObject::load: No such UserDataObject type '%s'\n", type.c_str() );
		return false;
	}
	std::string idStr = pSection->readString( "guid" );
	if ( idStr.empty() )
	{
		ERROR_MSG( "ChunkUserDataObject::load: UserDataObject has no GUID" );
		return false;
	}
	initData_.guid = UniqueID( idStr );
	DataSectionPtr pTransform = pSection->openSection( "transform" );
	if (pTransform)
	{
		Matrix m = pSection->readMatrix34( "transform", Matrix::identity );
		m.postMultiply( pChunk->transform() );
		initData_.position = m.applyToOrigin();
		initData_.direction = Direction3D(Vector3( m.roll(), m.pitch(), m.yaw() ));
	}
	else
	{
		ERROR_MSG( "ChunkUserDataObject::load: UserDataObject (of type '%s') has no position\n", type.c_str() );
		return false;
	}
	/* Now create an object with properties using the properties
	   section */
	initData_.propertiesDS = pSection->openSection( "properties" );
	if (!initData_.propertiesDS)
	{
		ERROR_MSG( "ChunkUserDataObject::load: UserDataObject has no 'properties' section\n" );
		return false;
	}
	return true;
}

/**
  * This will be called by the main thread once the chunk has been loaded.
  */
void ChunkUserDataObject::bind()
{
	BW_GUARD;
	/* Dont create the entity more than once */
	if (pUserDataObject_ != NULL)
		return;

	pUserDataObject_ = UserDataObject::findOrLoad( initData_, pType_ );
	pUserDataObject_->incChunkItemRefCount();
}


/**
 *	Puts a chunk entity description into the given chunk
 */
void ChunkUserDataObject::toss( Chunk * pChunk )
{
	BW_GUARD;
	// Don't toss it if it wasn't properly loaded.
	if ( pType_ == NULL )
		return;

	// short-circuit this method if we're adding to the same chunk.
	// a chunk can do this sometimes when its bindings change.
	if (pChunk == pChunk_) return;

	// now follow the standard toss schema
	// note that we only do this if our entity has been created.
	// this makes sure that we don't try to call the entity manager
	// or anything in python from the loading thread.

	if (pChunk_ != NULL)
	{
		ChunkUserDataObjectCache::instance( *pChunk_ ).del( this );
	}

	this->ChunkItem::toss( pChunk );

	if (pChunk_ != NULL)
	{
		ChunkUserDataObjectCache::instance( *pChunk_ ).add( this );
	}
}


// -----------------------------------------------------------------------------
// Section: ChunkUserDataObjectCache
// -----------------------------------------------------------------------------

/**
 *	Constructor
 */
ChunkUserDataObjectCache::ChunkUserDataObjectCache( Chunk & chunk )
{
}

/**
 *	Destructor
 */
ChunkUserDataObjectCache::~ChunkUserDataObjectCache()
{
}


/**
 *	Bind (and unbind) method, lets us know when our chunk boundness changes
 */
void ChunkUserDataObjectCache::bind( bool isUnbind )
{
	BW_GUARD;
	for (ChunkUserDataObjects::iterator it = userDataObjects_.begin();
		it != userDataObjects_.end();
		it++)
	{
		(*it)->bind();
	}
}


/**
 *	Add an entity to our list
 */
void ChunkUserDataObjectCache::add( ChunkUserDataObject * pUserDataObject )
{
	BW_GUARD;
	userDataObjects_.push_back( pUserDataObject );
}

/**
 *	Remove an entity from our list
 */
void ChunkUserDataObjectCache::del( ChunkUserDataObject * pUserDataObject )
{
	BW_GUARD_PROFILER( ChunkUserDataObjectCache_del );
	//TODO: instead of using linear time operations, use a set or dont call del
	// for individual objects.
	ChunkUserDataObjects::iterator found =
		std::find( userDataObjects_.begin(), userDataObjects_.end(), pUserDataObject );
	if (found != userDataObjects_.end()) userDataObjects_.erase( found );
}

/// Static instance object initialiser
ChunkCache::Instance<ChunkUserDataObjectCache> ChunkUserDataObjectCache::instance;

// chunk_user_data_object.cpp
