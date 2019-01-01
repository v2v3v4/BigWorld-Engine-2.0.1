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
#include "worldeditor/world/items/editor_chunk_vlo.hpp"
#include "worldeditor/world/items/editor_chunk_substance.ipp"
#include "worldeditor/world/editor_chunk.hpp"
#include "worldeditor/world/world_manager.hpp"
#include "worldeditor/world/vlo_manager.hpp"
#include "worldeditor/editor/chunk_item_placer.hpp"
#include "worldeditor/editor/item_editor.hpp"
#include "worldeditor/editor/item_properties.hpp"
#include "worldeditor/misc/sync_mode.hpp"
#include "worldeditor/misc/options_helper.hpp"
#include "worldeditor/world/item_info_db.hpp"
#include "appmgr/options.hpp"
#include "model/super_model.hpp"
#include "chunk/chunk.hpp"
#include "chunk/chunk_model.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_vlo_obstacle.hpp"
#include "gizmo/tool_manager.hpp" 
#include "math/oriented_bbox.hpp"
#include "cstdmf/unique_id.hpp"
#include "resmgr/resource_cache.hpp"
#if UMBRA_ENABLE
#include <umbraObject.hpp>
#include <umbraModel.hpp>
#endif

namespace
{
	// -----------------------------------------------------------------------------
	// Section: VLOExistenceOperation
	// -----------------------------------------------------------------------------
	class VLOExistenceOperation : public UndoRedo::Operation
	{
	public:
		typedef SmartPointer<EditorChunkVLO> EditorChunkVLOPtr;

		VLOExistenceOperation( EditorChunkVLOPtr pItem, Chunk * pOldChunk ) :
			UndoRedo::Operation( 0 ),
			pItem_( pItem ),
			pOldChunk_( pOldChunk )
		{
			BW_GUARD;

			addChunk( pOldChunk_ );
			if( pItem_ )
				addChunk( pItem_->chunk() );
		}

	protected:
		virtual void undo()
		{
			BW_GUARD;

			// Invalid op.
			if (!pItem_)
				return;

			// VLOs need a once-only update when undoing. When redoing, the VLO will
			// handle creating the new Undo point, etc, internally. The actual deletion
			// and recreation of the VLO is done by the default ChunkItemExistance
			// operation.
			VLOManager::instance()->markAsDeleted( pItem_->object()->getUID(), false );
			VLOManager::instance()->updateReferences( pItem_->object() );
			// Make sure the chunk(s) get marked as changed.
			if ( pOldChunk_ )
				WorldManager::instance().changedChunk( pOldChunk_ );
			if ( pItem_->chunk() )
				WorldManager::instance().changedChunk( pItem_->chunk() );
		}

		virtual bool iseq( const UndoRedo::Operation & oth ) const
		{
			// these operations never replace each other
			return false;
		}

		EditorChunkVLOPtr	pItem_;
		Chunk*				pOldChunk_;
	};

}


// -----------------------------------------------------------------------------
// Section: EditorChunkVLO
// -----------------------------------------------------------------------------


/**
 *	Constructor.
 */
EditorChunkVLO::EditorChunkVLO( std::string type ) :
	type_( type ),
	colliding_( false ),
	readonly_( false ),
	highlight_( false )
{
	BW_GUARD;

	s_drawAlways_ = false;	// turn off its draw flag	
	drawTransient_ = false;
	WorldManager::instance().connection().registerNotification( this );

	waterModel_ = Model::get( "resources/models/water.model" );
	ResourceCache::instance().addResource( waterModel_ );
}


/**
 *	Constructor.
 */
EditorChunkVLO::EditorChunkVLO( ) :
	colliding_( false ),
	readonly_( false ),
	highlight_( false )
{
	BW_GUARD;

	s_drawAlways_ = false;	// turn off its draw flag	
	drawTransient_ = false;
	WorldManager::instance().connection().registerNotification( this );

	waterModel_ = Model::get( "resources/models/water.model" );
	ResourceCache::instance().addResource( waterModel_ );
}


/**
 *	Destructor.
 */
EditorChunkVLO::~EditorChunkVLO()
{
	BW_GUARD;

	if (pObject_)
	{
		VeryLargeObject::ChunkItemList objectItems = pObject_->chunkItems();
		if ((objectItems.size() == 1) && (objectItems.front() == this))
		{
			// This is the last reference to the VLO, so it will get deleted
			// in this destructor. Notify the VLOManager.
			VLOManager::instance()->removeFromLists( uid_ );
		}
		pObject_->removeItem( this );
	}
	WorldManager::instance().connection().unregisterNotification( this );
}


void EditorChunkVLO::edPostCreate()
{
	BW_GUARD;

	if (pObject_)
	{
		objectCreated();

		MetaData::updateCreationInfo( pObject_->metaData() );
	}

	EditorChunkItem::edPostCreate();
}


/**
 * Returns the Bounding box to be used to show selection feedback. In addition,
 * it sets an internal flag that will make the water draw highlighted.
 */
void EditorChunkVLO::edSelectedBox( BoundingBox& bbRet ) const
{
	BW_GUARD;

	edBounds( bbRet );
	if ( WorldManager::instance().cursorOverGraphicsWnd() &&
		ToolManager::instance().tool() &&
		ToolManager::instance().tool()->locator() &&
		pObject_ ) 
	{
		Vector3 pos = ToolManager::instance().tool()->locator()->transform().applyToOrigin();
		Matrix invVloXform = pObject_->localTransform();
		invVloXform.invert();
		pos = invVloXform.applyPoint( pos );
		if ( bbRet.intersects( pos ) )
			highlight_ = true;
	}
}


/**
 * 
 */
void VeryLargeObject::edDelete( ChunkVLO* instigator )
{
	BW_GUARD;

	if (instigator && instigator->chunk())
		chunkPath_ = instigator->chunk()->mapping()->path();
	VLOManager::instance()->markAsDeleted( getUID() );
}


/**
 * Load the reference
 *
 */
bool EditorChunkVLO::load( const std::string& uid, Chunk * pChunk )
{
	BW_GUARD;

	if ( VLOManager::instance()->contains( pChunk, uid )  )
	{
		return false;
	}

	pObject_ = VeryLargeObject::getObject( uid );
	if (pObject_)
	{	
		uid_ = uid;
		updateTransform( pChunk );
		return true;
	}	
	return false;
}


/**
 *	This method saves the data section pointer before calling its
 *	base class's load method
 */
bool EditorChunkVLO::load( DataSectionPtr pSection, Chunk * pChunk, std::string* errorString )
{
	BW_GUARD;

	std::string uid = pSection->readString( "uid", "" );

	if ( VLOManager::instance()->isDeleted( uid ) )
	{
		return false;
	}

	if ( VLOManager::instance()->contains( pChunk, uid )  )
	{
		return false;
	}

	localPos_ = Vector3(50,0,50); //middle it
	edTransform(); //initialise the transform
	vloTransform_ = Matrix::identity;
	vloTransform_.translation( -localPos_ );
	vloTransform_.postMultiply( pChunk->transformInverse() );

	bool loaded = this->EditorChunkSubstance<ChunkVLO>::load( pSection, pChunk );
	bool created = loaded ? false : createVLO( pSection, pChunk );
	if (loaded || created)
	{
		type_ = pSection->readString( "type", "" );

		if (loaded)
			uid_ = uid;
		else
			uid_ = pObject_->getUID();

		vloTransform_.postMultiply( pObject_->origin() );

		pObject_->addItem( this );
		return true;
	}

	if ( errorString )
	{
		*errorString = "Failed to load " + pSection->readString( "type", "<unknown type>" ) +
			" VLO " + pSection->readString( "uid", "<unknown id>" );
	}
	return false;
}

bool EditorChunkVLO::legacyLoad( DataSectionPtr pSection, Chunk * pChunk, std::string& type )
{
	BW_GUARD;

	localPos_ = Vector3(50,0,50); //middle it
	edTransform(); //initialise the transform
	vloTransform_ = Matrix::identity;
	vloTransform_.translation( -localPos_ );
	vloTransform_.postMultiply( pChunk->transformInverse() );


	//pSection->setParent(NULL);
	WorldManager::instance().changedChunk( pChunk );

	if (createLegacyVLO( pSection, pChunk, type ))
	{
		Matrix m( pObject_->edTransform() );
		m.postMultiply( pChunk->transform() );
		pObject_->updateLocalVars( m );

		//edTransform(m,false);

		type_ = type;
		uid_ = pObject_->getUID();

		vloTransform_.postMultiply( pObject_->origin() );

		pObject_->addItem( this );
		return true;
	}
	return false;
}
void EditorChunkVLO::changed()
{
	BW_GUARD;

	readonly_ = !VLOManager::instance()->writable( this );
}

void EditorChunkSubstance<ChunkVLO>::toss( Chunk * pChunk )
{
	BW_GUARD;

	if (pChunk_ != NULL)
	{
		if (pOwnSect_)
		{
			EditorChunkCache::instance( *pChunk_ ).
				pChunkSection()->delChild( pOwnSect_ );
			pOwnSect_ = NULL;
		}
	}

	this->ChunkVLO::toss( pChunk );

	if (pChunk_ != NULL)
	{
		if (!pOwnSect_ && pChunk_->loaded())
		{
			pOwnSect_ = EditorChunkCache::instance( *pChunk_ ).
				pChunkSection()->newSection( this->sectName() );
			this->edSave( pOwnSect_ );
		}
	}

	if (pObject_)
	{
		if (pObject_->lastDbItem())
		{
			ItemInfoDB::instance().toss( pObject_->lastDbItem(), false );
			pObject_->lastDbItem( NULL );

			if (!pChunk)
			{
				// If we are removing the lastDbItem, try to find another
				// suitable VLO item in its place.
				VeryLargeObject::ChunkItemList items = pObject_->chunkItems();

				for (VeryLargeObject::ChunkItemList::iterator
					it = items.begin(); it != items.end(); ++it)
				{
					if ((*it) != this && (*it)->chunk())
					{
						// Found another suitable item, toss it in.
						ItemInfoDB::instance().toss( (*it), true );
						pObject_->lastDbItem( *it );
						break;
					}
				}
			}
		}

		if (pChunk)
		{
			ItemInfoDB::instance().toss( this, true );
			pObject_->lastDbItem( this );
		}
	}
}


/**
 *
 *
 */
void EditorChunkVLO::edPreDelete()
{
	BW_GUARD;

	// Don't delete a VLO object when deleting a shell if the VLO is not
	// entirely contained inside the shell (edBelongToChunk).
	if ( chunk() && EditorChunkCache::instance( *chunk() ).edIsDeleting() &&
		!chunk()->isOutsideChunk() && !edBelongToChunk() )
		return;

	if ( pObject_ )
	{
		UndoRedo::instance().add(
			new VLOExistenceOperation( this, chunk() ) );
		VLOManager::instance()->deleteFromLoadedChunks( this );
		pObject_->edDelete( this );		
		#if UMBRA_ENABLE
		delete pUmbraDrawItem_;
		pUmbraDrawItem_ = NULL;
		pObject_->syncInit( NULL );	
		this->updateUmbraLenders();
		#endif
	}
}


/**
 *	This method does extra stuff when this item is tossed between chunks.
 *
 *	It makes sure the world variables used to create the water get updated.
 */
void EditorChunkVLO::toss( Chunk * pChunk )
{
	BW_GUARD;

	// get the template class to do the real work
	this->EditorChunkSubstance<ChunkVLO>::toss( pChunk );

	// and update our world vars if we're in a chunk
	if (pChunk_ != NULL)
	{
		this->updateWorldVars( pChunk_->transform() );

		if ( pObject_->isObjectCreated() )
			objectCreated();
	}
}


/**
 *
 *
 */
void EditorChunkSubstance<ChunkVLO>::addAsObstacle()
{
	BW_GUARD;

	ModelPtr model = reprModel();
	if (model)
	{
		Matrix world( pChunk_->transform() );
		world.preMultiply( this->edTransform() );
		ChunkVLOObstacle::instance( *pChunk_ ).addModel(
			model, world, this );
	}
}


/**
 *	This is called after water has finished with the collision scene,
 *	i.e. we can now add stuff that it would otherwise collide with
 */
void EditorChunkVLO::objectCreated()
{
	BW_GUARD;

	if (!colliding_)
	{
		this->EditorChunkSubstance<ChunkVLO>::addAsObstacle();
		colliding_ = true;
	}
}


/**
 *	Additional save
 */
void EditorChunkVLO::edChunkSave( )
{
	BW_GUARD;

	if (pObject_)
		pObject_->saveFile(chunk());
}


/**
 *	Save any property changes to this data section
 */
bool EditorChunkVLO::edSave( DataSectionPtr pSection )
{
	BW_GUARD;

	if (!pObject_)
		return false;

	pSection->writeString( "uid", uid_ );
	pSection->writeString( "type", type_ );

	pObject_->save();	
	return true;
}

class VLOCloneNotifier : CloneNotifier
{
	std::map<UniqueID,UniqueID> idMap_;
	std::set<DataSectionPtr> sects_;
	void begin()
	{
		idMap_.clear();
		sects_.clear();
	}
	void end()
	{
		idMap_.clear();
		sects_.clear();
	}
public:
	UniqueID get( UniqueID id )
	{
		if( !in( id ) )
			idMap_[ id ] = UniqueID::generate();
		return idMap_[ id ];
	}
	bool in( UniqueID id ) const
	{
		return idMap_.find( id ) != idMap_.end();
	}
	void add( DataSectionPtr sect )
	{
		sects_.insert( sect );
	}
}
VLOCloneNotifier;

/**
 * get the DataSection for clone
 */
void EditorChunkVLO::edCloneSection( Chunk* destChunk, const Matrix& destMatrixInChunk, DataSectionPtr destDS )
{
	BW_GUARD;

	ChunkVLO::edCloneSection( destChunk, destMatrixInChunk, destDS );
	if( type_ == "water" )
	{
		if( !VLOCloneNotifier.in( uid_ ) )
		{
			std::string uid = VLOCloneNotifier.get( uid_ );
			strlwr( &uid[0] );

			DataSectionPtr newDS = BWResource::openSection( chunk()->mapping()->path() + '/' + uid + ".vlo", false );

			if (!newDS)
			{
				newDS = BWResource::openSection( chunk()->mapping()->path() + "/_" + uid + ".vlo", true );
			}

			newDS->copy( pObject_->section() );
			Matrix m = destMatrixInChunk;
			m.postMultiply( destChunk->transform() );
			newDS->writeVector3( "water/position", m.applyToOrigin() );
			VLOCloneNotifier.add( newDS );
		}
		std::string uid = VLOCloneNotifier.get( uid_ );
		strlwr( &uid[0] );
		destDS->writeString( "uid", uid );
	}
}

/**
 * refine the DataSection for chunk clone
 */
bool EditorChunkVLO::edPreChunkClone( Chunk* srcChunk, const Matrix& destChunkMatrix, DataSectionPtr chunkDS )
{
	BW_GUARD;

	if( type_ == "water" )
	{
		if( edBelongToChunk() )
		{
			// The water is completely contained inside the shell, so clone it.
			std::vector<DataSectionPtr> vlos;
			chunkDS->openSections( "vlo", vlos );

			bool in = VLOCloneNotifier.in( uid_ );

			std::string uid = VLOCloneNotifier.get( uid_ );
			strlwr( &uid[0] );

			for( std::vector<DataSectionPtr>::iterator iter = vlos.begin();
				iter != vlos.end(); ++iter )
			{
				if( (*iter)->readString( "uid" ) == uid_ )
				{
					(*iter)->writeString( "uid", uid );
				}
			}
			if( !in )
			{
				DataSectionPtr newDS = BWResource::openSection( chunk()->mapping()->path() +  '/' + uid + ".vlo", false );

				if (!newDS)
				{
					newDS = BWResource::openSection( chunk()->mapping()->path() + "/_" + uid + ".vlo", true );
				}

				newDS->copy( pObject_->section() );
				Matrix m = edTransform();
				m.postMultiply( destChunkMatrix );
				newDS->writeVector3( "water/position", m.applyToOrigin() );
				VLOCloneNotifier.add( newDS );
			}
		}
		else
		{
			std::vector<DataSectionPtr> vlos;
			chunkDS->openSections( "vlo", vlos );

			for( std::vector<DataSectionPtr>::iterator iter = vlos.begin();
				iter != vlos.end(); ++iter )
			{
				if( (*iter)->readString( "uid" ) == uid_ )
					chunkDS->delChild( *iter );
			}
		}
	}
	return true;
}


/**
 *	Returns true if the chunk is an outside chunk, or if it's a shell and the
 *  VLO is entirely contained inside the shell.
 */
bool EditorChunkVLO::edBelongToChunk()
{
	BW_GUARD;

	if ( !object() || !chunk() )
		return false;

	if ( chunk()->isOutsideChunk() )
		return true;

	// If the VLO is inside a shell, if it belongs to it only if the VLO's BB
	// is entirely containted inside the shell's BB.
	BoundingBox bb = object()->boundingBox();
	BoundingBox chunkBB = chunk()->boundingBox();

	return chunkBB.intersects( bb.minBounds() ) && chunkBB.intersects( bb.maxBounds() );
}


void EditorChunkVLO::edPostClone( EditorChunkItem* srcItem )
{
	BW_GUARD;

	VLOManager::instance()->updateReferences( this->object() );
	WorldManager::instance().changedChunk( pChunk_ );

	MetaData::updateCreationInfo( pObject_->metaData() );

	EditorChunkItem::edPostClone( srcItem );
}


void EditorChunkVLO::edPostModify()
{
	BW_GUARD;

	MetaData::updateModificationInfo( pObject_->metaData() );
}


/**
 *	Get the current transform
 */
const Matrix & EditorChunkVLO::edTransform()
{
	BW_GUARD;

	if (pObject_ && pChunk_)
		transform_ = pObject_->localTransform( pChunk_ );

	return transform_;
}


/**
 *	Change our transform, temporarily or permanently
 */
bool EditorChunkVLO::edTransform( const Matrix & m, bool transient )
{
	BW_GUARD;

	MF_ASSERT( Moo::g_renderThread );
	// it's permanent, so find out where we belong now
	Chunk * pOldChunk = pChunk_;
	Chunk * pNewChunk = this->edDropChunk( m.applyToOrigin() );	
	if (pNewChunk == NULL) return false;

	drawTransient_=transient;
	// if this is only a temporary change, keep it in the same chunk
	if (transient)
	{
		transform_ = m;
		this->updateLocalVars( transform_ , pChunk_ );
		this->updateWorldVars( pNewChunk->transform() );
		return true;
	}

	// ok, accept the transform change then
	Matrix oldTransform = transform_;
	transform_.multiply( m, pOldChunk->transform() );
	transform_.postMultiply( pNewChunk->transformInverse() );
	this->updateLocalVars( transform_, pNewChunk );

	// make sure the new chunks aren't readonly (we don't check the old chunks
	// because the water shouldn't be selectable if the old chunks are readonly
	// in the first place!).
	if ( !VLOManager::instance()->writable( this ) )
	{
		// Readonly, so restore position.
		transform_ = oldTransform;
		this->updateLocalVars( transform_, pOldChunk );
		WorldManager::instance().addCommentaryMsg(LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_VLO/POSITION_LOCKED"));
		return false;
	}

	// Tell the manager to mark the old and new areas have changed. To do this, 
	// we have to update the VLO to the old position again, and then restore to
	// the new position after marking the old chunks as changed.
	Matrix newTransform = transform_;
	transform_ = oldTransform;
	this->updateLocalVars( transform_, pOldChunk );
	VLOManager::instance()->markChunksChanged( this );
	
	transform_ = newTransform;
	this->updateLocalVars( transform_, pNewChunk );
	VLOManager::instance()->markChunksChanged( this );

	pOldChunk->delStaticItem( this );
	if ( pOldChunk != pNewChunk )
	{
		// If source and destination chunks are different, delete any
		// references to tho VLO object that are in the pNewChunk first.
		VLOManager::instance()->deleteReference( this->object().getObject(), pNewChunk );
	}
	pNewChunk->addStaticItem( this );
	// Notify the VLO manager, which will add/remove references accordingly.
	VLOManager::instance()->updateReferencesImmediately( this->object() );

	VLOManager::instance()->markAsDirty( uid_ );

	WorldManager::instance().changedChunk( pOldChunk );
	WorldManager::instance().changedChunk( pNewChunk );

	//NOTE: world vars will get updated when we are tossed into the new chunk
	return true;
}


/**
 *	Add the properties in the given editor
 */
bool EditorChunkVLO::edEdit( class GeneralEditor & editor )
{
	BW_GUARD;

	if (pObject_)
	{
		if (this->edFrozen())
			return false;
		
		return pObject_->edEdit( editor, this );
	}
	return true;
}


/**
 *	
 */
void EditorChunkVLO::removeCollisionScene()
{
	BW_GUARD;

	if (pChunk_)
	{
		ChunkVLOObstacle::instance( *pChunk_ ).delObstacles( this );
		colliding_ = false;
	}
}


/**
 *	
 */
void EditorChunkVLO::updateTransform( Chunk * pChunk )
{
	BW_GUARD;

	if (pObject_)
	{
		localPos_ = Vector3(50,0,50); //TODO: un-hardcode the chunk size stuff
		vloTransform_ = Matrix::identity;
		vloTransform_.translation( -localPos_ );
		vloTransform_.postMultiply( pChunk->transformInverse() );		
		vloTransform_.postMultiply( pObject_->origin() );
	}
}


/**
 *	This method updates our local vars from the transform
 */
void EditorChunkVLO::updateLocalVars( const Matrix & m, Chunk * pChunk )
{
	BW_GUARD;

	if ( pObject_ && pChunk )
	{
		Matrix world(m);
		world.postMultiply( pChunk->transform() );
		pObject_->updateLocalVars(world);
	}
}


/**
 *	This method updates our caches of world space variables
 */
void EditorChunkVLO::updateWorldVars( const Matrix & m )
{
	BW_GUARD;

	if (pObject_)
		pObject_->updateWorldVars( m );
}


/**
 *	Hide/Show the the chunk item.
 *
 */
/*virtual*/
void EditorChunkVLO::edHidden( bool value )
{
	BW_GUARD;

	if (pObject_)
		pObject_->edHidden( value );
}


/**
 *	Retreive the hidden status for this item.
 *
 * @return The current hidden status.
 */
/*virtual*/
bool EditorChunkVLO::edHidden( ) const
{
	BW_GUARD;

	if (pObject_)
		return pObject_->edHidden( );
	else
		return false;
}


/**
 *	Freeze/Unfreeze the the chunk item.
 *
 */
/*virtual*/
void EditorChunkVLO::edFrozen( bool value )
{
	BW_GUARD;

	if (pObject_)
		return pObject_->edFrozen( value );
}


/**
 *	Retreive the frozen status for this item.
 *
 * @return The current frozen status.
 */
/*virtual*/
bool EditorChunkVLO::edFrozen( ) const
{
	BW_GUARD;

	if (pObject_)
		return pObject_->edFrozen( );
	else
		return false;
}


bool EditorChunkVLO::edCanAddSelection() const
{ 
	return
		chunk() != NULL && !readonly_;
}


/**
 *	Notifies the VLO that it has been selected, so it updates the VLO's chunk
 *	item in the scene browser with the VLO chunk item just selected.
 */
void EditorChunkVLO::edSelected()
{
	BW_GUARD;

	MF_ASSERT( pChunk_ );

	if (pObject_->lastDbItem() != this)
	{
		if (pObject_->lastDbItem())
		{
			ItemInfoDB::instance().toss( pObject_->lastDbItem(), false );
			pObject_->lastDbItem( NULL );
		}

		ItemInfoDB::instance().toss( this, true );
		pObject_->lastDbItem( this );
	}
}


/**
 *	The overridden edShouldDraw method
 */
bool EditorChunkVLO::edShouldDraw()
{
	BW_GUARD;

	if( pOwnSect()->readString("type", "") == "water" )
	{
		// This is a hack for water, Since EditorChunkWater is not really a ChunkItem
		bool showOutside = 
			!s_hideAllOutside_ ||			// not hidding outside objects, or...
			(!chunk()->isOutsideChunk() &&	// hidding outside objects, but we are in a shell, and...
			 object() && object()->visibleInside()); // the VLO is visible inside shells.

		return OptionsScenery::waterVisible() && showOutside && !this->edHidden();
	}
	return EditorChunkSubstance<ChunkVLO>::edShouldDraw() && !this->edHidden();;
}


/**
 *	
 */
void EditorChunkVLO::draw()
{
	BW_GUARD;

	if( !edShouldDraw() )
		return;

	if (pObject_)
	{	
		bool drawRed = readonly_ && OptionsMisc::readOnlyVisible();

		bool projectModule = ProjectModule::currentInstance() == ModuleManager::instance().currentModule();

		pObject_->drawRed( drawRed && !projectModule );

		if ( highlight_ )
		{
			// Set the VLO object to highlight, and reset the flag.
			pObject_->highlight(true);
			highlight_ = false;
		}
	}

	ChunkVLO::draw();

	ModelPtr model = reprModel();
	if 
	( 
		(drawTransient_ || WorldManager::instance().drawSelection()) 
		&& 
		pChunk_ 
		&& 
		pObject_ 
		&& 
		model
	)
	{
		if (WorldManager::instance().drawSelection())
		{
			WorldManager::instance().registerDrawSelectionItem( this );
		}
		Moo::rc().push();
		Moo::rc().preMultiply( pObject_->localTransform(pChunk_) );
		static Matrix offset(Matrix::identity);
		offset.translation(Vector3(0.f,0.1f,0.f));
		Moo::rc().preMultiply( offset );
		Model::incrementBlendCookie();

		model->dress();	// should really be using a supermodel...
		model->draw( true );

		Moo::rc().pop();
	}
}


/**
 *	Return a modelptr that is the representation of this chunk item
 */
ModelPtr EditorChunkVLO::reprModel() const
{
	return waterModel_;
}


/// Write the factory statics stuff
ChunkItemFactory::Result EditorChunkVLO::create( Chunk * pChunk, DataSectionPtr pSection )
{
	BW_GUARD;

	//TODO: generalise the want flags...
    // wantsDraw, wantsSway, wantsNest.  The wantsNest option is specific to 
    // water and should be fixed when new VLOs are added.
	EditorChunkVLO * pVLO = new EditorChunkVLO();
	
	std::string errorString;
	bool result = pVLO->load( pSection, pChunk, &errorString );
	bool isDeleted = VLOManager::instance()->isDeleted( pSection->readString( "uid" ) );
	bool itemAlreadyExists = VLOManager::instance()->contains( pChunk, pSection->readString( "uid" ) );

	if ( !result || isDeleted || itemAlreadyExists )
	{
		delete pVLO;
		EditorChunkCache::instance( *pChunk ).addInvalidSection( pSection );
		WorldManager::instance().changedChunk( pChunk );
		if ( isDeleted || itemAlreadyExists )
		{
			// Marked as deleted, so remove from the chunk and report success.
			return ChunkItemFactory::SucceededWithoutItem();
		}
		else
		{
			// Couldn't load, so remove from the chunk and report an error.
			return ChunkItemFactory::Result( NULL, errorString );
		}
	}
	else
	{
		// Check to see if the chunk really contains part or all the VLO.
		BoundingBox bb;
		pVLO->edBounds( bb );
		Math::OrientedBBox vloBox( bb, pVLO->object()->localTransform() );
		Math::OrientedBBox chunkBox( pChunk->localBB(), pChunk->transform() );
		if ( !vloBox.intersects( chunkBox ) )
		{
			// This chunk shouldn't contain the VLO, so remove it.
			pVLO->object()->removeItem( pVLO );
			EditorChunkCache::instance( *pChunk ).addInvalidSection( pSection );
			WorldManager::instance().changedChunk( pChunk );
			return ChunkItemFactory::SucceededWithoutItem();
		}
		// Successful load.
		VLOManager::instance()->setOriginalBounds(
			pVLO->object()->getUID(), bb, pVLO->object()->localTransform() );
		VLOManager::instance()->setMovedBounds(
			pVLO->object()->getUID(), bb, pVLO->object()->localTransform() );

		pVLO->readonly_ = !VLOManager::instance()->writable( pVLO );

		pChunk->addStaticItem( pVLO );
		return ChunkItemFactory::Result( pVLO );
	}
}


/// Static factory initialiser
ChunkItemFactory EditorChunkVLO::factory_( "vlo", 0, EditorChunkVLO::create );
