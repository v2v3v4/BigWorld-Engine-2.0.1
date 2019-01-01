/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_TREE_HPP
#define CHUNK_TREE_HPP

#include "base_chunk_tree.hpp"
#include <memory>


/**
 *	This class is a body of water as a chunk item
 */
class ServerChunkTree : public BaseChunkTree
{
	DECLARE_CHUNK_ITEM( ServerChunkTree )

public:
	ServerChunkTree();
	~ServerChunkTree();

	bool load( DataSectionPtr pSection, Chunk * pChunk );

private:
	// Disallow copy
	ServerChunkTree( const ServerChunkTree & );
	const ServerChunkTree & operator = ( const ServerChunkTree & );
};


#endif // CHUNK_TREE_HPP
