/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif

INLINE Ecotype::ID ChunkFlora::ecotypeAt( const Vector2& chunkLocalPosition ) const
{
	if (ecotypeIDs_ != NULL)
	{	
		float xf = floorf( chunkLocalPosition.x / spacing_.x );
		float zf = floorf( chunkLocalPosition.y / spacing_.y );		

		uint32 x = uint32(xf+1) % width_;
		uint32 z = uint32(zf+1) % height_;
		uint32 index = x + ( z * width_ );

		return ecotypeIDs_[index];
	}
	else
	{
		return Ecotype::ID_AUTO;
	}
}


INLINE void ChunkFloraManager::chunkToGrid( Chunk* pChunk, int32& gridX, int32& gridZ )
{
	Vector3 chunkPos = pChunk->transform().applyToOrigin();
	gridX = (int32)floorf(chunkPos.x/(float)GRID_RESOLUTION + 0.5f);
	gridZ = (int32)floorf(chunkPos.z/(float)GRID_RESOLUTION + 0.5f);
}


INLINE
void ChunkFloraManager::chunkLocalPosition( const Vector3& pos, int32 gridX, 
	int32 gridZ, Vector2& ret )
{
	ret.x = pos.x - gridX * GRID_RESOLUTION;
	ret.y = pos.z - gridZ * GRID_RESOLUTION;
}

// chunk_flora.ipp