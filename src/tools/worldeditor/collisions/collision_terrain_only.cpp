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
#include "worldeditor/collisions/collision_terrain_only.hpp"


CollisionTerrainOnly CollisionTerrainOnly::s_default;


/*virtual*/ int CollisionTerrainOnly::operator()(
	const ChunkObstacle& /*obstacle*/, const WorldTriangle& triangle,
	float /*dist*/ )
{
	BW_GUARD;

	if (triangle.flags() & TRIANGLE_TERRAIN)
		return COLLIDE_BEFORE;
	else
		return COLLIDE_ALL;
}   
