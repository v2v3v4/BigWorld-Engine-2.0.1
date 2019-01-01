/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WPENTITY_HPP
#define WPENTITY_HPP

#include "chunk/chunk_item.hpp"
#include "chunk/chunk.hpp"


/**
 *	This class is an Entity for the purposes of waypoint generation
 */
class WPEntity : public ChunkItem, public Aligned
{
	DECLARE_CHUNK_ITEM( WPEntity )

public:
	WPEntity();
	~WPEntity();

	bool load( DataSectionPtr pSection );

	virtual void toss( Chunk * pChunk );
	virtual void draw();

	const Vector3 & position() const	{ return transform_.applyToOrigin(); }

private:
	std::string typeName_;
	DataSectionPtr pProps_;
	void finishLoad();

	WPEntity( const WPEntity& );
	WPEntity& operator=( const WPEntity& );

	Matrix		transform_;
	class SuperModel			* pSuperModel_;
};

typedef SmartPointer<WPEntity> WPEntityPtr;

typedef std::vector<WPEntityPtr> WPEntities;


/**
 *	This class is a cache of all the entities in a given chunk
 */
class WPEntityCache : public ChunkCache
{
public:
	WPEntityCache( Chunk & chunk );
	~WPEntityCache();

	WPEntities::iterator begin()	{ return entities_.begin(); }
	WPEntities::iterator end()		{ return entities_.end(); }

	void add( WPEntityPtr e );
	void del( WPEntityPtr e );

	static Instance<WPEntityCache> instance;

private:
	WPEntities	entities_;
};


#endif // WPENTITY_HPP
