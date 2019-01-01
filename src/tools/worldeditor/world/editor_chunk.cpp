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
#include "worldeditor/world/editor_chunk.hpp"
#include "worldeditor/world/world_manager.hpp"
#include "worldeditor/world/vlo_manager.hpp"
#include "worldeditor/world/items/editor_chunk_model.hpp"
#include "worldeditor/world/items/editor_chunk_portal.hpp"
#include "worldeditor/world/editor_chunk_overlapper.hpp"
#include "worldeditor/world/static_lighting.hpp"
#include "worldeditor/world/item_info_db.hpp"
#include "worldeditor/editor/autosnap.hpp"
#include "worldeditor/editor/chunk_editor.hpp"
#include "worldeditor/editor/chunk_item_placer.hpp"
#include "worldeditor/editor/snaps.hpp"
#include "worldeditor/misc/cvswrapper.hpp"
#include "worldeditor/project/chunk_photographer.hpp"
#include "worldeditor/project/space_helpers.hpp"
#include "worldeditor/project/world_editord_connection.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_light.hpp"
#include "chunk/chunk_terrain.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/bin_section.hpp"
#include "resmgr/xml_section.hpp"
#include "resmgr/data_section_census.hpp"
#include "resmgr/string_provider.hpp"
#include "appmgr/options.hpp"
#include "gizmo/general_properties.hpp"
#include "romp/geometrics.hpp"
#include "moo/texture_manager.hpp"
#include "cstdmf/debug.hpp"
#include <set>

#include "worldeditor/editor/item_properties.hpp"


DECLARE_DEBUG_COMPONENT2( "Editor", 0 )

int EditorChunkCache::s_readOnlyMark_ = 0;


namespace
{
	/**
	 *	Find the outside chunk that includes the given world position.
	 *
	 *	@param pos		The position to get the chunk at.
	 *	@param mustAlreadyBeLoaded If true then the chunk must be loaded, if false 
	 *					then a dummy chunk is created.
	 *	@returns		The outside chunk at the given location.  If the 
	 *					position is outside of the space or if the chunk is
	 *					not loaded then NULL is returned.
	 */
	ChunkPtr getOutsideChunk(Vector3 const &pos, bool mustAlreadyBeLoaded)
	{
		BW_GUARD;

		GeometryMapping *mapping = WorldManager::instance().geometryMapping();
		std::string chunkName = mapping->outsideChunkIdentifier(pos);
		if (!chunkName.empty())
		{
			return 
				ChunkManager::instance().findChunkByName
				(
					chunkName, 
					mapping, 
					!mustAlreadyBeLoaded 
				);
		}
		return NULL;
	}


	/**
	 *	Find the outside chunk at the given grid coordinates.
	 *
	 *	@param gx		The x grid coordinate.
	 *	@param gz		The z grid coordinate.
	 *	@param mustAlreadyBeLoaded If true then the chunk must be loaded, if false 
	 *					then a dummy chunk is created.
	 *	@returns		The outside chunk at the given location.  If the 
	 *					position is outside of the space or if the chunk is
	 *					not loaded then NULL is returned.
	 */
	ChunkPtr getOutsideChunk(int gx, int gz, bool mustAlreadyBeLoaded)
	{
		BW_GUARD;

		Vector3 pos = 
			Vector3
			( 
				ChunkSpace::gridToPoint( gx ) + GRID_RESOLUTION*0.5f,
				0.0f, 
				ChunkSpace::gridToPoint( gz ) + GRID_RESOLUTION*0.5f 
			);
		return getOutsideChunk(pos, mustAlreadyBeLoaded);
	}
}


/**
 *	This method initialises the update flags with a cdata section
 */
EditorChunkCache::UpdateFlags::UpdateFlags( DataSectionPtr cData )
	: cData_( cData )
{
	BW_GUARD;

	lighting_ = thumbnail_ = shadow_ = terrainLOD_ = 0;

	if (cData_)
	{
		DataSectionPtr flagSec = cData_->openSection( "dirtyFlags" );

		if (flagSec)
		{
			BinaryPtr bp = flagSec->asBinary();

			if (bp->len() == sizeof( UpdateFlagsBase ))
			{
				*(UpdateFlagsBase*)this = *(UpdateFlagsBase*)bp->data();
			}
		}
	}
	else
	{
		ERROR_MSG( "EditorChunkCache::UpdateFlags: NULL cData section passed into the constructor\n" );
	}
}


/**
 *	This method initialises the update flags invividually
 */
EditorChunkCache::UpdateFlags::UpdateFlags( uint32 lighting, uint32 thumbnail, uint32 shadow, uint32 terrainLOD )
{
	BW_GUARD;

	lighting_ = lighting;
	thumbnail_ = thumbnail;
	shadow_ = shadow;
	terrainLOD_ = terrainLOD;
}


/**
 *	This method saves the flags into a cdata section
 */
void EditorChunkCache::UpdateFlags::save( DataSectionPtr cData /*= NULL*/ )
{
	BW_GUARD;

	if (cData)
	{
		cData_ = cData;
	}

	MF_ASSERT( cData_ );

	DataSectionPtr flagSec = cData_->openSection( "dirtyFlags", true );

	flagSec->setBinary( new BinaryBlock(
		(UpdateFlagsBase*)this, sizeof( UpdateFlagsBase ), "BinaryBlock/WorldEditor" ) );
}


/**
 *	This method merges the dirty flags from another UpdateFlags object
 */
void EditorChunkCache::UpdateFlags::merge( const UpdateFlags& other )
{
	BW_GUARD;

	lighting_ = lighting_ && other.lighting_;
	thumbnail_ = thumbnail_ && other.thumbnail_;
	shadow_ = shadow_ && other.shadow_;
	terrainLOD_ = terrainLOD_ && other.terrainLOD_;
}


// -----------------------------------------------------------------------------
// Section: EditorChunk
// -----------------------------------------------------------------------------

/**
 *	This method finds the outside chunk at the given position if it is focussed.
 */
ChunkPtr EditorChunk::findOutsideChunk( const Vector3 & position,
	bool assertExistence )
{
	BW_GUARD;

	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (!pSpace) return NULL;

	ChunkSpace::Column * pColumn = pSpace->column( position, false );

	if (pColumn != NULL && pColumn->pOutsideChunk() != NULL)
		return pColumn->pOutsideChunk();
	else if (assertExistence)
	{
		CRITICAL_MSG( "EditorChunk::findOutsideChunk: "
			"No focussed outside chunk at (%f,%f,%f) when required\n",
			position.x, position.y, position.z );
	}

	return NULL;
}


/**
 *	This method finds all the focussed outside chunks within the given bounding
 *	box, and adds them to the input vector. The vector is cleared first.
 *	@return The count of chunks in the vector.
 */
int EditorChunk::findOutsideChunks(
		const BoundingBox & bb,
		ChunkPtrVector & outVector,
		bool assertExistence )
{
	BW_GUARD;

	outVector.clear();

	ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
	if (!pSpace) return 0;

	// go through all the columns that overlap this bounding box
	for (int x = ChunkSpace::pointToGrid( bb.minBounds().x );
		x <= ChunkSpace::pointToGrid( bb.maxBounds().x );
		x++)
		for (int z = ChunkSpace::pointToGrid( bb.minBounds().z );
			z <= ChunkSpace::pointToGrid( bb.maxBounds().z );
			z++)
		{
		const Vector3 apt( ChunkSpace::gridToPoint( x ) + GRID_RESOLUTION*0.5f,
			0, ChunkSpace::gridToPoint( z ) + GRID_RESOLUTION*0.5f );

		// extract their outside chunk
		ChunkSpace::Column * pColumn = pSpace->column( apt, false );
		if (pColumn != NULL && pColumn->pOutsideChunk() != NULL)
			outVector.push_back( pColumn->pOutsideChunk() );
		else if (assertExistence)
			{
				CRITICAL_MSG( "EditorChunk::findOutsideChunks: "
				"No focussed outside chunk at (%f,%f,%f) when required\n",
				apt.x, apt.y, apt.z );
		}
	}

	return outVector.size();
}


/**
 *	This method determines whether or not the outside chunks at the given
 *	position exists and is writeable.  If mustAlreadyBeLoaded is true
 *	then the chunks must also be loaded too, if false, then the chunk does
 *	not have to be loaded yet.
 */
bool EditorChunk::outsideChunkWriteable( const Vector3 & position, bool mustAlreadyBeLoaded )
{
	BW_GUARD;

	ChunkPtr chunk = getOutsideChunk(position, mustAlreadyBeLoaded);
	if (chunk == NULL)
		return false;
	else
		return EditorChunkCache::instance(*chunk).edIsWriteable();
}

/**
 *	This method determines whether or not all the outside chunks in the given
 *	bounding box exist and are writeable.  If mustAlreadyBeLoaded is true
 *	then the chunks must also be loaded too, if false, then the chunk does
 *	not have to be loaded yet.
 */
bool EditorChunk::outsideChunksWriteable( const BoundingBox & bb, bool mustAlreadyBeLoaded )
{
	BW_GUARD;

	// go through all the columns that overlap this bounding box
	for 
	(
		int x = ChunkSpace::pointToGrid( bb.minBounds().x );
		x <= ChunkSpace::pointToGrid( bb.maxBounds().x );
		++x
	)
	{
		for 
		(
			int z = ChunkSpace::pointToGrid( bb.minBounds().z );
			z <= ChunkSpace::pointToGrid( bb.maxBounds().z );
			++z
		)
		{
			ChunkPtr chunk = getOutsideChunk(x, z, mustAlreadyBeLoaded);
			if (chunk == NULL)
				return false;
			if (!EditorChunkCache::instance(*chunk).edIsWriteable())
				return false;
		}
	}

	return true;
}

/**
 *	This method determines whether or not all the outside chunks in the given
 *	bounding box exist and are writeable and are already loaded into space.  
 */
bool EditorChunk::outsideChunksWriteableInSpace( const BoundingBox & bb )
{
	BW_GUARD;

	// go through all the columns that overlap this bounding box
	for 
	(
		int x = ChunkSpace::pointToGrid( bb.minBounds().x );
		x <= ChunkSpace::pointToGrid( bb.maxBounds().x );
		++x
	)
	{
		for 
		(
			int z = ChunkSpace::pointToGrid( bb.minBounds().z );
			z <= ChunkSpace::pointToGrid( bb.maxBounds().z );
			++z
		)
		{
			ChunkPtr chunk = getOutsideChunk(x, z, true);
			if (chunk == NULL)
				return false;
			if (!EditorChunkCache::instance(*chunk).edIsWriteable())
				return false;
			if (!chunk->isBound())
				return false;
		}
	}

	return true;
}

/**
 *	This method determines whether or not the outside chunks at the given
 *	grid exists and is writeable.  If mustAlreadyBeLoaded is true
 *	then the chunk must also be loaded too, if false, then the chunk does
 *	not have to be loaded yet.
 */
bool EditorChunk::outsideChunkWriteable( int16 gridX, int16 gridZ, bool mustAlreadyBeLoaded )
{
	BW_GUARD;

	ChunkPtr chunk = getOutsideChunk(gridX, gridZ, mustAlreadyBeLoaded);
	if (chunk == NULL)
		return false;
	else
		return EditorChunkCache::instance(*chunk).edIsWriteable();
}


// -----------------------------------------------------------------------------
// Section: ChunkMatrixOperation
// -----------------------------------------------------------------------------

/**
 *	Cosntructor
 */
ChunkMatrixOperation::ChunkMatrixOperation( Chunk * pChunk, const Matrix & oldPose ) :
		UndoRedo::Operation( int(typeid(ChunkMatrixOperation).name()) ),
		pChunk_( pChunk ),
		oldPose_( oldPose )
{
	BW_GUARD;

	addChunk( pChunk_ );
}

void ChunkMatrixOperation::undo()
{
	BW_GUARD;

	// first add the current state of this block to the undo/redo list
	UndoRedo::instance().add( new ChunkMatrixOperation(
		pChunk_, pChunk_->transform() ) );

	// now change the matrix back
	EditorChunkCache::instance( *pChunk_ ).edTransform( oldPose_ );
}

bool ChunkMatrixOperation::iseq( const UndoRedo::Operation & oth ) const
{
	return pChunk_ ==
		static_cast<const ChunkMatrixOperation&>( oth ).pChunk_;
}



// -----------------------------------------------------------------------------
// Section: ChunkMatrix
// -----------------------------------------------------------------------------


/**
 *	This class handles the internals of moving a chunk around
 */
class ChunkMatrix : public MatrixProxy, public Aligned
{
public:
	ChunkMatrix( Chunk * pChunk );
	~ChunkMatrix();

	virtual void EDCALL getMatrix( Matrix & m, bool world );
	virtual void EDCALL getMatrixContext( Matrix & m );
	virtual void EDCALL getMatrixContextInverse( Matrix & m );
	virtual bool EDCALL setMatrix( const Matrix & m );

	virtual void EDCALL recordState();
	virtual bool EDCALL commitState( bool revertToRecord, bool addUndoBarrier );

	virtual bool EDCALL hasChanged();

private:
	Chunk *			pChunk_;
	Matrix			origPose_;
	Matrix			curPose_;
};


/**
 *	Constructor.
 */
ChunkMatrix::ChunkMatrix( Chunk * pChunk ) :
	pChunk_( pChunk ),
	origPose_( Matrix::identity ),
	curPose_( Matrix::identity )
{
}

/**
 *	Destructor
 */
ChunkMatrix::~ChunkMatrix()
{
}


void ChunkMatrix::getMatrix( Matrix & m, bool world )
{
	m = pChunk_->transform();
}

void ChunkMatrix::getMatrixContext( Matrix & m )
{
	m = Matrix::identity;
}

void ChunkMatrix::getMatrixContextInverse( Matrix & m )
{
	m = Matrix::identity;
}

bool ChunkMatrix::setMatrix( const Matrix & m )
{
	BW_GUARD;

	curPose_ = m;
	return EditorChunkCache::instance( *pChunk_ ).edTransform( m, true );
}

void ChunkMatrix::recordState()
{
	BW_GUARD;

	origPose_ = pChunk_->transform();
	curPose_ = pChunk_->transform();
}

bool ChunkMatrix::commitState( bool revertToRecord, bool addUndoBarrier )
{
	BW_GUARD;

	// reset the transient transform first regardless of what happens next
	EditorChunkCache::instance( *pChunk_ ).edTransform( origPose_, true );

	// ok, see if we're going ahead with this
	if (revertToRecord)
		return false;

	// if we're not reverting check a few things
	bool okToCommit = true;
	{
		BoundingBox spaceBB(ChunkManager::instance().cameraSpace()->gridBounds());
		BoundingBox chunkBB(pChunk_->localBB());
		chunkBB.transformBy(curPose_);
		if ( !(spaceBB.intersects( chunkBB.minBounds() ) &&
			spaceBB.intersects( chunkBB.maxBounds() )) )
		{
			okToCommit = false;
		}

		// make sure it's not an immovable outside chunk
		//  (this test probably belongs somewhere higher)
		if (pChunk_->isOutsideChunk())
		{
			okToCommit = false;
		}
	}

	// add the undo operation for it
	UndoRedo::instance().add(
		new ChunkMatrixOperation( pChunk_, origPose_ ) );

	ChunkItemPtr shell = EditorChunkCache::instance(*pChunk_).getShellModel();

	if (shell)
	{
		shell->edPostModify();
	}

	// set the barrier with a meaningful name
	if (addUndoBarrier)
	{
		UndoRedo::instance().barrier(
			LocaliseUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK/MOVE_CHUNK", pChunk_->identifier() ), false );
		// TODO: Don't always say 'Move ' ...
		//  figure it out from change in matrix
	}

	// check here, so push on an undo for multiselect
	if ( !okToCommit )
		return false;

	// and finally set the matrix permanently
	return EditorChunkCache::instance( *pChunk_ ).edTransform( curPose_, false );
}


bool ChunkMatrix::hasChanged()
{
	return !(origPose_ == pChunk_->transform());
}


// -----------------------------------------------------------------------------
// Section: EditorChunkCache
// -----------------------------------------------------------------------------
std::set<Chunk*> EditorChunkCache::chunks_;
static SimpleMutex chunksMutex;
static bool s_watchersInited = false;
static bool s_watchersDrawVlos = false;


void EditorChunkCache::lock()
{
	chunksMutex.grab();
}

void EditorChunkCache::unlock()
{
	chunksMutex.give();
}

/**
 *	Constructor
 */
EditorChunkCache::EditorChunkCache( Chunk & chunk ) :
	chunk_( chunk ),
	present_( true ),
	deleted_( false ),
	deleting_( false ),
	pChunkSection_( NULL ),
	updateFlags_( 1, 1, 1, 1 ),
	navmeshDirty_( true ),
	readOnly_( true ),
	readOnlyMark_( s_readOnlyMark_ - 1 ) 
{
	BW_GUARD;

	SimpleMutexHolder permission( chunksMutex );
	chunks_.insert( &chunk );
	chunkResourceID_ = chunk_.resourceID();
	if ( !s_watchersInited )
	{
		s_watchersInited = true;
		MF_WATCH(
			"Chunks/Very Large Objects/Show VLO References",
			s_watchersDrawVlos, 
			Watcher::WT_READ_WRITE,
			"Highlight chunks with VLO references?" );
	}
}

EditorChunkCache::~EditorChunkCache()
{
	BW_GUARD;

	SimpleMutexHolder permission( chunksMutex );
	chunks_.erase( chunks_.find( &chunk_ ) );
	// Make sure next time the chunk is loaded, it'll be loaded from disk,
	// because the editor changes the chunk's data section in memory while
	// editing.
	BWResource::instance().purge( chunkResourceID_ );
}


void EditorChunkCache::draw()
{
	BW_GUARD;

	if ( WorldManager::instance().drawSelection() )
		return; // don't draw anything if doing frustum drag select.

	// draw watchers
	if ( s_watchersDrawVlos && pChunkSection_ && pChunkSection_->openSection( "vlo" ) )
	{
		// This watcher shows a red bounding box if the chunk has a VLO.
		Moo::rc().push();

		BoundingBox bb;
		if ( chunk_.isOutsideChunk() )
		{
			bb = chunk_.visibilityBox();
			chunk_.nextVisibilityMark();
			Moo::rc().world( Matrix::identity );
			bb.expandSymmetrically( -0.5f, 0.1f, -0.5f );
		}
		else
		{
			bb = chunk_.localBB();
			Moo::rc().world( chunk_.transform() );
		}
		Moo::rc().setVertexShader( NULL );
		Moo::rc().setPixelShader( NULL );
		Moo::rc().setTexture( 0, NULL );
		Moo::rc().setTexture( 1, NULL );

		Moo::rc().setRenderState( D3DRS_ZENABLE, TRUE );
		Moo::rc().setRenderState( D3DRS_LIGHTING, FALSE );
		Moo::rc().setRenderState( D3DRS_ALPHATESTENABLE, FALSE );
		Moo::rc().setRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

		// draw using the colour, offset and tiling values.
		Geometrics::wireBox( bb, 0xFFFF0000 );

		Moo::rc().pop();
	}
}


/**
 *	Load this chunk. We just save the data section pointer
 */
bool EditorChunkCache::load( DataSectionPtr pSec )
{
	BW_GUARD;

	DataSectionPtr cdataSection = pCDataSection();

	updateFlags_.merge( UpdateFlags( cdataSection ) );

	if( DataSectionPtr navmeshSec = cdataSection->openSection( "navmeshDirty" ) )
	{
		BinaryPtr bp = navmeshSec->asBinary();
		if( bp->len() == sizeof( NavMeshDirtyType ) )
			navmeshDirty_ = *(NavMeshDirtyType*)bp->cdata();
	}
	else
	{
		navmeshDirty_ = false;
	}
	pChunkSection_ = pSec;

	// Load the thumbnail and clone it.  We need to create a clone of the 
	// thumbnail otherwise the binary data refers back to it's parent which is 
	// the whole .cdata file and is not used and is rather large.
	pThumbSection_ = cdataSection->openSection("thumbnail.dds");
	if (pThumbSection_ != NULL)
	{
		BinaryPtr oldThumbData = pThumbSection_->asBinary();
		if (oldThumbData)
		{
			BinaryPtr newThumbData = 
				new BinaryBlock(oldThumbData->data(), oldThumbData->len(), "BinaryBlock/EditorChunkCache/ethumbnail");
			pThumbSection_->setBinary(newThumbData);
		}
		else
		{
			// We don't have actual data in this section. There was a bug that
			// produced bad cdata so that the thumbnail was stored at 
			// "thumbnail.dds/thumbnail.dds". This code will fix these legacy 
			// chunks by deleting the section and mark the thumbnail as dirty 
			// so it gets regenerated correctly.
			BWResource::instance().purge( chunk_.binFileName(), true );
			DataSectionPtr tmp_cdatasection = BWResource::openSection( chunk_.binFileName() );
			tmp_cdatasection->deleteSection("thumbnail.dds");
			tmp_cdatasection->save();

			pThumbSection_ = NULL;
			WorldManager::instance().dirtyThumbnail( &chunk_ );
		}
	}

	MF_ASSERT( pSec );

	// Remove the sections marked invalid from the load.
	std::vector<DataSectionPtr>::iterator it = invalidSections_.begin();
	for (;it!=invalidSections_.end();it++)
	{
		pChunkSection_->delChild( (*it) );
	}
	invalidSections_.clear();
	return true;
}

void EditorChunkCache::addInvalidSection( DataSectionPtr section )
{
	BW_GUARD;

	invalidSections_.push_back( section );
}

void EditorChunkCache::bind( bool isUnbind )
{
	BW_GUARD;

	// Mark us as dirty if we weren't brought fully up to date in a previous session
	if( !isUnbind )
		WorldManager::instance().checkUpToDate( &chunk_ );
	else
		WorldManager::instance().onUnloadChunk( &chunk_ );

	MatrixMutexHolder lock( &chunk_ );
	std::vector<ChunkItemPtr>::iterator i = chunk_.selfItems_.begin();
	for (; i != chunk_.selfItems_.end(); ++i)
		(*i)->edChunkBind();
}

/**
 *	Reload the bounds of this chunk.
 */
void EditorChunkCache::reloadBounds()
{
	BW_GUARD;

	Matrix xform = chunk_.transform();

	this->takeOut();

	Chunk* pChunk = &chunk_;

	// Remove the portal items, as they refer to the boundary objects we're
	// about to delete
	{
		MatrixMutexHolder lock( &chunk_ );
		for (int i = chunk_.selfItems_.size()-1; i >= 0; i--)
		{
			DataSectionPtr ds = chunk_.selfItems_[i]->pOwnSect();
			if (ds && ds->sectionName() == "portal")
				chunk_.delStaticItem( chunk_.selfItems_[i] );
		}
	}

	chunk_.bounds_.clear();
	chunk_.joints_.clear();

	{
		MatrixMutexHolder lock( &chunk_ );
		chunk_.selfItems_.front()->edBounds( chunk_.localBB_ );
	}
	chunk_.boundingBox_ = chunk_.localBB_;
	chunk_.boundingBox_.transformBy( chunk_.transform() );
	chunk_.boundingBox_.transformBy( chunk_.pMapping_->mapper() );

	chunk_.formBoundaries( pChunkSection_ );

	chunk_.transform( xform );
	updateDataSectionWithTransform();

	// ensure the focus grid is up to date
	ChunkManager::instance().camera( Moo::rc().invView(),
		ChunkManager::instance().cameraSpace() );

	ChunkPyCache::instance( chunk_ ).createPortalsAgain();

	this->putBack();
}

/**
 *	Touch this chunk. We make sure there's one of us in every chunk.
 */
void EditorChunkCache::touch( Chunk & chunk )
{
	BW_GUARD;

	EditorChunkCache::instance( chunk );
}


/**
 *	Save this chunk and any items in it back to the XML file
 */
bool EditorChunkCache::edSave()
{
	BW_GUARD;

	if (!pChunkSection_)
	{
		WorldManager::instance().addError( &chunk_, NULL,
			LocaliseUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK/SAVE_CHUNK_WITHOUT_DATASECTION", chunk_.identifier() ).c_str() );
		return false;
	}

	if (!edIsLocked())
	{
		WorldManager::instance().addError( &chunk_, NULL,
			LocaliseUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK/SAVE_CHUNK_WITHOUT_LOCK", chunk_.identifier() ).c_str() );
		return false;
	}

	if (edReadOnly())
	{
		WorldManager::instance().addError( &chunk_, NULL,
			LocaliseUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK/SAVE_CHUNK_READONLY", chunk_.identifier() ).c_str() );
		return false;
	}

	// figure out what resource this chunk lives in
	std::string resourceID = chunk_.resourceID();

	// see if we're deleting it
	if (deleted_ && present_)
	{
		// delete the resource
		WorldManager::instance().eraseAndRemoveFile( resourceID );

		// also check for deletion of the corresponding .cdata file
		std::string binResourceID = chunk_.binFileName();
		if (BWResource::fileExists( binResourceID ))
		{
			WorldManager::instance().eraseAndRemoveFile( binResourceID );
		}

		// record that it's not here
		present_ = false;
		return true;
	}
	// see if we deleted it in the same session we created it
	else if (deleted_ && !present_)
	{
		return true;
	}
	// see if we're creating it
	else if (!deleted_ && !present_)
	{
		// it'll get saved to the right spot below

		// the data section cache and census will be well out of whack,
		// but that's OK because everything should be using our own
		// stored datasection variable and bypassing those.

		// record that it's here
		present_ = true;
	}

	pChunkSection_->deleteSections( "processed" );

	// first rewrite the boundary information
	//  (due to portal connection changes, etc)

	// delete all existing sections
	pChunkSection_->deleteSections( "boundary" );

	// give the items a chance to save any changes
	{
		MatrixMutexHolder lock( &chunk_ );
		std::vector<ChunkItemPtr>::iterator i = chunk_.selfItems_.begin();
		for (; i != chunk_.selfItems_.end(); ++i)
			(*i)->edChunkSave();
	}

	// update the bounding box and transform
	updateDataSectionWithTransform();

	// if we don't have a .terrain file, make sure to cvs remove it
	if (!ChunkTerrainCache::instance( chunk_ ).pTerrain() && chunk_.isOutsideChunk())
	{
		// TODO: This code seems to be outdated. Needs reviewing. 
		std::string terrainResource = chunk_.mapping()->path() +
			chunk_.identifier() + ".cdata/terrain";
		Terrain::BaseTerrainBlock::terrainVersion( terrainResource );

		if (BWResource::fileExists( terrainResource ))
			WorldManager::instance().eraseAndRemoveFile( terrainResource );
	}


	// now save out the datasection to the file
	//  (with any changes made by items to themselves)

	bool add = !BWResource::fileExists( resourceID );

	if (add)
		CVSWrapper( WorldManager::instance().getCurrentSpace() ).addFile( chunk_.identifier() + ".chunk", false, false );

	pChunkSection_->save( resourceID );

	if (add)
		CVSWrapper( WorldManager::instance().getCurrentSpace() ).addFile( chunk_.identifier() + ".chunk", false, false );

	// save the binary data
	return edSaveCData();
}


/*
 *	Save the binary data, such as lighting to the .cdata file
 */
bool EditorChunkCache::edSaveCData()
{
	BW_GUARD;

	// retrieve (and possibly create) our .cData file
	DataSectionPtr cData = this->pCDataSection();

	// delete lighting section, if any
	cData->deleteSections( "lighting" );
	cData->deleteSections( "staticLighting" );
	cData->deleteSections( "staticLightingCache" );

	bool savedOK = chunk_.lightValueCache()->save( cData );

	MF_ASSERT(cData);

	{
		MatrixMutexHolder lock( &chunk_ );
		std::vector<ChunkItemPtr>::iterator i = chunk_.selfItems_.begin();
		for (; i != chunk_.selfItems_.end(); ++i)
			(*i)->edChunkSaveCData(cData);
	}

	// save the thumbnail, if it exists
	if (pThumbSection_ != NULL)	
	{
		// If there is a cached thumbnail section then copy its binary data:
		if (DataSectionPtr tSect = cData->openSection("thumbnail.dds", true ))
		{
			BinaryPtr data = pThumbSection_->asBinary();
			tSect->setBinary(data);
		}		
	}

	updateFlags_.save( cData );

	if( DataSectionPtr navmeshSec = cData->openSection( "navmeshDirty", true ) )
	{
		navmeshSec->setBinary
		( 
			new BinaryBlock
			( 
				&navmeshDirty_, 
				sizeof( NavMeshDirtyType ),
				"BinaryBlock/EditorChunk"
			) 
		);
	}

	// check to see if need to save to disk (if already exists or there is data)
	if (cData->bytes() > 0)
	{
		const std::string fileName = chunk_.binFileName();
		bool add = !BWResource::fileExists( fileName );

		if (add)	// just in case its been deleted without cvs knwledge
			CVSWrapper( WorldManager::instance().getCurrentSpace() ).addFile( chunk_.identifier() + ".cdata", true, false );

		// save to disk
		if ( !cData->save(fileName) )
		{
			WorldManager::instance().addError( &chunk_, NULL,
				LocaliseUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK/CANNOT_OPEN_FILE", fileName ).c_str() );
			savedOK = false;
		}

		if (savedOK && add)	// let cvs know about the file
			CVSWrapper( WorldManager::instance().getCurrentSpace() ).addFile( chunk_.identifier() + ".cdata", true, false );
	}

	WorldManager::instance().updateChunk( &chunk_ );

	return savedOK;
}

/**
 *	Change the transform of this chunk, either transiently or permanently,
 *  either clear snapping history or not
 */
bool EditorChunkCache::doTransform( const Matrix & m, bool transient, bool cleanSnappingHistory )
{
	BW_GUARD;

	// For chunk items whose position is absolute, store their position for
	// later use if they belong to the chunk. Useful for VLOs that fit entirely
	// inside a shell.
	Matrix invChunkTransform = chunk_.transform();
	invChunkTransform.invert();
	std::set<ChunkItemPtr> itemsToMoveManually;
	std::map<ChunkItemPtr, Matrix> itemsMatrices;
	for( Chunk::Items::iterator iter = chunk_.selfItems_.begin();
		iter != chunk_.selfItems_.end(); ++iter )
	{
		if( !(*iter)->edIsPositionRelativeToChunk() &&
			 (*iter)->edBelongToChunk() )
		{
			itemsToMoveManually.insert( *iter );
			itemsMatrices[ *iter ] = (*iter)->edTransform();
		}
	}

	// if it's transient that's easy
	if (transient)
	{
		BoundingBox chunkBB = chunk_.localBB();
		chunkBB.transformBy( m );

		BoundingBox spaceBB(ChunkManager::instance().cameraSpace()->gridBounds());

		if ( !(spaceBB.intersects( chunkBB.minBounds() ) &&
			spaceBB.intersects( chunkBB.maxBounds() )) )
			return false;

		chunk_.transformTransiently( m );
		chunk_.syncInit();
		// Move items that need to be moved manually.
		for( std::set<ChunkItemPtr>::iterator iter = itemsToMoveManually.begin();
			iter != itemsToMoveManually.end(); ++iter )
		{
			(*iter)->edTransform( itemsMatrices[ *iter ], true );
		}

		return true;
	}

	if( cleanSnappingHistory )
		clearSnapHistory();

	// check that our source and destination are both loaded and writeable
	// (we are currently limited to movements within the focus grid...)
	if (!EditorChunk::outsideChunksWriteable( chunk_.boundingBox() ))
		return false;


	// ok, let's do the whole deal then

	int oldLeft = ChunkSpace::pointToGrid( chunk_.boundingBox().minBounds().x );
	int oldTop = ChunkSpace::pointToGrid( chunk_.boundingBox().minBounds().z );

	BoundingBox newbb = chunk_.localBB();
	newbb.transformBy( m );
	if (!EditorChunk::outsideChunksWriteableInSpace( newbb ))
		return false;

	int newLeft = ChunkSpace::pointToGrid( newbb.minBounds().x );
	int newTop = ChunkSpace::pointToGrid( newbb.minBounds().z );

	WorldManager::instance().connection().linkPoint( oldLeft, oldTop, newLeft, newTop );

	// make our lights mark the chunks they influence as dirty, provided we're
	// actually connected to something
	if (chunk_.pbegin() != chunk_.pend())
		StaticLighting::StaticChunkLightCache::instance( chunk_ ).markInfluencedChunksDirty();


	// Disable updating references while moving, so items moved manually don't
	// get tossed out when the 
	VLOManager::instance()->enableUpdateChunkReferences( false );

	// take it out of this space
	this->takeOut();

	// move it
	chunk_.transform( m );

	updateDataSectionWithTransform();

	// flag it as dirty
	WorldManager::instance().changedChunk( &chunk_ );
	WorldManager::instance().markTerrainShadowsDirty( chunk_.boundingBox() );

	// put it back in the space
	this->putBack();

	// Move items that need to be moved manually.
	for( std::set<ChunkItemPtr>::iterator iter = itemsToMoveManually.begin();
		iter != itemsToMoveManually.end(); ++iter )
	{
		(*iter)->edTransform( itemsMatrices[ *iter ], false );
	}

	// make our lights mark the chunks they now influence as dirty provided we're
	// actually connected to something
	if (chunk_.pbegin() != chunk_.pend())
		StaticLighting::StaticChunkLightCache::instance( chunk_ ).
			markInfluencedChunksDirty();

	// Update VLO references and turn on reference check on load again.
	VLOManager::instance()->enableUpdateChunkReferences( true );
	VLOManager::instance()->updateChunkReferences( &chunk_ );
	chunk_.syncInit();
	return true;
}

/**
 *	Change the transform of this chunk, either transiently or permanently,
 */
bool EditorChunkCache::edTransform( const Matrix & m, bool transient )
{
	BW_GUARD;

	return doTransform( m, transient, !transient );
}

/**
 *	Change the transform of this chunk, called from snapping functions
 */
bool EditorChunkCache::edTransformClone( const Matrix & m )
{
	BW_GUARD;

	return doTransform( m, false, false );
}

/** Write the current transform out to the datasection */
void EditorChunkCache::updateDataSectionWithTransform()
{
	BW_GUARD;

	pChunkSection_->delChild( "transform" );
	pChunkSection_->delChild( "boundingBox" );
	if( !chunk_.isOutsideChunk() )
	{
		pChunkSection_->writeMatrix34( "transform", chunk_.transform() );
		DataSectionPtr pDS = pChunkSection_->newSection( "boundingBox" );
		pDS->writeVector3( "min", chunk_.boundingBox().minBounds() );
		pDS->writeVector3( "max", chunk_.boundingBox().maxBounds() );
	}
}


/**
 *	This method is called when a chunk arrives on the scene.
 *	It saves out a file for it from its data section (assumed to already
 *	be in pChunkSection_) then binds it into its space.
 */
void EditorChunkCache::edArrive( bool fromNowhere )
{
	BW_GUARD;

	// clear the present flag if this is a brand new chunk
	if (fromNowhere) present_ = false;

	// clear the delete on save flag
	deleted_ = false;
	deleting_ = false;

	// flag the chunk as dirty
	WorldManager::instance().changedChunk( &chunk_ );

	// and add it back in to the space
	this->putBack();

	// We need to do this, as the chunk may be transformed before
	// being added (ie, when creating it), but we can't call edTransform
	// before edArrive, thus we simply save the transform here
	updateDataSectionWithTransform();


	// We also need to put this here for a hack when creating multiple
	// chunks in a single frame (ie, when undoing a delete operation)
	// otherwise the portals won't be connected
	ChunkManager::instance().camera( Moo::rc().invView(), ChunkManager::instance().cameraSpace() );

	// if we have any lights in the chunk (eg, we just got cloned ) then
	// mark us and our surrounds as dirty
	StaticLighting::StaticChunkLightCache::instance( chunk_ ).markInfluencedChunksDirty();
}

/**
 *	This method is called when a chunk departs from the scene.
 */
void EditorChunkCache::edDepart()
{
	BW_GUARD;

	// take it out of the space
	this->takeOut();

	// flag the chunk as dirty
	WorldManager::instance().changedChunk( &chunk_ );

	// set the chunk to delete on save
	deleted_ = true;
	deleting_ = false;
}

/**
 * Check that all our items are cool with being deleted
 */
bool EditorChunkCache::edCanDelete()
{
	BW_GUARD;

	MatrixMutexHolder lock( &chunk_ );
	std::vector<ChunkItemPtr>::iterator i = chunk_.selfItems_.begin();
	for (; i != chunk_.selfItems_.end(); ++i)
		if (!(*i)->edCanDelete())
			return false;

	return true;
}


/**
 * Put items back into the item info DB
 */
void EditorChunkCache::edPostUndelete()
{
	BW_GUARD;

	MatrixMutexHolder lock( &chunk_ );
    std::vector<ChunkItemPtr> &items = chunk_.selfItems_;

	for (std::vector<ChunkItemPtr>::iterator i = items.begin(); 
        i != items.end(); ++i)
    {
		// temporarily take it out, and the put it back in so it adds itself
		// to the item info DB if necessary.
		(*i)->toss( NULL );
		(*i)->toss( &chunk_ );
    }
}


/**
 * Inform our items that they'll be deleted
 */
void EditorChunkCache::edPreDelete()
{
	BW_GUARD;

    // We cannot simply iterate through selfItems_ and call edPreDelete on 
    // each.  This is because some items (such as patrol path nodes) can delete
    // items (such as links) in selfItems_.  Iterating through selfItems_ then
    // becomes invalid.  Instead we create a second copy of selfItems_ and 
    // iterate through it.  For each item we check that the item is still in 
    // selfItems_ before calling edPreDelete.
	MatrixMutexHolder lock( &chunk_ );
    std::vector<ChunkItemPtr> &items = chunk_.selfItems_;
    std::vector<ChunkItemPtr> origItems = items;

	// Set this flag to true so chunk items can know if they are being deleted
	// from a shell or not.
	deleting_ = true;

	for (std::vector<ChunkItemPtr>::iterator i = origItems.begin(); 
        i != origItems.end(); ++i)
    {
        if (std::find(items.begin(), items.end(), *i) != items.end())
		{
            (*i)->edPreDelete();

			// We need linkable objects to be deleted when the shell is deleted
			// since the linker manager relies on their tossRemove method being
			// called.  The undo redo recreates the items if needed.
			if ((*i)->isEditorEntity() || (*i)->isEditorUserDataObject())
			{
				// delete it now
				chunk_.delStaticItem( (*i) );

				// set up an undo which creates it
				UndoRedo::instance().add(
					new LinkerExistenceOperation( (*i), &chunk_ ) );
			}
			else
			{
				// just take out it of the item info db
				ItemInfoDB::instance().toss( (*i).get(), false );
			}
		}
    }
}

void EditorChunkCache::edPostClone(bool keepLinks)
{
	BW_GUARD;

	MatrixMutexHolder lock( &chunk_ );
	if (keepLinks)
	{
		std::vector<ChunkItemPtr>::iterator i = chunk_.selfItems_.begin();
		for (; i != chunk_.selfItems_.end(); ++i)
		{
			if ( ((*i)->edDescription() != "marker") &&
				((*i)->edDescription() != "marker cluster") &&
				((*i)->edDescription() != "patrol node") )
			{
				(*i)->edPostClone( NULL );
			}
		}
	}
	else
	{
		std::vector<ChunkItemPtr>::iterator i = chunk_.selfItems_.begin();
		for (; i != chunk_.selfItems_.end(); ++i)
			(*i)->edPostClone( NULL );
	}
}

/**
 *	This method takes a chunk out of its space
 */
void EditorChunkCache::takeOut()
{
	BW_GUARD;

	// flag all chunks it's connected to as dirty
	for (Chunk::piterator pit = chunk_.pbegin(); pit != chunk_.pend(); pit++)
	{
		if (pit->hasChunk())
		{
			// should not be in bound portals list if it's not bound!
			MF_ASSERT( pit->pChunk->isBound() );

			WorldManager::instance().changedChunk( pit->pChunk );
		}
	}

	// go through all the outside chunks we overlap
	ChunkPtrVector outsideChunks;
	EditorChunk::findOutsideChunks( chunk_.boundingBox(), outsideChunks, true );
	for (uint i = 0; i < outsideChunks.size(); i++)
	{
		// delete the overlapper item pointing to it (if present)
		EditorChunkOverlappers::instance( *outsideChunks[i] ).cut( &chunk_ );
	}

	chunk_.unbind( true );

	// ensure the focus grid is up to date
	ChunkManager::instance().camera( Moo::rc().invView(),
		ChunkManager::instance().cameraSpace() );
}

/**
 *	This method puts a chunk back in its space
 */
void EditorChunkCache::putBack()
{
	BW_GUARD;

	// bind it to its new position (a formative bind)
	chunk_.bind( true );

	// go through all the outside chunks we overlap
	ChunkPtrVector outsideChunks;
	EditorChunk::findOutsideChunks( chunk_.boundingBox(), outsideChunks, true );
	for (uint i = 0; i < outsideChunks.size(); i++)
	{
		// create an overlapper item pointing to it
		EditorChunkOverlappers::instance( *outsideChunks[i] ).form( &chunk_ );
	}

	// flag all the new connections as dirty too
	for (Chunk::piterator pit = chunk_.pbegin(); pit != chunk_.pend(); pit++)
	{
		if (pit->hasChunk())
		{
			// should not be in bound portals list if it's not bound!
			MF_ASSERT( pit->pChunk->isBound() );

			WorldManager::instance().changedChunk( pit->pChunk );
		}
	}

	// ensure the focus grid is up to date
	ChunkManager::instance().camera( Moo::rc().invView(),
		ChunkManager::instance().cameraSpace() );
}


/**
 *	Add the properties of this chunk to the given editor
 */
void EditorChunkCache::edEdit( ChunkEditor & editor )
{
	BW_GUARD;

	if (this->getShellModel() && this->getShellModel()->edFrozen())
		return;

	editor.addProperty( new StaticTextProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK/IDENTIFIER" ),
		new ConstantDataProxy<StringProxy>( chunk_.identifier() ) ) );

	MatrixProxy * pMP = new ChunkMatrix( &chunk_ );
	editor.addProperty( new GenPositionProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK/POSITION" ), pMP ) );
	editor.addProperty( new GenRotationProperty(
		LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK/ROTATION" ), pMP ) );

	EditorChunkModelPtr shell = getShellModel();

	if (shell)
	{
		editor.addProperty( new StaticTextProperty(
			LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK/SHELL_NAME" ),
				new ConstantDataProxy<StringProxy>( shell->getModelName() ) ) );

		editor.addProperty( new GenBoolProperty(

			LocaliseStaticUTF8( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_MODEL/REFLECTION_VISIBLE"),
			new AccessorDataProxy< EditorChunkModel, BoolProxy >(
			shell.get(), "reflectionVisible",
			&EditorChunkModel::getReflectionVis,
			&EditorChunkModel::setReflectionVis ) ) );


		shell->edCommonEdit( editor );
	}
}

/**
 *	Return if one of the chunk files is readOnly
 */
bool EditorChunkCache::edReadOnly() const
{
	BW_GUARD;

	if( readOnlyMark_ != s_readOnlyMark_ )
	{
		readOnlyMark_ = s_readOnlyMark_;
		std::string prefix = BWResource::resolveFilename(
			WorldManager::instance().geometryMapping()->path() + chunk_.identifier() );
		std::wstring wchunk, wcdata;
		bw_utf8tow( prefix + ".chunk", wchunk );
		bw_utf8tow( prefix + ".cdata", wcdata );
		DWORD chunkAttr = GetFileAttributes( wchunk.c_str() );
		DWORD cdataAttr = GetFileAttributes( wcdata.c_str() );
		if( chunkAttr != INVALID_FILE_ATTRIBUTES && ( chunkAttr & FILE_ATTRIBUTE_READONLY ) )
			readOnly_ = true;
		else if( cdataAttr != INVALID_FILE_ATTRIBUTES && ( cdataAttr & FILE_ATTRIBUTE_READONLY ) )
			readOnly_ = true;
		else
			readOnly_ = false;
	}
	return readOnly_;
}

/**
 *	Return the top level data section for this chunk
 */
DataSectionPtr EditorChunkCache::pChunkSection()
{
	return pChunkSection_;
}


/**
 *	Return and possibly create the .cdata section for this chunk.
 *
 *	@return the binary cData section for the chunk
 */
DataSectionPtr	EditorChunkCache::pCDataSection()
{
	BW_GUARD;

	// check to see if file already exists
	const std::string fileName = chunk_.binFileName();
	DataSectionPtr cData = BWResource::openSection( fileName, false );

	if (!cData)
	{
		// create a section
		size_t lastSep = fileName.find_last_of('/');
		std::string parentName = fileName.substr(0, lastSep);
		DataSectionPtr parentSection = BWResource::openSection( parentName );
		MF_ASSERT(parentSection);

		std::string tagName = fileName.substr(lastSep + 1);

		// make it
		cData = new BinSection( tagName, new BinaryBlock( NULL, 0, "BinaryBlock/EditorChunk" ) );
		cData->setParent( parentSection );
		cData = cData->convertToZip();
		cData = DataSectionCensus::add( fileName, cData );
	}
	return cData;
}


/**
 *	This gets the cached thumbnail section.  If no thumbnail section exists
 *	then an empty BinSection with the section name "thumbnail.dds" is created.
 */
DataSectionPtr EditorChunkCache::pThumbSection()
{
	BW_GUARD;

	if (pThumbSection_ == NULL)
		pThumbSection_ = new BinSection("thumbnail.dds", NULL);
	return pThumbSection_;
}


/**
 *	This gets the thumbnail texture if it exists.
 */
Moo::BaseTexturePtr EditorChunkCache::thumbnail()
{
	BW_GUARD;

	if (pThumbSection_ == NULL)
	{
		return NULL;
	}
	else
	{
		// Give the resource id a mangled bit so that it is not confused with
		// a file on disk.
		std::string resourceName = "@@chunk.thumbnail";
		return 
			Moo::TextureManager::instance()->get
			(
				pThumbSection_, 
				resourceName, 
				true,		// must exist
				false,		// don't load if missing
				true		// refresh cache from pThumbSection_
			);
	}
}


/**
 *	This returns whether there is a cached thumbnail.
 */
bool EditorChunkCache::hasThumbnail() const
{
	BW_GUARD;

	if (pThumbSection_ == NULL)
		return false;
	BinaryPtr data = pThumbSection_->asBinary();
	return data != NULL && data->len() > 0;
}


/**
 *	Return the first static item
 *	(for internal chunks, this should be the shell model)
 */
EditorChunkModelPtr EditorChunkCache::getShellModel() const
{
	BW_GUARD;

	MF_ASSERT( !chunk_.isOutsideChunk() );
	MatrixMutexHolder lock( &chunk_ );
	if (chunk_.selfItems_.empty() ||
		!chunk_.selfItems_.front()->isShellModel()) 
		return NULL;

	return (EditorChunkModel*)chunk_.selfItems_.front().get();
}

/**
 * Return all chunk items in the chunk
 */
std::vector<ChunkItemPtr> EditorChunkCache::staticItems() const
{
	BW_GUARD;

	return chunk_.selfItems_;
}

/**
 *	Get all the items in this chunk
 */
void EditorChunkCache::allItems( std::vector<ChunkItemPtr> & items ) const
{
	BW_GUARD;

	items.clear();
	MatrixMutexHolder lock( &chunk_ );
	items = chunk_.selfItems_;
	items.insert( items.end(),
		chunk_.dynoItems_.begin(), chunk_.dynoItems_.end() );
}


/**
 *	Invalid the existing static lighting data if there is any
 */
void EditorChunkCache::edInvalidateStaticLighting()
{
	BW_GUARD;

	chunk_.lightValueCache()->invalidateData();

	MatrixMutexHolder lock( &chunk_ );
	std::vector<ChunkItemPtr>::iterator i;

	for (i = chunk_.selfItems_.begin(); i != chunk_.selfItems_.end(); ++i)
	{
		if ((*i)->pOwnSect() &&
			( (*i)->pOwnSect()->sectionName() == "model" ||
			(*i)->pOwnSect()->sectionName() == "shell" ) )
		{
			static_cast<EditorChunkModel*>( &*(*i) )->edInvalidateStaticLighting();
		}
	}
}


/**
 *	Recalculate the lighting for this chunk
 */
bool EditorChunkCache::edRecalculateLighting( ProgressTask * task /*= NULL*/ )
{
	BW_GUARD;

	chunk_.lightValueCache()->startCalculating();

	MF_ASSERT( chunk_.isBound() );
	MF_ASSERT( pChunkSection() );

	INFO_MSG( "recalculating lighting for chunk %s\n", chunk_.identifier().c_str() );

	DWORD startTick = GetTickCount();
	// #1: Find all the lights influencing this chunk
	StaticLighting::StaticLightContainer lights;
	if (chunk_.space()->staticLightingOutside() && chunk_.canSeeHeaven())
	{
		lights.ambient( chunk_.space()->ambientLight() );
	}
	else
	{
		lights.ambient( ChunkLightCache::instance( chunk_ ).pOwnLights()->ambientColour() );
	}

	if (!StaticLighting::findLightsInfluencing( &chunk_, &chunk_, &lights))
		return false;

	lights.rebuildLightVolumes();

	// #2: Get all the EditorChunkModels to recalculate their lighting
	// Create a copy of the vector first, incase any items are removed while
	// recalculating the lighting
	std::vector<ChunkItemPtr> chunkItems;
	{
		MatrixMutexHolder lock( &chunk_ );
		chunkItems = chunk_.selfItems_;
	}
	std::vector<ChunkItemPtr>::iterator i = chunkItems.begin();
	for (; i != chunkItems.end() && WorldManager::instance().isWorkingChunk( &chunk_ ); ++i)
	{
		WorldManager::instance().escapePressed();
		MF_ASSERT( *i );
		if ((*i)->pOwnSect() &&
			( (*i)->pOwnSect()->sectionName() == "model" ||
			(*i)->pOwnSect()->sectionName() == "shell" ) )
			if( !static_cast<EditorChunkModel*>( &*(*i) )->edRecalculateLighting( lights ) )
				return false;
		if (task)
		{
			DWORD currTick = GetTickCount();
			if ((DWORD)(currTick - startTick) >= MAX_NO_RESPONDING_TIME)
			{
				startTick = currTick;
				task->step( 0 );
				WorldManager::processMessages();
			}
		}
	}

	// #3: Mark ourself as changed
	lightingUpdated( true );
	WorldManager::instance().changedChunk( &chunk_ );

	INFO_MSG( "finished calculating lighting for %s\n", chunk_.identifier().c_str() );

	chunk_.lightValueCache()->endCalculating();

	return true;
}

void EditorChunkCache::chunkThumbnailMode( bool mode )
{
	BW_GUARD;

	static bool hideOutsideFlag = false;
	const char * flagNames[] = {
		"render/gameObjects", 
		"render/lighting",
		"render/environment",
		"render/scenery",
		"render/scenery/particle",
		"render/scenery/drawWater",
		"render/terrain",
		"render/proxys"
	};
	const int defFlags[] = {
		0,	//render/gameObjects
		0,	//render/lighting
		0,	//render/environment
		1,	//render/scenery
		1,	//render/scenery/particle
		1,	//render/scenery/drawWater
		1,	//render/terrain
		0	//render/proxys

	};
	static int flags[] = {
		0,  //render/gameObjects
		0,	//render/lighting
		0,	//render/environment
		0,	//render/scenery
		0,	//render/scenery/particle
		0,	//render/scenery/drawWater
		0,	//render/terrain
		0	//render/proxys

	};

	if (mode)
	{
		hideOutsideFlag = EditorChunkItem::hideAllOutside();
		EditorChunkItem::hideAllOutside( false );
	}
	else
	{
		EditorChunkItem::hideAllOutside( hideOutsideFlag );
	}

	for (size_t i = 0; i < sizeof( flagNames ) / sizeof( char * ); ++i)
	{
		if (mode)
		{
			flags[i] = Options::getOptionInt( flagNames[i], defFlags[i] );
			Options::setOptionInt( flagNames[i], defFlags[i] );
		}
		else
		{
			Options::setOptionInt( flagNames[i], flags[i] );
		}
	}

}

bool EditorChunkCache::calculateThumbnail()
{
	BW_GUARD;

	chunkThumbnailMode( true );
	bool retv = ChunkPhotographer::photograph( chunk_ );
	chunkThumbnailMode( false );

	return retv;
}


namespace
{
	int worldToGridCoord( float w )
	{
		int g = int(w / GRID_RESOLUTION);

		if (w < 0.f)
			g--;

		return g;
	}
}

bool EditorChunkCache::edIsLocked()
{

	BW_GUARD;

	if (!WorldManager::instance().connection().enabled())
	{
		return true;
	}

	// Use bb.centre, as the chunk may not be bound, which means it's own
	// centre won't be valid
	Vector3 centre = chunk_.boundingBox().centre();
	GeometryMapping* dirMap = WorldManager::instance().geometryMapping();
	centre = dirMap->invMapper().applyPoint( chunk_.centre() );
	int gridX = worldToGridCoord( centre.x );
	int gridZ = worldToGridCoord( centre.z );

	return WorldManager::instance().connection().isLockedByMe( gridX, gridZ );
}

bool EditorChunkCache::edIsWriteable( bool bCheckSurroundings /*= true*/ )
{

	BW_GUARD;

	if( edReadOnly() )
		return false;

	BWLock::WorldEditordConnection& conn = WorldManager::instance().connection();

	if (!conn.enabled())
	{
		return true;
	}

	GeometryMapping* dirMap = WorldManager::instance().geometryMapping();
	const BoundingBox& bb = chunk_.boundingBox();
	if (chunk_.isOutsideChunk())
	{
		// Use bb.centre, as the chunk may not be bound, which means it's own
		// centre won't be valid
		Vector3 centre = bb.centre();

		centre = dirMap->invMapper().applyPoint( centre );
		int gridX = worldToGridCoord( centre.x );
		int gridZ = worldToGridCoord( centre.z );

		if ( !conn.isLockedByMe( gridX, gridZ ) )
			return false;

		if (bCheckSurroundings)
		{
			for (int x = -conn.xExtent(); x < conn.xExtent() + 1; ++x)
			{
				for (int y = -conn.zExtent(); y < conn.zExtent() + 1; ++y)
				{
					int curX = gridX + x;
					int curY = gridZ + y;

					if (!conn.isLockedByMe(curX,curY))
						return false;
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < 4; ++i)
		{
			Vector3 corner(
				i / 2 ? bb.minBounds().x : bb.maxBounds().x,
				0.f,
				i % 2 ? bb.minBounds().z : bb.maxBounds().z
				);

			GeometryMapping* dirMap = WorldManager::instance().geometryMapping();
			corner = dirMap->invMapper().applyPoint( corner );

			int gridX = worldToGridCoord( corner.x );
			int gridZ = worldToGridCoord( corner.z );

			if ( !conn.isLockedByMe( gridX, gridZ ) )
				return false;

			if (bCheckSurroundings)
			{
				for (int x = -conn.xExtent(); x < conn.xExtent() + 1; ++x)
				{
					for (int y = -conn.zExtent(); y < conn.zExtent() + 1; ++y)
					{
						int curX = gridX + x;
						int curY = gridZ + y;

						if (!conn.isLockedByMe(curX,curY))
							return false;
					}
				}
			}
		}
	}

	return true;
}


/**
 *	Inform the terrain cache of the first terrain item in the chunk
 *
 *	This is required as the cache only knows of a single item, and
 *	the editor will sometimes allow multiple blocks in the one chunk
 *	(say, when cloning).
 */
void EditorChunkCache::fixTerrainBlocks()
{
	BW_GUARD;

	if (ChunkTerrainCache::instance( chunk_ ).pTerrain())
		return;

	MatrixMutexHolder lock( &chunk_ );
	std::vector<ChunkItemPtr>::iterator i = chunk_.selfItems_.begin();
	for (; i != chunk_.selfItems_.end(); ++i)
	{
		ChunkItemPtr item = *i;
		if (item->edClassName() == std::string("ChunkTerrain"))
		{
			item->toss(item->chunk());
			break;
		}
	}
}





/// Static instance accessor initialiser
ChunkCache::Instance<EditorChunkCache> EditorChunkCache::instance;






// -----------------------------------------------------------------------------
// Section: ChunkExistenceOperation
// -----------------------------------------------------------------------------

void ChunkExistenceOperation::undo()
{
	BW_GUARD;

	// first add the redo operation
	UndoRedo::instance().add(
		new ChunkExistenceOperation( pChunk_, !create_ ) );

	std::vector<ChunkItemPtr> selection = WorldManager::instance().selectedItems();

	// now create or delete it
	if (create_)
	{
		EditorChunkCache::instance( *pChunk_ ).edArrive();
		EditorChunkCache::instance( *pChunk_ ).edPostUndelete();

		selection.push_back( EditorChunkCache::instance( *pChunk_ ).getShellModel() );
	}
	else
	{
		EditorChunkCache::instance( *pChunk_ ).edPreDelete();
		EditorChunkCache::instance( *pChunk_ ).edDepart();
		ChunkItemPtr shellModel = EditorChunkCache::instance( *pChunk_ ).getShellModel();
		std::vector<ChunkItemPtr>::iterator i =
			std::find( selection.begin(), selection.end(), shellModel);

		if (i != selection.end())
			selection.erase( i );
	}

	WorldManager::instance().setSelection( selection, false );
}

bool ChunkExistenceOperation::iseq( const UndoRedo::Operation & oth ) const
{
	// these operations never replace each other
	return false;
}
