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

INLINE void Tool::size( float s )
{
	size_ = s;
}


INLINE float Tool::size() const
{
	return size_;
}


INLINE void Tool::strength( float s )
{
	strength_ = s;
}


INLINE float Tool::strength() const
{
	return strength_;
}


INLINE ChunkPtrVector& Tool::relevantChunks()
{
	return relevantChunks_;
}


INLINE const ChunkPtrVector& Tool::relevantChunks() const
{
	return relevantChunks_;
}


INLINE ChunkPtr& Tool::currentChunk()
{
	return currentChunk_;
}


INLINE const ChunkPtr& Tool::currentChunk() const
{
	return currentChunk_;
}

INLINE bool Tool::applying() const
{
	return functor_ && functor_->applying();
}
