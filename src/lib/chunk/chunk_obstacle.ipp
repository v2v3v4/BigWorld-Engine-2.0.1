/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// chunk_obstacle.ipp

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif


/**
 *	This method returns the chunk item that caused the creation of this obstacle
 */
INLINE ChunkItemPtr ChunkObstacle::pItem() const
{
	return pItem_;
}

/**
 *	This method returns the chunk that this obstacle is in.
 */
INLINE Chunk * ChunkObstacle::pChunk() const
{
	return pItem_->chunk();
}


// chunk_obstacle.ipp
