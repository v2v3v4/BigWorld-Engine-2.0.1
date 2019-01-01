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
#include "worldeditor/world/vlo_manager.hpp"
#include "worldeditor/world/world_manager.hpp"
#include "worldeditor/world/editor_chunk.hpp"
#include "worldeditor/world/editor_chunk_overlapper.hpp"
#include "worldeditor/world/items/editor_chunk_vlo.hpp"
#include "math/polygon.hpp"
#include "math/oriented_bbox.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_manager.hpp"


// Instance pointer implementation.
/*static*/ VLOManager* VLOManager::s_pInstance_ = NULL;


/**
 *  Constructor
 */
VLOManager::VLOManager() :
	enableUpdateChunkReferences_( true )
{
}


/**
 *  This static method is the Singleton instance for the object.
 *
 *	@return		VLOManager singleton instance.
 */
/*static*/ VLOManager* VLOManager::instance()
{
	if (s_pInstance_ == NULL)
	{
		BW_GUARD;

		s_pInstance_ = new VLOManager();
	}
	return s_pInstance_;
}


/**
 *  This static method deallocates the Singleton instance.
 */
/*static*/ void VLOManager::fini()
{
	BW_GUARD;

	delete s_pInstance_;
	s_pInstance_ = NULL;
}


/**
 *  This method resets all internal lists, i.e. when changing space.
 */
void VLOManager::clearLists()
{
	BW_GUARD;

	SimpleMutexHolder mutex( listMutex_ );
	// Clearing by swap to make sure the capacity is reset to 0
	dirtySet_.swap( VLOIDSet() );
	deletedSet_.swap( VLOIDSet() );
	needsCleanupSet_.swap( VLOIDSet() );
	originalBoundsMap_.swap( BoundsMap() );
	movedBoundsMap_.swap( BoundsMap() );
}


/**
 *  This method resets the list of dirty (modified) VLOs.
 */
void VLOManager::clearDirtyList()
{
	BW_GUARD;

	SimpleMutexHolder mutex( listMutex_ );
	// Clearing by swap to make sure the capacity is reset to 0
	dirtySet_.swap( VLOIDSet() );
}


/**
 *  This method allows marking a VLO as dirty, and also to unmark it.
 *
 *  @param id		GUID of the VLO.
 *	@param mark		If true, the VLO is marked as dirty, otherwise it's
 *					marked as clean (not modified).
 */
void VLOManager::markAsDirty( const UniqueID& id, bool mark /*=true*/ )
{
	BW_GUARD;

	SimpleMutexHolder mutex( listMutex_ );
	if (mark)
	{
		dirtySet_.insert( id );
		
		// Also tag the VLO itself as dirty, so it gets saved.
		VeryLargeObjectPtr pVLO = VeryLargeObject::getObject( id );
		/* TODO: for BigWorld version 1.9.1
		 * This assert is temporarily disabled to prevent world editor from crashing
		 * due to a bug that occurs when multiple undo/redo operations on water objects
		 * are run in the same frame. This causes the editor to "lose" the water object 
		 * and which thereafter gets destructed and the VeryLargeObject::s_uniqueObjects_ map 
		 * has the UID mapped to NULL.
		 * See bug 17607 http://bugs.bigworldtech.com/show_bug.cgi?id=17607
		 * The assert should be reenabled when the bug is fixed. 
		 */

		//SY: got a better solution for this bug; http://bugs.bigworldtech.com/show_bug.cgi?id=17607
		MF_ASSERT( pVLO != NULL );
		pVLO->dirty();
	}
	else
	{
		dirtySet_.erase( id );
	}
}


/**
 *  This method returns true if the VLO is marked as dirty.
 *
 *  @param id	GUID of the VLO.
 *  @return		True if it's marked as dirty, false otherwise.
 */
bool VLOManager::isDirty( const UniqueID& id ) const
{
	BW_GUARD;

	SimpleMutexHolder mutex( listMutex_ );
	return dirtySet_.find( id ) != dirtySet_.end();
}


/**
 *  This method allows marking a VLO as deleted, and also to unmark it.
 *
 *  @param id		GUID of the VLO
 *	@param mark		If true, the VLO is marked as deleted, otherwise it's
 *					marked as not-deleted.
 */
void VLOManager::markAsDeleted( const UniqueID& id, bool mark /*=true*/ )
{
	BW_GUARD;

	SimpleMutexHolder mutex( listMutex_ );
	// If changing the deleted state, mark as dirty as well, i.e. when undoing.
	dirtySet_.insert( id );

	// Also tag the VLO itself as dirty, so it gets saved.
	VeryLargeObjectPtr pVLO = VeryLargeObject::getObject( id );
	if (pVLO)
	{
		pVLO->dirty();
	}

	if (mark)
	{
		deletedSet_.insert( id );
	}
	else
	{
		deletedSet_.erase( id );
	}
}


/**
 *  This method returns true if the VLO is marked as deleted.
 *
 *  @param id	GUID of the VLO.
 *  @return		True if it's marked as deleted, false otherwise.
 */
bool VLOManager::isDeleted( const UniqueID& id ) const
{
	BW_GUARD;

	SimpleMutexHolder mutex( listMutex_ );
	return deletedSet_.find( id ) != deletedSet_.end();
}


/**
 *  This method returns true if the VLO is marked as deleted.
 *
 *  @param id	GUID of the VLO.
 *  @return		True if it's marked as deleted, false otherwise.
 */
bool VLOManager::needsCleanup( const UniqueID& id ) const
{
	BW_GUARD;

	SimpleMutexHolder mutex( listMutex_ );
	return needsCleanupSet_.find( id ) != needsCleanupSet_.end();
}


/**
 *  This method updates the original bounds of a VLO in the manager, which are
 *  the bounds of the VLO as stored in the chunks in disk.
 *
 *  @param id		GUID of the VLO.
 *  @param bb		BoundingBox before transform.
 *  @param matrix	Transformation matrix VLO.
 */
void VLOManager::setOriginalBounds( const UniqueID& id,
								const BoundingBox& bb, const Matrix& matrix )
{
	BW_GUARD;

	SimpleMutexHolder mutex( listMutex_ );
	Bounds bounds( bb, matrix );
	originalBoundsMap_[ id ] = bounds;
}


/**
 *  This method removes a VLO from the lists. This is called when all chunks
 *	that contain a reference to it have been unloaded.
 *
 *  @param id		GUID of the VLO.
 */
void VLOManager::removeFromLists( const UniqueID& id )
{
	BW_GUARD;

	SimpleMutexHolder mutex( listMutex_ );

	dirtySet_.erase( id );
	deletedSet_.erase( id );
	needsCleanupSet_.erase( id );
	originalBoundsMap_.erase( id );
	movedBoundsMap_.erase( id );
}


/**
 *  This method updates the moved bounds of a VLO in the manager, which are the
 *  bounds of the VLO after it has been moved/scaled/rotated.
 *
 *  @param id		GUID of the VLO.
 *  @param bb		BoundingBox before transform.
 *  @param matrix	Transformation matrix VLO.
 */
void VLOManager::setMovedBounds( const UniqueID& id,
								const BoundingBox& bb, const Matrix& matrix )
{
	BW_GUARD;

	SimpleMutexHolder mutex( listMutex_ );
	Bounds bounds( bb, matrix );
	movedBoundsMap_[ id ] = bounds;
}


/**
 *  This method returns whether a chunk already contains a VLO.
 *
 *  @param pChunk	Chunk to test if it has the VLO.
 *  @param id		GUID of the VLO to search.
 *  @return			True if the chunk already contains the VLO.
 */
bool VLOManager::contains( const Chunk* pChunk, const UniqueID& id ) const
{
	BW_GUARD;

	VeryLargeObjectPtr pExistingVlo = VeryLargeObject::getObject( id );
	return (pExistingVlo != NULL) && pExistingVlo->containsChunk( pChunk );
}


/**
 *  This method returns the list of columns (as outside chunk IDs) that are
 *  touched by the transformed bounding box of a VLO.
 *
 *  @param bb			BoundingBox before transform.
 *  @param matrix		Transformation matrix VLO.
 *  @param returnSet	Return set that will contain the touched columns.
 */
void VLOManager::columnsTouchedByBounds(
		const BoundingBox& bb, const Matrix& matrix,
		std::set< std::string >& returnSet ) const
{
	BW_GUARD;

	// first, find out the potential grid or chunks from the axis-aligned bb
	BoundingBox axisBB = bb;
	axisBB.transformBy( matrix );
	int minGridX = int( floorf( axisBB.minBounds()[0] / GRID_RESOLUTION ) );
	int maxGridX = int( floorf( axisBB.maxBounds()[0] / GRID_RESOLUTION ) );
	int minGridZ = int( floorf( axisBB.minBounds()[2] / GRID_RESOLUTION ) );
	int maxGridZ = int( floorf( axisBB.maxBounds()[2] / GRID_RESOLUTION ) );
	for (int z = minGridZ; z <= maxGridZ; ++z)
	{
		for (int x = minGridX; x <= maxGridX; ++x)
		{
			// Intersect the grid with the Bounding Box, by intersecting each
			// of the 6 faces of the bb with the grid quad in 2D, projecting to
			// the X,Z plane.

			// create quad for the column
			float absX = x * GRID_RESOLUTION;
			float absZ = z * GRID_RESOLUTION;
			Math::Polygon2D gridQuad;
			gridQuad.addPoint( Vector2( absX, absZ ) );
			gridQuad.addPoint( Vector2( absX + GRID_RESOLUTION, absZ ) );
			gridQuad.addPoint( Vector2( absX + GRID_RESOLUTION, absZ + GRID_RESOLUTION ) );
			gridQuad.addPoint( Vector2( absX, absZ + GRID_RESOLUTION ) );

			// Get transformed BBox points, and project them to the X,Z plane
			Math::OrientedBBox bb3D( bb, matrix );
			std::vector< Vector2 > bbPts;
			for (int i = 0; i < 8; ++i)
			{
				bbPts.push_back( Vector2( bb3D.point(i)[0], bb3D.point(i)[2] ) );
			}

			std::vector< Math::Polygon2D > bbQuads;
			Math::Polygon2D quad;
			quad.addPoint( bbPts[0] );
			quad.addPoint( bbPts[1] );
			quad.addPoint( bbPts[2] );
			quad.addPoint( bbPts[3] );
			bbQuads.push_back( quad );

			quad.clear();
			quad.addPoint( bbPts[4] );
			quad.addPoint( bbPts[5] );
			quad.addPoint( bbPts[6] );
			quad.addPoint( bbPts[7] );
			bbQuads.push_back( quad );

			quad.clear();
			quad.addPoint( bbPts[0] );
			quad.addPoint( bbPts[1] );
			quad.addPoint( bbPts[5] );
			quad.addPoint( bbPts[4] );
			bbQuads.push_back( quad );

			quad.clear();
			quad.addPoint( bbPts[1] );
			quad.addPoint( bbPts[2] );
			quad.addPoint( bbPts[6] );
			quad.addPoint( bbPts[5] );
			bbQuads.push_back( quad );

			quad.clear();
			quad.addPoint( bbPts[2] );
			quad.addPoint( bbPts[3] );
			quad.addPoint( bbPts[7] );
			quad.addPoint( bbPts[6] );
			bbQuads.push_back( quad );

			quad.clear();
			quad.addPoint( bbPts[3] );
			quad.addPoint( bbPts[0] );
			quad.addPoint( bbPts[4] );
			quad.addPoint( bbPts[7] );
			bbQuads.push_back( quad );

			for (std::vector< Math::Polygon2D >::const_iterator it = bbQuads.begin();
				it != bbQuads.end(); ++it)
			{
				// 2D collision test
				if (gridQuad.intersects( *it ))
				{
					std::string columnId = 
						WorldManager::instance().geometryMapping()->outsideChunkIdentifier( x, z );
					if (!columnId.empty())
					{
						returnSet.insert( columnId );
					}
					break;
				}
			}
		}
	}
}


/**
 *  This method returns all the columns affected by all VLOs: the ones
 *  touched by the original bounds and the ones touched by the moved bounds.
 *
 *  @param bb			BoundingBox before transform.
 *  @param matrix		Transformation matrix VLO.
 *  @param returnSet	Return set that will contain the touched columns.
 */
void VLOManager::getDirtyColumns( std::set< std::string >& columns ) const
{
	BW_GUARD;

	SimpleMutexHolder mutex( listMutex_ );
	for (BoundsMap::const_iterator it = originalBoundsMap_.begin();
		it != originalBoundsMap_.end(); ++it)
	{
		if (dirtySet_.find( (*it).first ) != dirtySet_.end())
		{
			columnsTouchedByBounds(
				(*it).second.first, (*it).second.second, columns );
		}
	}

	for (BoundsMap::const_iterator it = movedBoundsMap_.begin();
		it != movedBoundsMap_.end(); ++it)
	{
		if (dirtySet_.find( (*it).first ) != dirtySet_.end())
		{
			columnsTouchedByBounds(
				(*it).second.first, (*it).second.second, columns );
		}
	}
}


/**
 *  This method resets the original bounds to be the same as the moved bounds.
 *  It also cleans up the VLOs that need to be cleaned up.
 *  This method is called after a VLO is saved.
 */
void VLOManager::postSave()
{
	BW_GUARD;

	SimpleMutexHolder mutex( listMutex_ );
	
	// Reset the original bounds
	for (BoundsMap::iterator it = originalBoundsMap_.begin();
		it != originalBoundsMap_.end(); ++it)
	{
		BoundsMap::iterator movedIt = movedBoundsMap_.find( (*it).first );
		if (movedIt != movedBoundsMap_.end())
		{
			originalBoundsMap_[ (*it).first ] =
				Bounds( (*movedIt).second.first, (*movedIt).second.second );
		}
	}

	// Mark VLOs that where deleted and saved, for later removed of the files
	// associated with the VLO.
	for (VLOIDSet::iterator it = dirtySet_.begin();
		it != dirtySet_.end(); ++it)
	{
		if (deletedSet_.find( *it ) != deletedSet_.end())
		{
			needsCleanupSet_.insert( *it );
		}
		else
		{
			VLOIDSet::iterator j = needsCleanupSet_.find( *it );
			if (j != needsCleanupSet_.end())
			{
				needsCleanupSet_.erase( j );
			}
		}
	}
}


/**
 *  This method deletes all loaded references to a vlo from the space, except
 *  the one passed in as a parameter. This reference gets deleted by WE later.
 *
 *  @param pVLO	Reference that won't get deleted inside this method.
 */
void VLOManager::deleteFromLoadedChunks( const EditorChunkVLO* pVLORef )
{
	BW_GUARD;

	if ((pVLORef == NULL) || (pVLORef->object() == NULL))
	{
		return;
	}

	VeryLargeObject::ChunkItemList items = pVLORef->object()->chunkItems();
	for (VeryLargeObject::ChunkItemList::iterator it = items.begin();
		it != items.end(); ++it)
	{
 		if ((*it)->chunk())
		{
			WorldManager::instance().changedChunk( (*it)->chunk() );
			// The item pointed by "vlo" will get deleted by delStaticItem by
			// the normal WE item delete code, so do it only for the others.
	 		if ((*it) != pVLORef)
			{
				(*it)->chunk()->delStaticItem( (*it) );
			}
		}
	}
}


/**
 *  This method collides the bounding box of the chunk against all VLOs, and
 *  creates or removes VLO references from the chunk accordingly. An oriented
 *  bounding box is used for the chunk as well in case it's a shell.
 *
 *	@param pChunk	Chunk where references will be created and/or deleted.
 */
void VLOManager::updateChunkReferences( Chunk* pChunk )
{
	BW_GUARD;

	if (!enableUpdateChunkReferences_)
	{
		return;
	}

	SimpleMutexHolder mutex( listMutex_ );
	for (BoundsMap::iterator it = movedBoundsMap_.begin();
		it != movedBoundsMap_.end(); ++it)
	{
		if (deletedSet_.find( (*it).first ) != deletedSet_.end())
		{
			continue;
		}

		VeryLargeObjectPtr pVLO = VeryLargeObject::getObject( (*it).first );
		if (pVLO == NULL)
		{
			// This can be NULL in some cases, such as when a chunk auto-fixes
			// itself on load, and is safe to continue.
			continue;
		}

		if ((!pVLO->visibleInside() && !pChunk->isOutsideChunk()) ||
			(!pVLO->visibleOutside() && pChunk->isOutsideChunk()))
		{
			// VLO is outside only, and the chunk is a shell, so remove the
			// reference if it has one.
			if (pVLO->containsChunk( pChunk ))
			{
				deleteReference( pVLO, pChunk );
			}
			continue;
		}

		Math::OrientedBBox vloBox( (*it).second.first, (*it).second.second );
		Math::OrientedBBox chunkBox( pChunk->localBB(), pChunk->transform() );
		if (vloBox.intersects( chunkBox ))
		{
			if (!pVLO->containsChunk( pChunk ))
			{
				createReference( pVLO, pChunk );

				if (!pVLO->visibleOutside() && !pChunk->isOutsideChunk())
				{
					// If this chunk is a shell, and the VLO is visible inside
					// only, make sure the VLO doesn't have references to outside
					// chunks. This can happen when an inside-only VLO is added
					// to a space and it doesn't intersect any shell, in which
					// case it's forced to be visible outside.
					VeryLargeObject::ChunkItemList items = pVLO->chunkItems();
					for (VeryLargeObject::ChunkItemList::iterator it =
																items.begin();
						it != items.end(); ++it)
					{
						if ((*it)->chunk() &&
							(*it)->chunk()->isOutsideChunk())
						{
							deleteReference( pVLO, (*it)->chunk() );
						}
					}
				}
			}
		}
		else
		{
			if (pVLO->containsChunk( pChunk ))
			{
				if (!pVLO->visibleOutside() && !pChunk->isOutsideChunk())
				{
					// If this chunk is a shell, and the VLO is visible inside only,
					// and the VLO has only one reference (must be in a shell), force
					// the VLO to be visible outside to avoid having the water invisible
					// all the time.
					VeryLargeObject::ChunkItemList items = pVLO->chunkItems();
					if (items.size() == 1)
					{
						// Get the outside chunks this VLO touches, and add
						// references to them.
						std::set< std::string > touchedColumns;
						columnsTouchedByBounds(
							(*it).second.first, (*it).second.second,
							touchedColumns );

						for (std::set< std::string >::iterator it =
								touchedColumns.begin();
							it != touchedColumns.end(); ++it)
						{
							Chunk* pChunk =
								ChunkManager::instance().findChunkByName(
									*it,
									WorldManager::instance().geometryMapping(),
									false );

							if (pChunk && pChunk->loaded())
							{
								createReference( pVLO, pChunk );
							}
						}
					}
				}

				// delete the reference to this chunk
				deleteReference( pVLO, pChunk );
			}
		}
	}
}


/**
 *	This method enables/disables updating chunk references. Used when moving
 *  shells that contain VLOs.
 *
 *  @param enable	True to enable checking chunk references.
 */
void VLOManager::enableUpdateChunkReferences( bool enable )
{
	enableUpdateChunkReferences_ = enable;
}


/**
 *  This method creates a reference to a VLO in a chunk.
 *
 *  @param pVLO		VLO to add to the chunk
 *  @param pChunk	Chunk that will get the reference to the VLO.
 */
bool VLOManager::createReference( const VeryLargeObjectPtr pVLO,
								 Chunk* pChunk )
{
	BW_GUARD;

	EditorChunkVLO* pItem = new EditorChunkVLO( pVLO->type() );
	if (pItem->load( pVLO->getUID(), pChunk ))
	{
		pChunk->addStaticItem( pItem );
		pVLO->addItem( pItem );
		WorldManager::instance().changedChunk( pChunk );
		pItem->syncInit();
		return true;
	}
	delete pItem;
	return false;
}


/**
 *  This method deletes a reference to a VLO from a chunk.
 *
 *  @param pVLO		VLO to remove from the chunk
 *  @param pChunk	Chunk that contains the reference to the VLO.
 */
bool VLOManager::deleteReference( const VeryLargeObjectPtr pVLO,
								 Chunk* pChunk )
{
	BW_GUARD;

	EditorChunkVLOPtr pItem =
		static_cast< EditorChunkVLO* >( pVLO->containsChunk( pChunk ) );
	if (pItem == NULL)
	{
		return false;
	}

	pVLO->removeItem( pItem.get() );
	pChunk->delStaticItem( pItem );
	pItem->syncInit();
	WorldManager::instance().changedChunk( pChunk );
	return true;
}


/**
 *	This method marks all the chunks touched by a VLO as changed, to ensure
 *  they get saved to disk on the next save.
 *
 *	@param root		An EditorChunkVLO object, used to get the actual VLO.
 */
void VLOManager::markChunksChanged( const EditorChunkVLO* pVLORef )
{
	BW_GUARD;

	MF_ASSERT( pVLORef != NULL );
	VeryLargeObjectPtr pVLO = pVLORef->object();
	MF_ASSERT( pVLO != NULL );
	if (pVLO == NULL)
	{
		return;
	}

	BoundingBox vloBB;
	pVLORef->edBounds( vloBB );

	// Mark the loaded chunks touched by the water as changed.
	std::set< std::string > touchedColumns;
	columnsTouchedByBounds( vloBB, pVLO->localTransform(), touchedColumns );

	for (std::set< std::string >::iterator it = touchedColumns.begin();
		it != touchedColumns.end(); ++it)
	{
		Chunk* pChunk = ChunkManager::instance().findChunkByName(
					*it, WorldManager::instance().geometryMapping(), false );
		if (pChunk && pChunk->loaded())
		{
			if (pVLO->visibleOutside())
			{
				WorldManager::instance().changedChunk( pChunk );
			}

			if (pVLO->visibleInside())
			{
				// find shells inside the chunk, and them changed too!
				EditorChunkOverlappers::Items shells =
					EditorChunkOverlappers::instance( *pChunk ).overlappers();
				for (EditorChunkOverlappers::Items::iterator it = shells.begin();
					it != shells.end(); ++it)
				{
					Chunk* pShell = (*it)->pOverlapper();

					if (pShell && pShell->loaded())
					{
						Math::OrientedBBox vloBox( vloBB, pVLO->localTransform() );
						Math::OrientedBBox shellBox(
										pShell->localBB(), pShell->transform() );
						if (vloBox.intersects( shellBox ))
						{
							WorldManager::instance().changedChunk( pShell );
						}
					}
				}
			}
		}
	}
}


/**
 *	This private helper method returns a VLO reference. If there is a reference
 *  in the chunk already, just use it. If not, then try to recycle one of
 *  the references in the "movableItems" list, but if that list is empty,
 *  simply create a new reference in the chunk.
 *
 *  @param pVLO				VLO to use when creating the new reference.
 *  @param pChunk			Chunk where we want to create the reference.
 *  @param removedChunks	List of chunks marked as removable. The returned
 *							item is removed from this list so it doesn't get
 *							removed.
 *	@param moveableItems	List of references that can be recycled.
 *	@param pSelectedItem	VLO Item currently selected, NULL if no VLO selected.
 */
void VLOManager::updateReferencesInChunk(
		const VeryLargeObjectPtr pVLO, Chunk* pChunk,
		std::set< Chunk* >& removedChunks,
		VeryLargeObject::ChunkItemList& movableItems,
		const EditorChunkVLO* pSelectedItem )
{
	BW_GUARD;

	EditorChunkVLOPtr pItem =
				static_cast< EditorChunkVLO* >( pVLO->containsChunk( pChunk ) );
	bool shellSelected = WorldManager::instance().isChunkSelected( pChunk );

	if (movableItems.empty() || shellSelected)
	{
		if (pItem == NULL)
		{
			// No item, and no items to recycle, so create a new one.
			createReference( pVLO, pChunk );
		}
		else
		{
			// Have to rebuild the collision scene for it anyway
			pChunk->delStaticItem( pItem );
			pChunk->addStaticItem( pItem );
		}
	}
	else
	{
		// Reuse existing objects, to prevent problems with
		// selection, etc.
		ChunkItemPtr pMovedItem;

		VeryLargeObject::ChunkItemList::iterator it = std::find(
			movableItems.begin(), movableItems.end(), pItem.getObject() );

		if (it != movableItems.end())
		{
			pMovedItem = pItem;
			movableItems.erase( it );
		}
		else
		{
			pMovedItem = movableItems.back();
			movableItems.pop_back();
		}

		if ((pItem != NULL) && (pItem.getObject() != pMovedItem.getObject()) &&
			(pItem != pSelectedItem))
		{
			deleteReference( pVLO, pChunk );
		}

		if (pMovedItem->chunk() != NULL)
		{
			pMovedItem->chunk()->delStaticItem( pMovedItem );
		}

		pChunk->addStaticItem( pMovedItem );
	}
	removedChunks.erase( pChunk );
}


/**
 *  This method updates references in all chunks touched by a VLO, allowing one
 *  chunk to be skipped (usually the chunk that contains the reference being
 *  moved/deleted).
 *
 *  @param root			Main VLO reference.
 */
void VLOManager::updateReferencesInternal( const EditorChunkVLO* pVLORef, const std::vector< ChunkItemPtr > * pcursel)
{
	BW_GUARD;

	VeryLargeObjectPtr pVLO = pVLORef->object();
	if (pVLORef == NULL)
	{
		return;
	}

	VeryLargeObject::ChunkItemList items = pVLO->chunkItems();

	// Find out if the water is selected, and if so, get the selected reference
	// to store it last in the movableItems vector, so it get's used first (so
	// the selection is kept after this method returns).

	EditorChunkVLO* pSelectedItem = NULL;
	for (VeryLargeObject::ChunkItemList::iterator it = items.begin();
		it != items.end(); ++it)
	{
		if ((*it)->chunk())
		{
			if (std::find( pcursel->begin(), pcursel->end(), *it ) != pcursel->end())
			{
				pSelectedItem = static_cast< EditorChunkVLO* >( *it );
				break;
			}
		}			
	}

	// Record references from the old position, but exclude the item 'root'.
	// Also, fill in the movableItems vector with the items to be removed, so
	// they get reused (which potentially prevents selection and undo issues)
	std::set< Chunk* > removedChunks;
	VeryLargeObject::ChunkItemList movableItems;
	for (VeryLargeObject::ChunkItemList::iterator it = items.begin();
		it != items.end(); ++it)
	{
		if ((*it)->chunk() && ((*it) != pSelectedItem))
		{
			removedChunks.insert( (*it)->chunk() );
			movableItems.push_back( *it );
		}
	}
	if (pSelectedItem != NULL)
	{
		// Put the selected item last, so it get's reused first, which ensures it
		// always gets reused.
		movableItems.push_back( pSelectedItem );
	}

	// Change bounds
	BoundingBox vloBB;
	pVLORef->edBounds( vloBB );
	setMovedBounds( pVLO->getUID(), vloBB, pVLO->localTransform() );

	// Now, update the loaded chunks touched by the water in the new pos, again
	// excluding the item 'root'.
	std::set< std::string > touchedColumns;
	columnsTouchedByBounds( vloBB, pVLO->localTransform(), touchedColumns );
	

	bool addedToShell = false;
	// First pass: add references to shells, if visibleInside.
	for (std::set< std::string >::iterator it = touchedColumns.begin();
		it != touchedColumns.end(); ++it)
	{

		Chunk* pChunk = ChunkManager::instance().findChunkByName( *it,
						WorldManager::instance().geometryMapping(), false );
		if (pChunk && pChunk->loaded() && pVLO->visibleInside())
		{
			// find shells inside the chunk, and create references for them too!
			EditorChunkOverlappers::Items shells =
					EditorChunkOverlappers::instance( *pChunk ).overlappers();
			for (EditorChunkOverlappers::Items::iterator it = shells.begin();
				it != shells.end(); ++it)
			{
				Chunk* pShell = (*it)->pOverlapper();

				if (pShell && pShell->loaded())
				{
					Math::OrientedBBox vloBox( vloBB, pVLO->localTransform() );
					Math::OrientedBBox shellBox( pShell->localBB(),
												pShell->transform() );
					if (vloBox.intersects( shellBox ))
					{
						updateReferencesInChunk( pVLO, pShell, removedChunks,
												movableItems, pSelectedItem );
						addedToShell = true;
					}
				}
			}
		}
	}

	// Second pass: add to outside chunks, if visibleOutside or if addToShell
	// is false (the VLO must be added to at least one chunk).
	for (std::set< std::string >::iterator it = touchedColumns.begin();
		it != touchedColumns.end(); ++it)
	{
		Chunk* pChunk = ChunkManager::instance().findChunkByName(
					*it, WorldManager::instance().geometryMapping(), false );
		if (pChunk && pChunk->loaded() &&
			(pVLO->visibleOutside() || !addedToShell))
		{
			 updateReferencesInChunk( pVLO, pChunk, removedChunks,
									movableItems, pSelectedItem );
		}
	}

	for (VeryLargeObject::ChunkItemList::iterator it = items.begin();
		it != items.end(); ++it)
	{
		(*it)->syncInit();
	}

	// Remove references from loaded chunks that no longer intersect the VLO
	for (std::set< Chunk* >::iterator it = removedChunks.begin();
		it != removedChunks.end(); ++it)
	{
		deleteReference( pVLO, *it );
	}
	
}

/**
 *	This class is called back when WorldEditor ticks.
 */
class UpdateReferencesTickable : public EditorTickable
{
public:
	UpdateReferencesTickable( VeryLargeObjectPtr pVLO ) :
	  pVLO_( pVLO )
	{
		BW_GUARD;

		objectNum++;
		//if more than 1 object created in the same frame, record the selectedItems to preObject;
		if(objectNum>1)
		{
			preObject->selectedItems_ = WorldManager::instance().selectedItems();
			preObject->bRecordSel_ = true;
		}
		preObject = this;
		bRecordSel_ = false;
	}

	void tick()
	{
		BW_GUARD;

		//once tick, it is in another tick, so reset objectNum;
		objectNum=0;

		// If the VLO is in the selection, we need to know in order to put it back
		// in the selection if the previously selected reference gets removed.
		VeryLargeObject::ChunkItemList items = pVLO_->chunkItems();

		// Need to recreate all references for this VLO.
		for (VeryLargeObject::ChunkItemList::iterator it = items.begin();
			it != items.end(); it++)
		{
			// only need to update one reference for all the refs to be
			// re-evaluated.
			if ((*it) && (*it)->chunk())
			{
				//use recorded selectedItems if there is, instead of get one from WoldManager.
				const std::vector< ChunkItemPtr > * pcursel;
				if(bRecordSel_ )
					pcursel = &selectedItems_;
				else
					pcursel = &WorldManager::instance().selectedItems();


				VLOManager::instance()->updateReferencesInternal(
					static_cast< EditorChunkVLO* >(*it), pcursel);
				break;
			}
		}
		VLOManager::instance()->markAsDirty( pVLO_->getUID() );

		WorldManager::instance().removeTickable( this );
	}

private:
	VeryLargeObjectPtr pVLO_;
	std::vector<ChunkItemPtr> selectedItems_;
	bool	bRecordSel_;
	static UpdateReferencesTickable *preObject;
	static int	objectNum;
};

UpdateReferencesTickable* UpdateReferencesTickable::preObject = NULL;
int		UpdateReferencesTickable::objectNum = 0;

/**
 *  This method updates all references to a VLO in chunks and shells. Updating
 *  is done in the next tick.
 *
 *  @param pVLO			Very Large Object.
 */
void VLOManager::updateReferences( VeryLargeObjectPtr pVLO )
{
	BW_GUARD;

	if (pVLO == NULL)
	{
		SimpleMutexHolder mutex( listMutex_ );
		for (BoundsMap::iterator it = movedBoundsMap_.begin();
			it != movedBoundsMap_.end(); ++it)
		{
			if (deletedSet_.find( (*it).first ) != deletedSet_.end())
			{
				continue;
			}

			VeryLargeObjectPtr p = VeryLargeObject::getObject( (*it).first );
			if (p == NULL)
			{
				// This can be NULL in some cases, such as when a chunk auto-fixes
				// itself on load, and is safe to continue.
				continue;
			}
			updateReferences( p );
		}
	}
	else
		WorldManager::instance().addTickable(
								new UpdateReferencesTickable( pVLO ) );
}


/**
 *  This method is same as updateReferences().
 *  The only difference is updating is done immediately instead
 *  of done in next tick.
 */
void VLOManager::updateReferencesImmediately( const VeryLargeObjectPtr pVLO )
{
	BW_GUARD;

	if (pVLO == NULL)
	{
		SimpleMutexHolder mutex( listMutex_ );
		for (BoundsMap::iterator it = movedBoundsMap_.begin();
			it != movedBoundsMap_.end(); ++it)
		{
			if (deletedSet_.find( (*it).first ) != deletedSet_.end())
			{
				continue;
			}

			VeryLargeObjectPtr p = VeryLargeObject::getObject( (*it).first );
			if (p == NULL)
			{
				// This can be NULL in some cases, such as when a chunk auto-fixes
				// itself on load, and is safe to continue.
				continue;
			}
			updateReferencesImmediately( p );
		}
	}
	else
	{
		EditorTickablePtr tickable = new UpdateReferencesTickable( pVLO );
		tickable->tick();
	}
}


/**
 *  This method returns whether a VLO is writable by checking that all the
 *  columns it touches are writable.
 *
 *  @param root			Main VLO reference.
 *  @return				True if the VLO is writable, false otherwise.
 */
bool VLOManager::writable( const EditorChunkVLO* pVLORef ) const
{
	BW_GUARD;

	if (!WorldManager::instance().connection().enabled())
	{
		return true;
	}

	// go through all the chunks it touches and check that all are editable,
	// not readonly.
	VeryLargeObjectPtr pVLO = pVLORef->object();

	GeometryMapping* dirMap = WorldManager::instance().geometryMapping();

	std::set< std::string > touchedColumns;
	BoundingBox bb;
	pVLORef->edBounds( bb );
	columnsTouchedByBounds( bb, pVLO->localTransform(), touchedColumns );

	for (std::set< std::string >::iterator it = touchedColumns.begin();
		it != touchedColumns.end(); ++it)
	{
		int16 gridX, gridZ;
		if (!dirMap->gridFromChunkName( (*it), gridX, gridZ ) ||
			!EditorChunk::outsideChunkWriteable( gridX, gridZ, false ))
		{
			return false;
		}
	}

	return true;
}
