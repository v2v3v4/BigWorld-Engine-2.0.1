/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_HPP
#define EDITOR_CHUNK_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/world/items/editor_chunk_model.hpp"
#include "chunk/chunk.hpp"
#include "chunk/chunk_terrain.hpp"
#include "gizmo/undoredo.hpp"
#include <vector>


/**
 *	This class is, at the moment, a utility class.
 */
class EditorChunk
{
public:
	static ChunkPtr findOutsideChunk( const Vector3 & position,
		bool assertExistence = false );
	static int findOutsideChunks( const BoundingBox & bb,
		ChunkPtrVector & outVector, bool assertExistence = false );

	static bool outsideChunkWriteable( const Vector3 & position, bool mustAlreadyBeLoaded = true );
	static bool outsideChunksWriteable( const BoundingBox & bb, bool mustAlreadyBeLoaded = true );
	static bool outsideChunkWriteable( int16 gridX, int16 gridZ, bool mustAlreadyBeLoaded = true );
	static bool outsideChunksWriteableInSpace( const BoundingBox & bb );
};


/**
 *	This is a chunk cache that caches editor specific chunk information.
 *	It is effectively the editor extensions to the Chunk class.
 *
 *	Note: Things that want to fiddle with datasections in a chunk should
 *	either keep the one they were loaded with (if they're an item) or use
 *	the root datasection stored in this cache, as it may be the only correct
 *	version of it. i.e. you cannot go back and get stuff from the .chunk
 *	file through BWResource as the cache will likely be well stuffed, and
 *	the file may not even be there (if the scene was saved with that chunk
 *	deleted and it's since been undone). i.e. use this datasection! :)
 */
class EditorChunkCache : public ChunkCache
{
public:
	struct UpdateFlagsBase
	{
		uint32 lighting_;
		uint32 thumbnail_;
		uint32 shadow_;
		uint32 terrainLOD_;
	};
	class UpdateFlags : public UpdateFlagsBase
	{
		DataSectionPtr cData_;
	public:
		explicit UpdateFlags( DataSectionPtr cData );
		UpdateFlags( uint32 lighting, uint32 thumbnail, uint32 shadow, uint32 terrainLOD );
		void save( DataSectionPtr cData = NULL );
		void merge( const UpdateFlags& other );
	};

	EditorChunkCache( Chunk & chunk );
	~EditorChunkCache();

	static std::set<Chunk*> chunks_;
	static void lock();
	static void unlock();

	virtual void draw();

	virtual bool load( DataSectionPtr pSec );

	virtual void bind( bool isUnbind );

	void reloadBounds();

	static void touch( Chunk & chunk );

	bool edSave();
	bool edSaveCData();

	bool edTransform( const Matrix & m, bool transient = false );
	bool edTransformClone( const Matrix & m );

	void edArrive( bool fromNowhere = false );
	void edDepart();

	void edEdit( class ChunkEditor & editor );
	bool edReadOnly() const;
	static void forwardReadOnlyMark()	{	++s_readOnlyMark_;	}

	/** 
	 * If the chunk is ok with being deleted
	 */
	bool edCanDelete();

	/**
	 * Notification that we were broght back after a delete
	 */
	void edPostUndelete();

	/**
	 * Notification that we're about to delete the chunk
	 */
	void edPreDelete();

	/**
	 * If the chunk is currently deleted
	 */
	bool edIsDeleted()			{ return deleted_; }

	/**
	 * If the chunk is about to be deleted
	 */
	bool edIsDeleting()			{ return deleting_; }

	/**
	 * Notification that we were just cloned
	 */
	void edPostClone(bool keepLinks = false);

	/**
	 * Invalid the existing static lighting data if there is any
	 */
	void edInvalidateStaticLighting();

	/**
	 * Recalculate the static lighting for this chunk
	 *
	 * Returns false if we aren't able to calculate at this time, eg, if the
	 * chunks which have have lights that influence us aren't loaded at this
	 * time
	 */
	bool edRecalculateLighting( ProgressTask * task = NULL );

	/**
	 * Calculate the thumbnail for this chunk.
	 *
	 * The thumbnail is a snapshot from above that is distributed and used to
	 * display a map of the whole world.
	 */
	bool calculateThumbnail();

	/**
	 * enter/leave thumbnail mode
	 */
	static void chunkThumbnailMode( bool mode );

	/** If the user has this chunk locked */
	bool edIsLocked();

	/** If the user may modify this chunk or it's contents */
	bool edIsWriteable( bool bCheckSurroundings = true );

	/** Inform the terrain cache of the first terrain item in the chunk */
	void fixTerrainBlocks();

	void addInvalidSection( DataSectionPtr section );

	/** Retrieve the chunk data section */
	DataSectionPtr	pChunkSection();

	/** Retrieve and possibly create the cData section for the chunk */
	DataSectionPtr	pCDataSection();

	/** Retrieve the cached thumbnail section */
	DataSectionPtr pThumbSection();

	/** Retrieve the thumbnail as a texture */
	Moo::BaseTexturePtr thumbnail();

	/** Is there a cached thumbnail? */
	bool hasThumbnail() const;

	EditorChunkModelPtr getShellModel() const;
	std::vector<ChunkItemPtr> staticItems() const;
	void allItems( std::vector<ChunkItemPtr> & items ) const;

	static Instance<EditorChunkCache>	instance;

	bool lightingUpdated() const
	{
		if ((chunk_.isOutsideChunk() && chunk_.space()->staticLightingOutside()) ||
			!chunk_.isOutsideChunk())
		{
			return updateFlags_.lighting_ != 0;
		}

		return 1;
	}
	void lightingUpdated( bool lightingUpdated )
	{
		updateFlags_.lighting_ = lightingUpdated ? 1 : 0;

		if (!updateFlags_.lighting_)
		{
			this->edInvalidateStaticLighting();
		}
	}
	bool shadowUpdated() const
	{
		return updateFlags_.shadow_ != 0 || !chunk_.isOutsideChunk();
	}
	void shadowUpdated( bool shadowUpdated )
	{
		updateFlags_.shadow_ = shadowUpdated ? 1 : 0;
	}
	bool thumbnailUpdated() const
	{
		return !chunk_.isOutsideChunk() || updateFlags_.thumbnail_ != 0;
	}
	void thumbnailUpdated( bool thumbnailUpdated )
	{
		updateFlags_.thumbnail_ = thumbnailUpdated ? 1 : 0;
	}
	bool terrainLODUpdated() const
	{
		return updateFlags_.terrainLOD_ != 0 ||
			!ChunkTerrainCache::instance( chunk_ ).pTerrain();
	}
	void terrainLODUpdated( bool terrainLODUpdated )
	{
		updateFlags_.terrainLOD_ = terrainLODUpdated ? 1 : 0;
	}
	bool navmeshDirty() const
	{
		return navmeshDirty_;
	}
	void navmeshDirty( bool dirty )
	{
		navmeshDirty_ = dirty;
	}

private:
	void takeOut();
	void putBack();
	bool doTransform( const Matrix & m, bool transient, bool cleanSnappingHistory );

	Chunk & chunk_;
	std::string chunkResourceID_;
	DataSectionPtr	pChunkSection_;
	DataSectionPtr  pThumbSection_;
	bool			present_;		///< Is the chunk present in a file
	bool			deleted_;		///< Should the chunk have its file
	mutable bool	readOnly_;
	mutable int		readOnlyMark_;
	static int		s_readOnlyMark_;
	// The deleting flag is for chunk items that want to know if they are being
	// deleted from a chunk (shell).
	bool			deleting_;		

	std::vector<DataSectionPtr> invalidSections_;

	void updateDataSectionWithTransform();

	UpdateFlags updateFlags_;

	//  This flag is stored in cdata and used by navgen to know when a chunk
	//  requires it's navigation mesh to be recalculated.
	typedef bool NavMeshDirtyType;
	NavMeshDirtyType navmeshDirty_;
};


class ChunkMatrixOperation : public UndoRedo::Operation, public Aligned
{
public:
	ChunkMatrixOperation( Chunk * pChunk, const Matrix & oldPose );
private:
	virtual void undo();
	virtual bool iseq( const UndoRedo::Operation & oth ) const;

	Chunk *			pChunk_;
	Matrix			oldPose_;
};


class ChunkExistenceOperation : public UndoRedo::Operation
{
public:
	ChunkExistenceOperation( Chunk * pChunk, bool create ) :
		UndoRedo::Operation( 0 ),
		pChunk_( pChunk ),
		create_( create )
	{
		addChunk( pChunk );
	}

private:

	virtual void undo();
	virtual bool iseq( const UndoRedo::Operation & oth ) const;

	Chunk			* pChunk_;
	bool			create_;
};


#endif // EDITOR_CHUNK_HPP
