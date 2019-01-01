/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_OVERLAPPER_HPP
#define CHUNK_OVERLAPPER_HPP

#include "chunk_item.hpp"
#include "chunk.hpp"

class ChunkOverlappers;

/**
 *	This class is a record in an outside chunk that another chunk overlaps it.
 */
class ChunkOverlapper : public ChunkItem
{
	DECLARE_CHUNK_ITEM( ChunkOverlapper )
public:
	ChunkOverlapper();
	~ChunkOverlapper();

	bool load( DataSectionPtr pSection, Chunk * pChunk, std::string* errorString = NULL );
	virtual void toss( Chunk * pChunk );
	void bind( bool isUnbind );

	bool bbReady() const			{ return pOverlapper_->boundingBoxReady(); }
	const BoundingBox & bb() const	{ return pOverlapper_->boundingBox(); }

	bool focussed() const			{ return pOverlapper_->focussed(); }
	void findAppointedChunk();

	Chunk * pOverlapper() const		{ return pOverlapper_; }

	void alsoInAdd( ChunkOverlappers * ai );
	void alsoInDel( ChunkOverlappers * ai );

private:
	ChunkOverlapper( const ChunkOverlapper& );
	ChunkOverlapper& operator=( const ChunkOverlapper& );

	std::string		overlapperID_;
	Chunk *			pOverlapper_;
	std::vector<ChunkOverlappers*>	alsoIn_;
};

typedef SmartPointer<ChunkOverlapper> ChunkOverlapperPtr;


/**
 *	This class is a cache of all the overlappers for a given chunk.
 *	It may include overlappers from neighbouring chunks which determine
 *	that they also overlap this chunk.
 */
class ChunkOverlappers : public ChunkCache
{
public:
	ChunkOverlappers( Chunk & chunk );
	virtual ~ChunkOverlappers();

	virtual void bind( bool isUnbind );
	static void touch( Chunk & chunk );

	bool empty() const						{ return overlappers_.empty(); }
	bool complete() const					{ return complete_; }

	void add( ChunkOverlapperPtr pOverlapper, bool foreign = false );
	void del( ChunkOverlapperPtr pOverlapper, bool foreign = false );

	typedef std::vector<ChunkOverlapperPtr> Overlappers;
	const Overlappers & overlappers() const	{ return overlappers_; }
	Overlappers& overlappers() { return overlappers_; }

	void findAppointedChunks();

	static Instance<ChunkOverlappers>	instance;

private:
	void share();
	void copyFrom( ChunkOverlappers & oth );
	void checkIfComplete( bool checkNeighbours );

	Chunk & chunk_;
	Overlappers	overlappers_;
	Overlappers foreign_;
	bool		bound_;
	bool		halfBound_;
	bool		complete_;
	bool		binding_;
};


#endif // CHUNK_OVERLAPPER_HPP
