/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BASE_CHUNK_TREE_HPP
#define BASE_CHUNK_TREE_HPP

#include "cstdmf/aligned.hpp"
#include "math/boundbox.hpp"
#include "math/matrix.hpp"
#include "chunk_item.hpp"
#include <memory>


class BSPTree;

/**
 *	This class is a body of water as a chunk item
 */
class BaseChunkTree : public ChunkItem, public Aligned
{
public:
	BaseChunkTree();
	~BaseChunkTree();

	virtual void toss( Chunk * pChunk );

protected:
	const Matrix & transform() const { return this->transform_; }
	virtual void setTransform( const Matrix & transform );

	const BSPTree * bspTree() const;
	void setBSPTree( const BSPTree * bspTree );
	
	const BoundingBox & boundingBox() const { return this->boundingBox_; }
	void setBoundingBox( const BoundingBox & bbox );

private:
	Matrix transform_;
	const BSPTree * bspTree_;
	BoundingBox     boundingBox_;

private:

	// Disallow copy
	BaseChunkTree( const BaseChunkTree & );
	const BaseChunkTree & operator = ( const BaseChunkTree & );
};


#endif // BASE_CHUNK_TREE_HPP
