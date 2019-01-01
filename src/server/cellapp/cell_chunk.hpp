/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CELL_CHUNK_HPP
#define CELL_CHUNK_HPP

#include "chunk/chunk.hpp"

class Entity;

/**
 *	This class is used to add extra data to chunks that is used by the CellApp.
 */
class CellChunk: public ChunkCache
{
public:
	CellChunk( Chunk & chunk );
	virtual ~CellChunk();

	virtual void bind( bool isUnbind );

	bool hasEntities() const { return pFirstEntity_ != NULL; }
	void addEntity( Entity* pEntity );
	void removeEntity( Entity* pEntity );

	void propagateNoise(const Entity* who,
						float propagationRange,
						Vector3 position,
						float remainingRange,
						int event,
						int info,
						uint32 mark = 0);


	/**
	 *	Helper iterator class for iterating over all entities
	 */
	class EntityIterator
	{
	public:
		void operator++(int);
		bool  operator==(EntityIterator other ) { return pEntity_ == other.pEntity_; }
		bool  operator!=(EntityIterator other ) { return pEntity_ != other.pEntity_; }
		Entity* & operator*() { return pEntity_; }

	private:
		EntityIterator(Entity* pEntity) : pEntity_(pEntity) {}
		Entity* pEntity_;

		friend class CellChunk;
	};

	EntityIterator begin()	{ return EntityIterator( pFirstEntity_); }
	EntityIterator end()	{ return EntityIterator( NULL); }

	void incNumOverlapped() { ++numOverlapped_; }
	void decNumOverlapped() { --numOverlapped_; }

	// The number of loaded outside chunks that we overlap.
	int numOverlapped() const	{ return numOverlapped_; }

	static Instance<CellChunk> instance;

private:
	void clearAllEntities();

	Chunk & chunk_;
	Entity* pFirstEntity_;
	Entity* pLastEntity_;

	// The number of loaded outside chunks that this overlapps
	int		numOverlapped_;
};

#endif //CELL_CHUNK_HPP
