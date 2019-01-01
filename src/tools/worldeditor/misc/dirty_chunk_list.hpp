/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DIRTY_CHUNK_LIST__
#define DIRTY_CHUNK_LIST__


#include "resmgr/datasection.hpp"
#include <set>
#include <map>
#include <string>


class Chunk;


class DirtyChunkList
{
	typedef std::map<std::string, Chunk*> ChunkMap;
public:
	typedef ChunkMap::iterator iterator;
	typedef ChunkMap::const_iterator const_iterator;

	DirtyChunkList( const std::string& type );

	bool isDirty( const std::string& chunkName ) const;
	void dirty( Chunk* pChunk );
	void clean( const std::string& chunkName );

	bool any() const;
	int num() const;
	iterator begin();
	iterator end();
	iterator erase( iterator iter );

	const_iterator begin() const;
	const_iterator end() const;

	bool empty() const;
	void clear();

private:
	std::string type_;

	ChunkMap chunks_;
};


class DirtyChunkLists
{
public:
	DirtyChunkList& operator[]( const std::string& type );
	const DirtyChunkList& operator[]( const std::string& type ) const;

	bool empty() const;
	bool any() const;
	bool isDirty( const std::string& chunkName ) const;
	void clean( const std::string& chunkName );
	void clear();

private:
	bool needSync_;
	std::string spaceName_;

	typedef std::map<std::string, DirtyChunkList> DirtyLists;
	DirtyLists dirtyLists_;
};


#endif//DIRTY_CHUNK_LIST__
