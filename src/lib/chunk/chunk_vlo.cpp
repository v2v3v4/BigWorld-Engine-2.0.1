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
#include "chunk_vlo.hpp"

#include "moo/render_context.hpp"
#include "romp/geometrics.hpp"

#if UMBRA_ENABLE
#include "umbra_chunk_item.hpp"
#endif

#include "chunk.hpp"
#include "chunk_obstacle.hpp"
#include "chunk_space.hpp"
#include "chunk_manager.hpp"

#include "cstdmf/unique_id.hpp"
#include "cstdmf/guard.hpp"
#include "resmgr/xml_section.hpp"

DECLARE_DEBUG_COMPONENT2( "Chunk", 0 )

VeryLargeObject::UniqueObjectList VeryLargeObject::s_uniqueObjects_;


#ifdef EDITOR_ENABLED
uint32 VeryLargeObject::currentSelectionMark_=0;
#pragma warning( disable: 4355 )
#endif

ChunkVLO::Factories * ChunkVLO::pFactories_ = NULL;


namespace
{
	SimpleMutex s_itemListMutex;
	SimpleMutex s_uniqueObjectsMutex_;
}


/**
 *	Constructor. Registers with chunk's static factory registry
 *	@param section			Section name associated with this factory.
 *	@param priority			Priority this factory has in relation to
 *							given name.
 *	@param creator			Creation callback.
 */
VLOFactory::VLOFactory(
		const std::string & section,
		int priority,
		Creator creator ) :
	priority_( priority ),
	creator_( creator )
{
	BW_GUARD;
	ChunkVLO::registerFactory( section, *this );
}


/**
 *	This virtual method calls the creator function that was passed in,
 *	as long as it's not NULL. It is called by a Chunk when it encounters
 *	the section name.
 *
 *	@param pChunk			Chunk to be created in.
 *	@param pSection			Source data section.
 *	@param uid				Unique identifier for the VLO.
 *	@return true if succeeded
 */
bool VLOFactory::create( Chunk * pChunk, DataSectionPtr pSection, 
						std::string uid ) const
{
	BW_GUARD;
	if (creator_ == NULL) return false;
	return (*creator_)( pChunk, pSection, uid );
}


/**
 *	This static method registers the input factory as belonging
 *	to the input section name. If there is already a factory
 *	registered by this name, then this factory supplants it if
 *	it has a (strictly) higher priority.
 *
 *	@param section			Section name associated with this factory.
 *	@param factory			Factory to register.
 */
void ChunkVLO::registerFactory( const std::string & section,
	const VLOFactory & factory )
{
	BW_GUARD;
	// avoid initialisation-order problems
	if (pFactories_ == NULL)
	{
		pFactories_ = new Factories();
	}

	// get a reference to the entry. if it's a new entry, the default
	// 'pointer' constructor will make it NULL.
	const VLOFactory *& pEntry = (*pFactories_)[ section ];

	// and whack it in
	if (pEntry == NULL || pEntry->priority() < factory.priority())
	{
		pEntry = &factory;
	}
}

/*static*/ void ChunkVLO::fini()
{
	BW_GUARD;
	delete pFactories_;
	pFactories_ = NULL;
}


// -----------------------------------------------------------------------------
// Section: ChunkVLO
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
ChunkVLO::ChunkVLO( WantFlags wantFlags ) :
	ChunkItem( wantFlags ),
	pObject_( NULL ),
	dirty_( false ),
	creationRoot_( false )
{
}


/**
 *	Destructor.
 */
ChunkVLO::~ChunkVLO()
{
	BW_GUARD;
	if (pObject_)
	{
		pObject_->removeItem( this );
		pObject_ = NULL;
	}
}


/**
 *	Load a specific VLO.
 *
 *	@param pChunk			Chunk destination for this item.
 *	@param pSection			Source data section.
 *  @return true if succeeded
 */
bool ChunkVLO::loadItem( Chunk* pChunk, DataSectionPtr pSection )
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( pFactories_ != NULL )
	{
		return false;
	}

	std::string type = pSection->readString( "type", "" );
	std::string uid = pSection->readString( "uid", "" );
	Factories::iterator found = pFactories_->find( type );	
	if (found != pFactories_->end() && (VeryLargeObject::getObject(uid) == NULL))
	{
		std::string file = pChunk->mapping()->path() + '_' + uid + ".vlo";
		DataSectionPtr pDS = BWResource::openSection( file + '/' + type );
		if (!pDS)
		{
			file = pChunk->mapping()->path() + uid + ".vlo";
			pDS = BWResource::openSection( file + '/' + type );
		}

		if (pDS && found->second->create( pChunk, pDS, uid ))
		{
#ifdef EDITOR_ENABLED
			VeryLargeObjectPtr pObj = VeryLargeObject::getObject( uid );
			DataSectionPtr pVLOSection = BWResource::openSection( file );
			if (pVLOSection)
				pObj->section(pVLOSection);
#endif //EDITOR_ENABLED
			return true;
		}
		else
        {
            ERROR_MSG( "ChunkVLO::loadItem invalid or missing file %s", file.c_str());
			return false;
        }
	}
	return true;
}

#ifdef EDITOR_ENABLED


/**
 *	Construct the VLO's data section
 */
bool ChunkVLO::buildVLOSection( DataSectionPtr pObjectSection, Chunk* pChunk, std::string& type, std::string& uid )
{
	BW_GUARD;
	if (pObjectSection)
	{
		Factories::iterator found = pFactories_->find( type );
		if (found != pFactories_->end())
		{
			if (found->second->create( pChunk, pObjectSection, uid ))
			{
				pObject_ = VeryLargeObject::getObject( uid );
				pObject_->addItem( this );
			}
		}
	}
	else
		return false;

	if ( pObject_ )
	{
		//setup the VLOs section..
		std::string fileName( '_' + uid + ".vlo" );

		DataSectionPtr ds = pChunk->mapping()->pDirSection();
		DataSectionPtr pVLOSection = ds->newSection( fileName );

		if (!pVLOSection)
			return false;

		DataSectionPtr waterSection = pVLOSection->newSection( type );
		waterSection->setParent( pVLOSection );
		pObject_->section( pVLOSection );
		pObject_->save();
		waterSection->setParent( NULL );

		return true;
	}
	else
		return false;
}


/**
 *	This method returns the number of triangles required to draw the VLO.
 */
int ChunkVLO::edNumTriangles() const
{
	return pObject_ ? pObject_->numTriangles() : 0;
}


/**
 *	This method returns the number of primitive calls required to draw the VLO.
 */
int ChunkVLO::edNumPrimitives() const
{
	return pObject_ ? pObject_->numPrimitives() : 0;
}


/**
 *	ChunkVLO creation
 */
bool ChunkVLO::createVLO( DataSectionPtr pSection, Chunk* pChunk )
{
	BW_GUARD;
	std::string type = pSection->readString( "type", "" );
	std::string uid = pSection->readString( "uid", "" );
	if (uid == "" && type != "")
	{
		uid = VeryLargeObject::generateUID();
		return buildVLOSection( pSection->openSection( type ), pChunk, type, uid );
	}
	return false;
}

/**
 *	Creation path used for some legacy loading... (old water sections)
 */
bool ChunkVLO::createLegacyVLO( DataSectionPtr pSection, Chunk* pChunk, std::string& type )
{	
	BW_GUARD;
	if (type != "")
	{
		std::string uid = VeryLargeObject::generateUID();
		return buildVLOSection( pSection, pChunk, type, uid );
	}
	return false;
}


/**
 *	ChunkVLO creation
 */
bool ChunkVLO::cloneVLO( DataSectionPtr pSection, Chunk* pChunk, VeryLargeObjectPtr pSource )
{
	BW_GUARD;
	std::string type = pSection->readString( "type", "" );
	if (type != "")
	{
		DataSectionPtr objectSection = pSource->section()->openSection(type);
		return buildVLOSection( objectSection, pChunk, type, VeryLargeObject::generateUID() );
	}
	return false;
}
#endif //EDITOR_ENABLED


/**
 *	ChunkVLO load method. Passes on the load to the global loadItem method.
 *
 *	@param pSection			Source data section.
 *	@param pChunk			Chunk destination for this item. 
 *  @return true if succeeded
 */
bool ChunkVLO::load( DataSectionPtr pSection, Chunk * pChunk )
{
	BW_GUARD;
	std::string uid = pSection->readString( "uid", "" );
	std::string type = pSection->readString( "type", "" );
	if (uid != "" && type != "")
	{
		bool ret = ChunkVLO::loadItem( pChunk, pSection );
		if (ret)
		{
			pObject_ = VeryLargeObject::getObject( uid );
			pObject_->addItem( this );
		}

		return ret;
	}
	return false;
}

/**
 *  Initialisation of the required UMBRA data.
 *
 */
void ChunkVLO::syncInit()
{
	BW_GUARD;	
#if UMBRA_ENABLE
	// Delete any old umbra draw item
	delete pUmbraDrawItem_;
	pUmbraDrawItem_ = NULL;

	if (pChunk_)
	{
		// Get the bounding box of the vlo intersected with the chunk
		BoundingBox bb = pObject_->chunkBB( this->pChunk_ );
		if (!bb.insideOut())
		{
			// Create the umbra resources for the vlo
			UmbraChunkItem* pUmbraChunkItem = new UmbraChunkItem;
			pUmbraChunkItem->init( this, bb, pChunk_->transform(), pChunk_->getUmbraCell() );
			pUmbraDrawItem_ = pUmbraChunkItem;
		}
	}
	this->updateUmbraLenders();
#endif // UMBRA_ENABLE

	pObject_->syncInit( this );
}

/**
 *  Lending method. Passes along the lending to the referenced VLO.
 *
 *	@param pChunk			Chunk being lent.
 */
void ChunkVLO::lend( Chunk * pChunk )
{
	BW_GUARD;
	if (pObject_)
		pObject_->lend( pChunk);
}


/**
 *	Move this VLO around between chunks.
 *
 *	@param pChunk			Chunk destination to toss this ChunkVLO into.
 */
void ChunkVLO::toss( Chunk * pChunk )
{
	BW_GUARD;
	if (pChunk_)
	{
		removeCollisionScene( );
#if UMBRA_ENABLE
		if (pChunk == NULL && pObject_)
		{
			pObject_->unlend( pChunk_ );
		}
#endif //UMBRA_ENABLE
	}

	// call base class method
	this->ChunkItem::toss( pChunk );
}


/**
 *	Pass on the sway info through to the VLO.
 *
 *	@param src				Source movement position.
 *	@param dst				Destination movement position.
 *	@param diameter			Diameter of movement.
 */
void ChunkVLO::sway( const Vector3 & src, const Vector3 & dst, const float diameter )
{
	BW_GUARD;
	if (pObject_)
		pObject_->sway( src, dst, diameter );
}


/**
 *	Draw the referenced VLO
 */
void ChunkVLO::draw()
{
	BW_GUARD;
	//TODO: flag the object as drawn (here or inside the objects draw call?)
	if (pObject_ && pChunk_)
		pObject_->draw( pChunk_ );
}


/**
 *	Check if the VLO has a reference in the specified chunk already
 *
 *	@param pChunk			Chunk to check for a reference.
 *	@return	The reference found or NULL if one does not exist.
 */
ChunkVLO* VeryLargeObject::containsChunk( const Chunk * pChunk ) const
{
	BW_GUARD;
	SimpleMutexHolder holder(s_itemListMutex);

	ChunkItemList::const_iterator it;
	for (it = itemList_.begin(); it != itemList_.end(); it++)
	{
		if ((*it)->chunk() == pChunk)
			return (*it);
	}
	return NULL;
}
//
//
//bool ChunkVLO::legacyLoad( DataSectionPtr pSection, Chunk * pChunk, std::string& type )
//{
//	if (createLegacyVLO( pSection, pChunk, type ))
//	{
////		Matrix m( pObject_->edTransform() );
////		m.postMultiply( pChunk->transform() );
////		pObject_->updateLocalVars( m );
//
//		//edTransform(m,false);
//
////		type_ = type;
////		uid_ = pObject_->getUID();
//
////		vloTransform_.postMultiply( pObject_->origin() );
//
//		pObject_->addItem( this );
//		return true;
//	}
//	return false;
//}
//

/**
 *	This static method creates a VLO reference from the input section and adds
 *	it to the given chunk.
 *
 *	@param pChunk			Chunk to add a VLO reference to.
 *	@param pSection			Source data section.
 *	@return	Result of the load/reference addition.
 */
ChunkItemFactory::Result ChunkVLO::create( Chunk * pChunk, DataSectionPtr pSection )
{
	BW_GUARD;
	//TODO: generalise the want flags...
    ChunkVLO * pVLO = new ChunkVLO( (WantFlags)(WANTS_DRAW | WANTS_SWAY) );

	if (!pVLO->load( pSection, pChunk ))
	{
		delete pVLO;
		return ChunkItemFactory::Result( NULL,
			"Failed to create " + pSection->readString( "type", "<unknown type>" ) +
			" VLO " + pSection->readString( "uid", "<unknown id>" ) );
	}
	else
	{
		pChunk->addStaticItem( pVLO );
		return ChunkItemFactory::Result( pVLO );
	}
}


/// Static factory initialiser
ChunkItemFactory ChunkVLO::factory_( "vlo", 0, ChunkVLO::create );


// -----------------------------------------------------------------------------
// Section: VeryLargeObject
// -----------------------------------------------------------------------------

/**
 *	Constructor
 */
VeryLargeObject::VeryLargeObject()
	: rebuild_( false )
#ifdef EDITOR_ENABLED
	, dataSection_( NULL ),
	listModified_( false ),
	objectCreated_( false ),
	lastDbItem_( NULL ),
	selectionMark_( 0 ),
	chunkPath_( "" ),
	metaData_( (EditorChunkCommonLoadSave*)this )
#endif
{ 
}


/**
 *	Constructor
 */
VeryLargeObject::VeryLargeObject( std::string uid, std::string type ) :
	type_( type ),
	rebuild_( false )
#ifdef EDITOR_ENABLED
	, dataSection_( NULL ),
	listModified_( false ),
	objectCreated_( false ),
	lastDbItem_( NULL ),
	selectionMark_( 0 ),
	chunkPath_( "" ),
	metaData_( (EditorChunkCommonLoadSave*)this )
#endif
{
	setUID( uid );		 
}


/**
 *	Destructor
 */
VeryLargeObject::~VeryLargeObject()
{
}


#ifdef EDITOR_ENABLED

/*
 * Save this VLO.
 */
/*virtual*/
void VeryLargeObject::save()
{
	if (dataSection_)
	{
		edCommonSave(dataSection_);
	}
}

/*
 * Deletes VLOs that are not in use in the space any more
 */
void VeryLargeObject::deleteUnused()
{
	BW_GUARD;
	SimpleMutexHolder smh( s_uniqueObjectsMutex_ );
	UniqueObjectList::iterator it = s_uniqueObjects_.begin();
	for (;it != s_uniqueObjects_.end(); it++)
	{
		VeryLargeObjectPtr obj = (*it).second;
		if (obj)
			obj->cleanup();
	}
}


/*
 * Saves outstanding VLOs.
 */
void VeryLargeObject::saveAll()
{
	BW_GUARD;
	SimpleMutexHolder smh( s_uniqueObjectsMutex_ );
	UniqueObjectList::iterator it = s_uniqueObjects_.begin();
	for (;it != s_uniqueObjects_.end(); it++)
	{
		VeryLargeObjectPtr obj = (*it).second;
		if (obj)
			obj->saveFile(NULL);
	}
}

/**
 *	shall we draw?
 */
bool VeryLargeObject::edShouldDraw()
{
	BW_GUARD;
	if( !EditorChunkItem::hideAllOutside() )
		return true;

	SimpleMutexHolder holder(s_itemListMutex);
	for( ChunkItemList::iterator iter = itemList_.begin(); iter != itemList_.end(); ++iter )
	{
		if( !(*iter)->chunk()->isOutsideChunk() )
			return true;
	}
	return false;
}


/**
 *  Returns true if the object is ready to be added to the collision scene.
 */
bool VeryLargeObject::isObjectCreated() const
{
	return objectCreated_;
}


/**
 *	This method returns the list of references to this VLO.
 */
VeryLargeObject::ChunkItemList VeryLargeObject::chunkItems() const
{
	SimpleMutexHolder holder( s_itemListMutex );
	return itemList_;
}


/**
 *	Generate a unique id for a VLO
 *
 *  @return The generated GUID.
 */
std::string VeryLargeObject::generateUID()
{
	BW_GUARD;
	std::string s( UniqueID::generate() );
	std::transform( s.begin(), s.end(), s.begin(), tolower );
	return s;
}


#endif //EDITOR_ENABLED


/**
 *	Tells all loaded items that the actual VLO has been created, so things like
 *  the collision scene can be updated. Also, sets a flag so other loaded VLO 
 *  items get their "objectCreated" callback called when they are loaded.
 */
void VeryLargeObject::objectCreated()
{
	BW_GUARD;	
#ifdef EDITOR_ENABLED
	if ( objectCreated_ )
		return;
	objectCreated_ = true;
#endif //EDITOR_ENABLED

	for (ChunkItemList::iterator it = itemList_.begin();
		it != itemList_.end(); it++)
	{
		if ((*it) && (*it)->chunk())
			(*it)->objectCreated();			
	}
}


/**
 *	Assign a UID for this VLO.
 *	TODO: remove this function from prying public eyes...
 *
 *	@param uid				Unique ID for this VLO.
 */
void VeryLargeObject::setUID( std::string uid ) 
{
	BW_GUARD;	
	uid_ = uid;
#ifdef EDITOR_ENABLED
	std::transform( uid_.begin(), uid_.end(), uid_.begin(), tolower );
#endif // EDITOR_ENABLED
	SimpleMutexHolder smh( s_uniqueObjectsMutex_ );
	s_uniqueObjects_[uid_] = this;
}


/**
 *	Add a reference item.
 *
 *	@param item				Reference item being registered..
 */
void VeryLargeObject::addItem( ChunkVLO* item )
{
	BW_GUARD;
	SimpleMutexHolder holder(s_itemListMutex);

	itemList_.push_back( item );
	itemList_.sort( );
	itemList_.unique( );
#ifdef EDITOR_ENABLED
	listModified_ = true;
#endif
}


/**
 *	Remove a reference item
 * 
 *	@param item				Reference item being removed.
 */
void VeryLargeObject::removeItem( ChunkVLO* item)
{
	BW_GUARD;
	SimpleMutexHolder holder(s_itemListMutex);

	itemList_.remove( item );

	if (itemList_.size() == 0)
	{
		SimpleMutexHolder smh( s_uniqueObjectsMutex_ );
		s_uniqueObjects_[uid_] = NULL;
	}
#ifdef EDITOR_ENABLED
	listModified_ = true;
#endif
}


/**
 *	This static method ticks all the vlo's
 */
void VeryLargeObject::tickAll( float dTime )
{
	SimpleMutexHolder smh( s_uniqueObjectsMutex_ );
	UniqueObjectList::iterator vloIt = s_uniqueObjects_.begin();
	while (vloIt != s_uniqueObjects_.end())
	{
		VeryLargeObjectPtr pVLO = vloIt->second;
		if (pVLO.exists())
		{
			pVLO->tick( dTime );
		}
		++vloIt;
	}
}



// chunk_vlo.cpp