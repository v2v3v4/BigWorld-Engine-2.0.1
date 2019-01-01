/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef AUTOSNAP_HPP
#define AUTOSNAP_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "chunk/chunk.hpp"
#include "chunk/chunk_boundary.hpp"
#include <set>


class SnappedChunkSet
{
	std::vector<Chunk*> chunks_;
	std::vector<ChunkBoundary::Portal*> snapPortals_;
	std::vector<Matrix> portalTransforms_;
public:
	explicit SnappedChunkSet( const std::set<Chunk*>& chunks );
	unsigned int chunkSize() const	{	return chunks_.size();	}
	Chunk* chunk( unsigned int index ) const	{	return chunks_.at( index );	}
	unsigned int portalSize() const	{	return snapPortals_.size();	}
	ChunkBoundary::Portal* portal( unsigned int index ) const	{	return snapPortals_.at( index );	}
	const Matrix& transform( unsigned int index ) const	{	return portalTransforms_.at( index );	}
};


class SnappedChunkSetSet
{
	bool isConnected( Chunk* chunk1, Chunk* chunk2 ) const;
	void init( std::set<Chunk*> chunks );
	std::vector<SnappedChunkSet> chunkSets_;
public:
	explicit SnappedChunkSetSet( const std::vector<Chunk*>& chunks );
	explicit SnappedChunkSetSet( const std::set<Chunk*>& chunks );
	unsigned int size() const	{	return chunkSets_.size();	}
	const SnappedChunkSet& item( unsigned int index ) const	{	return chunkSets_.at( index );	}
};


/**
 * Find the minimum translation required to translate snapChunks (as a group)
 * so they will have a connecting portal to snapToChunk
 */
Matrix findAutoSnapTransform( Chunk* snapChunk, Chunk* snapToChunk );
Matrix findAutoSnapTransform( Chunk* snapChunk, std::vector<Chunk*> snapToChunks );
Matrix findRotateSnapTransform( const SnappedChunkSet& snapChunk, bool clockWise, Chunk* referenceChunk );
void clearSnapHistory();

#endif // AUTOSNAP_HPP
