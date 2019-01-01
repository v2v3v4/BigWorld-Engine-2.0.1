/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "server_chunk_tree.hpp"

#include "chunk.hpp"
#include "geometry_mapping.hpp"

#include "physics2/bsp.hpp"
#include "speedtree/speedtree_collision.hpp"

#include <stdexcept>

DECLARE_DEBUG_COMPONENT2( "Chunk", 0 )

// -----------------------------------------------------------------------------
// Section: ServerChunkTree
// -----------------------------------------------------------------------------

int ServerChunkTree_token;

/**
 *	Constructor.
 */
ServerChunkTree::ServerChunkTree() :
	BaseChunkTree()
{}



/**
 *	Destructor.
 */
ServerChunkTree::~ServerChunkTree()
{}


/**
 *	Load method
 */
bool ServerChunkTree::load( DataSectionPtr pSection, Chunk * pChunk )
{
	bool result = false;

	try
	{
		uint seed = pSection->readInt("seed", 1);
		std::string filename = pSection->readString( "spt" );

		BoundingBox bbox  = BoundingBox::s_insideOut_;
		BSPTree * bspTree = speedtree::getBSPTree( 
			filename.c_str(), 
			seed, bbox );

		this->setBSPTree( bspTree );
		this->setBoundingBox( bbox );
		this->setTransform( pSection->readMatrix34( "transform" ) );
		result = true;
	}
	catch (const std::runtime_error &err)
	{
		ERROR_MSG( "Error loading tree: %s\n", err.what() );
	}

	return result;
}


/// Static factory initialiser
#undef IMPLEMENT_CHUNK_ITEM_ARGS
#define IMPLEMENT_CHUNK_ITEM_ARGS ( pSection, pChunk )
IMPLEMENT_CHUNK_ITEM( ServerChunkTree, speedtree, 0 )

// server_chunk_tree.cpp
