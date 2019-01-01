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
#include "chunk_marker.hpp"
#include "chunk_marker_cluster.hpp"

#include "chunk.hpp"
#include "resmgr/bwresource.hpp"


// -----------------------------------------------------------------------------
// Section: ChunkMarker
// -----------------------------------------------------------------------------

/**
 *	This static method creates a marker from the input section and adds
 *	it to the given chunk.
 */
ChunkItemFactory::Result ChunkMarker::create( Chunk* pChunk, DataSectionPtr pSection )
{
	ChunkMarker* node = new ChunkMarker();
	if (!node->load( pSection ))
	{
		std::string err = "Failed to load marker";
		if ( node->id() != UniqueID::zero() )
		{
			err += ' ' + node->id().toString();
		}
		delete node;
		return ChunkItemFactory::Result( NULL, err );
	}
	else
	{
		pChunk->addStaticItem( node );
		return ChunkItemFactory::Result( node );
	}
}


/// Static factory initialiser
ChunkItemFactory ChunkMarker::factory_( "marker", 0, ChunkMarker::create );


/**
 *	Load the properties from the data section.
 *	This could be from the default section (in res/objects/misc) or the chunk
 */
bool ChunkMarker::load( DataSectionPtr pSection )
{
	if (!ChunkItemTreeNode::load( pSection))
		return false;

	transform_ = pSection->readMatrix34( "transform", Matrix::identity );
	category_ = pSection->readString( "category", "" );

	return true;
}
