/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SERVER_CHUNK_MODEL_HPP
#define SERVER_CHUNK_MODEL_HPP

#include "chunk_item.hpp"

#include "cstdmf/smartpointer.hpp"
#include "cstdmf/aligned.hpp"

class SuperModel;

/**
 *	This class defines your standard stocky solid SceneUnit-like model.
 */
class ServerChunkModel : public ChunkItem, public Aligned
{
	DECLARE_CHUNK_ITEM( ServerChunkModel )
	DECLARE_CHUNK_ITEM_ALIAS( ServerChunkModel, shell )

public:
	ServerChunkModel();
	~ServerChunkModel();
	BoundingBox localBB() const;
protected:
	virtual void toss( Chunk * pChunk );
	bool load( DataSectionPtr pSection );

	virtual const char * label() const;

protected:
	SuperModel *				pSuperModel_;
	std::string					label_;
	Matrix						transform_;

	// virtual void lend( Chunk * pLender );
};

#endif // SERVER_CHUNK_MODEL_HPP
