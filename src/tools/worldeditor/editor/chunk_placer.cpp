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
#include "worldeditor/editor/chunk_placer.hpp"
#include "worldeditor/editor/chunk_item_placer.hpp"
#include "worldeditor/editor/item_view.hpp"
#include "worldeditor/editor/autosnap.hpp"
#include "worldeditor/editor/snaps.hpp"
#include "worldeditor/world/editor_chunk.hpp"
#include "worldeditor/world/editor_chunk_item_linker_manager.hpp"
#include "worldeditor/world/world_manager.hpp"
#include "appmgr/options.hpp"
#include "gizmo/tool_locator.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_manager.hpp"
#include "moo/visual_manager.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/string_provider.hpp"


DECLARE_DEBUG_COMPONENT2( "Editor", 0 )


// -----------------------------------------------------------------------------
// Section: ChunkPlacer
// -----------------------------------------------------------------------------

PY_MODULE_STATIC_METHOD( ChunkPlacer, createChunk, WorldEditor )
PY_MODULE_STATIC_METHOD( ChunkPlacer, cloneAndAutoSnap, WorldEditor )
//PY_MODULE_STATIC_METHOD( ChunkPlacer, deleteChunk, WorldEditor )
PY_MODULE_STATIC_METHOD( ChunkPlacer, createInsideChunkName, WorldEditor )
PY_MODULE_STATIC_METHOD( ChunkPlacer, chunkDataSection, WorldEditor )
PY_MODULE_STATIC_METHOD( ChunkPlacer, createInsideChunkDataSection, WorldEditor )
//PY_MODULE_STATIC_METHOD( ChunkPlacer, cloneChunkDataSection, WorldEditor )
//PY_MODULE_STATIC_METHOD( ChunkPlacer, cloneChunks, WorldEditor )
PY_MODULE_STATIC_METHOD( ChunkPlacer, recreateChunks, WorldEditor )
PY_MODULE_STATIC_METHOD( ChunkPlacer, saveChunkTemplate, WorldEditor )


/**
 * Create and return the Chunk, but don't notify the editor,
 * or setup an undo operation, etc
 *
 * ie, don't expose this to the world at large
 *
 * NULL is returned if anything went wrong
 */
Chunk* ChunkPlacer::utilCreateChunk( DataSectionPtr pDS, const std::string& chunkID, Matrix* m/* = NULL*/ )
{
	BW_GUARD;

	if(!pDS->openSection( "transform" ))
	{
		PyErr_SetString( PyExc_ValueError, "utilCreateChunk() "
			"expects a data section of a chunk. It needs at least "
			"a chunk name including '.chunk' as section name and "
			"a transform" );
		return NULL;
	}

	BoundingBox bb( pDS->readVector3( "boundingBox/min" ), pDS->readVector3( "boundingBox/max" ) );
	if( m )
	{
		Matrix transform = pDS->readMatrix34( "transform" );
		transform.invert();
		transform.postMultiply( *m );
		bb.transformBy( transform );
	}
	if( !EditorChunk::outsideChunksWriteable( bb ) )
	{
		PyErr_SetString( PyExc_ValueError, "utilCreateChunk() "
			"The newly created chunk is not inside the editable area" );
		return NULL;
	}

	// make sure that name doesn't already exist
	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (pSpace->chunks().find( chunkID ) != pSpace->chunks().end())
	{
		PyErr_Format( PyExc_ValueError, "utilCreateChunk() "
			"Chunk %s already known to space", chunkID.c_str() );
		return NULL;
	}

	// ok, here we go then... make the chunk
	Chunk * pChunk = new Chunk( chunkID, WorldManager::instance().geometryMapping() );

	// Get the appointed chunk
	pChunk = pSpace->findOrAddChunk( pChunk );

	// load it from this data section
	// TODO: make sure no assumptions about running in OTHER thread!
	pChunk->loading( true );
	pChunk->load( pDS );
	pChunk->loading( false );

	return pChunk;
}

/*~ function WorldEditor.createChunk
 *	@components{ worldeditor }
 *
 *	This function creates a chunk given the chunk's datasection and name.
 *	It places the newly created chunk at the position of the optional ToolLocator.
 *
 *	@param datasection	The DataSection of the chunk to create.
 *	@param name			The full name of the chunk to create.
 *	@param locator		An optional ToolLocator object which provides a position for the
 *						newly created chunk.
 *
 *	@return Returns a ChunkItemGroup object of the newly created chunk.
 */
PyObject * ChunkPlacer::py_createChunk( PyObject * args )
{
	BW_GUARD;

	// get the arguments
	PyObject * pPyDS, * pPyLoc = NULL;
	char * chunkName;
	if (!PyArg_ParseTuple( args, "Os|O", &pPyDS, &chunkName, &pPyLoc) ||
		!PyDataSection::Check(pPyDS) ||
		(pPyLoc && !ToolLocator::Check(pPyLoc)))
	{
		PyErr_SetString( PyExc_TypeError, "WorldEditor.createChunk() "
			"expects a PyDataSection, the full name of the chunk, and an optional ToolLocator" );
		return NULL;
	}

	// get the data section for it
	DataSectionPtr pSect = static_cast<PyDataSection*>( pPyDS )->pSection();

	// find out where it goes
	Matrix pose = Matrix::identity;
	if (pPyLoc != NULL)
	{
		// we have a locator, check section transform is identity
		Matrix sectPose = pSect->readMatrix34( "transform" );
		bool treq = true;
		for (int i = 0; i < 16; i++)
		{
			treq &= ( ((float*)sectPose)[i] == ((float*)pose)[i] );
		}
		if (!treq)
		{
			PyErr_SetString( PyExc_ValueError, "WorldEditor.createChunk() "
				"expected data section to have an identity transform "
				"since ToolLocator was given." );
			return NULL;
		}

		// now use the locator's
		ToolLocator * pLoc = static_cast<ToolLocator*>( pPyLoc );
		pose = pLoc->transform();

		// snap the transform
		Vector3 t = pose.applyToOrigin();
		Vector3 snapAmount = Options::getOptionVector3( "shellSnaps/movement",
			Vector3( 1.f, 1.f, 1.f ) );
		Snap::vector3( t, snapAmount );
		pose.translation( t );
	}

	if (!WorldManager::instance().isPointInWriteableChunk( pose.applyToOrigin() ))
		Py_Return;



	// move it to the locator's transform (so creator needn't repeat
	// work of calculating offset boundaries and all that)
	if (pPyLoc != NULL)
	{
		ToolLocator * pLoc = static_cast<ToolLocator*>( pPyLoc );
		if (!pLoc->positionValid())
		{
			if (Options::getOptionInt( "snaps/itemSnapMode", 0 ) != 0)
			{
				PyErr_SetString( PyExc_ValueError, "WorldEditor.createChunk() "
				"invalid snapping mode for camera relative placing, must be Free");
				return NULL;
			}
			Vector3 dim( pSect->readVector3( "boundingBox/max" ) - pSect->readVector3( "boundingBox/min" ) );
			Vector3 dir = pLoc->direction();
			Vector3 trans( dir.x * dim.x,  dir.y * dim.y, dir.z * dim.z);		

			BoundingBox spaceBB(ChunkManager::instance().cameraSpace()->gridBounds());
			const Vector3& minB = spaceBB.minBounds();
			const Vector3& maxB = spaceBB.maxBounds();
			bool canFit = true;
			/* Check the transformed chunk can fit into the space 
			*/
			if ((pose.applyToOrigin().x + trans.x + dim.x) > maxB.x)
			{
				canFit = false;
			}		
			if ((pose.applyToOrigin().x + trans.x + dim.x) < minB.x)
			{
				canFit = false;
			}				
			if ((pose.applyToOrigin().z + trans.z + dim.z) > maxB.z)
			{
				canFit = false;
			}
			if ((pose.applyToOrigin().z + trans.z + dim.z) < minB.z)
			{
				canFit = false;
			}	
			if (!canFit)
			{
				PyErr_SetString( PyExc_ValueError, "WorldEditor.createChunk() "
				"too close to the boundary of space");
				return NULL;
			}
			pose.postTranslateBy ( trans );
		}

	}
	Chunk* pChunk = utilCreateChunk( pSect, chunkName, &pose );
	if (!pChunk)
		return NULL;
	if (pPyLoc != NULL)
	{
		pChunk->transform( pose );
	}

	// and tell the cache that it has arrived!
	EditorChunkCache::instance( *pChunk ).edArrive( true );

	// this is mainly for indoor chunks, create a static lighting data section.
	EditorChunkCache::instance( *pChunk ).edRecalculateLighting();

	// now add an undo which deletes it
	UndoRedo::instance().add(
		new ChunkExistenceOperation( pChunk, false ) );

	ChunkItemPtr modelItem = EditorChunkCache::instance(*pChunk).getShellModel();
	MetaData::updateCreationInfo( modelItem->metaData() );
	std::vector<ChunkItemPtr> newItems;
	newItems.push_back(modelItem);
	WorldManager::instance().setSelection( newItems );

	// set a meaningful barrier name
	UndoRedo::instance().barrier(
		LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/CHUNK_PLACER/CREATE_CHUNK", pChunk->identifier() ), false );

	// check to see if in the space, if not -> can't place this chunk, undo create
	BoundingBox spaceBB(ChunkManager::instance().cameraSpace()->gridBounds());
	if ( !(spaceBB.intersects( pChunk->boundingBox().minBounds() ) &&
			spaceBB.intersects( pChunk->boundingBox().maxBounds() )) )
	{
		UndoRedo::instance().undo();
		Py_Return;
	}

	WorldManager::instance().markTerrainShadowsDirty( pChunk->boundingBox() );

	// and that's it then. return a group that contains the chunk's item
	ChunkItemGroup * pRes = new ChunkItemGroup();
	pRes->add( modelItem );
	return pRes;
}

// defined below
bool visualFromModel(
	const DataSectionPtr& pModelDS,
	std::string& visualName,
	DataSectionPtr& visualDS,
	Moo::VisualPtr& pVis);

/*~ function WorldEditor.recreateChunks
 *	@components{ worldeditor }
 *
 *	This function recreates the chunk which is referenced by the ChunkItemRevealer object.
 *	This is primarily used for shells that have been edited outside of WorldEditor,
 *	and need to be updated in WorldEditor without having to reload the entire space.
 *
 *	@param revealer The ChunkItemRevealer object of the chunk to recreate.
 */
PyObject * ChunkPlacer::py_recreateChunks( PyObject * args )
{
	BW_GUARD;

	// get args
	PyObject * pPyRev;
	if (!PyArg_ParseTuple( args, "O", &pPyRev ) ||
		!ChunkItemRevealer::Check( pPyRev ))
	{
		PyErr_SetString( PyExc_ValueError, "WorldEditor.recreateChunks() "
			"expects a ChunkItemRevealer" );
		return NULL;
	}


	ChunkItemRevealer* pRevealer = static_cast<ChunkItemRevealer*>( pPyRev );
	ChunkItemRevealer::ChunkItems items;
	pRevealer->reveal( items );

	ChunkItemRevealer::ChunkItems::iterator i = items.begin();
	for (; i != items.end(); ++i)
	{
		ChunkItemPtr pItem = *i;
		Chunk* pChunk = pItem->chunk();

		if (!pItem->isShellModel())
		{
			WARNING_MSG( "Trying to recreate something that isn't a shell\n" );
			continue;
		}

		MF_ASSERT( pChunk );

		if (!pChunk->isBound())
		{
			WARNING_MSG( "Chunk isn't bound, can't recreate\n" );
			continue;
		}

		DataSectionPtr pChunkSection = EditorChunkCache::instance( *pChunk ).pChunkSection();
		MF_ASSERT( pChunkSection );
		std::string modelName = pItem->pOwnSect()->readString( "resource" );

		DataSectionPtr pModelDS = BWResource::openSection( modelName );

		std::string visualName;
		DataSectionPtr visualDS;
		Moo::VisualPtr pVis;
		if ( !visualFromModel(pModelDS, visualName, visualDS, pVis) )
		{
			//above function sets the PyErr string.
			continue;
		}

		EditorChunkCache::instance( *pChunk ).reloadBounds();
		WorldManager::instance().changedChunk( pChunk );
	}

	// and that's it
	Py_Return;
}

/*~ function WorldEditor.saveChunkTemplate
 *	@components{ worldeditor }
 *
 *	This function is used to save a shell's light arrangement as a template for the shell.
 *	When the shell is placed elsewhere in the world, its light setup will be loaded from 
 *	the template.
 *
 *	@param revealer The ChunkItemRevealer object of the selected shell which needs to have 
 *					its template saved.
 */
PyObject * ChunkPlacer::py_saveChunkTemplate( PyObject * args )
{
	BW_GUARD;

	// get args
	PyObject * pPyRev;
	if (!PyArg_ParseTuple( args, "O", &pPyRev ) ||
		!ChunkItemRevealer::Check( pPyRev ))
	{
		PyErr_SetString( PyExc_ValueError, "WorldEditor.recreateChunks() "
			"expects a ChunkItemRevealer" );
		return NULL;
	}


	ChunkItemRevealer* pRevealer = static_cast<ChunkItemRevealer*>( pPyRev );
	ChunkItemRevealer::ChunkItems items;
	pRevealer->reveal( items );

	bool gotOneShell = false;
	ChunkItemRevealer::ChunkItems::iterator i = items.begin();
	for (; i != items.end(); ++i)
	{
		ChunkItemPtr pItem = *i;
		Chunk* pChunk = pItem->chunk();

		MF_ASSERT( pChunk );

		if (pChunk->isOutsideChunk())
		{
			WARNING_MSG( "Chunk isn't indoors, can't save as template\n" );
			continue;
		}

		if (!pChunk->isBound())
		{
			WARNING_MSG( "Chunk isn't bound, can't save as template\n" );
			continue;
		}
		if( !pItem->isShellModel() )
			continue;

		gotOneShell = true;
		DataSectionPtr pChunkSection = EditorChunkCache::instance( *pChunk ).pChunkSection();
		std::string modelName = pItem->pOwnSect()->readString( "resource" );

		MF_ASSERT( !modelName.empty() );

		std::string templateName = modelName + ".template";

        // Create a new file in the models dir
		std::string templateDir = BWResource::getFilePath( templateName );

		DataSectionPtr dir = BWResource::openSection( templateDir );
		if (!dir)
		{
			ERROR_MSG( "saveChunkTemplate() - Couldn't open dir %s\n",
				templateDir.c_str() );
			continue;
		}

		DataSectionPtr pDS = dir->newSection( BWResource::getFilename( templateName ) );
		if (!pDS)
		{
			ERROR_MSG( "saveChunkTemplate() - Couldn't create file\n" );
			continue;
		}

		// Only save lights and flares
		pDS->copySections( pChunkSection, "omniLight" );
		pDS->copySections( pChunkSection, "spotLight" );
		pDS->copySections( pChunkSection, "directionalLight" );
		pDS->copySections( pChunkSection, "ambientLight" );
		pDS->copySections( pChunkSection, "flare" );
		pDS->copySections( pChunkSection, "pulseLight" );


		/*
		// Copy everything from the chunks datasection
		pDS->copy( pChunkSection );

		// Strip the chunk specific stuff out of it
		pDS->deleteSections( "waypointGenerationTime" );
		pDS->deleteSections( "waypointSet" );
		pDS->deleteSections( "boundary" );
		pDS->deleteSections( "boundingBox" );
		pDS->deleteSections( "transform" );
		// Stations too, we don't want them connecting to the same graph
		pDS->deleteSections( "station" );

		std::vector<DataSectionPtr> modelSections;
		pDS->openSections( "model", modelSections );
		std::vector<DataSectionPtr>::iterator i = modelSections.begin();
		for (; i != modelSections.end(); ++i)
			(*i)->delChild( "lighting" );
		*/

		pDS->save();

		INFO_MSG( "Saved chunk template %s\n", templateName.c_str() );
	}

	if( !gotOneShell )
	{
		WorldManager::instance().addCommentaryMsg( LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/CHUNK_PLACER/SHELL_ONLY" ) );
	}
	else
		WorldManager::instance().addCommentaryMsg( LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/CHUNK_PLACER/TEMPLATE_SAVED" ) );

	Py_Return;
}

void ChunkPlacer::utilCloneChunkDataSection( Chunk* chunk, DataSectionPtr ds, const std::string& newChunkName )
{
	BW_GUARD;

	DataSectionPtr sourceChunkSection = EditorChunkCache::instance( *chunk ).pChunkSection();
	utilCloneChunkDataSection(sourceChunkSection, ds, newChunkName);
}

void ChunkPlacer::utilCloneChunkDataSection( DataSectionPtr sourceChunkSection, DataSectionPtr ds, const std::string& newChunkName )
{
	BW_GUARD;

	ds->copy( sourceChunkSection );
	ds->deleteSections( "boundary" );
}

/*~ function WorldEditor.cloneAndAutoSnap
 *	@components{ worldeditor }
 *
 *	This function clones the given shell and then snaps the clone
 *	to the selected shell.
 *
 *	No action is performed if no matching or available portals can be found.
 *
 *	@param cloneRevealer	The ChunkItemRevealer object of the shell to clone.
 *	@param snapRevealer		The ChunkItemRevealer object of the shell to snap
 *							the clone to.
 *
 *	@return Returns a ChunkItemGroup object of the cloned shell if the operation
 *			was successful.
 */
PyObject* ChunkPlacer::py_cloneAndAutoSnap( PyObject * args )
{
	BW_GUARD;

	// get args
	PyObject *pPyRev1, *pPyRev2;
	if (!PyArg_ParseTuple( args, "OO", &pPyRev1, &pPyRev2 ) ||
		!ChunkItemRevealer::Check( pPyRev1 ) ||
		!ChunkItemRevealer::Check( pPyRev2 ))
	{
		PyErr_SetString( PyExc_ValueError, "WorldEditor.autoSnap() "
			"expects two ChunkItemRevealers" );
		return NULL;
	}

	ChunkItemRevealer* chunkRevealer = static_cast<ChunkItemRevealer*>( pPyRev1 );
	ChunkItemRevealer* snapToRevealer = static_cast<ChunkItemRevealer*>( pPyRev2 );

	ChunkItemRevealer::ChunkItems sourceChunkItems;
	ChunkItemRevealer::ChunkItems snapToChunkItems;
	chunkRevealer->reveal( sourceChunkItems );
	snapToRevealer->reveal( snapToChunkItems );

	if (sourceChunkItems.size() != 1)
	{
		PyErr_Format( PyExc_ValueError, "WorldEditor.cloneAndAutoSnap() "
			"Can only clone from one item" );
		return NULL;
	}
	if (!sourceChunkItems.front()->isShellModel())
	{
		PyErr_Format( PyExc_ValueError, "WorldEditor.cloneAndAutoSnap() "
			"Can only clone from a chunk" );
		return NULL;
	}

	Chunk* chunk = sourceChunkItems.front()->chunk();

	std::vector<Chunk*> snapToChunks;
	ChunkItemRevealer::ChunkItems::iterator i = snapToChunkItems.begin();
	for (; i != snapToChunkItems.end(); ++i)
	{
		if ((*i)->isShellModel())
			snapToChunks.push_back( (*i)->chunk() );
	}

	if (snapToChunks.empty())
	{
		PyErr_Format( PyExc_ValueError, "WorldEditor.cloneAndAutoSnap() "
			"No chunks to snap to" );
		return NULL;
	}



	// make sure there's only one chunk that we're copying from
	/*
	std::vector<Chunk*> sourceChunks = extractChunks( chunkRevealer );
	std::vector<Chunk*> snapToChunks = extractChunks( snapToRevealer );
	if (sourceChunks.size() != 1)
	{
		PyErr_Format( PyExc_ValueError, "WorldEditor.cloneAndAutoSnap() "
			"Can only clone from one item" );
		return NULL;
	}
	Chunk* chunk = sourceChunks.front();
	*/


	Matrix autoSnapTransform = findAutoSnapTransform( chunk, snapToChunks );

	if (autoSnapTransform != chunk->transform())
	{
		std::string newChunkName;
		DataSectionPtr pDS = utilCreateInsideChunkDataSection( snapToChunks[0], newChunkName );
		if (!pDS)
			Py_Return;

		utilCloneChunkDataSection( chunk, pDS, newChunkName );

		Chunk* pChunk = utilCreateChunk( pDS, newChunkName, &autoSnapTransform );
		if( !pChunk )
			return NULL;

		// move it to the autoSnap position
		pChunk->transform( autoSnapTransform );

		// and tell the cache that it has arrived!
		EditorChunkCache::instance( *pChunk ).edArrive( true );

		// tell the chunk we just cloned it
		EditorChunkCache::instance( *pChunk ).edPostClone();

		// now add an undo which deletes it
		UndoRedo::instance().add(
			new ChunkExistenceOperation( pChunk, false ) );

		// set a meaningful barrier name
		UndoRedo::instance().barrier(
			LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/CHUNK_PLACER/AUTO_CLONE_CHUNK", pChunk->identifier() ), false );

		// select the appropriate shell model
		ChunkItemPtr modelItem = EditorChunkCache::instance(*pChunk).getShellModel();

		// and that's it then. return a group that contains the chunk's item
		ChunkItemGroup * pRes = new ChunkItemGroup();
		pRes->add( modelItem );
		return pRes;
	}
	else
		Py_Return;
}

/**
 *	This method allows scripts to delete a chunk item
 */
/*
PyObject * ChunkPlacer::py_deleteChunk( PyObject * args )
{
	// get args
	PyObject * pPyRev;
	if (!PyArg_ParseTuple( args, "O", &pPyRev ) ||
		!ChunkItemRevealer::Check( pPyRev ))
	{
		PyErr_SetString( PyExc_ValueError, "WorldEditor.deleteChunk() "
			"expects a ChunkItemRevealer" );
		return NULL;
	}

	ChunkItemRevealer* pRevealer = static_cast<ChunkItemRevealer*>( pPyRev );

	ChunkItemRevealer::ChunkItems items;
	pRevealer->reveal( items );

	ChunkItemRevealer::ChunkItems::iterator i = items.begin();
	for (; i != items.end(); ++i)
	{
		ChunkItemPtr pItem = *i;
		Chunk* pChunk = pItem->chunk();

		if (pChunk == NULL)
		{
			PyErr_Format( PyExc_ValueError, "WorldEditor.deleteChunk() "
				"Item has already been deleted!" );
			return NULL;
		}

		// see if it wants to be deleted
		if (!EditorChunkCache::instance( *pChunk ).edCanDelete())
			continue;

		// tell the chunk we're going to delete it
		EditorChunkCache::instance( *pChunk ).edPreDelete();

		// ok, now delete its chunk
		EditorChunkCache::instance( *pChunk ).edDepart();

		// set up an undo which creates it
		UndoRedo::instance().add(
			new ChunkExistenceOperation( pChunk, true ) );
	}

	// set a meaningful barrier name
	if (items.size() == 1)
		UndoRedo::instance().barrier(
			"Delete Chunk " + items.front()->chunk()->identifier(), false );
	else
		UndoRedo::instance().barrier( "Delete Chunks", false );


	// and that's it
	Py_Return;
}
*/
bool ChunkPlacer::deleteChunk( Chunk * pChunk )
{
	BW_GUARD;

	// see if it wants to be deleted
	if (!EditorChunkCache::instance( *pChunk ).edCanDelete())
		return false;

	// tell the chunk we're going to delete it
	EditorChunkCache::instance( *pChunk ).edPreDelete();

	// ok, now delete its chunk
	EditorChunkCache::instance( *pChunk ).edDepart();

	// set up an undo which creates it
	UndoRedo::instance().add(
		new ChunkExistenceOperation( pChunk, true ) );

	return true;
}


/**
 * Hide all the items in this chunk.
 * 
 * @param pChunk chunk to use.
 * @param value value of the hidden flag to set items to.
 * @return true if the process succeeded.
 */
/*static*/
bool ChunkPlacer::chunkHidden( Chunk * pChunk, bool value )
{
	BW_GUARD;

	if (!pChunk || !pChunk->isBound() || !EditorChunkCache::instance( *pChunk ).edIsWriteable())
	{
		return false;
	}

	// Add all items in the chunk
	MatrixMutexHolder lock( pChunk );
	std::vector<ChunkItemPtr> chunkItems =
		EditorChunkCache::instance( *pChunk ).staticItems();

	std::vector<ChunkItemPtr>::const_iterator k;
	for (k = chunkItems.begin(); k != chunkItems.end(); ++k)
	{
		ChunkItemPtr item = *k;
		item->edHidden( value );
		item->edSave( item->pOwnSect() );
		item->edPostModify();
	}
	return true;
}


/**
 * Freeze all the items in this chunk.
 * 
 * @param pChunk chunk to use.
 * @param value value of the frozen flag to set items to.
 * @return true if the process succeeded.
 */
/*static*/
bool ChunkPlacer::chunkFrozen( Chunk * pChunk, bool value )
{
	BW_GUARD;

	if (!pChunk || !pChunk->isBound() || !EditorChunkCache::instance( *pChunk ).edIsWriteable())
	{
		return false;
	}

	// Add all items in the chunk
	MatrixMutexHolder lock( pChunk );
	std::vector<ChunkItemPtr> chunkItems =				
		EditorChunkCache::instance( *pChunk ).staticItems();

	std::vector<ChunkItemPtr>::const_iterator k;
	for (k = chunkItems.begin(); k != chunkItems.end(); ++k)
	{
		ChunkItemPtr item = *k;
		item->edFrozen( value );
		item->edSave( item->pOwnSect() );
		item->edPostModify();
	}
	return true;
}

std::string ChunkPlacer::utilCreateInsideChunkName( Chunk * pNearbyChunk )
{
	BW_GUARD;

	char chunkName[256];
	char backupChunkName[256];
	std::string spacesubdir;
	uint lslash = pNearbyChunk->identifier().find_last_of( '/' );

	if (lslash < pNearbyChunk->identifier().length())
	{
		spacesubdir = pNearbyChunk->identifier().substr( 0, lslash + 1 );
	}

	const std::string& path = WorldManager::instance().geometryMapping()->path();
	uint32 timeTag = uint32( timestamp() % 0x10000 );

	for (int i = 0; i < 1000; ++i)
	{
		uint32 rand = bw_random() % 0x10000;

		bw_snprintf( chunkName, sizeof( chunkName ), "%s%04x%04xi.chunk",
			spacesubdir.c_str(), rand, timeTag );
		bw_snprintf( backupChunkName, sizeof( backupChunkName ), "%s%04x%04xi.~chunk~",
			spacesubdir.c_str(), rand, timeTag );

		// check for file existence
		std::string filePath = path + chunkName;
		std::string backupFilePath = path + backupChunkName;

		if (!BWResource::fileExists( filePath ) && !BWResource::fileExists( backupFilePath ))
		{
			// sanity check, make sure it's not known to the space
			std::string chunkIdentifier = chunkName;
			ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();

			chunkIdentifier.resize( chunkIdentifier.size() - 6 );

			if (pSpace->chunks().find( chunkIdentifier ) == pSpace->chunks().end())
			{
				return chunkName;
			}
			else
			{
				WARNING_MSG( "Chunk %s doesn't exist in the file system, but is known to the space\n", chunkName );
			}
		}
	}

	CRITICAL_MSG( "ChunkPlacer::utilCreateInsideChunkName: Failed to find a shell name after 1000 iterations\n" );
	return "";
}


/*~ function WorldEditor.createInsideChunkName
 *	@components{ worldeditor }
 *
 *	This function creates an unique chunk name.
 *
 *	@return		An unique chunk name, or "error" if the function failed.
 */
PyObject * ChunkPlacer::py_createInsideChunkName( PyObject * args )
{
	BW_GUARD;

	std::string name = utilCreateInsideChunkName( ChunkManager::instance().cameraChunk() );

	return PyString_FromString( name.c_str() );
}

/*~ function WorldEditor.chunkDataSection
 *	@components{ worldeditor }
 *	
 *	This function returns the DataSection for the given chunk.
 *
 *	@param revealer The ChunkItemRevealer object of the chunk which
 *					DataSection is to be returned.
 *
 *	@return The DataSection of the given chunk.
 */
PyObject * ChunkPlacer::py_chunkDataSection( PyObject * args )
{
	BW_GUARD;

	// get args
	PyObject * pPyRev;
	if (!PyArg_ParseTuple( args, "O", &pPyRev ) ||
		!ChunkItemRevealer::Check( pPyRev ))
	{
		PyErr_SetString( PyExc_ValueError, "WorldEditor.deleteChunkItem() "
			"expects a ChunkItemRevealer" );
		return NULL;
	}

	ChunkItemRevealer* pRevealer = static_cast<ChunkItemRevealer*>( pPyRev );

	// make sure there's only one
	ChunkItemRevealer::ChunkItems items;
	pRevealer->reveal( items );
	if (items.size() != 1)
	{
		PyErr_Format( PyExc_ValueError, "WorldEditor.deleteChunk() "
			"Revealer must reveal exactly one item, not %d", items.size() );
		return NULL;
	}

	Chunk* pChunk = items.front()->chunk();

	return new PyDataSection( EditorChunkCache::instance( *pChunk ).pChunkSection() );
}

DataSectionPtr ChunkPlacer::utilCreateInsideChunkDataSection( Chunk * pNearbyChunk, std::string& retChunkID )
{
	BW_GUARD;

	if (pNearbyChunk == NULL)
	{
		ERROR_MSG( "ChunkPlacer::createInsideChunkDataSection() - camera chunk is NULL\n" );
		return NULL;
	}
	std::string newChunkName = utilCreateInsideChunkName( pNearbyChunk );
	if (newChunkName == "error")
	{
		ERROR_MSG( "ChunkPlacer::createInsideChunkDataSection() - Couldn't create chunk name\n" );
		return NULL;
	}

	std::string dirName = Options::getOptionString("space/mru0");

	DataSectionPtr dir = BWResource::openSection(dirName);
	if (!dir)
	{
		ERROR_MSG( "ChunkPlacer::createInsideChunkDataSection() - Couldn't open %s\n", dirName.c_str() );
		return NULL;
	}

	std::string sectionName = "chunk." + newChunkName;

	DataSectionPtr pDS = dir->openSection( sectionName, /*makeNewSection:*/true );
	if (!pDS)
		ERROR_MSG( "ChunkPlacer::createInsideChunkDataSection() - Couldn't create DataSection\n" );

	//strip off the .chunk and return the identifier.
	retChunkID = newChunkName.substr(0,newChunkName.size()-6);
	return pDS;
}

/*~ function WorldEditor.createInsideChunkDataSection
 *	@components{ worldeditor }
 *
 *	This function creates a chunk DataSection for a shell.
 *
 *	@return Returns a tuple with the chunk DataSection being the first
 *			tuple value and an unique chunk name being the second tuple value.
 */
PyObject* ChunkPlacer::py_createInsideChunkDataSection( PyObject * args )
{
	BW_GUARD;

	std::string newChunkName;
	DataSectionPtr pDS = utilCreateInsideChunkDataSection(
		ChunkManager::instance().cameraChunk(),
		newChunkName );

	PyObject* tuple = PyTuple_New(2);
	if( pDS )
		PyTuple_SetItem( tuple, 0, new PyDataSection(pDS) );
	else
		PyTuple_SetItem( tuple, 0, Py_None );
	PyTuple_SetItem( tuple, 1, PyString_FromString( newChunkName.c_str() ) );

	return tuple;
}


/*PyObject* ChunkPlacer::py_cloneChunkDataSection( PyObject * args )
{
	// get args
	PyObject *pPyRev, *pPyDS;
	if (!PyArg_ParseTuple( args, "OO", &pPyRev, &pPyDS ) ||
		!ChunkItemRevealer::Check( pPyRev ) ||
		!PyDataSection::Check(pPyDS))
	{
		PyErr_SetString( PyExc_ValueError, "WorldEditor.cloneChunkDataSection() "
			"expects a ChunkItemRevealer and a DataSection" );
		return NULL;
	}

	ChunkItemRevealer* pRevealer = static_cast<ChunkItemRevealer*>( pPyRev );

	// make sure there's only one
	ChunkItemRevealer::ChunkItems items;
	pRevealer->reveal( items );
	if (items.size() != 1)
	{
		PyErr_Format( PyExc_ValueError, "WorldEditor.deleteChunk() "
			"Revealer must reveal exactly one item, not %d", items.size() );
		return NULL;
	}

	Chunk* pChunk = items.front()->chunk();

	DataSectionPtr pSect = static_cast<PyDataSection*>( pPyDS )->pSection();

	utilCloneChunkDataSection( pChunk, pSect );

	Py_Return;
}*/

/*
PyObject* ChunkPlacer::py_cloneChunks( PyObject * args )
{
	// get args
	PyObject * pPyRev;
	if (!PyArg_ParseTuple( args, "O", &pPyRev ) ||
		!ChunkItemRevealer::Check( pPyRev ))
	{
		PyErr_SetString( PyExc_ValueError, "WorldEditor.deleteChunkItem() "
			"expects a ChunkItemRevealer" );
		return NULL;
	}

	ChunkItemRevealer* pRevealer = static_cast<ChunkItemRevealer*>( pPyRev );
	ChunkItemRevealer::ChunkItems items;
	pRevealer->reveal( items );

	ChunkItemRevealer::ChunkItems newItems;

	ChunkItemRevealer::ChunkItems::iterator i = items.begin();
	for (; i != items.end(); ++i )
	{
		Chunk* chunk = (*i)->chunk();

		DataSectionPtr pDS = utilCreateInsideChunkDataSection();
		if (!pDS)
			Py_Return;

		utilCloneChunkDataSection( chunk, pDS );

		Chunk* pChunk = utilCreateChunk( pDS );

		// and tell the cache that it has arrived!
		EditorChunkCache::instance( *pChunk ).edArrive( true );

		// now add an undo which deletes it
		UndoRedo::instance().add(
			new ChunkExistenceOperation( pChunk, false ) );

		// tell the chunk we just cloned it
		EditorChunkCache::instance( *pChunk ).edPostClone();

		// select the appropriate shell model
		newItems.push_back( EditorChunkCache::instance(*pChunk).getShellModel() );
	}

	// Cloning is always done as part of a clone & move tool, don't set a barrier here
#if 0
	// set a meaningful barrier name
	if (items.size() == 1)
		UndoRedo::instance().barrier( "Clone Chunk " + items.front()->chunk()->identifier(), false );
	else
		UndoRedo::instance().barrier( "Clone Chunks", false );
#endif

	return new ChunkItemGroup( newItems );
}
*/
/*static*/ ChunkItem * ChunkPlacer::cloneChunk(
	Chunk * pChunk, const Matrix & newTransform,
	EditorChunkItemLinkableManager::GuidToGuidMap & linkerCloneGuidMapping )
{
	BW_GUARD;

	std::string newChunkName;
	DataSectionPtr pDS = ChunkPlacer::utilCreateInsideChunkDataSection( pChunk, newChunkName );
	if (!pDS)
	{
		return NULL;
	}

	BoundingBox bb;
	EditorChunkCache::instance( *pChunk ).getShellModel()->edBounds( bb );
	bb.transformBy( newTransform );

	ChunkPlacer::utilCloneChunkDataSection( pChunk, pDS, newChunkName );

	std::vector<ChunkItemPtr> chunkItems;
	EditorChunkCache::instance( *pChunk ).allItems( chunkItems );
	for( std::vector<ChunkItemPtr>::iterator iter = chunkItems.begin(); iter != chunkItems.end() ; ++iter )
		(*iter)->edPreChunkClone( pChunk, newTransform, pDS );

	pDS->writeMatrix34( "transform", newTransform );
	pDS->writeVector3( "boundingBox/min", bb.minBounds() );
	pDS->writeVector3( "boundingBox/max", bb.maxBounds() );

	// look inside the chunk for linker objects
	DataSectionIterator iChild;
	for(iChild = pDS->begin(); iChild != pDS->end(); ++iChild)
	{
		DataSectionPtr pChildDS = *iChild;

		WorldManager::instance().linkerManager().updateLinkerGuid( pChildDS, linkerCloneGuidMapping );
	}

	// Create new shell from the cloned datasection
	Chunk* pNewChunk = ChunkPlacer::utilCreateChunk( pDS, newChunkName );
	if (!pNewChunk)
	{
		return NULL;
	}

	// Check that all its outside chunks are loaded before placing
	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (!pSpace)
	{
		return NULL;
	}

	const BoundingBox & newChunkBB = pNewChunk->boundingBox();
	for (int x = ChunkSpace::pointToGrid( newChunkBB.minBounds().x );
		x <= ChunkSpace::pointToGrid( newChunkBB.maxBounds().x ); x++)
	{
		for (int z = ChunkSpace::pointToGrid( newChunkBB.minBounds().z );
			z <= ChunkSpace::pointToGrid( newChunkBB.maxBounds().z ); z++)
		{
			const Vector3 centrePt(
				ChunkSpace::gridToPoint( x ) + GRID_RESOLUTION*0.5f, 0,
				ChunkSpace::gridToPoint( z ) + GRID_RESOLUTION*0.5f );

			// extract their outside chunk
			ChunkSpace::Column * pColumn = pSpace->column( centrePt, false );
			if (pColumn == NULL || pColumn->pOutsideChunk() == NULL)
			{
				// not all outside chunks it overlaps are loaded. return.
				delete pNewChunk;
				return 0;
			}
		}
	}

	// and tell the cache that it has arrived!
	EditorChunkCache::instance( *pNewChunk ).edArrive( true );

	// now add an undo which deletes it
	UndoRedo::instance().add(
		new ChunkExistenceOperation( pNewChunk, false ) );

	// tell the chunk we just cloned it
	EditorChunkCache::instance( *pNewChunk ).edPostClone();

	// select the appropriate shell model
	return &*EditorChunkCache::instance( *pNewChunk ).getShellModel();
}


// -----------------------------------------------------------------------------
// Section: PyLoadedChunks
// -----------------------------------------------------------------------------

#include "chunk_editor.hpp"

/**
 *	This class provides access to all loaded chunks through a map-like interface
 */
class PyLoadedChunks : public PyObjectPlus
{
	Py_Header( PyLoadedChunks, PyObjectPlus )

public:
	PyLoadedChunks( PyTypePlus * pType = &s_type_ );

	PyObject * pyGetAttribute( const char * attr );

	int has_key( PyObjectPtr pObject );
	PY_AUTO_METHOD_DECLARE( RETDATA, has_key, ARG( PyObjectPtr, END ) )

	PY_INQUIRY_METHOD( pyMap_length )
	PY_BINARY_FUNC_METHOD( pyMap_subscript )
};

static PyMappingMethods PyLoadedChunks_mapfns =
{
	PyLoadedChunks::_pyMap_length,		// mp_length
	PyLoadedChunks::_pyMap_subscript,	// mp_subscript
	0									// mp_ass_subscript
};

PY_TYPEOBJECT_WITH_MAPPING( PyLoadedChunks, &PyLoadedChunks_mapfns )

PY_BEGIN_METHODS( PyLoadedChunks )
	PY_METHOD( has_key )
PY_END_METHODS()

PY_BEGIN_ATTRIBUTES( PyLoadedChunks )
PY_END_ATTRIBUTES()


/**
 *	Constructor
 */
PyLoadedChunks::PyLoadedChunks( PyTypePlus * pType ) :
	PyObjectPlus( pType )
{
}

/**
 *	Python get attribute method
 */
PyObject * PyLoadedChunks::pyGetAttribute( const char * attr )
{
	BW_GUARD;

	PY_GETATTR_STD();
	return this->PyObjectPlus::pyGetAttribute( attr );
}

int PyLoadedChunks::has_key( PyObjectPtr pObject )
{
	BW_GUARD;

	PyObject * pRet = this->pyMap_subscript( &*pObject );
	PyErr_Clear();
	Py_XDECREF( pRet );

	return pRet != NULL ? 1 : 0;
}

int PyLoadedChunks::pyMap_length()
{
	BW_GUARD;

	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (!pSpace) return 0;

	int len = 0;
	for (ChunkMap::iterator i = pSpace->chunks().begin();
		i != pSpace->chunks().end();
		i++)
	{
		//len += i->second.size();
		std::vector<Chunk*> & homonyms = i->second;

		for (std::vector<Chunk*>::iterator j = homonyms.begin();
			j != homonyms.end();
			j++)
		{
			if ((*j)->isBound()) len++;
		}
	}

	return len;
}

PyObject * PyLoadedChunks::pyMap_subscript( PyObject * pKey )
{
	BW_GUARD;

	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (!pSpace)
	{
		PyErr_SetString( PyExc_EnvironmentError, "PyLoadedChunks[] "
			"Camera is not in any chunk space!" );
		return NULL;
	}

	Chunk * pFound = NULL;

	// try it as an index
	int index = -1;
	if (pFound == NULL && Script::setData( pKey, index ) == 0)
	{
		for (ChunkMap::iterator i = pSpace->chunks().begin();
			i != pSpace->chunks().end() && index >= 0;
			i++)
		{
			std::vector<Chunk*> & homonyms = i->second;

			for (std::vector<Chunk*>::iterator j = homonyms.begin();
				j != homonyms.end() && index >= 0;
				j++)
			{
				if ((*j)->isBound())
				{
					if (index-- == 0) pFound = *j;
				}
			}
		}
	}

	// try it as a string
	std::string chunkName;
	if (pFound == NULL && Script::setData( pKey, chunkName ) == 0)
	{
		ChunkMap::iterator i = pSpace->chunks().find( chunkName );
		if (i != pSpace->chunks().end())
		{
			std::vector<Chunk*> & homonyms = i->second;
			for (std::vector<Chunk*>::iterator j = homonyms.begin();
				j != homonyms.end();
				j++)
			{
				if ((*j)->isBound())
				{
					pFound = *j;
					break;
				}
			}
		}
	}

	// if we didn't find it by here then it's not there
	if (pFound != NULL)
	{
		return new ChunkEditor( pFound );
	}
	else
	{
		PyErr_SetString( PyExc_KeyError,
			"PyLoadedChunks: No such loaded chunk" );
		return NULL;
	}
}


/**
 *	This attribute is an object that provides access to all currently
 *	loaded chunks
 */
PY_MODULE_ATTRIBUTE( WorldEditor, chunks, new PyLoadedChunks() )



// -----------------------------------------------------------------------------
// Section: Misc python functions
// -----------------------------------------------------------------------------
float roundFloat( float f )
{
	return Snap::value( f, 0.1f );
}

Vector3 roundVector3( Vector3 v )
{
	Snap::vector3( v, Vector3( 0.1f, 0.1f, 0.1f ) );
	return v;
}

/**
 *	This utility function finds the visual name, datasection
 *	and the visual itself, given a .model file data section.
 *
 *	It returns false if the visual could not be found, and
 *	sets the PyErr string.
 */
static bool visualFromModel(
	const DataSectionPtr& pModelDS,
	std::string& visualName,
	DataSectionPtr& pVisualDS,
	Moo::VisualPtr& pVis)
{
	BW_GUARD;

	// find the visual and its datasection
	std::string visName =
		pModelDS->readString( "nodelessVisual" );
	if (visName.size()<=7) visName =
		pModelDS->readString( "nodefullVisual" );

	if (visName.empty())
	{
		PyErr_SetString( PyExc_ValueError, "WorldEditor.chunkFromModel() "
			"could not find nodeless or nodefull visual in model" );
		return false;
	}

	// load it as a .static.visual
	visualName = visName + ".static.visual";
	pVis = Moo::VisualManager::instance()->get(visualName);

	// try adding a .visual
	if (!pVis)
	{
		visualName = visName + ".visual";
		pVis = Moo::VisualManager::instance()->get( visualName );
	}

	if (!pVis)
	{
		PyErr_SetString( PyExc_ValueError, "WorldEditor.visualFromModel() "
			"could not load nodeless or nodefull visual from resource specified in model" );
		return false;
	}

	// grab the xml file, from the visual that was found.
	pVisualDS = BWResource::openSection( visualName );
	MF_ASSERT( pVisualDS );

	return true;
}

/*~ function WorldEditor.chunkFromModel
 *	@components{ worldeditor }
 *
 *	This function creates a chunk DataSection from a model file. It requires an
 *	already created chunk DataSection which it will copy the relevant chunk data to.
 *
 *	@see WorldEditor.createInsideChunkDataSection
 *
 *	@param chunkDatasection	A pointer to the DataSection to which the chunk
 *							data needs to be stored.
 *	@param modelDataSection	The model's DataSection to create the chunk DataSection from.
 */
static PyObject * chunkFromModel( PyDataSectionPtr pChunkDS,
	PyDataSectionPtr pModelDS )
{
	BW_GUARD;

	// grab the visual	
	std::string visName;
	DataSectionPtr pVisualDS;
	Moo::VisualPtr pVis;

	if ( !visualFromModel( pModelDS->pSection(), visName, pVisualDS, pVis ) )
	{
		//above function sets the PyErr string.
		return NULL;
	}

	// make sure it has portals
	/*if (pVis->nPortals() == 0)
	{
		PyErr_Format( PyExc_ValueError, "WorldEditor.chunkFromModel() "
			"no portals in visual %s", visName.c_str() );
		return NULL;
	}*/

	// grab the destination chunk data section.
	DataSectionPtr pSec = pChunkDS->pSection();

	// and write out the rest of the file.
	pSec->writeMatrix34("transform", Matrix::identity);
	const BoundingBox& bb = pVis->boundingBox();
	pSec->writeVector3( "boundingBox/min", roundVector3(bb.minBounds()) );
	pSec->writeVector3( "boundingBox/max", roundVector3(bb.maxBounds()) );

	Py_Return;
}
PY_AUTO_MODULE_FUNCTION( RETOWN, chunkFromModel,
	NZARG( PyDataSectionPtr, NZARG( PyDataSectionPtr, END ) ), WorldEditor )
