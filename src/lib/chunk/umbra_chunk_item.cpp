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

#include "umbra_config.hpp"

#if UMBRA_ENABLE

#include "umbra_chunk_item.hpp"
#include "moo/render_context.hpp"
#include "chunk_manager.hpp"

#include <umbramodel.hpp>
#include <umbraobject.hpp>


DECLARE_DEBUG_COMPONENT2( "Chunk", 0 );


UmbraChunkItem::UmbraChunkItem()
{
}


UmbraChunkItem::~UmbraChunkItem()
{
}


/**
 *	This method draws the associated chunk item as part of the colour pass.
 *	@param pChunkContext the chunk that is currently set up 
 *	(i.e. the currently set lighting information)
 *	@return the Chunk that is set after the draw, this is in case the current
 *	chunk item sets up a different chunk
 */
/*virtual*/ Chunk* UmbraChunkItem::draw(Chunk* pChunkContext)
{
	BW_GUARD;

	// Only draw if this chunk item has already been drawn
	if (pItem_->drawMark() != Chunk::s_nextMark_ &&
		pItem_->chunk() != NULL)
	{
		// Update the chunk
		pChunkContext = updateChunk( pChunkContext );
		
		pItem_->draw();
		
		// Set the draw mark
		pItem_->drawMark( Chunk::s_nextMark_ );
	}
	return pChunkContext;
}


/**
 *	This method draws the associated chunk item as part of the depth pass.
 *	It assumes that RenderContext::drawDepth has already been set up
 *	@param pChunkContext the chunk that is currently set up 
 *	(i.e. the currently set lighting information)
 *	@return the Chunk that is set after the draw, this is in case the current
 *	chunk item sets up a different chunk
 */
/*virtual*/ Chunk* UmbraChunkItem::drawDepth(Chunk* pChunkContext)
{
	BW_GUARD;

	// Check if the depth mark has been set up, if not draw the
	// depth version of the item
	if (pItem_->depthMark() != Chunk::s_nextMark_ &&
		pItem_->chunk() != NULL)
	{
		// Update the chunk
		pChunkContext = updateChunk( pChunkContext );

		pItem_->draw();

		// Mark the item as being rendered in the depth pass
		pItem_->depthMark( Chunk::s_nextMark_ );

		// If the item does not know how to draw depth, mark it as drawn
		// in the colour pass as well
		if (!( pItem_->typeFlags() & ChunkItemBase::TYPE_DEPTH_ONLY ))
		{
			pItem_->drawMark( Chunk::s_nextMark_ );
		}
	}
	return pChunkContext;
}


/**
 *	This method inits the UmbraChunkItem
 *	It creates an umbra object based on the bounding box passed in
 *	@param pItem the chunk item to use
 *	@param bb the bounding box to use for visibility testing
 *	@param transform the transform of the bounding box
 *	@param pCell the umbra cell to place this item in
 */
void UmbraChunkItem::init( ChunkItem* pItem, const BoundingBox& bb, const Matrix& transform, Umbra::Cell* pCell )
{
	BW_GUARD;

	UmbraModelProxyPtr pModel = UmbraModelProxy::getObbModel( bb.minBounds(), bb.maxBounds() );
	init( pItem, pModel, transform, pCell );
}


/**
 *	This method inits the UmbraChunkItem
 *	It creates an umbra object based on the vertices passed in
 *	@param pItem the chunk item to use
 *	@param pVertices the list of vertices to create the umbra object from
 *	@param nVertices the number of vertices in the list
 *	@param transform the transform of the bounding box
 *	@param pCell the umbra cell to place this item in
 */
void UmbraChunkItem::init( ChunkItem* pItem, const Vector3* pVertices, uint32 nVertices, const Matrix& transform, Umbra::Cell* pCell )
{
	BW_GUARD;

	UmbraModelProxyPtr pModel = UmbraModelProxy::getObbModel( pVertices, nVertices );
	init( pItem, pModel, transform, pCell );
}


/**
 *	This method inits the UmbraChunkItem
 *	It creates an umbra object based on umbra model passed in
 *	@param pItem the chunk item to use
 *	@param pModel the model proxy to use when creating the umbra object
 *	@param transform the transform of the bounding box
 *	@param pCell the umbra cell to place this item in
 */
void UmbraChunkItem::init( ChunkItem* pItem, UmbraModelProxyPtr pModel, const Matrix& transform, Umbra::Cell* pCell )
{
	BW_GUARD;
	
	init( pItem, UmbraObjectProxy::get( pModel ), transform, pCell );
}

/**
 *	This method inits the UmbraChunkItem
 *	It uses the passed in umbra object
 *	@param pItem the chunk item to use
 *	@param pObject the umbra ubject to use
 *	@param transform the transform of the bounding box
 *	@param pCell the umbra cell to place this item in
 */
void UmbraChunkItem::init( ChunkItem* pItem, UmbraObjectProxyPtr pObject, const Matrix& transform, Umbra::Cell* pCell )
{
	pItem_ = pItem;

	pObject_ = pObject;
	pObject_->object()->setCell( pCell );
	pObject_->object()->setObjectToCellMatrix( (const Umbra::Matrix4x4&)transform );
	pObject_->object()->setUserPointer( (void*)this );
}


/**
 *	This method updates the current chunk we are in
 *	@param pChunkContext the chunk we want to move to
 *	@return the chunk that has been inited
 */
Chunk* UmbraChunkItem::updateChunk( Chunk* pChunkContext )
{
	BW_GUARD;
	Chunk* pChunk = pItem_->chunk();
	if (pChunkContext != pChunk)
	{
		MF_ASSERT( pChunk != NULL );
		pChunk->drawCaches();
		Moo::rc().world( pChunk->transform() );
		
		// Update the drawmark for the outside chunks as this may
		// be needed post-traversal. Indoor chunk marks are updated 
		// when drawn through the regular portal culling
		if (pChunk->isOutsideChunk())
		{
			pChunk->drawMark(Chunk::s_nextMark_ );
		}
	}
	return pChunk;
}

#endif // UMBRA_ENABLE
