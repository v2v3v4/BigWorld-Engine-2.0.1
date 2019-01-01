/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_BINDING_HPP
#define CHUNK_BINDING_HPP


#include "chunk_item.hpp"
#include "chunk_stationnode.hpp"
#include <list>


class ChunkItemTreeNode;
typedef SmartPointer<ChunkItemTreeNode> ChunkItemTreeNodePtr;
class ChunkItemTreeNodeCache;
class ChunkBinding;
typedef SmartPointer<ChunkBinding> ChunkBindingPtr;
class ChunkBindingCache;



class ChunkBinding : public ChunkItem
{
public:
	virtual bool load( DataSectionPtr pSection );
	virtual bool save( DataSectionPtr pSection );

	UniqueID fromID_;
	UniqueID toID_;

	ChunkItemTreeNodePtr from() const { return from_; }
	ChunkItemTreeNodePtr to() const { return to_; }

private:
	ChunkItemTreeNodePtr from_;
	ChunkItemTreeNodePtr to_;

	static ChunkItemFactory::Result create( Chunk * pChunk, DataSectionPtr pSection );
	static ChunkItemFactory	factory_;
};


class ChunkBindingCache
{
public:
	void add(ChunkBindingPtr binding);

	void addBindingFrom_OnLoad(ChunkBindingPtr binding);
	void addBindingTo_OnLoad(ChunkBindingPtr binding);

	void connect(ChunkItemTreeNodePtr node);

private:
	typedef std::list<ChunkBindingPtr> BindingList;
	BindingList bindings_;

	// for when the bindings are loaded before the necessary chunk items
	BindingList waitingBindingFrom_;
	BindingList waitingBindingTo_;
};


#endif // CHUNK_BINDING_HPP