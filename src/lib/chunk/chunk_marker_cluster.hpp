/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_MARKER_CLUSTER_HPP
#define CHUNK_MARKER_CLUSTER_HPP


#include "chunk_item_tree_node.hpp"
#include "chunk_stationnode.hpp"
#include <list>
#include <map>


class ChunkMarker;
class ChunkMarkerClusterCache;

/**
 *	This class is the world editor tool's representation
 *	of a marker cluster.
 */
class ChunkMarkerCluster : public ChunkItemTreeNode
{
public:
	ChunkMarkerCluster()
		: availableMarkers_( 0 )
		, position_( Vector3::zero() )
	{
	}

	bool load( DataSectionPtr pSection, Chunk * pChunk = 0);

protected:
	Vector3 position_;
    int availableMarkers_;

private:
	static ChunkItemFactory::Result create( Chunk * pChunk, DataSectionPtr pSection );
	static ChunkItemFactory	factory_;
};


typedef SmartPointer<ChunkMarkerCluster> ChunkMarkerClusterPtr;



#endif // CHUNK_MARKER_CLUSTER_HPP