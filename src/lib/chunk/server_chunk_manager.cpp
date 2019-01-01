/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "chunk_manager.hpp"
#include "chunk_space.hpp"

DECLARE_DEBUG_COMPONENT2( "Chunk", 0 )

/**
 *	@file This file contains the server's implementation of the Chunk Manager.
 */

/**
 *	Constructor.
 */
ChunkManager::ChunkManager() :
	cameraTrans_( Matrix::identity ),
	pCameraSpace_( NULL ),
	cameraChunk_( NULL )
{

}

/**
 *	Destructor.
 */
ChunkManager::~ChunkManager()
{
}


/**
 *	This method is called by a chunk space to add itself to our list
 */
void ChunkManager::addSpace( ChunkSpace * pSpace )
{
	MF_ASSERT( spaces_.find( pSpace->id() ) == spaces_.end() );
	spaces_.insert( std::make_pair( pSpace->id(), pSpace ) );
}

/**
 *	This method is called by a chunk space to remove itself from our list
 */
void ChunkManager::delSpace( ChunkSpace * pSpace )
{
	ChunkSpaces::iterator found = spaces_.find( pSpace->id() );
	if (found != spaces_.end())
		spaces_.erase( found );
}


/**
 *	This static method returns the singleton ChunkManager instance.
 */
ChunkManager & ChunkManager::instance()
{
	static	ChunkManager	chunky;
	return chunky;
}


/**
 *	This method returns the space with the input id.
 *
 *	@note	For the server, createIfMissing must be false.
 */
ChunkSpacePtr ChunkManager::space( ChunkSpaceID spaceID, bool createIfMissing )
{
	if (createIfMissing)
	{
		WARNING_MSG( "ChunkManager::space: "
			"createIfMissing must be false for the server\n" );
	}

	ChunkSpaces::iterator iter = spaces_.find( spaceID );
	return (iter != spaces_.end()) ? iter->second : NULL;
}


/**
 *	This method returns the space that the camera is in.
 *
 *	@note	For the server, NULL is always returned.
 */
ChunkSpacePtr ChunkManager::cameraSpace() const
{
	return NULL;
}


/**
 *      This method sets the space that the camera is in.
 *
 *      @note   For the server, do nothing.
 */
void ChunkManager::camera( const Matrix &, ChunkSpacePtr, Chunk*  )
{
}
// server_chunk_manager.cpp
