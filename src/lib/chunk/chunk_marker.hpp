/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_MARKER_HPP
#define CHUNK_MARKER_HPP


#include "chunk_item_tree_node.hpp"
#include "chunk_stationnode.hpp"
#include "cstdmf/smartpointer.hpp"
#include <list>

class ChunkMarkerCluster;
typedef SmartPointer<ChunkMarkerCluster> ChunkMarkerClusterPtr;


/**
 *	This class is a chunk item for markers 
 *	(locations where items can be dynamically placed)
 */
class ChunkMarker : public ChunkItemTreeNode
{
public:
	ChunkMarker()
		: ChunkItemTreeNode()
	{
	}

	bool load( DataSectionPtr pSection );


protected:
	Matrix transform_;
	std::string category_;

private:
	static ChunkItemFactory::Result create( Chunk * pChunk, DataSectionPtr pSection );
	static ChunkItemFactory	factory_;
};

typedef SmartPointer<ChunkMarker> ChunkMarkerPtr;


#endif // CHUNK_MARKER_HPP