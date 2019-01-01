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

#include "math/vector2.hpp"
#include "math/vector3.hpp"
#include "base_chunk_tree.hpp"
#include "borrowed_light_combiner.hpp"
#include <memory>

// Forward Declarations
namespace speedtree {
class SpeedTreeRenderer;
};

class ChunkBSPObstacle;

/**
 *	This class is a body of tree as a chunk item
 */
class ChunkTree : public BaseChunkTree
{
	DECLARE_CHUNK_ITEM( ChunkTree )

public:
	ChunkTree();
	~ChunkTree();

	bool load( DataSectionPtr pSection, Chunk * pChunk, std::string* errorString = NULL );

	virtual void draw();
	virtual void lend( Chunk * pLender );

	virtual uint32 typeFlags() const;

	virtual void setTransform( const Matrix & tranform );

	uint seed() const;
	bool seed(uint seed);

	const char * filename() const;
	bool filename(const char * filename);

	bool getReflectionVis() const;
	bool setReflectionVis(const bool& reflVis);

	bool loadFailed() const;
	const char * lastError() const;

	virtual bool reflectionVisible() { return reflectionVisible_; }

protected:
	bool loadTree(const char * filename, int seed, Chunk * chunk);

	virtual bool addYBounds( BoundingBox& bb ) const;
	virtual void syncInit();

	std::auto_ptr< speedtree::SpeedTreeRenderer > tree_;

	bool        reflectionVisible_;
	BorrowedLightCombiner borrowedLightCombiner_;

	/**
	 *	this structure describes the error
	 *	it contains the reason (what), the filename and the seed
	 */
	struct ErrorInfo
	{
		std::string what;
		std::string filename;
		int seed;
	};
	std::auto_ptr<ErrorInfo> errorInfo_;

	// Disallow copy
	ChunkTree( const ChunkTree & );
	const ChunkTree & operator = ( const ChunkTree & );
};


#endif // CHUNK_TREE_HPP
