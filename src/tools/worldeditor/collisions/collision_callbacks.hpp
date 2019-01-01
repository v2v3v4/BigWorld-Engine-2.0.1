/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef COLLISION_CALLBACKS_HPP
#define COLLISION_CALLBACKS_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "chunk/chunk_obstacle.hpp"
#include "physics2/worldtri.hpp"


/**
 *	This class finds the first collision with a forward
 *	facing polygon
 */
class ObstacleLockCollisionCallback : public CollisionCallback
{
public:
	virtual int operator()( const ChunkObstacle & obstacle,
		const WorldTriangle & triangle, float dist );

	virtual void clear();

	float dist_;
	Vector3 normal_;
	ChunkItemPtr pItem_;
	Vector3 triangleNormal_;
	static ObstacleLockCollisionCallback s_default;
};


#endif // COLLISION_CALLBACKS_HPP
