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

#include "resmgr/bwresource.hpp"

#include "chunk_loader.hpp"
#include "chunk_manager.hpp"
#include "chunk_space.hpp"
#include "chunk.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/diary.hpp"
#include "cstdmf/guard.hpp"


DECLARE_DEBUG_COMPONENT2( "Chunk", 0 );
MEMTRACKER_DECLARE( Chunk, "Chunk", 0);

// -----------------------------------------------------------------------------
// Section: LoadChunkTask
// -----------------------------------------------------------------------------

/**
 *	This class is used to perform the background loading of chunks.
 */
class LoadChunkTask : public BackgroundTask
{
public:
	LoadChunkTask( Chunk * pChunk ) : pChunk_( pChunk ) {}

	/**
	 *	Load the empty chunk pChunk now.
	 *
	 *	This is called from the loading thread
	 */
	virtual void doBackgroundTask( BgTaskManager & mgr )
	{
		LoadChunkTask::load( pChunk_ );
	}

	static void load( Chunk * pChunk )
	{
		BW_GUARD;

		MEMTRACKER_SCOPED( Chunk );

		DiaryEntryPtr de =
			Diary::instance().add( "chunk " + pChunk->identifier() );
		DiaryEntryPtr de2 = Diary::instance().add( "section" );
		DataSectionPtr	pDS = BWResource::openSection(
			pChunk->resourceID() );
		//Keep the cData around in the cache for the duration of chunk->load
		DataSectionPtr	pCData = BWResource::openSection(
			pChunk->binFileName() );
		de2->stop();

		pChunk->load( pDS );
		de->stop();

		// This spam is useful to some people. Disable locally, but do not 
		// remove. 
		TRACE_MSG( "ChunkLoader: Loaded chunk '%s'\n",
			pChunk->resourceID().c_str() );
	}

private:
	Chunk * pChunk_;
};


// -----------------------------------------------------------------------------
// Section: FindSeedTask
// -----------------------------------------------------------------------------

/**
 *	This class is used to find a seed chunk.
 */
class FindSeedTask : public BackgroundTask
{
public:
	/**
	 *	Constructor.
	 *
	 *	@param	pSpace	the space to find seed chunk in, e.g. the camera space
	 *	@param	where	the point used to find seed chunk,
	 *					it could be the center point on near plane
	 *					of the camera frustrum
	 *	@param	rpChunk	Set to the found seed chunk
	 */
	FindSeedTask( ChunkSpace * pSpace, const Vector3 & where,
			Chunk *& rpChunk ) :
		pSpace_( pSpace ),
		where_( where ),
		rpChunk_( rpChunk ),
		pFoundChunk_( NULL )
	{}

	virtual void doBackgroundTask( BgTaskManager & mgr )
	{
		pFoundChunk_ = pSpace_->guessChunk( where_ );
		mgr.addMainThreadTask( this );
	}

	virtual void doMainThreadTask( BgTaskManager & mgr )
	{
		rpChunk_ = pFoundChunk_;
	}

private:
	ChunkSpacePtr	pSpace_;
	Vector3			where_;
	Chunk *&		rpChunk_;
	Chunk *			pFoundChunk_;
};


// -----------------------------------------------------------------------------
// Section: ChunkLoader
// -----------------------------------------------------------------------------

/**
 *	This method loads the empty chunk pChunk (its identifier is set).
 *
 *	This is called from the main thread
 *
 *	@param pChunk	The chunk to load.
 *	@param priority	The priority at which to load. 0 is normal, lower numbers
 *		mean that are loaded earlier.
 */
void ChunkLoader::load( Chunk * pChunk, int priority )
{
	BW_GUARD;
	MF_ASSERT( !pChunk->loading() );
	pChunk->loading( true );

	BgTaskManager::instance().addBackgroundTask(
		new LoadChunkTask( pChunk ),
		priority );
}


/**
 *	This method loads the input chunk immediately in the main thread.
 */
void ChunkLoader::loadNow( Chunk * pChunk )
{
	BW_GUARD;
	MF_ASSERT( !pChunk->loading() );
	pChunk->loading( true );
	LoadChunkTask::load( pChunk );
}


/**
 *	Create a seed chunk for the given space at the given location.
 */
void ChunkLoader::findSeed( ChunkSpace * pSpace, const Vector3 & where,
	Chunk *& rpChunk )
{
	BW_GUARD;
	BgTaskManager::instance().addBackgroundTask(
		new FindSeedTask( pSpace, where, rpChunk ), 15 );
}

// chunk_loader.cpp
