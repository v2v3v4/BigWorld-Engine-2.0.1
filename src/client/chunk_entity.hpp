/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_ENTITY_HPP
#define CHUNK_ENTITY_HPP

#include "network/basictypes.hpp"
#include "chunk/chunk_item.hpp"
#include "chunk/chunk.hpp"

class Entity;
class EntityType;
class MemoryOStream;

/**
 *	This class is a static or client-side entity in a saved chunk.
 */
class ChunkEntity : public ChunkItem
{
	DECLARE_CHUNK_ITEM( ChunkEntity )

public:
	ChunkEntity();
	~ChunkEntity();

	bool load( DataSectionPtr pSection, Chunk * pChunk );

	virtual void toss( Chunk * pChunk );

	void bind();

private:
	ChunkEntity( const ChunkEntity& );
	ChunkEntity& operator=( const ChunkEntity& );

	void makeEntity();

	Entity *			pEntity_;

	uint32				id_;
	const EntityType *	pType_;
	Position3D			position_;
	float				yaw_;
	float				pitch_;
	float				roll_;
	DataSectionPtr		pPropertiesDS_;
};


/**
 *	This class is a cache of ChunkEntities, so we can create their python
 *	objects when they are bound and not before.
 */
class ChunkEntityCache : public ChunkCache
{
public:
	ChunkEntityCache( Chunk & chunk );
	~ChunkEntityCache();

	virtual void bind( bool isUnbind );

	void add( ChunkEntity * pEntity );
	void del( ChunkEntity * pEntity );

	static Instance<ChunkEntityCache> instance;

private:
	typedef std::vector< ChunkEntity * > ChunkEntities;
	ChunkEntities	entities_;
};


#endif // CHUNK_ENTITY_HPP
