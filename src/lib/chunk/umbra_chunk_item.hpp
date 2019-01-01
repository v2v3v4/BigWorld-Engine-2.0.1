/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef UMBRA_CHUNK_ITEM_HPP
#define UMBRA_CHUNK_ITEM_HPP

#include "umbra_config.hpp"

#if UMBRA_ENABLE

#include "umbra_draw_item.hpp"
#include "chunk_item.hpp"

/**
 *	This class implements the UmbraDrawItem interface for single chunk item.
 *	It handles the drawing of chunk items.
 */
class UmbraChunkItem : public UmbraDrawItem
{
public:
	UmbraChunkItem();
	~UmbraChunkItem();
	virtual Chunk* draw(Chunk* pChunkContext);
	virtual Chunk* drawDepth(Chunk* pChunkContext);

	void init( ChunkItem* pItem, const BoundingBox& bb, const Matrix& transform, Umbra::Cell* pCell );
	void init( ChunkItem* pItem, const Vector3* pVertices, uint32 nVertices, const Matrix& transform, Umbra::Cell* pCell );
	void init( ChunkItem* pItem, UmbraModelProxyPtr pModel, const Matrix& transform, Umbra::Cell* pCell );
	void init( ChunkItem* pItem, UmbraObjectProxyPtr pObject, const Matrix& transform, Umbra::Cell* pCell );
private:
	Chunk*					updateChunk( Chunk* pChunkContext );

	ChunkItem*				pItem_;
};

#endif // UMBRA_ENABLE

#endif // UMBRA_CHUNK_ITEM_HPP
