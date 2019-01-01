/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHUNK_VLO_OBSTACLE_HPP
#define CHUNK_VLO_OBSTACLE_HPP

#include "chunk_model_obstacle.hpp"

/**
 *	This class is the collection of obstacles formed by chunk VLO models
 *	in a chunk.
 */
class ChunkVLOObstacle : public ChunkModelObstacle
{
public:
	ChunkVLOObstacle( Chunk & chunk );
	~ChunkVLOObstacle();

	static Instance<ChunkVLOObstacle> instance;
protected:
	virtual int addToSpace( ChunkObstacle & cso );
	virtual void delFromSpace( ChunkObstacle & cso );
};

#endif // CHUNK_VLO_OBSTACLE_HPP
